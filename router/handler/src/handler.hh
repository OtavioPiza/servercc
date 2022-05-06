#ifndef HANDLER_HH
#define HANDLER_HH

#include <vector>
#include <string>
#include <utility>


namespace restpp
{

    class Handler
    {
    public:
        void (*handle_get)(Request &, Response &);        // Handler for GET requests
        void (*handle_post)(Request &, Response &);       // Handler for POST requests
        void (*handle_put)(Request &, Response &);        // Handler for PUT requests
        void (*handle_patch)(Request &, Response &);      // Handler for PATCH requests
        void (*handle_delete)(Request &, Response &);     // Handler for DELETE requests
        void (*handle_options)(Request &, Response &);    // Handler for OPTIONS requests
        std::vector<std::pair<std::string, Handler>> next; // Next handlers

        /**
         * @brief Construct a new Handler object
         */
        Handler();
    };

}

#endif