#include "tcp_server.h"

#include "absl/log/log.h"
#include "tcp_request.h"

namespace ostp::servercc {

// See tcp.h for documentation.
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

    LOG(INFO) << "Created TCP server on port " << port << " with socket fd " << serverSocketFd;
}

// See tcp.h for documentation.
TcpServer::~TcpServer() { close(serverSocketFd); }

// See server.h for documentation.
[[noreturn]] void TcpServer::run() {
    sockaddr clientAddr;
    in_addr_t clientAddrIp;
    socklen_t addr_len = sizeof(clientAddr);
    char ipStr[INET_ADDRSTRLEN];
    int clientSocketFd;
    while (true) {
        // Try to accept a connection.
        if ((clientSocketFd = accept(serverSocketFd, &clientAddr, &addr_len)) < 0) {
            perror("accept");
            continue;
        }

        // Log the connection.
        clientAddrIp = ((sockaddr_in *)&clientAddr)->sin_addr.s_addr;
        inet_ntop(AF_INET, &clientAddrIp, ipStr, INET_ADDRSTRLEN);
        LOG(INFO) << "Opened TCP connection with '" << ipStr << "' with socket fd "
                  << clientSocketFd;

        // Create a request checking for errors.
        auto [status, message] = readMessage(clientSocketFd);
        if (!status.ok()) {
            perror("readMessage");
            close(clientSocketFd);
            continue;
        }
        auto res = handleRequest(
            std::make_unique<TcpRequest>(clientSocketFd, clientAddr, std::move(message)));
        if (!res.ok()) {
            LOG(ERROR) << "Failed to handle request: " << res.message();
            continue;
        }
    }
}

}  // namespace ostp::servercc
