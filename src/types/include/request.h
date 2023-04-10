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
    std::shared_ptr<struct sockaddr> addr;

    /// The protocol of the request.
    std::string protocol;

    /// The request body.
    std::string data;

    /// Constructs an empty request.
    Request() : addr(std::make_shared<struct sockaddr>()){};

    /// Constructs a request with the specified file descriptor and address.
    ///
    /// Arguments:
    ///     fd: The file descriptor of the client.
    ///     addr: The internet address of the client.
    Request(const int fd, const std::shared_ptr<struct sockaddr> addr) : fd(fd), addr(addr){};
};

}  // namespace ostp::servercc

#endif
