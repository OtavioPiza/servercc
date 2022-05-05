#include <bits/stdc++.h>
#include "server/src/server.hh"

int main(int argc, char **argv) {
	restpp::Server s(8080, THREAD_MODE);
	s.run();
	return 0;
}