#ifndef SERVERCC_INTERNAL_CHANNEL_H
#define SERVERCC_INTERNAL_CHANNEL_H

#include <memory>
#include <mutex>

#include "absl/log/log.h"
#include "channel_types.h"
#include "inttypes.h"
#include "message_buffer.h"
#include "types.h"

namespace ostp::servercc {

// Allows internal communication between servercc processes with multiplexing. The protocol is
// used to identify the type of message sent through the channel.
//
// Arguments:
//     WriteProtocol: The protocol when writing to the channel.
//     WriteEndProtocol: The protocol used to end the channel.
template <protocol_t WriteProtocol, protocol_t WriteEndProtocol>
class InternalChannel {
   public:
    // Opens a new channel with the specified ID and write file descriptor.
    //
    // Arguments:
    //     id: The ID of the channel.
    //     writeFd: The write file descriptor of the channel.
    //     writeMutex: The mutex protecting the write operations.
    //     closeCallback: The callback to call when the channel is closed.
    InternalChannel(const channel_id_t id, const int writeFd,
                    const std::shared_ptr<std::mutex> writeMutex,
                    const std::function<void(channel_id_t)> closeCallback)
        : id(id), writeFd(writeFd), writeMutex(writeMutex), closeCallback(closeCallback) {
        LOG(INFO) << "Constructed channel " << id;
    }

    // Destructor for the channel. Closes the channel by calling close().
    ~InternalChannel() {
        close();
        LOG(INFO) << "Destructed channel " << id;
    }

    // Reads a message from the channel. Blocks until a message is available.
    //
    // Returns:
    //     A pair containing the status of the operation and the message.
    std::pair<absl::Status, std::unique_ptr<Message>> read() { return messageBuffer.pop(); }

    // Reads a message from the channel. Blocks until a message is available or the timeout is
    // reached.
    //
    // Arguments:
    //     timeout: The timeout in milliseconds.
    // Returns:
    //     A pair containing the status of the operation and the message.
    std::pair<absl::Status, std::unique_ptr<Message>> read(int timeout) {
        return messageBuffer.pop(timeout);
    }

    // Writes a message to the channel delivering it to the other end.
    //
    // Arguments:
    //     message: The message to write.
    // Returns:
    //     A status indicating whether the operation was successful.
    absl::Status write(std::unique_ptr<Message> message) {
        if (isClosed) {
            return absl::FailedPreconditionError("Channel is closed");
        }
        writeMutex->lock();
        auto status =
            writeMessage(writeFd, wrapMessage<channel_id_t, WriteProtocol>(id, std::move(message)));
        writeMutex->unlock();
        return std::move(status);
    }

    // Pushes a message to the channel's message buffer to be read by this end.
    //
    // Arguments:
    //     message: The message to push.
    // Returns:
    //     A status indicating whether the operation was successful.
    absl::Status push(std::unique_ptr<Message> message) {
        if (isClosed) {
            return absl::FailedPreconditionError("Channel is closed");
        }
        return messageBuffer.push(std::move(message));
    }

    // Closes the channel.
    void close() {
        if (isClosed) {
            return;
        }
        messageBuffer.close();
        LOG(INFO) << "Closed channel " << id;

        // Close the channel by sending a close message.
        auto closeMessage = std::make_unique<Message>();
        closeMessage->header.protocol = WriteEndProtocol;
        closeMessage->header.length = sizeof(channel_id_t);
        closeMessage->body.data.resize(sizeof(channel_id_t));
        memcpy(closeMessage->body.data.data(), &id, sizeof(channel_id_t));

        writeMutex->lock();
        auto status = writeMessage(writeFd, std::move(closeMessage));
        if (!status.ok()) {
            LOG(ERROR) << "Failed to send close message to channel " << id << ": "
                       << status.message();
        }
        writeMutex->unlock();

        // Call the close callback to remove the channel from the channel manager.
        isClosed = true;
        closeCallback(id);
    }

   private:
    // The ID of the channel.
    const channel_id_t id;

    // The write file descriptor of the channel.
    const int writeFd;

    // Mutex protecting the write operations.
    const std::shared_ptr<std::mutex> writeMutex;

    // The callback to call when the channel is closed.
    const std::function<void(channel_id_t)> closeCallback;

    // Whether the channel is closed.
    bool isClosed = false;

    // The message buffer used to read messages to the channel.
    ostp::libcc::data_structures::MessageBuffer<std::unique_ptr<Message>> messageBuffer;
};

}  // namespace ostp::servercc

#endif
