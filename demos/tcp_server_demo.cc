#include <bits/stdc++.h>

#include "absl/strings/string_view.h"
#include "servercc.h"

using namespace std;
using ostp::servercc::handler_t;
using ostp::servercc::kMessageHeaderLength;
using ostp::servercc::Message;
using ostp::servercc::protocol_t;
using ostp::servercc::Request;
using ostp::servercc::TcpServer;

int main() {
    // Create the default handler.
    handler_t defaultHandler = [](unique_ptr<Request> request) -> absl::Status {
        do {
            auto [status, message] = request->receiveMessage();
            if (!status.ok()) {
                perror("receiveMessage");
                break;
            }

            // Print the request.
            cout << "Received request: " << message->header.length << endl;
            cout << "Protocol: " << message->header.protocol << endl;
            cout << "As string: " << string(message->body.data.begin(), message->body.data.end())
                 << endl;

            // If the protocol is not 0 close the connection.
            if (message->header.protocol == 0) {
                cout << "Closing connection." << endl;
                break;
            }

        } while (true);
        return absl::OkStatus();
    };

    vector<absl::string_view> interfaces = {"lo"};

    // Create the server.
    TcpServer server(7000, defaultHandler);
    cout << "Server listening on port 7000." << endl;
    server.run();

    return 0;
}