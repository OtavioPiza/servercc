#include <bits/stdc++.h>

#include "absl/strings/string_view.h"
#include "servercc.h"

using namespace std;
using ostp::servercc::handler_t;
using ostp::servercc::kMessageHeaderLength;
using ostp::servercc::Message;
using ostp::servercc::protocol_t;
using ostp::servercc::Request;
using ostp::servercc::UdpServer;

int main() {
    // Create the default handler.
    handler_t defaultHandler = [](unique_ptr<Request> request) {
        do {
            auto [status, message] = request->receiveMessage();
            if (!status.ok()) {
                break;
            }

            // Print the request.
            cout << "Received request: " << message->header.length << endl;
            cout << "Protocol: " << message->header.protocol << endl;
            cout << "As string: " << string(message->body.data.begin(), message->body.data.end())
                 << endl;

        } while (true);
    };

    vector<absl::string_view> interfaces = {"127.0.0.1"};

    // Create the server.
    UdpServer server(6000, "224.0.0.1", interfaces, defaultHandler);
    cout << "Server listening on port 6000." << endl;
    server.run();

    return 0;
}