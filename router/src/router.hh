#ifndef ROUTER_HH
#define ROUTER_HH

#include <string>

#include "../handler/src/handler.hh"

#include "../../request/src/request.hh"
#include "../../response/src/response.hh"

/* restpp */

namespace restpp
{
    /**
     * @brief The router class
     * @details The router class is used to define handler for specific verbs and paths.
     */
    class Router
    {
    public:
        /**
         * @brief adds a handler for a specific verb and path
         *
         * @throws std::runtime_exception if a handler is already defined for the path
         *
         * @param method verb
         * @param path path
         * @param handler handler function
         */
        void handle(std::string method, std::string path, handler_t *handler);

        /**
         * @brief processses a request and sends a response
         * 
         * @param raw_request request string
         * @param socket socket fd
         */
        void process(std::string raw_request, int socket);

    private:
        Handler handler;
    };
}

#endif