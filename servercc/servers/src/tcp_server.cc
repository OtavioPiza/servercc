#include "tcp_server.h"

#include <cstring>
#include <vector>

#include "absl/container/flat_hash_map.h"

using ostp::servercc::handler_t;
using ostp::servercc::kMessageHeaderLength;
using ostp::servercc::protocol_t;
using ostp::servercc::Request;
using ostp::servercc::Server;
using ostp::servercc::TcpServer;
using std::function;
using std::vector;

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
        std::unique_ptr<Request> request = std::make_unique<Request>(clientSocketFd, clientAddr);
        int bytesRead = recv(clientSocketFd, &request->message.header, kMessageHeaderLength, 0);
        if (bytesRead <= kMessageHeaderLength) {
            perror("recv");
            close(clientSocketFd);
            continue;
        }
        request->message.header.length = ntohl(request->message.header.length);
        request->message.header.protocol = ntohs(request->message.header.protocol);
        request->message.body.data.resize(request->message.header.length);
        bytesRead = recv(clientSocketFd, request->message.body.data.data(),
                         request->message.header.length, 0);
        if (bytesRead <= request->message.header.length) {
            perror("recvfrom");
            close(clientSocketFd);
            continue;
        }
        handleRequest(std::move(request));
    }
}
