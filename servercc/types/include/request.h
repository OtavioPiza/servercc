#ifndef SERVERCC_REQUEST_H
#define SERVERCC_REQUEST_H

#include <netinet/in.h>

#include <memory>
#include "message.h"

using std::string;

namespace ostp::servercc {

// A request to the server.
struct Request {
    // The file descriptor of the client.
    int fd;

    // The internet address of the client.
    sockaddr addr;

    // The message header.
    Message message;
};

}  // namespace ostp::servercc

#endif
