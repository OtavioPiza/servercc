#ifndef SERVERCC_SERVER_UDP_H
#define SERVERCC_SERVER_UDP_H

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
class UdpServer : virtual public Server {
   private:
    /// Associates a communication protocol with a processor id that handles it.
    DefaultTrie<char, std::function<void(const Request)>> protocol_processors;

    const int16_t port;
    const ServerMode mode;
    struct addrinfo *server_addr;
    int server_socket_fd;

   public:
    /// Constructor for the server.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    ///     mode: The mode the server will run in.
    UdpServer(int16_t port, ServerMode mode);

    /// Constructs a server with default mode.
    ///
    /// Arguments:
    ///     port: The port the server will listen on.
    UdpServer(int16_t port);

    /// Constructs a server with default port and mode.
    UdpServer();

    /// Destructor for the server.
    ~UdpServer();

    // See server.h for documentation.
    [[noreturn]] void run();

    // See server.h for documentation.
    void register_processor(const std::string protocol,
                            std::function<void(const Request)> processor);

    // See server.h for documentation.
    void register_default_processor(std::function<void(const Request)> processor);
};

}  // namespace ostp::severcc::server

#endif
