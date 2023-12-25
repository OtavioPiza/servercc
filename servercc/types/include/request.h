#ifndef SERVERCC_REQUEST_H
#define SERVERCC_REQUEST_H

#include <bits/stdc++.h>
#include <netinet/in.h>

#include <memory>

#include "absl/status/status.h"
#include "message.h"

namespace ostp::servercc {

// A request to the server.
class Request {
   public:
    // Destructor of the request. Should be called when the request is no longer needed.
    virtual ~Request() = default;

    // Returns the socket address of the client.
    //
    // Returns:
    //     The socket address of the client.
    virtual sockaddr getAddr() = 0;

    // Returns the protocol of the request.
    //
    // Returns:
    //     The protocol of the request.
    virtual protocol_t getProtocol() = 0;

    // Receives a message from the client.
    //
    // Returns:
    //     A pair of the status and the message.
    virtual std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage() = 0;

    // Receives a message from the client with the specified timeout.
    //
    // Arguments:
    //     timeout: The timeout in milliseconds.
    // Returns:
    //     A pair of the status and the message.
    virtual std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage(int timeout) = 0;

    // Sends a message to the client.
    //
    // Arguments:
    //     message: The message to send.
    // Returns:
    //     The status of the operation.
    virtual absl::Status sendMessage(std::unique_ptr<Message> message) = 0;

    // Terminates the request.
    virtual void terminate() = 0;
};

// The type of a protocol handler.
typedef std::function<absl::Status(std::unique_ptr<ostp::servercc::Request>)> handler_t;

}  // namespace ostp::servercc

#endif
