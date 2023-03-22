#ifndef SERVERCC_SERVER_TCP_H
#define SERVERCC_SERVER_TCP_H

#include "abstract_server.h"
#include "default_trie.h"
#include "status_or.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::libcc::utils::StatusOr;
using ostp::severcc::server::Request;
using ostp::severcc::server::Server;
using ostp::severcc::server::ServerMode;

namespace ostp::severcc::server {
/// A generic server to handle multiple protocols.
class TcpServer : virtual public Server {
   public:
    /// Constructor for the server.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    ///     mode: The mode the server will run in.
    TcpServer(int16_t port, ServerMode mode);

    /// Constructs a server with default mode.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    TcpServer(int16_t port);

    /// Constructs a server with default port and mode.
    TcpServer();

    /// Destructor for the server.
    ~TcpServer();

    // See server.h for documentation.
    [[noreturn]] void run();
};

}  // namespace ostp::severcc::server

#endif
