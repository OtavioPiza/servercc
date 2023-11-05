#ifndef SERVERCC_SERVER_H
#define SERVERCC_SERVER_H

#include <netdb.h>

#include <functional>
#include <memory>

#include "absl/container/flat_hash_map.h"
#include "../../servercc/types/types.h"

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

    // Sets the handler for the specified protocol.
    //
    // Arguments:
    //     protocol: the protocol to set the handler for.
    //     handler: the handler to set.
    void setHandler(ostp::servercc::protocol_t protocol, ostp::servercc::handler_t handler) {
        handlers[protocol] = handler;
    }

    // Methods

    // Handles a request.
    //
    // Arguments:
    //     request: The request to handle.
    void handleRequest(std::unique_ptr<ostp::servercc::Request> request) {
        // Execute the handler for the protocol's request or the default handler.
        ostp::servercc::handler_t handler = handlers[request->message.header.protocol];
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

}  // namespace ostp::servercc::server

#endif
