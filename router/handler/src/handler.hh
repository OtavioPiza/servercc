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
        bool (*handle_get)(Response &, Response &);        // Handler for GET requests
        bool (*handle_post)(Response &, Response &);       // Handler for POST requests
        bool (*handle_put)(Response &, Response &);        // Handler for PUT requests
        bool (*handle_patch)(Response &, Response &);      // Handler for PATCH requests
        bool (*handle_delete)(Response &, Response &);     // Handler for DELETE requests
        bool (*handle_options)(Response &, Response &);    // Handler for OPTIONS requests
        std::vector<std::pair<std::string, Handler>> next; // Next handlers

        /**
         * @brief Construct a new Handler object
         */
        Handler();
    };

}

#endif