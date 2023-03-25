#ifndef SERVERCC_TCP_CLIENT_H
#define SERVERCC_TCP_CLIENT_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <optional>

#include "abstract_client.h"
#include "status_or.h"

using ostp::libcc::utils::StatusOr;

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
    TcpClient(const std::string server_address, const uint16_t port);

    /// Constructs a TCP client from the specified socket.
    ///
    /// Arguments:
    ///     socket: The socket.
    ///     server_address: The server's address.
    ///     port: The server's port.
    TcpClient(const int socket, const std::string server_address, const uint16_t port);

    // Client methods.

    /// See abstract_client.h
    StatusOr<bool> open_socket() override;

    /// See abstract_client.h
    StatusOr<bool> close_socket() override;

    /// See abstract_client.h
    StatusOr<int> send_message(const std::string &message) override;

    /// See abstract_client.h
    StatusOr<std::string> receive_message() override;
};

}  // namespace ostp::servercc::client

#endif
