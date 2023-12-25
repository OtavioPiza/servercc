#include "tcp_client.h"

#include "absl/log/log.h"

namespace ostp::servercc {

// See tcp_client.h for documentation.
TcpClient::TcpClient(const absl::string_view serverAddress, const uint16_t port)
    : Client(serverAddress, port){};

// See tcp_client.h for documentation.
TcpClient::TcpClient(const int socket, const absl::string_view serverAddress, const uint16_t port,
                     sockaddr clientAddr)
    : Client(serverAddress, port, clientAddr) {
    clientFd = socket;
    isSocketOpen = true;
};

// See tcp_client.h for documentation.
TcpClient::TcpClient(const int socket, const absl::string_view serverAddress, const uint16_t port)
    : Client(serverAddress, port) {
    clientFd = socket;
    isSocketOpen = true;
}

// See tcp_client.h for documentation.
absl::Status TcpClient::openSocket() {
    // If the socket is already open, return.
    if (isSocketOpen) {
        return absl::OkStatus();
    }

    // Allocate structures for socket address.
    struct addrinfo *serverInfo, *hints = new addrinfo;
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = 0;

    // Try to resolve the server address.
    if (getaddrinfo(getAddress().data(), std::to_string(getPort()).c_str(), hints, &serverInfo) !=
        0) {
        delete hints;
        return absl::InternalError("Could not resolve server address");
    }

    // Go through the list of addresses and try to connect to the server.
    for (struct addrinfo *p = serverInfo; p != nullptr; p = p->ai_next) {
        // Create a socket and try to connect to the server.
        if ((clientFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        if (connect(clientFd, p->ai_addr, p->ai_addrlen) == -1) {
            close(clientFd);
            clientFd = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(serverInfo);
    delete hints;

    // If we could not connect to the server, throw an exception.
    if (clientFd == -1) {
        return absl::Status(absl::StatusCode::kInternal, "Could not connect to server");
    }

    // Mark the socket as open, set the client address.
    isSocketOpen = true;
    memcpy(&clientAddr, serverInfo->ai_addr, serverInfo->ai_addrlen);
    LOG(INFO) << "Opened socket: " << clientFd << " for client: " << getAddress() << ":"
              << getPort();
    return absl::OkStatus();
}

// See tcp_client.h for documentation.
absl::Status TcpClient::closeSocket() {
    if (clientFd == -1) {
        return absl::OkStatus();
    }
    close(clientFd);
    LOG(INFO) << "Closed socket: " << clientFd << " for client: " << getAddress() << ":"
              << getPort();
    clientFd = -1;
    isSocketOpen = false;
    return absl::OkStatus();
}

// See tcp_client.h for documentation.
absl::Status TcpClient::sendMessage(std::unique_ptr<Message> message) {
    // If the socket is not open, throw an exception.
    if (clientFd == -1) {
        return absl::FailedPreconditionError("Socket is not open");
    }
    // Send the message.
    auto sent = send(clientFd, &message->header, kMessageHeaderLength, 0);
    if (sent < kMessageHeaderLength) {
        perror("send header");
        return absl::InternalError("Failed to send message header");
    }
    sent = send(clientFd, message->body.data.data(), message->header.length, 0);
    if (sent < message->header.length) {
        perror("send body");
        return absl::InternalError("Failed to send message body");
    }
    fsync(clientFd);
    return absl::OkStatus();
}

// See tcp_client.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> TcpClient::receiveMessage() {
    if (clientFd == -1) {
        return {absl::FailedPreconditionError("Socket is not open"), nullptr};
    }
    return std::move(readMessage(clientFd));
}

}  // namespace ostp::servercc
