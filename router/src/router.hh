#ifndef ROUTER_HH
#define ROUTER_HH

#include <string>

#include "../handler/src/handler.hh"

namespace restpp {

    class Router {

        public:
            void handle(std::string method, std::string path, bool (*handler)(int, int));

        private:
            Handler handler;

    };


}

#endif