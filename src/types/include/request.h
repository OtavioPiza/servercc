#ifndef SERVERCC_REQUEST_H
#define SERVERCC_REQUEST_H

#include <netinet/in.h>

#include <memory>

namespace ostp::servercc {

/// A request to the server.
struct Request {
    /// The file descriptor of the client.
    int fd;

    /// The internet address of the client.
    std::unique_ptr<struct sockaddr> addr;

    /// The protocol of the request.
    std::string protocol;

    /// The request body.
    std::string data;

    /// Constructs a request.
    Request() { addr = std::make_unique<struct sockaddr>(); }
};

}  // namespace ostp::servercc

#endif
