#ifndef SERVERCC_SERVER_TCP_H
#define SERVERCC_SERVER_TCP_H

#include "server.h"

namespace ostp::servercc {

// A TCP server.
class TcpServer : virtual public Server {
   public:
    // Constructor for the server.
    //
    // Arguments:
    //     port: The port the server will listen on.
    //     default_processor: The default processor for the server.
    TcpServer(int16_t port, handler_t defaultHandler);

    // Destructor for the server.
    ~TcpServer();

    // See server.h for documentation.
    [[noreturn]] void run();
};

}  // namespace ostp::servercc

#endif
