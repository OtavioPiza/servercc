#include "tcp_request.h"

namespace ostp::servercc {

// See tcp_request.h for documentation.
TcpRequest::TcpRequest(const int fd, const sockaddr& clientAddr, std::unique_ptr<Message> message)
    : clientSocketFd(fd),
      clientAddr(clientAddr),
      protocol(message->header.protocol),
      message(std::move(message)) {}

// See tcp_request.h for documentation.
TcpRequest::~TcpRequest() { terminate(); }

// See tcp_request.h for documentation.
sockaddr TcpRequest::getAddr() { return clientAddr; }

// See tcp_request.h for documentation.
protocol_t TcpRequest::getProtocol() { return protocol; }

// See tcp_request.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> TcpRequest::receiveMessage() {
    if (message) {
        auto temp = std::move(message);
        message = nullptr;
        return {absl::OkStatus(), std::move(temp)};
    }
    return readMessage(clientSocketFd);
}

// See tcp_request.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> TcpRequest::receiveMessage(int timeout) {
    if (message) {
        auto temp = std::move(message);
        message = nullptr;
        return {absl::OkStatus(), std::move(temp)};
    }
    return readMessage(clientSocketFd, timeout);
}

// See tcp_request.h for documentation.
absl::Status TcpRequest::sendMessage(std::unique_ptr<Message> message) {
    return writeMessage(clientSocketFd, std::move(message));
}

// See tcp_request.h for documentation.
void TcpRequest::terminate() { close(clientSocketFd); }

}  // namespace ostp::servercc