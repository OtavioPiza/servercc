#include <cstring>
#include <stdio.h>

#include "../include/server.h"
#include "defaults.h"
#include "default_trie.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::severcc::Server;
using ostp::severcc::ServerMode;

// See server.h for documentation.
Server::Server(int16_t port, ServerMode mode)
    : protocol_processors(-1), port(port), mode(mode)
{
    this->server_addr = new struct sockaddr_in;
    memset(this->server_addr, 0, sizeof(struct sockaddr_in));

    // Configure server address.
    this->server_addr->sin_family = AF_INET;
    this->server_addr->sin_port = htons(this->port);
    this->server_addr->sin_addr.s_addr = INADDR_ANY;

    // Create socket.
    if ((this->server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        throw "Error creating socket";
    }

    // Configure socket.
    int optval = 1;
    if (setsockopt(this->server_socket, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&optval, sizeof(int)) < 0)
    {
        perror("setsockopt");
        throw "Error configuring socket";
    }

    // Bind.
    if (bind(this->server_socket, (struct sockaddr *)this->server_addr,
             sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        throw "Error binding socket";
    }

    // Listen.
    if (listen(this->server_socket, SOMAXCONN) < 0)
    {
        perror("listen");
        throw "Error listening on socket";
    }

}

// See server.h for documentation.
Server::Server(int16_t port) : Server(port, SERVERCC_DEFAULT_MODE) {}

// See server.h for documentation.
Server::Server() : Server(SERVERCC_DEFAULT_PORT, SERVERCC_DEFAULT_MODE) {}

// See server.h for documentation.
Server::~Server()
{
    delete this->server_addr;
}

