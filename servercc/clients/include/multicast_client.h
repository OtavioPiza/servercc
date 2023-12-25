#ifndef SERVERCC_UDP_CLIENT_H
#define SERVERCC_UDP_CLIENT_H

#include "client.h"

// A UDP client.
namespace ostp::servercc {

class MulticastClient : virtual public Client {
   private:
    // The interface to use.
    const absl::string_view interface;

    // The time-to-live for multicast packets.
    const uint8_t ttl;

   public:
    // Constructors

    // Constructs a Multicast client over the specified interface on the specified interface
    // in the specified multicast group on the specified port with the specified time-to-live.
    //
    // Arguments:
    //     interface: The interface to use.
    //     multicast_group: The multicast group to join.
    //     port: The server's port.
    MulticastClient(const absl::string_view interface, const absl::string_view multicastGroup,
                    const uint16_t port, const uint8_t ttl);

    // Destructor that closes the socket.
    ~MulticastClient();

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
