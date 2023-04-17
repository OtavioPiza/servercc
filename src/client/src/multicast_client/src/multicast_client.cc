#include "multicast_client.h"

#include <arpa/inet.h>
#include <netdb.h>

using ostp::libcc::utils::Status;
using ostp::libcc::utils::StatusOr;
using ostp::servercc::client::MulticastClient;

/// See udp_client.h for documentation.
MulticastClient::MulticastClient(const std::string interface, const std::string multicast_group,
                                 const uint16_t port)
    : Client(multicast_group, port), interface(interface), ttl(1) {}

/// See udp_client.h for documentation.
MulticastClient::MulticastClient(const std::string interface, const std::string multicast_group,
                                 const uint16_t port, const uint8_t ttl)
    : Client(multicast_group, port), interface(interface), ttl(ttl) {}

/// See udp_client.h for documentation.
StatusOr<bool> MulticastClient::open_socket() {
    // If the socket is already open, return.
    if (is_socket_open) {
        return StatusOr<bool>(Status::OK, "Socket is already open.", true);
    }

    // Create a socket to multicast.
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return StatusOr<bool>(Status::ERROR, "Failed to create socket.", 0);
    }

    // Set the socket options.
    if (setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt");
        return StatusOr<bool>(Status::ERROR, "Failed to set socket options.", 0);
    }

    // Set the socket interface.
    if (setsockopt(socket_fd, SOL_SOCKET, SO_BINDTODEVICE, interface.c_str(), interface.size()) <
        0) {
        perror("setsockopt");
        return StatusOr<bool>(Status::ERROR, "Failed to set socket interface.", 0);
    }

    // Set the address.
    sockaddr_in *client_address = (sockaddr_in *)&client_addr;
    client_address->sin_family = AF_INET;
    client_address->sin_addr.s_addr = inet_addr(get_address().c_str());
    client_address->sin_port = htons(get_port());

    // Set the file descriptor.
    client_fd = socket_fd;
    is_socket_open = true;

    // Return.
    return StatusOr<bool>(Status::OK, "Socket opened successfully.", true);
}

/// See udp_client.h for documentation.
StatusOr<bool> MulticastClient::close_socket() {
    // If the socket is already closed, return.
    if (!is_socket_open) {
        return StatusOr<bool>(Status::OK, "Socket is already closed.", true);
    }

    // Close the socket.
    if (close(client_fd) < 0) {
        perror("close");
        return StatusOr<bool>(Status::ERROR, "Failed to close socket.", 0);
    }

    // Set the socket file and return.
    client_fd = -1;
    is_socket_open = false;
    // client_address = {};

    // Return.
    return StatusOr<bool>(Status::OK, "Socket closed successfully.", true);
};

/// See udp_client.h for documentation.
StatusOr<int> MulticastClient::send_message(const std::string &message) {
    // If the socket is not open, return.
    if (!is_socket_open) {
        return StatusOr<int>(Status::ERROR, "Socket is not open.", 0);
    }

    // Send the message.
    int bytes_sent =
        sendto(client_fd, message.c_str(), message.size(), 0, &client_addr, sizeof(client_addr));
    if (bytes_sent < 0) {
        perror("sendto");
        return StatusOr<int>(Status::ERROR, "Failed to send message.", 0);
    }

    // Return.
    return StatusOr<int>(Status::OK, "Message sent successfully.", bytes_sent);
}

/// See udp_client.h for documentation.
StatusOr<std::string> MulticastClient::receive_message() {
    // If the socket is not open, return.
    if (!is_socket_open) {
        return StatusOr<std::string>(Status::ERROR, "Socket is not open.", "");
    }

    // Receive the message.
    char buffer[1024];
    int bytes_received = recvfrom(client_fd, buffer, 1024, 0, NULL, NULL);
    if (bytes_received < 0) {
        perror("recvfrom");
        return StatusOr<std::string>(Status::ERROR, "Failed to receive message.", "");
    }

    // Return.
    return StatusOr<std::string>(Status::OK, "Message received successfully.",
                                 std::string(buffer, bytes_received));
}
