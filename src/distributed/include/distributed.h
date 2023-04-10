#ifndef SERVERCC_DISTRIBUTED_H
#define SERVERCC_DISTRIBUTED_H

#include <string>
#include <unordered_map>
#include <vector>

#include "connector.h"
#include "multicast_client.h"
#include "request.h"
#include "tcp_server.h"
#include "udp_server.h"

using ostp::servercc::Request;
using ostp::servercc::client::MulticastClient;
using ostp::servercc::connector::Connector;
using ostp::servercc::server::TcpServer;
using ostp::servercc::server::UdpServer;
using std::string;
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

    /// The TCP server to handle TCP requests.
    TcpServer tcp_server;

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

    /// Method to forward the specified request to the protocol processors.
    ///
    /// Arguments:
    ///     request: The request to forward.
    void forward_request_to_protocol_processors(const Request request);

    /// Method to handle a disconnect request.
    ///
    /// Arguments:
    ///     request: The request to handle.
    void handle_peer_disconnect(int fd);

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
                      const uint16_t port,
                      std::function<void(const Request)> default_handler);

    // Methods

    void run();
};

}  // namespace ostp::servercc::distributed

#endif
