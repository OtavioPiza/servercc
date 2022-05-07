#ifndef HANDLER_HH
#define HANDLER_HH

#include <vector>
#include <string>
#include <utility>
#include <unordered_map>

#include "../../../request/src/request.hh"
#include "../../../response/src/response.hh"

/* verbs definitions */

#ifndef GET
#define GET "GET"
#endif

#ifndef POST
#define POST "POST"
#endif

#ifndef PUT
#define PUT "PUT"
#endif

#ifndef PATCH
#define PATCH "PATCH"
#endif

#ifndef DELETE
#define DELETE "DELETE"
#endif

#ifndef OPTIONS
#define OPTIONS "OPTIONS"
#endif

/* restpp */

namespace restpp
{
    typedef void handler_t(restpp::Request &, restpp::Response &);

    /**
     * @brief The Handler class
     * @details The Handler class is a simple handler for the Router. It is used to define actions
     * for specific verbs. It is also used to store handlers for paths lower in the hierarchy.
     */
    class Handler
    {
    public:
        std::unordered_map<std::string, handler_t*> handlers; 
        std::vector<std::pair<std::string, Handler>> next; // Next handlers

        /**
         * @brief Construct a new Handler object
         */
        Handler();
    };
}

#endif