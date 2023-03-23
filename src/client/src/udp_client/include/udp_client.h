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

#include "abstract_client.h"
#include "status_or.h"

using ostp::libcc::utils::StatusOr;

/// A UDP client.
namespace ostp::servercc::client {

class UdpClient : virtual public Client {
   private:
    /// The interface to use.
    const std::string interface;

   public:
    // Constructors

    /// Constructs a UDP client with the specified address, port, and interface
    ///
    /// Arguments:
    ///     address: The server's address.
    ///     port: The server's port.
    ///     interface: The interface to use.
    UdpClient(const std::string address, const uint16_t port, const std::string interface);

    // Methods

    /// Multicasts a request to the specified multicast group over the specified interface.
    ///
    /// Arguments:
    ///     request: The request to send.
    ///     interface: The interface to use.
    ///     multicast_group: The multicast group to use.
    ///
    /// Returns:
    ///     A StatusOr<int> indicating whether the request was sent successfully and the number of
    ///     bytes sent.
    StatusOr<int> multicast_request(const std::string request, const std::string interface,
                                    const std::string multicast_group);
};

}  // namespace ostp::servercc::client

#endif
