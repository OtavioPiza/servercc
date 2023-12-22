#include "udp_server.h"

#include "udp_request.h"

namespace ostp::servercc {

// See tcp.h for documentation.
UdpServer::UdpServer(int16_t port, absl::string_view groupAddress,
                     std::vector<absl::string_view> interfaces, handler_t defaultProcessor)
    : Server(port, defaultProcessor), groupAddress(groupAddress) {
    // Setup hints for udp with multicast.
    struct addrinfo *result = nullptr, *hints = new struct addrinfo;
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_DGRAM;
    hints->ai_flags = AI_PASSIVE;

    // Try to get the address info.
    if (getaddrinfo(NULL, std::to_string(port).c_str(), hints, &result) != 0) {
        perror("getaddrinfo");
        throw "Error getting address info";
    }
    delete hints;
    hints = nullptr;

    // Setup the group address.
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(this->groupAddress.data());

    // Bind to the first address.
    struct addrinfo *addr = result;
    int server_socket_fd = -1;
    while (addr != NULL) {
        // Try to create a socket.
        int yes = 1;
        // Try to create a socket.
        if ((server_socket_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) <
            0) {
            perror("socket");
            addr = addr->ai_next;
            continue;
        }

        // Try to configure the socket.
        if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
            perror("setsockopt");
            close(server_socket_fd);
            addr = addr->ai_next;
            continue;
        }

        // Try to bind.
        if (bind(server_socket_fd, addr->ai_addr, addr->ai_addrlen) < 0) {
            perror("bind");
            close(server_socket_fd);
            addr = addr->ai_next;
            continue;
        }

        // Try to join the multicast group on every interface.
        bool success = true;
        for (auto interface : interfaces) {
            // Set multicast interface.
            mreq.imr_interface.s_addr = inet_addr(interface.data());

            // Try to join the multicast group on that interface.
            if (setsockopt(server_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) <
                0) {
                perror("setsockopt");
                close(server_socket_fd);
                addr = addr->ai_next;
                success = false;
                break;
            }
        }

        // If we failed to join the multicast group on any interface, continue.
        if (!success) {
            continue;
        }

        // Break if we were able to bind.
        break;
    }

    // Free the address info.
    freeaddrinfo(result);

    // Check for a valid address.
    if (addr == NULL) {
        throw "Error binding to address";
    }

    // Save the server address.
    this->serverAddress = addr;
    this->serverSocketFd = server_socket_fd;
}

// See tcp.h for documentation.
UdpServer::~UdpServer() { close(this->serverSocketFd); }

// See server.h for documentation.
[[noreturn]] void UdpServer::run() {
    socklen_t addr_len = sizeof(struct sockaddr);

    while (true) {
        sockaddr addr;
        // Create a request.
        auto message = std::make_unique<Message>();
        int bytes_read = recvfrom(this->serverSocketFd, &message->header, kMessageHeaderLength, 0,
                                  &addr, &addr_len);
        if (bytes_read < kMessageHeaderLength) {
            perror("recvfrom");
            continue;
        }
        message->body.data.resize(message->header.length);
        bytes_read = recvfrom(this->serverSocketFd, message->body.data.data(),
                              message->header.length, 0, &addr, &addr_len);
        if (bytes_read < message->header.length) {
            perror("recvfrom");
            continue;
        }
        handleRequest(std::make_unique<UdpRequest>(addr, std::move(message)));
    }
}

}  // namespace ostp::servercc
