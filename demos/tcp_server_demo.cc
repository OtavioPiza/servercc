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
    handler_t defaultHandler = [](unique_ptr<Request> request) {
        do {
            // Print the request.
            cout << "Received request: " << request->message->header.length << endl;
            cout << "Protocol: " << request->message->header.protocol << endl;
            cout << "As string: "
                 << string(request->message->body.data.begin(), request->message->body.data.end())
                 << endl;

            // If the protocol is not 0 close the connection.
            if (request->message->header.protocol != 0) {
                cout << "Closing connection." << endl;
                close(request->fd);
                return;
            }

            // Listen for another request.
            auto [status, message] = ostp::servercc::readMessage(request->fd);
            if (!status.ok()) {
                perror("readMessage");
                close(request->fd);
                return;
            }
            request->message = std::move(message);

        } while (true);
    };

    vector<absl::string_view> interfaces = {"lo"};

    // Create the server.
    TcpServer server(7000, defaultHandler);
    cout << "Server listening on port 7000." << endl;
    server.run();

    return 0;
}