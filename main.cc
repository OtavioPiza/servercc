#include <bits/stdc++.h>

#include "server/src/server.hh"
#include "response/src/response.hh"

using restpp::Server, restpp::Response, restpp::Request;

int main(int argc, char **argv)
{
	Server s(8080);

	s.router.handle(GET, "/", [](Request &req, Response &res)
					{ std::cout << "let's go!!!" << std::endl; });

	s.router.handle(GET, "/hello", [](Request &req, Response &res)
					{ std::cout << "let's go!!!" << std::endl; });

	s.run();
}