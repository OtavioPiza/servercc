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
        bool (*handle_get)(int, int);                      // Handler for GET requests
        bool (*handle_post)(int, int);                     // Handler for POST requests
        bool (*handle_put)(int, int);                      // Handler for PUT requests
        bool (*handle_patch)(int, int);                    // Handler for PATCH requests
        bool (*handle_delete)(int, int);                   // Handler for DELETE requests
        bool (*handle_options)(int, int);                  // Handler for OPTIONS requests
        std::vector<std::pair<std::string, Handler>> next; // Next handlers

        /**
         * @brief Construct a new Handler object
         */
        Handler();
    };

}

#endif