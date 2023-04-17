#ifndef SERVERCC_TCP_CLIENT_H
#define SERVERCC_TCP_CLIENT_H

#include <memory>

#include "abstract_client.h"
#include "status_or.h"

using ostp::libcc::utils::StatusOr;
using std::shared_ptr;
using std::string;

namespace ostp::servercc::client {

/// A TCP client.
class TcpClient : virtual public Client {
   public:
    // Constructors

    /// Constructs a TPC client from the specified server address and port.
    ///
    /// Arguments:
    ///     server_address: The server's address.
    ///     port: The server's port.
    TcpClient(const string server_address, const uint16_t port);

    /// Constructs a TCP client from the specified socket.
    ///
    /// Arguments:
    ///     socket: The socket.
    ///     server_address: The server's address.
    ///     port: The server's port.
    TcpClient(const int socket, const string server_address, const uint16_t port);

    /// Constructs a TCP client from the specified socket.
    ///
    /// Arguments:
    ///     socket: The socket.
    ///     server_address: The server's address.
    ///     port: The server's port.
    ///     client_addr: The client's addr.
    TcpClient(const int socket, const string server_address, const uint16_t port,
              sockaddr client_addr);

    // Client methods.

    /// See abstract_client.h
    StatusOr<bool> open_socket() override;

    /// See abstract_client.h
    StatusOr<bool> close_socket() override;

    /// See abstract_client.h
    StatusOr<int> send_message(const string &message) override;

    /// See abstract_client.h
    StatusOr<string> receive_message() override;
};

}  // namespace ostp::servercc::client

#endif
