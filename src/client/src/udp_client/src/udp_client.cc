#include "udp_client.h"

#include <net/if.h>

using ostp::libcc::utils::Status;
using ostp::libcc::utils::StatusOr;
using ostp::servercc::client::UdpClient;

/// See udp_client.h for documentation.
UdpClient::UdpClient(const std::string interface, const std::string server_address,
                     const uint16_t port, const uint8_t ttl)
    : Client(server_address, port), interface(interface), ttl(ttl), multicast_group(std::nullopt){};

/// See udp_client.h for documentation.
UdpClient::UdpClient(const std::string interface, const std::string server_address,
                     const uint16_t port, const uint8_t ttl, const std::string multicast_group)
    : Client(server_address, port),
      interface(interface),
      ttl(ttl),
      multicast_group(multicast_group){};

StatusOr<bool> UdpClient::open_socket() {
    // If the socket is already open, return.
    if (is_socket_open) {
        return StatusOr<bool>(Status::SUCCESS, "Socket is already open.", true);
    }

    // Create a socket to connect to the server.
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return StatusOr<bool>(Status::ERROR, "Failed to create socket.", 0);
    }

    // Set the socket options.
    int reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        return StatusOr<bool>(Status::ERROR, "Failed to set socket options.", 0);
    }

    // Set the interface.
    struct ip_mreqn mreqn;
    memset(&mreqn, 0, sizeof(mreqn));
    mreqn.imr_ifindex = if_nametoindex(interface.c_str());
    if (setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_IF, &mreqn, sizeof(mreqn)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        return StatusOr<bool>(Status::ERROR, "Failed to set socket options.", 0);
    }

    // If a multicast group was specified, join it.
    if (multicast_group.has_value()) {
        struct ip_mreqn mreqn;
        memset(&mreqn, 0, sizeof(mreqn));
        mreqn.imr_ifindex = if_nametoindex(interface.c_str());
        mreqn.imr_address.s_addr = htonl(INADDR_ANY);
        mreqn.imr_multiaddr.s_addr = inet_addr(multicast_group.value().c_str());
        if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreqn, sizeof(mreqn)) < 0) {
            perror("setsockopt");
            close(socket_fd);
            return StatusOr<bool>(Status::ERROR, "Failed to set socket options.", 0);
        }
    }

    // Set the time-to-live.
    if (setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        return StatusOr<bool>(Status::ERROR, "Failed to set socket options.", 0);
    }

    // Set the socket address.
    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = htonl(INADDR_ANY);
    client_address.sin_port = htons(get_port());

    // Set the socket file and return.
    client_fd = socket_fd;
    is_socket_open = true;
    return StatusOr<bool>(Status::SUCCESS, "Socket opened successfully.", true);
}

/// See udp_client.h for documentation.
StatusOr<bool> UdpClient::close_socket() {};
