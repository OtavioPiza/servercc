#include <cstring>
#include <stdio.h>
#include <vector>

#include "default_trie.h"
#include "defaults.h"
#include "server.h"
#include "logger.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::libcc::utils::log_info;
using ostp::libcc::utils::Status;
using ostp::libcc::utils::StatusOr;
using ostp::severcc::Server;
using ostp::severcc::ServerMode;

// See server.h for documentation.
Server::Server(int16_t port, ServerMode mode)
    : protocol_processors(nullptr), port(port), mode(mode)
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

// See server.h for documentation.
[[noreturn]] void Server::run()
{
    int client_socket;              // F.D. for the client socket.
    struct sockaddr_in client_addr; // Address of the client.
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    while (true)
    {
        // Try to accept a connection.
        if ((client_socket = accept(this->server_socket,
                                    (struct sockaddr *)&client_addr,
                                    &client_addr_len)) < 0)
        {
            perror("accept");
            continue;
        }
        std::vector<char> buffer(SERVERCC_BUFFER_SIZE);

        // Try to read from the client.
        if ((recv(client_socket, &buffer[0], buffer.size(), 0)) < 0)
        {
            perror("recv");
            close(client_socket);
            continue;
        }

        // Find the first whitespace character.
        int i;
        for (i = 0; i < buffer.size() && isspace(buffer[i]); i++)
            ;

        // Look for the processor that handles the provided protocol and send the request to it.
        auto processor = this->protocol_processors.get(&buffer[0], i);
        if (processor)
        {
            processor(
                Request{
                    client_socket,
                    std::string(buffer.begin(), buffer.begin() + i),
                    std::string(buffer.begin(), buffer.end())});
        }
        else
        {
            close(client_socket);
        }
    }
}

// See server.h for documentation.
void Server::register_processor(
    std::string protocol,
    std::function<void(const Request)> processor)
{
    this->protocol_processors.insert(protocol.c_str(), protocol.length(), processor);
}

// See server.h for documentation.
void Server::register_default_processor(std::function<void(const Request)> processor)
{
    this->protocol_processors.update_default_return(processor);
}
