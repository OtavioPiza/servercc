#ifndef SERVERCC_SERVER_H
#define SERVERCC_SERVER_H

#include <stdint.h>
#include <netinet/in.h>

#include "default_trie.h"
#include "server_mode.h"
#include "status_or.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::libcc::utils::StatusOr;
using ostp::severcc::ServerMode;

namespace ostp::severcc
{
    /// A generic server to handle multiple protocols.
    class Server
    {
    public:
        /// Constructor for the server.
        ///
        /// Arguments:
        ///     port: The port the server will listen on.
        ///     mode: The mode the server will run in.
        Server(int16_t port, ServerMode mode);

        /// Constructs a server with default mode.
        ///
        /// Arguments:
        ///     port: The port the server will listen on.
        Server(int16_t port);

        /// Constructs a server with default port and mode.
        Server();

        /// Destructor for the server.
        ~Server();

        /// Runs the server.
        ///
        /// Returns:
        ///     A status code indicating the success of the operation and the port the server is
        ///     listening on.
        [[nodiscard]] StatusOr<const int16_t> run();

    private:
        /// Associates a communication protocol with a processor id that handles it.
        DefaultTrie<char /* protocol */, int /* processor id */> protocol_processors;

        /// \todo Add a list of processors.
        const int16_t port;
        const ServerMode mode;
        struct sockaddr_in *server_addr;
        int server_socket;
    };

} // namespace ostp::severcc

#endif
