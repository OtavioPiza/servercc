#ifndef SERVERCC_TCP_CLIENT_H
#define SERVERCC_TCP_CLIENT_H

#include <memory>

#include "../../servercc/types/types.h"
#include "absl/strings/string_view.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
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
    TcpClient(const int socket, const absl::string_view server_address, const uint16_t port);

    // Constructs a TCP client from the specified socket.
    //
    // Arguments:
    //     socket: The socket.
    //     server_address: The server's address.
    //     port: The server's port.
    //     client_addr: The client's addr.
    TcpClient(const int socket, const absl::string_view server_address, const uint16_t port,
              sockaddr client_addr);

    // Client methods.

    // See abstract_client.h
    absl::Status openSocket() override;

    // See abstract_client.h
    absl::Status closeSocket() override;

    // See abstract_client.h
    absl::Status sendMessage(const Message& message) override;

    // See abstract_client.h
    std::pair<absl::Status, std::unique_ptr<Message>> receiveMessage() override;
};

}  // namespace ostp::servercc

#endif
