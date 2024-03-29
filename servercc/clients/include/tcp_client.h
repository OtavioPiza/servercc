#ifndef SERVERCC_TCP_CLIENT_H
#define SERVERCC_TCP_CLIENT_H

#include "client.h"

namespace ostp::servercc {

// A TCP client.
class TcpClient : virtual public Client {
   public:
    // Constructors

    // Constructs a TPC client from the specified server address and port.
    //
    // Arguments:
    //     server_address: The server's address.
    //     port: The server's port.
    TcpClient(const absl::string_view server_address, const uint16_t port);

    // Constructs a TCP client from the specified socket.
    //
    // Arguments:
    //     socket: The socket.
    //     server_address: The server's address.
    //     port: The server's port.
    //     client_addr: The client's addr.
    TcpClient(const int socket, const absl::string_view server_address, const uint16_t port,
              sockaddr client_addr);

    // Destructor that closes the socket.
    ~TcpClient();

    // Client methods.

    // See abstract_client.h
    absl::Status openSocket() final;

    // See abstract_client.h
    void closeSocket() final;

    // See abstract_client.h
    absl::Status sendMessage(std::unique_ptr<Message> message) final;

    // See abstract_client.h
    std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage() final;
};

}  // namespace ostp::servercc

#endif
