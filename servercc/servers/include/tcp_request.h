#ifndef SERVERCC_TCP_REQUEST_H
#define SERVERCC_TCP_REQUEST_H

#include "types.h"

namespace ostp::servercc {

// TCP request class for the server extending the request class.
class TcpRequest : public virtual Request {
   private:
    // The file descriptor of the client.
    const int clientSocketFd;

    // The address of the client.
    const sockaddr clientAddr;

    // The protocol of the request.
    const protocol_t protocol;

    // The current message.
    std::unique_ptr<Message> message;

    // Whether or not the request should be kept alive.
    bool keepAlive = false;

   public:
    // Constructor for the request.
    //
    // Arguments:
    //     fd: The file descriptor of the client.
    //     clientAddr: The address of the client.
    //     message: The message to initialize the request with.
    TcpRequest(const int fd, const sockaddr& clientAddr, std::unique_ptr<Message> message);

    // Destructor for the request. Closes the socket by calling terminate().
    ~TcpRequest() final;

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

    // Keeps the request alive by not closing the socket.
    //
    // Returns:
    //    The file descriptor of the socket.
    int setKeepAlive();
};

}  // namespace ostp::servercc

#endif
