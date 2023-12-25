#ifndef SERVERCC_INTERNAL_REQUEST_H
#define SERVERCC_INTERNAL_REQUEST_H

#include <netinet/in.h>

#include <memory>

#include "internal_channel.h"
#include "types.h"

namespace ostp::servercc {

// A request between servercc processes.
template <protocol_t ResponseProtocol, protocol_t ResponseEndProtocol>
class InternalRequest : public virtual Request {
   public:
    // Constructs a new request with the specified channel.
    //
    // Arguments:
    //     protocol: The protocol of the first message.
    //     addr: The address of the client.
    //     channel: The internal channel that the request is using to communicate.
    InternalRequest(const protocol_t protocol, const sockaddr& addr,
                    std::shared_ptr<InternalChannel<ResponseProtocol, ResponseEndProtocol>> channel)
        : protocol(protocol), addr(addr), channel(channel) {}

    // Constructs a new request.
    //
    // Arguments:
    //     channel: The internal channel that the request is using to communicate.
    InternalRequest(std::shared_ptr<InternalChannel<ResponseProtocol, ResponseEndProtocol>> channel)
        : protocol(0), addr({}), channel(channel) {}

    // See request.h for documentation.
    ~InternalRequest() final { terminate(); }

    // See request.h for documentation.
    sockaddr getAddr() final { return addr; }

    // See request.h for documentation.
    protocol_t getProtocol() final { return protocol; }

    // See request.h for documentation.
    std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage() final {
        return channel->read();
    }

    // See request.h for documentation.
    std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage(int timeout) final {
        return channel->read(timeout);
    }

    // See request.h for documentation.
    absl::Status sendMessage(std::unique_ptr<Message> message) final {
        return channel->write(std::move(message));
    }

    // See request.h for documentation.
    void terminate() final { channel->close(); }

   private:
    // The protocol of the first message.
    const protocol_t protocol;

    // The address of the client.
    const sockaddr addr;

    // The channel to read and write messages.
    const std::shared_ptr<InternalChannel<ResponseProtocol, ResponseEndProtocol>> channel;
};

// The type of a protocol handler for internal requests.
template <protocol_t ResponseProtocol, protocol_t ResponseEndProtocol>
using internal_handler_t = std::function<void(
    std::unique_ptr<ostp::servercc::InternalRequest<ResponseProtocol, ResponseEndProtocol>>)>;

}  // namespace ostp::servercc

#endif
