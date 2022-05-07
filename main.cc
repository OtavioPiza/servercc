#include <bits/stdc++.h>

#include "server/src/server.hh"
#include "response/src/response.hh"

int main(int argc, char **argv)
{
	restpp::Server s(8080, THREAD_MODE);

	s.router.handle(GET, "/", [](restpp::Request &req, restpp::Response &res) {
		
	});

	s.run();
	return 0;
}