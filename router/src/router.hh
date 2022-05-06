#ifndef ROUTER_HH
#define ROUTER_HH

#include <string>

#include "../handler/src/handler.hh"

#include "../../request/src/request.hh"
#include "../../response/src/response.hh"

namespace restpp {

    class Router {

        public:
            void handle(std::string method, std::string path, bool (*handler)(restpp::Response &, restpp::Request &));

        private:
            Handler handler;

    };


}

#endif