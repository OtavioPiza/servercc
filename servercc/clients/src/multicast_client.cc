#include "multicast_client.h"

namespace ostp::servercc {

// See udp_client.h for documentation.
MulticastClient::MulticastClient(const absl::string_view interface,
                                 const absl::string_view multicast_group, const uint16_t port,
                                 const uint8_t ttl)
    : Client(multicast_group, port), interface(interface), ttl(ttl) {}

// See udp_client.h for documentation.
absl::Status MulticastClient::openSocket() {
    // If the socket is already open, return.
    if (isSocketOpen) {
        return absl::OkStatus();
    }

    // Create a socket to multicast.
    int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0) {
        perror("socket");
        return absl::InternalError("Failed to create socket.");
    }

    // Set the socket options.
    if (setsockopt(socketFd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt ttl");
        return absl::InternalError("Failed to set socket TTL option.");
    }

    // Set the socket interface.
    if (setsockopt(socketFd, SOL_SOCKET, SO_BINDTODEVICE, interface.data(), interface.length()) <
        0) {
        perror("setsockopt interface");
        return absl::InternalError("Failed to set socket interface.");
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
    return absl::OkStatus();
}

// See udp_client.h for documentation.
absl::Status MulticastClient::closeSocket() {
    // If the socket is already closed, return.
    if (!isSocketOpen) {
        return absl::OkStatus();
    }

    // Close the socket.
    if (close(clientFd) < 0) {
        perror("close");
        return absl::InternalError("Failed to close socket.");
    }

    // Set the socket file and return.
    clientFd = -1;
    isSocketOpen = false;
    clientAddr = {};

    // Return.
    return absl::OkStatus();
};

// See udp_client.h for documentation.
absl::Status MulticastClient::sendMessage(std::unique_ptr<Message> message) {
    // If the socket is not open, return.
    if (!isSocketOpen) {
        return absl::FailedPreconditionError("Socket is not open.");
    }

    // Send the message.
    auto sent = sendto(clientFd, &message->header, kMessageHeaderLength, 0, &clientAddr,
                       sizeof(clientAddr));
    if (sent < kMessageHeaderLength) {
        perror("sendto header");
        return absl::InternalError("Failed to send message header.");
    }
    sent = sendto(clientFd, message->body.data.data(), message->header.length, 0, &clientAddr,
                  sizeof(clientAddr));
    if (sent < message->header.length) {
        perror("sendto body");
        return absl::InternalError("Failed to send message body.");
    }
    return absl::OkStatus();
}

// See udp_client.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> MulticastClient::receiveMessage() {
    // If the socket is not open, return.
    if (!isSocketOpen) {
        return {absl::FailedPreconditionError("Socket is not open."), nullptr};
    }

    // Receive the message.
    return std::move(readMessage(clientFd));
}

}  // namespace ostp::servercc
