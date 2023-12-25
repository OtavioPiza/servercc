#ifndef SERVERCC_CONNECTOR_H
#define SERVERCC_CONNECTOR_H

#include <functional>
#include <memory>
#include <thread>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "clients.h"
#include "connector_types.h"
#include "internal_channel_manager.h"
#include "internal_request.h"
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
    Connector(handler_t defaultHandler, std::function<void(in_addr_t)> disconnectCallback);

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
    //
    // Returns:
    //     The status ofr the operation and a request if successful.
    std::pair<absl::Status, std::unique_ptr<Request>>
    sendRequest(in_addr_t address);

   private:
    // Represents an internal client with a channel manager and write mutex.
    class InternalClient {
       public:
        InternalClient(const std::shared_ptr<TcpClient> client,
                       const std::shared_ptr<std::mutex> writeMutex,
                       const std::shared_ptr<connector_channel_manager_t> channelManager)
            : client(client), writeMutex(writeMutex), channelManager(channelManager) {}

        const std::shared_ptr<TcpClient> client;
        const std::shared_ptr<std::mutex> writeMutex;
        const std::shared_ptr<connector_channel_manager_t> channelManager;
    };

    // The map of protocol handlers.
    absl::flat_hash_map<protocol_t, handler_t> handlers;

    // The default handler to use when no handler is found for a protocol.
    handler_t defaultHandler;

    // The handler to use when a client disconnects.
    std::function<void(in_addr_t)> disconnectCallback;

    // A map of the current TCP clients identified by their address.
    absl::flat_hash_map<in_addr_t, InternalClient> clients;

    // Runs the specified client in a thread.
    //
    // Arguments:
    //     client: The client to run.
    absl::Status runClient(in_addr_t address);

    // The mutex protecting the clients map.
    std::mutex clientsMutex;

    // The mutex protecting the handlers map.
    std::mutex handlersMutex;
};

}  // namespace ostp::servercc

#endif
