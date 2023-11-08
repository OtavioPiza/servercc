#ifndef SERVERCC_DISTRIBUTED_SERVER_H_
#define SERVERCC_DISTRIBUTED_SERVER_H_

#include <inttypes.h>

#include <optional>
#include <queue>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "clients.h"
#include "connectors.h"
#include "message_buffer.h"
#include "servers.h"
#include "types.h"

namespace ostp::servercc {

// A distributed server that can be used to create a distributed system.
class DistributedServer {
   public:
    // Server attributes.

    // The name of the interface to use for the distributed server.
    absl::string_view interfaceName;

    // The interface ip address.
    const std::vector<absl::string_view> interfaces;

    // The group ip address.
    absl::string_view group;

    // The port to use for the distributed server.
    const uint16_t port;

    // Constructors.

    // Creates a new DistributedServer on the specified interface, group and port.
    //
    // A connect request has the following format:
    //     connect <port>
    //
    // Arguments:
    //     interfaceName: The name of the interface to use for the distributed server.
    //     group: The group ip address.
    //     interfaces: The interface ip address.
    //     port: The port to use for the distributed server.
    //     default_handler: The default handler to use for the distributed server.
    //     peer_connect_callback: The callback to call when a peer connects.
    //     peer_disconnect_callback: The callback to call when a peer disconnects.
    DistributedServer(
        absl::string_view interfaceName, absl::string_view group,
        std::vector<absl::string_view> interfaces, const uint16_t port, handler_t default_handler,
        const std::function<void(absl::string_view, DistributedServer &server)> peerConnectCallback,
        const std::function<void(absl::string_view string, DistributedServer &server)>
            peerDisconnectCallback);

    // Methods

    // Method to run the distributed server.
    absl::Status run();

    // Method to add a new handler for the specified protocol.
    //
    // Arguments:
    //     protocol: The protocol to add a handler for.
    //     handler: The handler to add.
    //
    // Returns:
    //     A StatusOr<void> with the status of the operation.
    absl::Status addHandler(protocol_t protocol, handler_t handler);

    // Server utilities.

    // Method to send a multicast message to all the servers.
    //
    // Arguments:
    //     message: The message to send.
    //
    // Returns:
    //     The number of bytes sent or an error.
    absl::Status multicastMessage(std::unique_ptr<Message> message);

    // Method to send a multicast a connect request to all the servers.
    //
    // Returns:
    //    The number of bytes sent or an error.
    absl::Status sendConnectMessage();

    // Method to send a message to a specific server.
    //
    // Arguments:
    //     ip: The ip address of the server to send the message to.
    //     message: The message to send.
    //
    // Returns:
    //     The ID of the message or an error.
    std::pair<absl::Status, uint32_t> sendMessage(absl::string_view address,
                                                  std::unique_ptr<Message> message);

    // Method to wait for a response to a message.
    //
    // Arguments:
    //     id: The ID of the message to wait for.
    //
    // Returns:
    //     The next segment of the response or an error.
    std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage(const uint32_t id);

   private:
    // Server components.

    // The TCP server to handle TCP requests.
    TcpServer tcpServer;

    // The UDP server to handle multicast requests.
    UdpServer udpServer;

    // The connector to handle inter-server requests.
    Connector connector;

    // The multicast client to send multicast requests.
    MulticastClient multicastClient;

    // Service threads.

    // The thread to run the TCP server.
    std::thread tcpServerThread;

    // The thread to run the UDP server.
    std::thread udpServerThread;

    // Peer server datastructures.

    // The list of peers connected to the distributed server.
    absl::flat_hash_set<absl::string_view> peers;

    // Handling datastructures.

    // The map of protocol handlers.
    absl::flat_hash_map<protocol_t, handler_t> handlers;

    // The default handler to use for the distributed server if no handler is found for a
    // protocol.
    handler_t defaultHandler;

    // Maps a message ID to a semaphore to wait for a response and a queue to store the
    // response.
    absl::flat_hash_map<
        uint32_t,
        std::shared_ptr<ostp::libcc::data_structures::MessageBuffer<std::unique_ptr<Message>>>>
        messageBuffers;

    // Maps a peer IP to a set of message IDs that have been sent to it.
    absl::flat_hash_map<absl::string_view, absl::flat_hash_set<int>> peersToMessageIds;

    // Maps a message ID to the peer IP that it was sent to.
    absl::flat_hash_map<int, absl::string_view> messageIdsToPeers;

    // Callbacks.

    // The callback to call when a peer connects.
    const std::function<void(absl::string_view, DistributedServer &server)> peerConnectCallback;

    // The callback to call when a peer disconnects.
    const std::function<void(absl::string_view, DistributedServer &server)> peerDisconnectCallback;

    // Server service methods.

    // Method to run the TCP server.
    absl::Status runTcpServer();

    // Method to run the UDP server.
    absl::Status runUdpServer();

    // Internal callback methods.

    // Method to handle a Connector disconnect.
    //
    // Arguments:
    //     ip: The ip address of the peer that disconnected.
    void onConnectorDisconnect(absl::string_view ip);

    // Handler methods.

    // Method to handle a connect request.
    //
    // Arguments:
    //     request: The request to handle.
    void handleConnect(std::unique_ptr<Request> request);

    // Method to handle a connect_ack request.
    //
    // Arguments:
    //     request: The request to handle.
    void handleConnectAck(std::unique_ptr<Request> request);

    // Method to forward the specified request to the protocol processors.
    //
    // Arguments:
    //     request: The request to forward.
    void forwardRequestToHandler(std::unique_ptr<Request> request);

    // Method to handle an internal request.
    //
    // Arguments:
    //     request: The request to handle.
    void handleInternalRequest(std::unique_ptr<Request> request);

    // Method to handle an internal response.
    //
    // Arguments:
    //     request: The request to handle.
    void handleInternalResponse(std::unique_ptr<Request> request);

    // Method to handle an internal response end.
    //
    // Arguments:
    //     request: The request to handle.
    void handleInternalResponseEnd(std::unique_ptr<Request> request);
};

}  // namespace ostp::servercc

#endif
