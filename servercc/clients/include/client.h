#ifndef SERVERCC_CLIENT_H
#define SERVERCC_CLIENT_H

#include <inttypes.h>
#include <netdb.h>

#include <cstring>
#include <memory>
#include <string>

#include "../../servercc/types/types.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace ostp::servercc {

// A generic client to handle multiple transports layer protocols.
class Client {
   private:
    // The server's address.
    const absl::string_view serverAddress;

    // The server's port.
    const uint16_t port;

   protected:
    // Instance variables

    // The client's socket file descriptor.
    int clientFd;

    // Whether the socket is open.
    bool isSocketOpen;

    // The clients addr.
    sockaddr clientAddr;

    // Constructors

    // Constructs a client with the specified address and port.
    //
    // Arguments:
    //     serverAddress: The server's address.
    //     port: The server's port.
    Client(const absl::string_view serverAddress, const uint16_t port)
        : serverAddress(serverAddress), port(port), isSocketOpen(false), clientFd(-1) {
        memset(&clientAddr, 0, sizeof(clientAddr));
    }

    // Constructs a client with the address, port, and sockaddr.
    //
    // Arguments:
    //     serverAddress: The server's address.
    //     port: The server's port.
    //     clientAddr: The client's addr.
    Client(const absl::string_view serverAddress, const uint16_t port, sockaddr clientAddr)
        : serverAddress(serverAddress),
          port(port),
          isSocketOpen(false),
          clientFd(-1),
          clientAddr(clientAddr) {}

   public:
    // Getters

    // Gets the server's address.
    //
    // Returns:
    //     The server's address.
    virtual absl::string_view getAddress() { return serverAddress; }

    // Gets the server's port.
    //
    // Returns:
    //     The server's port.
    virtual uint16_t getPort() { return port; }

    // Gets the client's socket file descriptor.
    //
    // Returns:
    //     The client's socket file descriptor.
    virtual int getClientFd() { return clientFd; }

    // Checks whether the socket is open.
    //
    // Returns:
    //     Whether the socket is open.
    virtual bool isOpen() { return isSocketOpen; }

    // Gets the client's addr.
    //
    // Returns:
    //     A shared pointer to the client's addr.
    virtual sockaddr getClientAddr() { return clientAddr; }

    // Virtual Methods

    // Opens the client's socket.
    //
    // Returns:
    //     A status indicating whether the socket was opened successfully.
    virtual absl::Status openSocket() = 0;

    // Closes the client's socket.
    //
    // Returns:
    //     A status indicating whether the socket was closed successfully.
    virtual absl::Status closeSocket() = 0;

    // Sends a message to the server.
    //
    // Arguments:
    //     message: The message to send.
    //
    // Returns:
    //     A status indicating whether the message was sent successfully and the
    //     number of bytes sent.
    virtual absl::Status sendMessage(const Message& message) = 0;

    // Blocks until a message is received from the server.
    //
    // Returns:
    //     A status indicating whether a message was received successfully and
    //     the message received.
    virtual std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage() = 0;
};

}  // namespace ostp::servercc

#endif
