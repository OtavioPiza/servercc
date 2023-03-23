#ifndef SERVERCC_SERVER_REQUEST_H
#define SERVERCC_SERVER_REQUEST_H

#include <memory>

namespace ostp::servercc::server {
/// A request to the server.
struct Request {
    /// The file descriptor for the server socket.
    const int server_fd;

    /// The file descriptor for the client socket.
    const std::optional<const int> client_fd;

    /// The file descriptor for the client socket.
    const std::optional<const std::string> client_addr;

    /// The protocol of the request.
    const std::string protocol;

    /// The raw data of the request.
    const std::string data;
};
}  // namespace ostp::severcc::server

#endif
