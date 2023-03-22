#ifndef SERVERCC_ABSTRACT_SERVER_H
#define SERVERCC_ABSTRACT_SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <functional>
#include <optional>

#include "server_mode.h"
#include "server_request.h"

using ostp::severcc::server::Request;
using ostp::severcc::server::ServerMode;

namespace ostp::severcc::server {
/// A generic server to handle multiple protocols.
class Server {
   public:
    /// Runs the server.
    ///
    /// Returns:
    ///     A status code indicating the success of the operation and the port the server is
    ///     listening on.
    [[noreturn]] virtual void run() = 0;

    /// Registers or updates a protocol with a processor.
    ///
    /// Arguments:
    ///     protocol: The protocol to register.
    ///     processor: The processor to handle the protocol.
    virtual void register_processor(const std::string protocol,
                                    std::function<void(const Request)> processor) = 0;

    /// Registers the default processor
    ///
    /// Arguments:
    ///     processor: The default processor to handle unregistered protocols.
    virtual void register_default_processor(std::function<void(const Request)> processor) = 0;
};

}  // namespace ostp::severcc::server

#endif