#include "tcp_server.h"

#include <cstring>
#include <vector>

#include "request.h"
#include "server_defaults.h"

using ostp::servercc::Request;
using ostp::servercc::server::Server;
using ostp::servercc::server::ServerMode;
using ostp::servercc::server::TcpServer;
using std::vector;

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
    sockaddr client_addr;
    int client_socket_fd;

    while (true) {
        socklen_t addr_len = sizeof(client_addr);

        // Try to accept a connection.
        if ((client_socket_fd = accept(server_socket_fd, &client_addr, &addr_len)) < 0) {
            perror("accept");
            continue;
        }

        // Try to read the first 4 bytes of the request to get the length.
        uint32_t request_length = 0;
        int bytes_read = recv(client_socket_fd, &request_length, 4, 0);

        // Check for errors.
        if (bytes_read < 0) {
            perror("recv");
            close(client_socket_fd);
            continue;
        }

        // Check for a closed connection.
        if (bytes_read == 0) {
            close(client_socket_fd);
            continue;
        }

        // Get the length of the request.
        request_length = ntohl(request_length);

        // Create a buffer for the request of the correct size.
        vector<char> buffer(request_length);

        // Try to read from the client and records its address.
        bytes_read =
            recvfrom(client_socket_fd, buffer.data(), buffer.size(), 0, &client_addr, &addr_len);

        // Check for errors.
        if (bytes_read < 0) {
            perror("recvfrom");
            close(client_socket_fd);
            continue;
        }

        // Check for a closed connection.
        if (bytes_read == 0) {
            close(client_socket_fd);
            continue;
        }

        // Find the first whitespace character.
        int i;
        for (i = 0; i < bytes_read && !isspace(buffer[i]); i++)
            ;

        // Create a request from the buffer.
        Request request(client_socket_fd, std::make_shared<sockaddr>(client_addr));
        request.protocol = std::string(buffer.begin(), buffer.begin() + i);
        request.data = std::string(buffer.begin(), buffer.end());

        // Process the request. Move is safe because the request is not used after this and
        // is initialized again in the next iteration.
        protocol_processors.get(request.protocol.c_str(),
                                request.protocol.length())(std::move(request));
    }
}
