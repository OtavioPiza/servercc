#ifndef SERVERCC_REQUEST_H
#define SERVERCC_REQUEST_H

#include <netinet/in.h>

#include <memory>

#include "message.h"

namespace ostp::servercc {

// A request to the server.
struct Request {
    // The file descriptor of the client.
    int fd;

    // Channel to read and write messages.

    // The internet address of the client.
    sockaddr addr;

    // The message header.
    std::unique_ptr<Message> message;
};

// The type of a protocol handler.
typedef std::function<void(std::unique_ptr<ostp::servercc::Request>)> handler_t;

}  // namespace ostp::servercc

#endif
