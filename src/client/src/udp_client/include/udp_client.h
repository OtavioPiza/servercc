#ifndef SERVERCC_UDP_CLIENT_H
#define SERVERCC_UDP_CLIENT_H

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

/// A UDP client.
namespace ostp::servercc::client {

class UdpClient : virtual public Client {
   private:
    /// The client's socket address.
    struct sockaddr_in client_address;

    /// The interface to use.
    const std::string interface;

    /// The multicast group to join if any.
    const std::optional<std::string> multicast_group;

    /// The time-to-live for multicast packets.
    const uint8_t ttl;

   public:
    // Constructors

    /// Constructs a UDP client over the specified interface with the specified
    /// address, port, ttl.
    ///
    /// Arguments:
    ///     interface: The interface to use.
    ///     server_address: The server's address.
    ///     port: The server's port.
    ///     ttl: The time-to-live for multicast packets.
    UdpClient(const std::string interface, const std::string server_address, const uint16_t port,
              const uint8_t ttl);

    /// Constructs a UDP client over the specified interface with the specified
    /// address, port, ttl and multicast group.
    ///
    /// Arguments:
    ///     interface: The interface to use.
    ///     server_address: The server's address.
    ///     port: The server's port.
    ///     ttl: The time-to-live for multicast packets.
    ///     multicast_group: The multicast group to join.
    UdpClient(const std::string interface, const std::string server_address, const uint16_t port,
              const uint8_t ttl, const std::string multicast_group);

    // Client methods.

    /// See abstract_client.h
    StatusOr<bool> open_socket() override;

    /// See abstract_client.h
    StatusOr<bool> close_socket() override;
};

}  // namespace ostp::servercc::client

#endif
