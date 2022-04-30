#ifndef RESTPP_SERVER_HH
#define RESTPP_SERVER_HH

#ifndef PROTOCOL
#define PROTOCOL "HTTP/1.1"
#endif

#ifndef DEFAULT_MODE
#define DEFAULT_MODE 0
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
        Server(unsigned short port) : Server(port, DEFAULT_MODE) {};

        /**
         * @brief Destroy the Server object
         */
        ~Server();

    private:
        unsigned short port;             // Port to listen on
        unsigned char mode;              // Mode to run in
        struct sockaddr_in *server_addr; // Server address
        int master_socket;               // Master socket
        bool running;                    // Running flag

        /**
         * @brief runs the server in iterative mode, listening for requests and processing
         * theme one at a time
         */
        void _run_in_iterative_mode();
    };
}

#endif