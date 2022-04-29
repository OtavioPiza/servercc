#include <stdio.h>
#include "server.hh"

/**
 * @brief Construct a new restpp::Server::Server object
 * 
 * @param port port to listen on
 */
restpp::Server::Server(int port)
{
    this->port = port;
    fprintf(stderr, "Server listening on port %d\n", port);
}