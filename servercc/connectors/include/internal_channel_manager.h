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
            return {status, -1, nullptr};
        }
        auto unwrappedHeaderProtocol = unwrapped->header.protocol;

        // Get the channel ID.
        auto id = *channelId;
        channelId = nullptr;

        // If the protocol is a response push the message to the requesting channel. Else if the
        // protocol is a response end close the requesting channel. Else if the protocol is a
        // request push the message to the responding channel. Else if the protocol is a request
        // end close the responding channel. Else return an error.
        if (protocol == ResponseProtocol) {
            if (requestChannel[id] == nullptr) {
                return {absl::NotFoundError("Channel does not exist"), unwrappedHeaderProtocol,
                        nullptr};
            }
            return {requestChannel[id]->push(std::move(message)), unwrappedHeaderProtocol, nullptr};

        } else if (protocol == ResponseEndProtocol) {
            if (requestChannel[id] == nullptr) {
                return {absl::NotFoundError("Channel does not exist"), unwrappedHeaderProtocol,
                        nullptr};
            }
            removeRequestChannel(id);
            return {absl::OkStatus(), unwrappedHeaderProtocol, nullptr};

        } else if (protocol == RequestProtocol) {
            // Check if the channel exists otherwise try to create it.
            absl::Status status;
            if (responseChannel[id] == nullptr && !(status = createResponseChannel(id)).ok()) {
                return {status, unwrappedHeaderProtocol, nullptr};
            }
            return {responseChannel[id]->push(std::move(unwrapped)), unwrappedHeaderProtocol,
                    responseChannel[id]};

        } else if (protocol == RequestEndProtocol) {
            // Check if the channel exists.
            if (responseChannel[id] == nullptr) {
                return {absl::NotFoundError("Channel does not exist"), unwrappedHeaderProtocol,
                        nullptr};
            }
            removeResponseChannel(id);
            return {absl::OkStatus(), unwrappedHeaderProtocol, nullptr};
        }

        else {
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
            id, writeFd, writeMutex, [this](channel_id_t id) { this->removeRequestChannel(id); });

        // Return the channel ID.
        return {absl::OkStatus(), requestChannel[id]};
    }

   private:
    // The write file descriptor of the channel (write-only).
    const int writeFd;

    // Mutex protecting the write operations.
    const std::shared_ptr<std::mutex> writeMutex;

    // The free list of channel IDs.
    std::stack<channel_id_t> freeList;

    // Mutex protecting the free list.
    std::mutex freeListMutex;

    // Semaphore used to block on the free list.
    std::binary_semaphore freeListSemaphore;

    // The array of request channels used to send requests to another peer.
    std::array<std::shared_ptr<request_channel_t>, MaxChannels> requestChannel;

    // The array of response channels used to send responses to another peer.
    std::array<std::shared_ptr<response_channel_t>, MaxChannels> responseChannel;

    // Tries to create a new responseChannel channel with the specified ID.
    //
    // Arguments:
    //     id: The ID of the channel to create.
    // Returns:
    //     The status of the operation.
    absl::Status createResponseChannel(channel_id_t id) {
        // Check if the channel exists.
        if (responseChannel[id] != nullptr) {
            return absl::AlreadyExistsError("Channel already exists");
        }
        responseChannel[id] =
            std::make_shared<InternalChannel<ResponseProtocol, ResponseEndProtocol>>(
                id, writeFd, writeMutex,
                [this](channel_id_t id) { this->removeResponseChannel(id); });
        return absl::OkStatus();
    }

    // Removes the response channel with the specified ID from the channel manager.
    //
    // Arguments:
    //     id: The ID of the channel to remove.
    void removeResponseChannel(channel_id_t id) {
        if (responseChannel[id] == nullptr) {
            return;
        }
        responseChannel[id]->close();
        responseChannel[id] = nullptr;
    }

    // Removes the request channel with the specified ID from the channel manager and returns the
    // channel to the free list.
    //
    // Arguments:
    //     id: The ID of the channel to remove.
    void removeRequestChannel(channel_id_t id) {
        if (requestChannel[id] == nullptr) {
            return;
        }
        requestChannel[id]->close();
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