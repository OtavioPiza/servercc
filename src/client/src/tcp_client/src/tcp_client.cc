#include "tcp_client.h"

using ostp::libcc::utils::Status;
using ostp::libcc::utils::StatusOr;
using ostp::servercc::client::TcpClient;

/// See tcp_client.h for documentation.
TcpClient::TcpClient(const std::string server_address, const uint16_t port)
    : Client(server_address, port) {};

/// See tcp_client.h for documentation.
StatusOr<bool> TcpClient::open_socket() {
    // If the socket is already open, return.
    if (is_socket_open) {
        return StatusOr<bool>(Status::SUCCESS, "Socket is already open.", true);
    }

    // Allocate structures for socket address.
    struct addrinfo *server_info, *hints = new addrinfo;
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = 0;

    // Try to resolve the server address.
    if (getaddrinfo(get_address().c_str(), std::to_string(get_port()).c_str(), hints,
                    &server_info) != 0) {
        delete hints;
        return StatusOr<bool>(Status::ERROR, "Could not resolve server address.", false);
    }

    // Go through the list of addresses and try to connect to the server.
    for (struct addrinfo *p = server_info; p != nullptr; p = p->ai_next) {
        // Create a socket.
        if ((client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        // Connect to the server.
        if (connect(client_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(client_fd);
            client_fd = -1;
            continue;
        }

        // If we get here, we have successfully connected to the server.
        break;
    }

    // Free the server info and hints.
    freeaddrinfo(server_info);
    delete hints;

    // If we could not connect to the server, throw an exception.
    if (client_fd == -1) {
        return StatusOr<bool>(Status::ERROR, "Could not connect to server.", false);
    }

    // Mark the socket as open and return.
    is_socket_open = true;
    return StatusOr<bool>(Status::SUCCESS, "Socket opened successfully.", true);
}

/// See tcp_client.h for documentation.
StatusOr<bool> TcpClient::close_socket() {
    // If the socket is already closed, return.
    if (client_fd == -1) {
        return StatusOr<bool>(Status::SUCCESS, "Socket is already closed.", true);
    }

    // Close the socket.
    close(client_fd);
    client_fd = -1;
    is_socket_open = false;

    // Return.
    return StatusOr<bool>(Status::SUCCESS, "Socket closed successfully.", true);
}

/// See tcp_client.h for documentation.
StatusOr<int> TcpClient::send_message(const std::string message) {
    // If the socket is not open, throw an exception.
    if (client_fd == -1) {
        return StatusOr<int>(Status::ERROR, "Socket is not open.", -1);
    }

    // Send the message.
    int bytes_sent = send(client_fd, message.c_str(), message.length(), 0);

    // If we could not send the message, throw an exception.
    if (bytes_sent == -1) {
        return StatusOr<int>(Status::ERROR, "Could not send message.", -1);
    }

    // Return the number of bytes sent.
    return StatusOr<int>(Status::SUCCESS, "Message sent successfully.", bytes_sent);
}

/// See tcp_client.h for documentation.
StatusOr<std::string> TcpClient::receive_message() {
    // If the socket is not open, return an error.
    if (client_fd == -1) {
        return StatusOr<std::string>(Status::ERROR, "Socket is not open.", "");
    }

    // Receive the message.
    char buffer[1024];
    int bytes_received = recv(client_fd, buffer, 1024, 0);

    // If we could not receive the message, return an error.
    if (bytes_received == -1) {
        return StatusOr<std::string>(Status::ERROR, "Could not receive message.", "");
    }

    // Return the message.
    return StatusOr<std::string>(Status::SUCCESS, "Message received successfully.",
                                 std::string(buffer, bytes_received));
}