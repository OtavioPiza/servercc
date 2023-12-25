#include "udp_request.h"

namespace ostp::servercc {

// See tcp_request.h for documentation.
UdpRequest::UdpRequest(const sockaddr& clientAddr, std::unique_ptr<Message> message)
    : clientAddr(clientAddr), protocol(message->header.protocol), message(std::move(message)) {}

// See tcp_request.h for documentation.
UdpRequest::~UdpRequest() {
    terminate();
}

// See tcp_request.h for documentation.
sockaddr UdpRequest::getAddr() { return clientAddr; }

// See tcp_request.h for documentation.
protocol_t UdpRequest::getProtocol() { return protocol; }

// See tcp_request.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> UdpRequest::receiveMessage() {
    if (message) {
        return {absl::OkStatus(), std::move(message)};
    }
    return {absl::NotFoundError("No remaining messages"), nullptr};
}

// See tcp_request.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> UdpRequest::receiveMessage(int timeout) {
    return receiveMessage();
}

// See tcp_request.h for documentation.
absl::Status UdpRequest::sendMessage(std::unique_ptr<Message> message) {
    return absl::UnimplementedError("Cannot write to UDP socket");
}

// See tcp_request.h for documentation.
void UdpRequest::terminate() {}

}  // namespace ostp::servercc