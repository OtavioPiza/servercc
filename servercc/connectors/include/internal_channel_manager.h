#ifndef SERVERCC_INTERNAL_CHANNEL_MANAGER_H
#define SERVERCC_INTERNAL_CHANNEL_MANAGER_H

#include <inttypes.h>

#include <array>
#include <memory>
#include <mutex>
#include <optional>
#include <semaphore>
#include <stack>

#include "absl/status/status.h"
#include "channel_types.h"
#include "internal_channel.h"
#include "types.h"

namespace ostp::servercc {

// Allows internal communication between servercc processes with multiplexing. The protocol is
// used to identify the type of message sent through the channel.
//
// Arguments:
//     RequestProtocol: The protocol used for requests.
//     RequestEndProtocol: The protocol used to end requests.
//     ResponseProtocol: The protocol used for responses.
//     ResponseEndProtocol: The protocol used to end responses.
//     ErrorProtocol: The protocol used for errors.
//     MaxChannels: The maximum number of channels.
template <protocol_t RequestProtocol, protocol_t RequestEndProtocol, protocol_t ResponseProtocol,
          protocol_t ResponseEndProtocol, protocol_t ErrorProtocol, channel_id_t MaxChannels>
class InternalChannelManager {
   public:
    // The type of the request channel used to write requests to another peer.
    typedef InternalChannel<RequestProtocol, RequestEndProtocol> request_channel_t;

    // The type of the response channel used to write responses to another peer.
    typedef InternalChannel<ResponseProtocol, ResponseEndProtocol> response_channel_t;

    // Creates a new InternalChannelManager.
    //
    // Arguments:
    //     writeFd: The write file descriptor of the channel (write-only).
    //     writeMutex: The mutex protecting the write operations.
    InternalChannelManager(const int writeFd, std::shared_ptr<std::mutex> writeMutex)
        : writeFd(writeFd),
          writeMutex(writeMutex),
          freeListMutex(),
          freeListSemaphore(MaxChannels) {
        // Initialize the free list.
        for (channel_id_t i = 0; i < MaxChannels; i++) {
            freeList.push(i);
            responseChannel[i] = nullptr;
            requestChannel[i] = nullptr;
        }
    }

    // Tries to forward a ResponseProtocol message to the appropiate requestChannel channel.
    //
    // Arguments:
    //     message: The message to forward.
    // Returns:
    //    The status of the operation the protocol of the message and the response channel if
    //    one was created.
    std::tuple<absl::Status, protocol_t, std::shared_ptr<response_channel_t>> forwardMessage(
        std::unique_ptr<Message> message) {
        // Unwrap the message.
        auto protocol = message->header.protocol;
        auto [status, channelId, unwrapped] = unwrapMessage<channel_id_t>(std::move(message));
        message = nullptr;
        if (!status.ok()) {
            LOG(ERROR) << "Failed to unwrap message: " << status.message();
            return {status, -1, nullptr};
        }
        auto unwrappedHeaderProtocol = unwrapped->header.protocol;

        // Get the channel ID.
        auto id = *channelId;
        channelId = nullptr;

        // If the message is a response, we need to forward it to the request channel that created
        // the request. Otherwise, if the message is a request, we need to find if there is a
        // response channel that can handle the request and forward it to that channel. Finally, if
        // the message is a request end, we need to close the channel.
        if (protocol == ResponseProtocol) {
            if (requestChannel[id] == nullptr) {
                LOG(ERROR) << "Channel " << channelId << " does not exist.";
                return {absl::NotFoundError("Channel does not exist"), unwrappedHeaderProtocol,
                        nullptr};
            }
            return {requestChannel[id]->push(std::move(message)), unwrappedHeaderProtocol, nullptr};

        } else if (protocol == RequestProtocol) {
            // Check if the channel exists otherwise try to create it.
            absl::Status status;
            if (responseChannel[id] == nullptr && !(status = createResponseChannel(id)).ok()) {
                LOG(ERROR) << "Channel " << channelId
                           << " does not exist and could not be created: " << status.message();
                return {status, unwrappedHeaderProtocol, nullptr};
            }
            return {responseChannel[id]->push(std::move(unwrapped)), unwrappedHeaderProtocol,
                    responseChannel[id]};

        } else if (protocol == RequestEndProtocol) {
            // Check if the channel exists.
            if (responseChannel[id] == nullptr) {
                LOG(ERROR) << "Channel " << channelId << " does not exist.";
                return {absl::NotFoundError("Channel does not exist"), unwrappedHeaderProtocol,
                        nullptr};
            }
            responseChannel[id]->close();
            responseChannel[id] = nullptr;
            return {absl::OkStatus(), unwrappedHeaderProtocol, nullptr};
        }

        else {
            LOG(ERROR) << "Invalid protocol: " << protocol;
            return {absl::Status(absl::StatusCode::kInvalidArgument, "Invalid protocol"),
                    unwrappedHeaderProtocol, nullptr};
        }
    }

    // Tries to create a new requestChannel channel and returns the ID of the channel.
    //
    // Returns:
    //     The status of the operation and a pointer to the channel if successful.
    std::pair<absl::Status, std::shared_ptr<request_channel_t>> createRequestChannel() {
        // Wait on the free list.
        freeListSemaphore.acquire();
        freeListMutex.lock();
        auto id = freeList.top();
        freeList.pop();
        freeListMutex.unlock();

        // Create the channel.
        requestChannel[id] = std::make_shared<InternalChannel<RequestProtocol, RequestEndProtocol>>(
            id, writeFd, writeMutex, [this](channel_id_t id) { removeChannel(id); });

        // Return the channel ID.
        return {absl::OkStatus(), requestChannel[id]};
    }

   private:
    // The free list of channel IDs.
    std::stack<channel_id_t> freeList;

    // The array of request channels used to send requests to another peer.
    std::array<std::shared_ptr<request_channel_t>, MaxChannels> requestChannel;

    // The array of response channels used to send responses to another peer.
    std::array<std::shared_ptr<response_channel_t>, MaxChannels> responseChannel;

    // The array to map channel IDs to the other channel ID.
    std::array<channel_id_t, MaxChannels> externalToInternal;

    // Mutex protecting the free list.
    std::mutex freeListMutex;

    // Semaphore used to block on the free list.
    std::binary_semaphore freeListSemaphore;

    // Mutex protecting the write operations.
    std::shared_ptr<std::mutex> writeMutex;

    // The write file descriptor of the channel (write-only).
    const int writeFd;

    // Tries to create a new responseChannel channel with the specified ID.
    //
    // Arguments:
    //     id: The ID of the channel to create.
    // Returns:
    //     The status of the operation.
    absl::Status createResponseChannel(channel_id_t id) {
        // Check if the channel exists.
        if (responseChannel[id] != nullptr) {
            return absl::Status(absl::StatusCode::kAlreadyExists, "Channel already exists");
        }

        // Create the channel.
        responseChannel[id] =
            std::make_shared<InternalChannel<ResponseProtocol, ResponseEndProtocol>>(
                id, writeFd, writeMutex, [](channel_id_t id) {});
        return absl::OkStatus();
    }

    // Removes the channel with the specified ID from the channel manager and returns the channel
    // to the free list.
    //
    // Arguments:
    //     id: The ID of the channel to remove.
    void removeChannel(channel_id_t id) {
        // Remove the channel from the channel manager.
        requestChannel[id] = nullptr;

        // Add the channel ID to the free list.
        freeListMutex.lock();
        freeList.push(id);
        freeListMutex.unlock();
        freeListSemaphore.release();
    }
};

}  // namespace ostp::servercc

#endif
