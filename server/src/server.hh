#ifndef RESTPP_SERVER_HH
#define RESTPP_SERVER_HH

#ifndef PROTOCOL
#define PROTOCOL "HTTP/1.1"
#endif

#ifndef DEFAULT_PORT
#define DEFAULT_PORT 8080
#endif

#ifndef DEFAULT_MODE
#define DEFAULT_MODE 0
#endif

#ifndef REQUEST_READ_BUFFER_SIZE
#define REQUEST_READ_BUFFER_SIZE 1024
#endif

#ifndef REQUEST_READ_TIMEOUT
#define REQUEST_READ_TIMEOUT 5000
#endif

#include <string>

namespace restpp
{
    /**
     * @brief The Server class
     *
     * @details The Server class is a simple HTTP server.
     */
    class Server
    {
    public:
        const std::string protocol = PROTOCOL; // Protocol to use

        /**
         * @brief Construct a new Server object
         *
         * @param port port the server will listen on
         * @param mode the mode the server will run in
         */
        Server(unsigned short port, unsigned char mode);

        /**
         * @brief Construct a new Server object with default options
         *
         * @param port port the server will listen on
         */
        Server(unsigned short port) : Server(port, DEFAULT_MODE){};

        /**
         * @brief Construct a new Server object with default options
         */
        Server() : Server(DEFAULT_PORT, DEFAULT_MODE){};

        /**
         * @brief Destroy the Server object
         */
        ~Server();

        /**
         * @brief Starts the server
         */
        void run();

    private:
        unsigned short port;             // Port to listen on
        unsigned char mode;              // Mode to run in
        struct sockaddr_in *server_addr; // Server address
        int master_socket;               // Master socket
        bool running;                    // Running flag

        /**
         * @brief reads a request from a socket; if the request times out, the socket is closed
         * and the timeout flag is set to true
         *
         * @param slave_socket slave socket descriptor
         * @param request request string to read into
         * @return bool true if the request was read successfully, false if the request timed out
         */
        bool _read_request(int slave_socket, std::string &request);

        /**
         * @brief processes a request
         *
         * @param slave_socket
         * @param client_addr
         */
        void _process_request(int slave_socket, struct sockaddr_in client_addr);

        /**
         * @brief runs the server in iterative mode, listening for requests and processing
         * theme one at a time
         */
        void _run_in_iterative_mode();
    };
}

#endif