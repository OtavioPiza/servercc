#ifndef SERVERCC_REQUEST_H
#define SERVERCC_REQUEST_H

#include <netinet/in.h>

#include <memory>

using std::string;

namespace ostp::servercc {

/// A request to the server.
struct Request {
    /// The file descriptor of the client.
    int fd;

    /// The internet address of the client.
    sockaddr addr;

    /// The protocol of the request.
    string protocol;

    /// The request body.
    string data;
};

}  // namespace ostp::servercc

#endif
