#ifndef SERVERCC_ABSTRACT_CLIENT_H
#define SERVERCC_ABSTRACT_CLIENT_H

#include <netdb.h>

#include <memory>
#include <string>

#include "status_or.h"

using ostp::libcc::utils::StatusOr;

namespace ostp::servercc::client {

/// A generic client to handle multiple transports layer protocols.
class Client {
   private:
    /// The server's address.
    const std::string server_address;

    /// The server's port.
    const uint16_t port;

   protected:
    // Instance variables

    /// The client's socket file descriptor.
    int client_fd;

    /// Whether the socket is open.
    bool is_socket_open;

    /// The clients addr.
    std::shared_ptr<struct sockaddr> client_addr;

    // Constructors

    /// Constructs a client with the specified address and port.
    ///
    /// Arguments:
    ///     server_address: The server's address.
    ///     port: The server's port.
    Client(const std::string server_address, const uint16_t port)
        : server_address(server_address), port(port), is_socket_open(false), client_fd(-1) {
        client_addr = std::make_shared<sockaddr>();
    }

    /// Constructs a client with the address, port, and sockaddr.
    ///
    /// Arguments:
    ///     server_address: The server's address.
    ///     port: The server's port.
    ///     client_addr: The client's addr.
    Client(const std::string server_address, const uint16_t port,
           std::shared_ptr<struct sockaddr> client_addr)
        : server_address(server_address),
          port(port),
          is_socket_open(false),
          client_fd(-1),
          client_addr(client_addr) {}

   public:
    // Getters

    /// Gets the server's address.
    ///
    /// Returns:
    ///     The server's address.
    virtual std::string get_address() { return server_address; }

    /// Gets the server's port.
    ///
    /// Returns:
    ///     The server's port.
    virtual uint16_t get_port() { return port; }

    /// Gets the client's socket file descriptor.
    ///
    /// Returns:
    ///     The client's socket file descriptor.
    virtual int get_fd() { return client_fd; }

    /// Checks whether the socket is open.
    ///
    /// Returns:
    ///     Whether the socket is open.
    virtual bool is_open() { return is_socket_open; }

    /// Gets the client's addr.
    ///
    /// Returns:
    ///     A shared pointer to the client's addr.
    virtual std::shared_ptr<struct sockaddr> get_addr() { return client_addr; }

    // Virtual Methods

    /// Opens the client's socket.
    ///
    /// Returns:
    ///     A status indicating whether the socket was opened successfully.
    virtual StatusOr<bool> open_socket() = 0;

    /// Closes the client's socket.
    ///
    /// Returns:
    ///     A status indicating whether the socket was closed successfully.
    virtual StatusOr<bool> close_socket() = 0;

    /// Sends a message to the server.
    ///
    /// Arguments:
    ///     message: The message to send.
    ///
    /// Returns:
    ///     A status indicating whether the message was sent successfully and the
    ///     number of bytes sent.
    virtual StatusOr<int> send_message(const std::string &message) = 0;

    /// Blocks until a message is received from the server.
    ///
    /// Returns:
    ///     A status indicating whether a message was received successfully and
    ///     the message received.
    virtual StatusOr<std::string> receive_message() = 0;
};

}  // namespace ostp::servercc::client

#endif
