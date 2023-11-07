#ifndef SERVERCC_SERVER_H
#define SERVERCC_SERVER_H

#include <arpa/inet.h>
#include <netdb.h>

#include <functional>
#include <memory>

#include "absl/container/flat_hash_map.h"
#include "types.h"

namespace ostp::servercc {

// A generic server.
class Server {
   private:
    // The port the server is listening on.
    const uint16_t port;

    // Maps protocols to their handlers.
    absl::flat_hash_map<ostp::servercc::protocol_t, ostp::servercc::handler_t> handlers;

    // The default handler used if the protocol does not have a handler.
    ostp::servercc::handler_t defaultHandler;

   protected:
    // The server socket file descriptor.
    int serverSocketFd;

    // The server address.
    struct addrinfo *serverAddress;

   public:
    // Constructors

    // Constructs a server with the specified port, mode, and interfaces.
    //
    // Arguments:
    //     port: The port the server will listen on.
    //     mode: The mode the server will run in.
    //     default_processor: The default processor for the server.
    Server(uint16_t port, ostp::servercc::handler_t defaultHandler)
        : port(port), defaultHandler(defaultHandler){};

    // Getters

    // Gets the port the server is listening on.
    //
    // Returns:
    //     The port the server is listening on.
    uint16_t getPort() const { return port; }

    // Gets the server socket file descriptor.
    //
    // Returns:
    //     The server socket file descriptor.
    int getServerSocketFd() const { return serverSocketFd; }

    // Setters

    // Adds a handler for the specified protocol.
    //
    // Arguments:
    //     protocol: the protocol to add the handler for.
    //     handler: the handler to add.
    // Returns:
    //     An error if the handler already exists for the protocol, otherwise ok.
    absl::Status addHandler(ostp::servercc::protocol_t protocol,
                            ostp::servercc::handler_t handler) {
        if (handlers.contains(protocol)) {
            return absl::AlreadyExistsError("Handler already exists for protocol.");
        }
        handlers.emplace(protocol, handler);
        return absl::OkStatus();
    }

    // Updates the handler for the specified protocol.
    //
    // Arguments:
    //     protocol: the protocol to set the handler for.
    //     handler: the handler to set.
    // Returns:
    //     An error if the handler does not exist for the protocol, otherwise ok.
    absl::Status updateHandler(ostp::servercc::protocol_t protocol,
                               ostp::servercc::handler_t handler) {
        auto it = handlers.find(protocol);
        if (it == handlers.end()) {
            return absl::NotFoundError("Handler does not exist for protocol.");
        }
        it->second = handler;
        return absl::OkStatus();
    }

    // Methods

    // Handles a request.
    //
    // Arguments:
    //     request: The request to handle.
    void handleRequest(std::unique_ptr<ostp::servercc::Request> request) {
        // Execute the handler for the protocol's request or the default handler.
        ostp::servercc::handler_t handler = handlers[request->message->header.protocol];
        if (handler == nullptr) {
            handler = defaultHandler;
        }
        handler(std::move(request));
    }

    // Virtual methods

    // Runs the server.
    //
    // Returns:
    //     A status code indicating the success of the operation and the port the server is
    //     listening on.
    [[noreturn]] virtual void run() = 0;
};

}  // namespace ostp::servercc

#endif
