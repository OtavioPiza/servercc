#include "tcp_server.h"

#include <cstring>
#include <vector>

#include "request.h"
#include "server_defaults.h"

using ostp::servercc::Request;
using ostp::servercc::server::Server;
using ostp::servercc::server::ServerMode;
using ostp::servercc::server::TcpServer;

/// See tcp.h for documentation.
TcpServer::TcpServer(int16_t port, ServerMode mode,
                     std::function<void(const Request)> default_processor)
    : Server(port, mode, default_processor) {
    // Setup hints.
    struct addrinfo *result = nullptr, *hints = new struct addrinfo;
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;

    // Try to get the address info.
    if (getaddrinfo(NULL, std::to_string(port).c_str(), hints, &result) != 0) {
        perror("getaddrinfo");
        throw "Error getting address info";
    }

    // Free hints.
    delete hints;
    hints = nullptr;

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

        // Break if we were able to bind.
        break;
    }

    // Free the address info.
    freeaddrinfo(result);

    // Check for a valid address.
    if (addr == NULL) {
        throw "Error binding to address";
    }

    // Try to listen.
    if (listen(server_socket_fd, SERVERCC_DEFAULT_BACKLOG) < 0) {
        perror("listen");
        throw "Error listening";
    }

    // Save the server address.
    this->server_socket_fd = server_socket_fd;
    this->server_addr = addr;
}

/// See tcp.h for documentation.
TcpServer::TcpServer(int16_t port, std::function<void(const Request)> default_processor)
    : TcpServer(port, SERVERCC_DEFAULT_MODE, default_processor){};

/// See tcp.h for documentation.
TcpServer::TcpServer(std::function<void(const Request)> default_processor)
    : TcpServer(SERVERCC_DEFAULT_PORT, SERVERCC_DEFAULT_MODE, default_processor) {}

/// See tcp.h for documentation.
TcpServer::~TcpServer() { close(this->server_socket_fd); }

/// See server.h for documentation.
[[noreturn]] void TcpServer::run() {
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // Create a buffer for the request that is reused for each request.
    std::vector<char> buffer(SERVERCC_BUFFER_SIZE);

    while (true) {
        Request request;

        // Try to accept a connection.
        if ((request.fd = accept(server_socket_fd, request.addr.get(), &addr_len)) < 0) {
            perror("accept");
            continue;
        }

        // Try to read from the client and records its address.
        int bytes_read = recvfrom(request.fd, &buffer[0], buffer.size(), 0, request.addr.get(),
                                  &addr_len);

        // Check for errors.
        if (bytes_read < 0) {
            perror("recvfrom");
            close(request.fd);
            continue;
        }

        // Find the first whitespace character.
        int i;
        for (i = 0; i < bytes_read && !isspace(buffer[i]); i++)
            ;

        // Copy the protocol and data to the request.
        request.protocol = std::string(buffer.begin(), buffer.begin() + i);
        request.data = std::string(buffer.begin() + i, buffer.begin() + bytes_read);

        // Process the request. Move is safe because the request is not used after this and
        // is initialized again in the next iteration.
        protocol_processors.get(request.protocol.c_str(),
                                request.protocol.length())(std::move(request));
    }
}
