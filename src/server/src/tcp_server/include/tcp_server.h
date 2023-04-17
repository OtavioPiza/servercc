#ifndef SERVERCC_SERVER_TCP_H
#define SERVERCC_SERVER_TCP_H

#include "abstract_server.h"

using ostp::servercc::server::Server;
using ostp::servercc::server::ServerMode;
using std::function;

namespace ostp::servercc::server {
/// A generic server to handle multiple protocols.
class TcpServer : virtual public Server {
   public:
    /// Constructor for the server.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    ///     mode: The mode the server will run in.
    ///     default_processor: The default processor for the server.
    TcpServer(int16_t port, ServerMode mode, function<void(const Request)> default_processor);

    /// Constructs a server with default mode.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    TcpServer(int16_t port, function<void(const Request)> default_processor);

    /// Constructs a server with default port and mode.
    TcpServer(function<void(const Request)> default_processor);

    /// Destructor for the server.
    ~TcpServer();

    /// See server.h for documentation.
    [[noreturn]] void run();
};

}  // namespace ostp::servercc::server

#endif
