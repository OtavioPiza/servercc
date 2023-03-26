#ifndef SERVERCC_ABSTRACT_SERVER_H
#define SERVERCC_ABSTRACT_SERVER_H

#include <netdb.h>

#include <functional>

#include "default_trie.h"
#include "request.h"
#include "server_mode.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::servercc::Request;
using ostp::servercc::server::ServerMode;

namespace ostp::servercc::server {

/// A generic server to handle multiple protocols.
class Server {
   private:
    /// The port the server is listening on.
    const uint16_t port;

    /// The mode the server is running in.
    const ServerMode mode;

   protected:
    /// The server socket file descriptor.
    int server_socket_fd;

    /// The server address.
    struct addrinfo *server_addr;

    /// Trie to store the protocol processors.
    DefaultTrie<char, std::function<void(const Request)>> protocol_processors;

   public:
    // Constructors

    /// Constructs a server with the specified port, mode, and interfaces.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    ///     mode: The mode the server will run in.
    ///     interfaces: The interfaces the server will listen on.
    Server(uint16_t port, ServerMode mode) : port(port), mode(mode), protocol_processors(nullptr) {}

    // Getters

    /// Gets the port the server is listening on.
    ///
    /// Returns:
    ///     The port the server is listening on.
    uint16_t get_port() const { return port; }

    /// Gets the mode the server is running in.
    ///
    /// Returns:
    ///     The mode the server is running in.
    ServerMode get_mode() const { return mode; }

    /// Gets the server socket file descriptor.
    ///
    /// Returns:
    ///     The server socket file descriptor.
    int get_server_socket_fd() const { return server_socket_fd; }

    // Setters

    /// Sets the default processor for the server to the specified processor.
    ///
    /// Arguments:
    ///     processor: The processor to set as the default.
    void set_default_processor(std::function<void(const Request)> processor) {
        protocol_processors.update_default_return(processor);
    }

    /// Sets the processor for the specified protocol to the specified processor.
    ///
    /// Arguments:
    ///     protocol: The protocol to set the processor for.
    ///     processor: The processor to set for the protocol.
    void set_processor(const std::string protocol, std::function<void(const Request)> processor) {
        protocol_processors.insert(protocol.c_str(), protocol.size(), processor);
    }

    // Virtual methods

    /// Runs the server.
    ///
    /// Returns:
    ///     A status code indicating the success of the operation and the port the server is
    ///     listening on.
    [[noreturn]] virtual void run() = 0;
};

}  // namespace ostp::servercc::server

#endif
