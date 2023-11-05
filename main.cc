#include <bits/stdc++.h>
#include "servercc/servers/servers.h"
#include "servercc/clients/clients.h"
#include "absl/strings/string_view.h"

using namespace std;
using ostp::servercc::handler_t;
using ostp::servercc::protocol_t;
using ostp::servercc::TcpServer;
using ostp::servercc::UdpServer;
using ostp::servercc::Request;

int main() {

    // Create the default handler.
    handler_t defaultHandler = [](unique_ptr<Request> request) {
        // Print the request.
        cout << "Received request: " << request->message.header.length << endl;
    };

    vector<absl::string_view> interfaces = {"lo"};

    // Create the server.
    TcpServer server(8080, defaultHandler);
    UdpServer server2(7070, "224.1.1.1", interfaces, defaultHandler);

    server2.run();


    return 0;
}