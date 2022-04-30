#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <cstring>
#include <string>

#include "server.hh"
#include "../../util/logger/src/logger.hh"

/* public methods */

/**
 * @brief Construct a new Server object
 *
 * @param port port the server will listen on
 * @param mode the mode the server will run in
 */
restpp::Server::Server(unsigned short port, unsigned char mode)
{
    this->port = port;
    this->mode = mode;
    this->running = false;

    if (this->mode > 0)
    {
        this->mode = DEFAULT_MODE;
    }

    /* cofigure server address */

    this->server_addr = new struct sockaddr_in;
    std::memset(this->server_addr, 0, sizeof(struct sockaddr_in));
    this->server_addr->sin_family = AF_INET;
    this->server_addr->sin_port = htons(this->port);
    this->server_addr->sin_addr.s_addr = INADDR_ANY;

    /* create socket */

    if ((this->master_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        throw "Error creating socket";
    }

    /* configure socket */

    int optval = 1;
    if (setsockopt(this->master_socket, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&optval, sizeof(int)) < 0)
    {
        perror("setsockopt");
        throw "Error configuring socket";
    }

    /* bind */

    if (bind(this->master_socket, (struct sockaddr *)this->server_addr,
             sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind");
        throw "Error binding socket";
    }

    restpp::log_info("Server started on port " + std::to_string(this->port), "Server");
}

/**
 * @brief Destroy the restpp::Server::Server object
 */
restpp::Server::~Server()
{
    delete this->server_addr;
    this->server_addr = nullptr;
    restpp::log_info("Server destroyed", "Server");
}

/* private methods */

/**
 * @brief runs the server in iterative mode, listening for requests and processing
 * theme one at a time
 */
void restpp::Server::_run_in_iterative_mode()
{
    restpp::log_info("Server started in iterative mode", "Server");
    this->running = true;

    while (this->running)
    {
        struct sockaddr_in client_addr;
        auto client_addr_len = sizeof(struct sockaddr_in);
        auto slave_socket = accept(this->master_socket,
                                   (struct sockaddr *)&client_addr,
                                   (socklen_t *)&client_addr_len);

        if (slave_socket < 0)
        {
            perror("accept");
        }
    }
}
