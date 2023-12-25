#ifndef SERVERCC_UDP_REQUEST_H
#define SERVERCC_UDP_REQUEST_H

#include <bits/stdc++.h>

#include "types.h"

namespace ostp::servercc {

// UDP request class for the server extending the request class.
class UdpRequest : public virtual Request {
   private:
    // The address of the client.
    const sockaddr clientAddr;

    // The protocol of the request.
    const protocol_t protocol;

    // The current message.
    std::unique_ptr<Message> message;

   public:
    // Constructor for the request.
    //
    // Arguments:
    //     fd: The file descriptor of the client.
    //     clientAddr: The address of the client.
    //     message: The message to initialize the request with.
    UdpRequest(const sockaddr& clientAddr, std::unique_ptr<Message> message);

    // Destructor for the request. Closes the socket by calling terminate().
    ~UdpRequest() final;

    // See request.h for documentation.
    sockaddr getAddr() final;

    // See request.h for documentation.
    protocol_t getProtocol() final;

    // See request.h for documentation.
    std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage() final;

    // See request.h for documentation.
    std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage(int timeout) final;

    // See request.h for documentation.
    absl::Status sendMessage(std::unique_ptr<Message> message) final;

    // Terminates the request by closing the socket.
    void terminate() final;
};

}  // namespace ostp::servercc

#endif
