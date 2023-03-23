#include "udp_client.h"

#include <net/if.h>

using ostp::libcc::utils::Status;
using ostp::libcc::utils::StatusOr;
using ostp::servercc::client::UdpClient;

UdpClient::UdpClient(const std::string address, const uint16_t port, const std::string interface)
    : Client(address, port), interface(interface) {}

StatusOr<int> UdpClient::multicast_request(const std::string request, const std::string interface,
                                           const std::string multicast_group) {
    // Create a socket.
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return StatusOr<int>(Status::ERROR, "Failed to create socket.", 0);
    }

    // Set the socket options.
    int reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        return StatusOr<int>(Status::ERROR, "Failed to set socket options.", 0);
    }

    // Set the interface.
    struct ip_mreqn mreqn;
    memset(&mreqn, 0, sizeof(mreqn));
    mreqn.imr_ifindex = if_nametoindex(interface.c_str());
    if (setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_IF, &mreqn, sizeof(mreqn)) < 0) {
        perror("setsockopt");
        return StatusOr<int>(Status::ERROR, "Failed to set socket options.", 0);
    }

    // Set the multicast group.
    struct sockaddr_in group_address;
    memset(&group_address, 0, sizeof(group_address));
    group_address.sin_family = AF_INET;
    group_address.sin_addr.s_addr = inet_addr(multicast_group.c_str());
    group_address.sin_port = htons(get_port());

    // Send the request.
    int bytes_sent = sendto(socket_fd, request.c_str(), request.length(), 0,
                            (struct sockaddr *)&group_address, sizeof(group_address));
    if (bytes_sent < 0) {
        perror("sendto");
        return StatusOr<int>(Status::ERROR, "Failed to send request.", 0);
    }
}