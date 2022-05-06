#ifndef ROUTER_HH
#define ROUTER_HH

#include <string>
#include <functional>

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
        void handle(std::string method, std::string path, std::function<void(Request &, Response &)> handler);

    private:
        Handler handler;
    };
}

#endif