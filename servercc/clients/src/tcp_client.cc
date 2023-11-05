#include "tcp_client.h"

#include <netdb.h>

#include <vector>

#include "client.h"

using ostp::servercc::Client;
using ostp::servercc::Message;
using ostp::servercc::TcpClient;

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
        return absl::Status(absl::StatusCode::kAlreadyExists, "Socket is already open.");
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
        return absl::Status(absl::StatusCode::kInternal, "Could not resolve server address.");
    }

    // Go through the list of addresses and try to connect to the server.
    for (struct addrinfo *p = serverInfo; p != nullptr; p = p->ai_next) {
        // Create a socket.
        if ((clientFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        // Connect to the server.
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
        return absl::Status(absl::StatusCode::kInternal, "Could not connect to server.");
    }

    // Mark the socket as open, set the client address.
    isSocketOpen = true;
    memcpy(&clientAddr, serverInfo->ai_addr, serverInfo->ai_addrlen);

    // Return.
    return absl::Status(absl::StatusCode::kOk, "Socket opened successfully.");
}

// See tcp_client.h for documentation.
absl::Status TcpClient::closeSocket() {
    // If the socket is already closed, return.
    if (clientFd == -1) {
        return absl::Status(absl::StatusCode::kFailedPrecondition, "Socket is already closed.");
    }

    // Close the socket.
    close(clientFd);
    clientFd = -1;
    isSocketOpen = false;

    // Return.
    return absl::Status(absl::StatusCode::kOk, "Socket closed successfully.");
}

// See tcp_client.h for documentation.
absl::Status TcpClient::sendMessage(std::unique_ptr<Message> message) {
    // If the socket is not open, throw an exception.
    if (clientFd == -1) {
        return absl::Status(absl::StatusCode::kFailedPrecondition, "Socket is not open.");
    }

    // Send the message.
    auto sent = send(clientFd, &message->header, kMessageHeaderLength, 0);
    if (sent < kMessageHeaderLength) {
        perror("send");
        return absl::Status(absl::StatusCode::kInternal, "Failed to send message header.");
    }
    sent = send(clientFd, message->body.data.data(), message->header.length, 0);
    if (sent < message->header.length) {
        perror("send");
        return absl::Status(absl::StatusCode::kInternal, "Failed to send message body.");
    }

    // Return.
    return absl::Status(absl::StatusCode::kOk, "Message sent successfully.");
}

// See tcp_client.h for documentation.
absl::StatusOr<std::unique_ptr<Message>> TcpClient::receiveMessage() {
    // If the socket is not open, return an error.
    if (clientFd == -1) {
        return absl::Status(absl::StatusCode::kFailedPrecondition, "Socket is not open.");
    }

    // Receive message.
    auto message = std::make_unique<Message>();
    auto received = recv(clientFd, &message->header, kMessageHeaderLength, 0);
    if (received < kMessageHeaderLength) {
        perror("recv");
        return absl::Status(absl::StatusCode::kInternal, "Failed to receive message header.");
    }
    message->header.length = ntohl(message->header.length);
    message->header.protocol = ntohl(message->header.protocol);
    message->body.data.resize(message->header.length);
    received = recv(clientFd, message->body.data.data(), message->header.length, 0);
    if (received < message->header.length) {
        perror("recv");
        return absl::Status(absl::StatusCode::kInternal, "Failed to receive message body.");
    }

    // Return.
    return std::move(message);
}
