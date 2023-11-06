#include "tcp_server.h"

#include <cstring>
#include <vector>

#include "absl/container/flat_hash_map.h"

namespace ostp::servercc {

/// See tcp.h for documentation.
TcpServer::TcpServer(int16_t port, handler_t defaultProcessor) : Server(port, defaultProcessor) {
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
    int serverSocketFd = -1;
    while (addr != NULL) {
        // Try to create a socket.
        int yes = 1;

        // Try to create a socket.
        if ((serverSocketFd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0) {
            perror("socket");
            addr = addr->ai_next;
            continue;
        }

        // Try to configure the socket.
        if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
            perror("setsockopt");
            close(serverSocketFd);
            addr = addr->ai_next;
            continue;
        }

        // Try to bind.
        if (bind(serverSocketFd, addr->ai_addr, addr->ai_addrlen) < 0) {
            perror("bind");
            close(serverSocketFd);
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
    if (listen(serverSocketFd, 100) < 0) {
        perror("listen");
        throw "Error listening";
    }

    // Save the server address.
    this->serverSocketFd = serverSocketFd;
    this->serverAddress = addr;
}

/// See tcp.h for documentation.
TcpServer::~TcpServer() { close(serverSocketFd); }

/// See server.h for documentation.
[[noreturn]] void TcpServer::run() {
    sockaddr clientAddr;
    socklen_t addr_len = sizeof(clientAddr);
    int clientSocketFd;
    while (true) {
        // Try to accept a connection.
        if ((clientSocketFd = accept(serverSocketFd, &clientAddr, &addr_len)) < 0) {
            perror("accept");
            continue;
        }

        // Create a request checking for errors.
        std::unique_ptr<Request> request = std::make_unique<Request>();
        request->fd = clientSocketFd;
        request->addr = clientAddr;
        auto [status, message] = readMessage(clientSocketFd);
        if (!status.ok()) {
            perror("readMessage");
            close(clientSocketFd);
            continue;
        }
        request->message = std::move(message);
        handleRequest(std::move(request));
    }
}

} // namespace ostp::servercc
