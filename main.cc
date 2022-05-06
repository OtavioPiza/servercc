#include <bits/stdc++.h>
#include "server/src/server.hh"
#include "router/src/router.hh"

#include "request/src/request.hh"
#include "response/src/response.hh"

int main(int argc, char **argv)
{
	restpp::Server s(8080, THREAD_MODE);

	s.router.handle("GET", "/", [](restpp::Response &res, restpp::Request &req) {

	});

	s.run();
	return 0;
}