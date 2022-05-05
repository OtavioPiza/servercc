#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <cstring>
#include <string>

#include <future>
#include <thread>
#include <chrono>

#include <stdio.h>

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

    /* listen */

    if (listen(this->master_socket, SOMAXCONN) < 0)
    {
        perror("listen");
        throw "Error listening on socket";
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

/**
 * @brief Starts the server
 */
void restpp::Server::run()
{
    this->running = true;

    switch (this->mode)
    {
    case ITERATIVE_MODE:
        this->_run_in_iterative_mode();
        break;

    case THREAD_MODE:
        this->_run_in_thread_mode();
        break;

    default:
        restpp::log_error("Invalid mode", "Server");
        break;
    }
}

/* private methods */

/**
 * @brief reads a request from a socket; if the request times out, the socket is closed
 * and the timeout flag is set to true
 *
 * @param slave_socket slave socket descriptor
 * @param request request string to read into
 * @return bool true if the request was read successfully, false if the request timed out
 */
bool restpp::Server::_read_request(int slave_socket, std::string &request)
{
    auto read_request = [&request, slave_socket]()
    {
        char buffer[REQUEST_READ_BUFFER_SIZE];
        int bytes_read;

        while ((bytes_read = recv(slave_socket, buffer, REQUEST_READ_BUFFER_SIZE, 0)) > 0)
        {
            request += std::string(buffer, bytes_read);

            /* check if request is terminated */

            if (request.length() >= 4 &&
                request[request.length() - 4] == '\r' && request[request.length() - 3] == '\n' &&
                request[request.length() - 2] == '\r' && request[request.length() - 1] == '\n')
            {
                break;
            }
        }
    };

    auto read_request_future = std::async(std::launch::async, read_request);
    auto status = read_request_future.wait_for(std::chrono::milliseconds(REQUEST_READ_TIMEOUT));

    if (status == std::future_status::timeout)
    {
        std::string time_out_response = "HTTP/1.1 408 Request Timeout\r\n\r\n";
        send(slave_socket, time_out_response.c_str(), time_out_response.length(), 0);
        shutdown(slave_socket, SHUT_RDWR);
        close(slave_socket);
        read_request_future.get();
        return false;
    }
    read_request_future.get();
    return true;
}

/**
 * @brief processes a request
 *
 * @param slave_socket
 * @param client_addr
 */
void restpp::Server::_process_request(int slave_socket, struct sockaddr_in client_addr)
{
    /* read request */

    std::string request;
    if (!this->_read_request(slave_socket, request))
    {
        restpp::log_warn("Request from " + std::string(inet_ntoa(client_addr.sin_addr)) + " timed out", "Server");
    }
    else
    {
        close(slave_socket);
    }
}

/**
 * @brief runs the server in iterative mode, listening for requests and processing
 * them one at a time
 */
void restpp::Server::_run_in_iterative_mode()
{
    restpp::log_info("Server started in iterative mode", "Server");

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

        this->_process_request(slave_socket, client_addr);
    }
}

/**
 * @brief runs the server in thread mode, listening for requests and processing
 * them in a separate thread
 */
void restpp::Server::_run_in_thread_mode()
{
    restpp::log_info("Server started in thread mode", "Server");

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

        auto process_request_lambda = [this, slave_socket, client_addr]()
        {
            this->_process_request(slave_socket, client_addr);
        };

        std::thread process_request_thread(process_request_lambda);

        process_request_thread.detach();
    }
}