#ifndef SERVERCC_REQUEST_H
#define SERVERCC_REQUEST_H

#include <memory>

namespace ostp::severcc
{
    /// A request to the server.
    struct Request
    {
        /// The file descriptor for the client socket.
        const int client_fd;

        /// The protocol of the request.
        const std::string protocol;

        /// The raw data of the request.
        const std::string data;
    };
}

#endif