#ifndef SERVERCC_DISTRIBUTED_H
#define SERVERCC_DISTRIBUTED_H

#include <queue>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "connector.h"
#include "multicast_client.h"
#include "request.h"
#include "status_or.h"
#include "tcp_server.h"
#include "udp_server.h"

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
using std::string;
using std::thread;
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
    DistributedServer(const string interface_name, const string interface_ip, const string group,
                      const uint16_t port, function<void(const Request)> default_handler);

    // Methods

    /// Method to run the distributed server.
    void run();

    /// Method to add a new handler for the specified protocol.
    ///
    /// Arguments:
    ///     protocol: The protocol to add a handler for.
    ///     handler: The handler to add.
    StatusOr<void> add_handler(const string protocol, function<void(const Request)> handler);

    // Server utilities.

    /// Adds a log message to the log queue.
    ///
    /// Arguments:
    ///     status: The status of the log message.
    ///     message: The log message.
    void log(const Status status, const string message);

    /// Method to send a multicast message to all the servers.
    ///
    /// Arguments:
    ///     message: The message to send.
    StatusOr<bool> multicast_message(const string message);

    /// Method to send a message to a specific server.
    ///
    /// Arguments:
    ///     ip: The ip address of the server to send the message to.
    ///     message: The message to send.
    StatusOr<bool> send_message(const string ip, const string message);

    /// Method to send a message to a specific server.
    ///
    /// Arguments:
    ///     fd: The file descriptor of the server to send the message to.
    ///     message: The message to send.
    StatusOr<bool> send_message(const int fd, const string message);

   private:
    // Protocol handling datastructures.

    /// Trie to store the protocol commands supported by the distributed server.
    DefaultTrie<char, function<void(const Request)>> protocol_processors;

    // Logging datastructures.

    /// The semaphore to protect the log queue.
    binary_semaphore log_queue_semaphore;

    /// The queue of log messages.
    queue<pair<const Status, const string>> log_queue;

    // Server components.

    /// The TCP server to handle TCP requests.
    TcpServer tcp_server;

    /// The UDP server to handle multicast requests.
    UdpServer udp_server;

    /// The connector to handle inter-server requests.
    Connector connector;

    /// The multicast client to send multicast requests.
    MulticastClient multicast_client;

    // Peer server datastructures.
    unordered_set<string> peers;

    /// The list of peers connected to the distributed server.

    // Service threads.

    /// The thread to run the TCP server.
    thread tcp_server_thread;

    /// The thread to run the UDP server.
    thread udp_server_thread;

    /// The thread to run the logger service.
    thread logger_service_thread;

    // Server services.

    /// Method to run the TCP server.
    void run_tcp_server();

    /// Method to run the UDP server.
    void run_udp_server();

    /// Waits for a log message to be added to the log queue and prints it indefinitely.
    void run_logger_service();

    // Handlers.

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

    /// Method to handle a disconnect request.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_peer_disconnect(const string &ip);

    /// Method to forward the specified request to the protocol processors.
    ///
    /// Arguments:
    ///     request: The request to forward.
    void forward_request_to_protocol_processors(const Request request);
};

}  // namespace ostp::servercc::distributed

#endif
