#ifndef SERVERCCC_INTERNAL_CHANNEL_H
#define SERVERCCC_INTERNAL_CHANNEL_H

#include <memory>

#include "inttypes.h"
#include "message_buffer.h"
#include "types.h"

namespace ostp::servercc {

typedef uint32_t channel_id_t;

// Allows internal communication between servercc processes with multiplexing.
template <int Protocol, int MaxSize>
class InternalChannel {
    friend class InternalChannelManager;

   public:
    // Opens a new channel with the specified ID and write file descriptor.
    InternalChannel(channel_id_t id, int writeFd) : channel_id_t(id), writeFd(writeFd) {}

    // Sends a message through closing the channel.
    ~InternalChannel();

    // Reads a message from the channel. Blocks until a message is available.
    std::pair<absl::Status, std::unique_ptr<Message>> read();

    // Reads a message from the channel. Blocks until a message is available or the timeout is
    std::pair<absl::Status, std::unique_ptr<Message>> read(std::chrono::milliseconds timeout);

    // Writes a message to the channel.
    absl::Status write(std::unique_ptr<Message> message);

   private:
    // Sends a message closing the channel.
    void close();

    // The ID of the channel.
    channel_id_t id;

    // The write file descriptor of the channel.
    int writeFd;

    // The message buffer used to read messages to the channel.
    ostp::libcc::data_structures::MessageBuffer<std::unique_ptr<Message>> messageBuffer;
};

}  // namespace ostp::servercc

#endif