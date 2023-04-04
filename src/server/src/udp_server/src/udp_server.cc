#include "udp_server.h"

#include <arpa/inet.h>
#include <netdb.h>

#include <cstring>

#include "request.h"
#include "server_defaults.h"

using ostp::servercc::Request;
using ostp::servercc::server::Server;
using ostp::servercc::server::ServerMode;
using ostp::servercc::server::UdpServer;
using std::string;
using std::vector;

/// See tcp.h for documentation.
UdpServer::UdpServer(int16_t port, ServerMode mode, const string group,
                     const vector<string> interfaces)
    : Server(port, mode), group(group) {
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

    // Free hints.
    delete hints;
    hints = nullptr;

    // Setup the group address.
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(this->group.c_str());

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
        for (const string &interface : interfaces) {
            // Set multicast interface.
            mreq.imr_interface.s_addr = inet_addr(interface.c_str());

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
    this->server_socket_fd = server_socket_fd;
    this->server_addr = addr;
}

/// See tcp.h for documentation.
UdpServer::UdpServer(int16_t port, const string group, const vector<string> interfaces)
    : UdpServer(port, SERVERCC_DEFAULT_MODE, group, interfaces) {}

/// See tcp.h for documentation.
UdpServer::UdpServer(const string group, const vector<string> interfaces)
    : UdpServer(SERVERCC_DEFAULT_PORT, SERVERCC_DEFAULT_MODE, group, interfaces) {}

/// See tcp.h for documentation.
UdpServer::~UdpServer() { close(this->server_socket_fd); }

/// See server.h for documentation.
[[noreturn]] void UdpServer::run() {
    while (true) {
        // Read from the socket.
        std::vector<char> buffer(SERVERCC_BUFFER_SIZE);

        // Try to read from the client and store the
        Request request(-1, std::make_shared<struct sockaddr>());
        int addr_len = sizeof(struct sockaddr);
        int bytes_read = recvfrom(this->server_socket_fd, &buffer[0], buffer.size(), 0,
                                  request.addr.get(), (socklen_t *)&addr_len);

        // Find the first whitespace character.
        int i;
        for (i = 0; i < bytes_read && !isspace(buffer[i]); i++)
            ;

        // Move data into the request.
        request.protocol = std::string(buffer.begin(), buffer.begin() + i);
        request.data = std::string(buffer.begin(), buffer.begin() + bytes_read);

        // Look for the processor that handles the provided protocol and send the request to it.
        protocol_processors.get(&buffer[0], i)(std::move(request));
    }
}
