#ifndef REQUEST_HH
#define REQUEST_HH

#include <string>
#include <unordered_map>
#include <vector>

namespace restpp
{
    class Request
    {
    public:
        int socket;                                           // socket fd
        std::string raw_request;                              // raw request string
        std::string method;                                   // request method
        std::string version;                                  // request version
        std::vector<std::string> path;                        // request path
        std::unordered_map<std::string, std::string> params;  // request params
        std::unordered_map<std::string, std::string> headers; // request headers
        std::unordered_map<std::string, std::string> body;    // request body

        /**
         * @brief Construct a new Request object
         *
         * @param raw_request The raw request string
         * @param socket socket fd
         */
        Request(std::string raw_request, int socket);

    private:
        /**
         * @brief parses the raw request string
         */
        void parse_request();
    };
}

#endif