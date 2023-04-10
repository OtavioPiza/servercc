#ifndef SERVERCC_SERVER_UDP_H
#define SERVERCC_SERVER_UDP_H

#include <string>
#include <vector>

#include "abstract_server.h"

using ostp::servercc::server::Server;
using ostp::servercc::server::ServerMode;
using std::string;
using std::vector;

namespace ostp::servercc::server {
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
    ///     default_processor: The default processor for the server.
    UdpServer(int16_t port, ServerMode mode, const string group, const vector<string> interfaces,
              std::function<void(const Request)> default_processor);

    /// Constructor for the server with default mode.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    ///     group_address: The group address the server will listen on.
    ///     interfaces: The interfaces the server will listen on.
    ///     default_processor: The default processor for the server.
    UdpServer(int16_t port, const string group, const vector<string> interfaces,
              std::function<void(const Request)> default_processor);

    /// Constructs a server with default port and mode.
    ///
    /// Arguments:
    ///     group_address: The group address the server will listen on.
    ///     interfaces: The interfaces the server will listen on.
    ///     default_processor: The default processor for the server.
    UdpServer(const string group, const vector<string> interfaces,
              std::function<void(const Request)> default_processor);

    /// Destructor for the server.
    ~UdpServer();

    /// See server.h for documentation.
    [[noreturn]] void run();
};

}  // namespace ostp::servercc::server

#endif
