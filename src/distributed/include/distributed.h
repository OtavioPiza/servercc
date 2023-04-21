#ifndef SERVERCC_DISTRIBUTED_H
#define SERVERCC_DISTRIBUTED_H

#include <queue>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "connector.h"
#include "message_buffer.h"
#include "multicast_client.h"
#include "request.h"
#include "status_or.h"
#include "tcp_server.h"
#include "udp_server.h"

using ostp::libcc::data_structures::MessageBuffer;
using ostp::libcc::utils::Status;
using ostp::servercc::Request;
using ostp::servercc::client::MulticastClient;
using ostp::servercc::connector::Connector;
using ostp::servercc::server::TcpServer;
using ostp::servercc::server::UdpServer;
using std::binary_semaphore;
using std::function;
using std::pair;
using std::queue;
using std::shared_ptr;
using std::string;
using std::thread;
using std::unordered_map;
using std::unordered_set;

namespace ostp::servercc::distributed {

/// A distributed server that can be used to create a distributed system.
class DistributedServer {
   public:
    // Server attributes.

    /// The name of the interface to use for the distributed server.
    const string interface_name;

    /// The interface ip address.
    const string interface_ip;

    /// The group ip address.
    const string group;

    /// The port to use for the distributed server.
    const uint16_t port;

    // Constructors.

    /// Creates a new DistributedServer on the specified interface, group and port.
    ///
    /// A connect request has the following format:
    ///     connect <port>
    ///
    /// Arguments:
    ///     interface_name: The name of the interface to use for the distributed server.
    ///     interface_ip: The interface ip address.
    ///     group: The group ip address.
    ///     port: The port to use for the distributed server.
    ///     default_handler: The default handler to use for the distributed server.
    ///     peer_connect_callback: The callback to call when a peer connects.
    ///     peer_disconnect_callback: The callback to call when a peer disconnects.
    DistributedServer(
        const string interface_name, const string interface_ip, const string group,
        const uint16_t port, const function<void(const Request)> default_handler,
        const function<void(const string, DistributedServer &server)> peer_connect_callback,
        const function<void(const string, DistributedServer &server)> peer_disconnect_callback);

    // Methods

    /// Method to run the distributed server.
    void run();

    /// Method to add a new handler for the specified protocol.
    ///
    /// Arguments:
    ///     protocol: The protocol to add a handler for.
    ///     handler: The handler to add.
    ///
    /// Returns:
    ///     A StatusOr<void> with the status of the operation.
    StatusOr<void> add_handler(const string &protocol, function<void(const Request)> handler);

    // Server utilities.

    /// Method to send a multicast message to all the servers.
    ///
    /// Arguments:
    ///     message: The message to send.
    ///
    /// Returns:
    ///     The number of bytes sent or an error.
    StatusOr<int> multicast_message(const string &message);

    /// Method to send a multicast a connect request to all the servers.
    ///
    /// Returns:
    ///    The number of bytes sent or an error.
    StatusOr<int> send_connect_message();

    /// Method to send a message to a specific server.
    ///
    /// Arguments:
    ///     ip: The ip address of the server to send the message to.
    ///     message: The message to send.
    ///
    /// Returns:
    ///     The ID of the message or an error.
    StatusOr<int> send_message(const string &address, const string &message);

    /// Method to wait for a response to a message.
    ///
    /// Arguments:
    ///     id: The ID of the message to wait for.
    ///
    /// Returns:
    ///     The next segment of the response or an error.
    StatusOr<const string> receive_message(const int id);

    /// Adds a log message to the log queue.
    ///
    /// Arguments:
    ///     status: The status of the log message.
    ///     message: The log message.
    void log(const Status status, const string message);

   private:
    // Server components.

    /// The TCP server to handle TCP requests.
    TcpServer tcp_server;

    /// The UDP server to handle multicast requests.
    UdpServer udp_server;

    /// The connector to handle inter-server requests.
    Connector connector;

    /// The multicast client to send multicast requests.
    MulticastClient multicast_client;

    // Service threads.

    /// The thread to run the TCP server.
    thread tcp_server_thread;

    /// The thread to run the UDP server.
    thread udp_server_thread;

    /// The thread to run the logger service.
    thread logger_service_thread;

    // Peer server datastructures.

    /// The list of peers connected to the distributed server.
    unordered_set<string> peers;

    // Handling datastructures.

    /// Trie to store the protocol commands supported by the distributed server.
    DefaultTrie<char, function<void(const Request)>> protocol_processors;

    /// Maps a message ID to a semaphore to wait for a response and a queue to store the response.
    unordered_map<int, shared_ptr<MessageBuffer>> message_buffers;

    /// Maps a peer IP to a set of message IDs that have been sent to it.
    unordered_map<string, unordered_set<int>> peers_to_message_ids;

    /// Maps a message ID to the peer IP that it was sent to.
    unordered_map<int, string> message_ids_to_peers;

    // Callbacks.

    /// The callback to call when a peer connects.
    const function<void(const string, DistributedServer &server)> peer_connect_callback;

    /// The callback to call when a peer disconnects.
    const function<void(const string, DistributedServer &server)> peer_disconnect_callback;

    // Logging datastructures.

    /// The semaphore to protect the log queue.
    binary_semaphore log_queue_semaphore;

    /// The queue of log messages.
    queue<pair<const Status, const string>> log_queue;

    // Server service methods.

    /// Method to run the TCP server.
    void run_tcp_server();

    /// Method to run the UDP server.
    void run_udp_server();

    /// Waits for a log message to be added to the log queue and prints it indefinitely.
    void run_logger_service();

    // Internal callback methods.

    /// Method to handle a Connector disconnect.
    ///
    /// Arguments:
    ///     ip: The ip address of the peer that disconnected.
    void on_connector_disconnect(const string &ip);

    // Handler methods.

    /// Method to handle a connect request.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_connect(const Request request);

    /// Method to handle a connect_ack request.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_connect_ack(const Request request);

    /// Method to forward the specified request to the protocol processors.
    ///
    /// Arguments:
    ///     request: The request to forward.
    void forward_request_to_protocol_processors(const Request request);

    /// Method to handle an internal request.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_internal_request(const Request request);

    /// Method to handle an internal response.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_internal_response(const Request request);

    /// Method to handle an internal response end.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_internal_response_end(const Request request);
};

}  // namespace ostp::servercc::distributed

#endif
