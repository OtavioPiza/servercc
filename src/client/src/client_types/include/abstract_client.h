#ifndef SERVERCC_ABSTRACT_CLIENT_H
#define SERVERCC_ABSTRACT_CLIENT_H

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

   public:
    // Constructors

    /// Constructs a client with the specified address and port.
    ///
    /// Arguments:
    ///     server_address: The server's address.
    ///     port: The server's port.
    Client(const std::string server_address, const uint16_t port)
        : server_address(server_address), port(port) {}

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

    // Virtual Methods

    /// Sends a request to the server.
    ///
    /// Arguments:
    ///     request: The request to send.
    ///
    /// Returns:
    ///     A StatusOr<void> indicating whether the request was sent successfully and the number of
    ///     bytes sent.
    virtual StatusOr<int> send_request(const std::string request) = 0;

    /// Receives a response from the server.
    ///
    /// Returns:
    ///     The response from the server.
    virtual std::string receive_response() = 0;
};

}  // namespace ostp::servercc::client

#endif
