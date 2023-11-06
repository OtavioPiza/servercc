#ifndef SERVERCC_SERVER_UDP_H
#define SERVERCC_SERVER_UDP_H

#include <vector>

#include "absl/strings/string_view.h"
#include "server.h"

namespace ostp::servercc {

// A generic server to handle multiple protocols.
class UdpServer : virtual public Server {
   private:
    // The group address for the server to listen on for multicast.
    const absl::string_view groupAddress;

   public:
    // Constructor for the server.
    //
    // Arguments:
    //     port: The port the server will listen on.
    //     group_address: The group address the server will listen on.
    //     interfaces: The interfaces the server will listen on.
    //     default_processor: The default processor for the server.
    UdpServer(int16_t port, absl::string_view groupAddress,
              std::vector<absl::string_view> &interfaces, handler_t defaultHandler);

    // Destructor for the server.
    ~UdpServer();

    // See server.h for documentation.
    [[noreturn]] void run();
};

}  // namespace ostp::servercc

#endif
