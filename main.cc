#include <bits/stdc++.h>
#include "server/src/server.hh"

int main(int argc, char **argv) {
	restpp::Server s(8080);
	s.run();
	return 0;
}