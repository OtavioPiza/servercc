#ifndef SERVERCC_DISTRIBUTED_H
#define SERVERCC_DISTRIBUTED_H

#include <queue>
#include <semaphore>
#include <string>
#include <thread>
#include <unordered_map>
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
using std::pair;
using std::queue;
using std::string;
using std::thread;
using std::unordered_map;

namespace ostp::servercc::distributed {

/// A distributed server that can be used to create a distributed system.
class DistributedServer {
   private:
    // Server attributes.

    /// The name of the interface to use for the distributed server.
    const string interface_name;

    /// The interface ip address.
    const string interface_ip;

    /// The group ip address.
    const string group;

    /// The port to use for the distributed server.
    const uint16_t port;

    // Server components.

    /// The UDP server to handle multicast requests.
    UdpServer udp_server;

    /// The thread to run the UDP server.
    thread udp_server_thread;

    /// The TCP server to handle TCP requests.
    TcpServer tcp_server;

    /// The thread to run the TCP server.
    thread tcp_server_thread;

    /// The connector to handle inter-server requests.
    Connector connector;

    /// The multicast client to send multicast requests.
    MulticastClient multicast_client;

    // Data structures.

    /// Mapping of peer servers' ip addresses to their file descriptors.
    unordered_map<string, int> peer_ip_to_fd;

    /// Mapping of peer servers' file descriptors to their ip addresses.
    unordered_map<int, string> peer_fd_to_ip;

    /// Mapping of peer servers (identified by a file descriptor) to their
    /// supported commands.
    unordered_map<int, std::vector<string>> peer_fd_to_commands;

    /// Mapping of commands to the peer servers that support them (identified by
    /// a file descriptor).
    unordered_map<string, std::vector<int>> command_to_peer_fds;

    /// Trie to store the protocol commands supported by the distributed server.
    DefaultTrie<char, std::function<void(const Request)>> protocol_processors;

    // Logging datastructures.

    /// The semaphore to protect the log queue.
    binary_semaphore log_queue_semaphore;

    /// The queue of log messages.
    queue<pair<const Status, const string>> log_queue;

    /// The thread to run the logger service.
    thread logger_service_thread;

    // Server services.

    /// Method to run the TCP server.
    void run_tcp_server();

    /// Method to run the UDP server.
    void run_udp_server();

    // Logging services.

    /// Waits for a log message to be added to the log queue and prints it indefinitely.
    void run_logger_service();

    /// Adds a log message to the log queue.
    ///
    /// Arguments:
    ///     status: The status of the log message.
    ///     message: The log message.
    void log(const Status status, const string message);

    // Handlers.

    /// Method to handle a connect request.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_connect_request(const Request request);

    /// Method to handle a connect_ack request.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_connect_ack_request(const Request request);

    /// Method to handle a disconnect request.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_peer_disconnect(int fd);

    /// Method to forward the specified request to the protocol processors.
    ///
    /// Arguments:
    ///     request: The request to forward.
    void forward_request_to_protocol_processors(const Request request);

   public:
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
                      const uint16_t port, std::function<void(const Request)> default_handler);

    // Methods

    /// Method to run the distributed server.
    void run();

    /// Method to add a new handler for the specified protocol.
    ///
    /// Arguments:
    ///     protocol: The protocol to add a handler for.
    ///     handler: The handler to add.
    StatusOr<bool> add_handler(const string protocol, std::function<void(const Request)> handler);
};

}  // namespace ostp::servercc::distributed

#endif
