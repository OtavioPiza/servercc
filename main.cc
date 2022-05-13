#include <bits/stdc++.h>

#include "server/src/server.hh"
#include "response/src/response.hh"

#include "llhttp/src/llhttp.h"

int main(int argc, char **argv)
{
	restpp::Server s(8080);

	s.router.handle(GET, "/", [](restpp::Request &req, restpp::Response &res) {
		std::cout << "let's go!!!" << std::endl;
	});

	s.router.handle(GET, "/hello", [](restpp::Request &req, restpp::Response &res) {
		std::cout << "let's go!!!" << std::endl;
	});

	s.run();
}