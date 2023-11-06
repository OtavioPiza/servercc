#ifndef SERVERCC_CONNECTOR_H
#define SERVERCC_CONNECTOR_H

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "clients.h"
#include "types.h"

namespace ostp::servercc {

// A TCP connection manager that.
class Connector {
   public:
    // Constructors

    // Constructs a new connector with the specified default processor and disconnect handler.
    //
    // Arguments:
    //     default_processor: The default processor to use.
    //     disconnect_handler: The handler to use when a client disconnects.
    Connector(handler_t defaultHandler, std::function<void(absl::string_view)> disconnectHandler);

    // Destructor
    ~Connector();

    // Methods

    // Adds the specified processor for the specified path.
    //
    // Arguments:
    //     path: The path to add the processor for.
    //     processor: The processor to add.
    absl::Status addHandler(protocol_t protocol, handler_t handler);

    // Adds a TCP client to the connector.
    //
    // Arguments:
    //     client: The client to add.
    // Returns:
    //     The address of the client if successful, otherwise an error.
    absl::Status addClient(std::unique_ptr<TcpClient> client);

    // Send a message through the specified client.
    //
    // Arguments:
    //     address: The address of the client to send the message to.
    //     message: The message to send.
    //
    // Returns:
    //     The number of bytes sent if successful, otherwise an error.
    absl::Status sendMessage(absl::string_view address, const Message& message);

   private:
    // The map of protocol handlers.
    absl::flat_hash_map<protocol_t, handler_t> handlers;

    // The default handler to use when no handler is found for a protocol.
    handler_t defaultHandler;

    // The handler to use when a client disconnects.
    std::function<void(absl::string_view)> disconnectHandler;

    // A map of the current TCP clients identified by their address.
    absl::flat_hash_map<absl::string_view, std::shared_ptr<TcpClient>> clients;

    // Runs the specified client in a thread.
    //
    // Arguments:
    //     client: The client to run.
    absl::Status runClient(absl::string_view address);
};

}  // namespace ostp::servercc

#endif
