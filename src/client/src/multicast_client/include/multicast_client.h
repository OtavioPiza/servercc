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

class MulticastClient : virtual public Client {
   private:
    /// The interface to use.
    const std::string interface;

    /// The time-to-live for multicast packets.
    const uint8_t ttl;

    /// The socket address of the multicast group.
    struct sockaddr_in client_address;

   public:
    // Constructors

    /// Constructs a Multicast client over the specified interface on the specified interface
    /// in the specified multicast group on the specified port.
    ///
    /// Arguments:
    ///     interface: The interface to use.
    ///     multicast_group: The multicast group to join.
    ///     port: The server's port.
    MulticastClient(const std::string interface, const std::string multicast_group,
                    const uint16_t port);

    /// Constructs a Multicast client over the specified interface on the specified interface
    /// in the specified multicast group on the specified port with the specified time-to-live.
    ///
    /// Arguments:
    ///     interface: The interface to use.
    ///     multicast_group: The multicast group to join.
    ///     port: The server's port.
    MulticastClient(const std::string interface, const std::string multicast_group,
                    const uint16_t port, const uint8_t ttl);

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
