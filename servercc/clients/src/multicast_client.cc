#include "multicast_client.h"

#include <arpa/inet.h>
#include <netdb.h>

#include "client.h"

using ostp::servercc::kMessageHeaderLength;
using ostp::servercc::Message;
using ostp::servercc::MulticastClient;

// See udp_client.h for documentation.
MulticastClient::MulticastClient(const absl::string_view interface,
                                 const absl::string_view multicast_group, const uint16_t port,
                                 const uint8_t ttl)
    : Client(multicast_group, port), interface(interface), ttl(ttl) {}

// See udp_client.h for documentation.
absl::Status MulticastClient::openSocket() {
    // If the socket is already open, return.
    if (isSocketOpen) {
        return absl::Status(absl::StatusCode::kAlreadyExists, "Socket is already open.");
    }

    // Create a socket to multicast.
    int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0) {
        perror("socket");
        return absl::Status(absl::StatusCode::kInternal, "Failed to create socket.");
    }

    // Set the socket options.
    if (setsockopt(socketFd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt");
        return absl::Status(absl::StatusCode::kInternal, "Failed to set socket options.");
    }

    // Set the socket interface.
    if (setsockopt(socketFd, SOL_SOCKET, SO_BINDTODEVICE, interface.data(), interface.length()) <
        0) {
        perror("setsockopt");
        return absl::Status(absl::StatusCode::kInternal, "Failed to set socket interface.");
    }

    // Set the address.
    sockaddr_in *clientAddress = (sockaddr_in *)&clientAddr;
    clientAddress->sin_family = AF_INET;
    clientAddress->sin_addr.s_addr = inet_addr(getAddress().data());
    clientAddress->sin_port = htons(getPort());

    // Set the file descriptor.
    clientFd = socketFd;
    isSocketOpen = true;

    // Return.
    return absl::Status(absl::StatusCode::kOk, "Socket opened successfully.");
}

// See udp_client.h for documentation.
absl::Status MulticastClient::closeSocket() {
    // If the socket is already closed, return.
    if (!isSocketOpen) {
        return absl::Status(absl::StatusCode::kFailedPrecondition, "Socket is already closed.");
    }

    // Close the socket.
    if (close(clientFd) < 0) {
        perror("close");
        return absl::Status(absl::StatusCode::kInternal, "Failed to close socket.");
    }

    // Set the socket file and return.
    clientFd = -1;
    isSocketOpen = false;
    clientAddr = {};

    // Return.
    return absl::Status(absl::StatusCode::kOk, "Socket closed successfully.");
};

// See udp_client.h for documentation.
absl::Status MulticastClient::sendMessage(std::unique_ptr<Message> message) {
    // If the socket is not open, return.
    if (!isSocketOpen) {
        return absl::Status(absl::StatusCode::kFailedPrecondition, "Socket is not open.");
    }

    // Send the message.
    auto sent = sendto(clientFd, &message->header, kMessageHeaderLength, 0, &clientAddr,
                       sizeof(clientAddr));
    if (sent < kMessageHeaderLength) {
        perror("sendto");
        return absl::Status(absl::StatusCode::kInternal, "Failed to send message header.");
    }
    sent = sendto(clientFd, message->body.data.data(), message->header.length, 0,
                             &clientAddr, sizeof(clientAddr));
    if (sent < message->header.length) {
        perror("sendto");
        return absl::Status(absl::StatusCode::kInternal, "Failed to send message body.");
    }
    return absl::Status(absl::StatusCode::kOk, "Message sent successfully.");
}

// See udp_client.h for documentation.
absl::StatusOr<std::unique_ptr<Message>> MulticastClient::receiveMessage() {
    // If the socket is not open, return.
    if (!isSocketOpen) {
        return absl::Status(absl::StatusCode::kFailedPrecondition, "Socket is not open.");
    }

    // Receive the message.
    auto message = std::make_unique<Message>();
    auto received = recvfrom(clientFd, &message->header, kMessageHeaderLength, 0, NULL, NULL);
    if (received < kMessageHeaderLength) {
        perror("recvfrom");
        return absl::Status(absl::StatusCode::kInternal, "Failed to receive message.");
    }
    message->header.length = ntohl(message->header.length);
    message->header.protocol = ntohl(message->header.protocol);
    message->body.data.resize(message->header.length);
    received = recvfrom(clientFd, message->body.data.data(), message->header.length, 0, NULL, NULL);
    if (received < message->header.length) {
        perror("recvfrom");
        return absl::Status(absl::StatusCode::kInternal, "Failed to receive message.");
    }
    return std::move(message);
}
