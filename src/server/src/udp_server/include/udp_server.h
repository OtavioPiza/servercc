#ifndef SERVERCC_SERVER_UDP_H
#define SERVERCC_SERVER_UDP_H

#include <string>
#include <vector>

#include "abstract_server.h"
#include "default_trie.h"
#include "status_or.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::libcc::utils::StatusOr;
using ostp::severcc::server::Request;
using ostp::severcc::server::Server;
using ostp::severcc::server::ServerMode;
using std::string;
using std::vector;

namespace ostp::severcc::server {
/// A generic server to handle multiple protocols.
class UdpServer : virtual public Server {
   private:
    /// The group address for the server to listen on for multicast.
    const string group;

   public:
    /// Constructor for the server.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    ///     mode: The mode the server will run in.
    ///     group_address: The group address the server will listen on.
    ///     interfaces: The interfaces the server will listen on.
    UdpServer(int16_t port, ServerMode mode, const string group, const vector<string> interfaces);

    /// Constructor for the server with default mode.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    ///     group_address: The group address the server will listen on.
    ///     interfaces: The interfaces the server will listen on.
    UdpServer(int16_t port, const string group, const vector<string> interfaces);

    /// Constructs a server with default port and mode.
    ///
    /// Arguments:
    ///     group_address: The group address the server will listen on.
    ///     interfaces: The interfaces the server will listen on.
    UdpServer(const string group, const vector<string> interfaces);

    /// Destructor for the server.
    ~UdpServer();

    // See server.h for documentation.
    [[noreturn]] void run();
};

}  // namespace ostp::severcc::server

#endif
