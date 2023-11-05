#include <bits/stdc++.h>

#include "absl/strings/string_view.h"
#include "servercc/servers/servers.h"

using namespace std;
using ostp::servercc::handler_t;
using ostp::servercc::protocol_t;
using ostp::servercc::Request;
using ostp::servercc::TcpServer;
using ostp::servercc::kMessageHeaderLength;

int main() {
    // Create the default handler.
    handler_t defaultHandler = [](unique_ptr<Request> request) {
        do {
            // Print the request.
            cout << "Received request: " << request->message.header.length << endl;
            cout << "Protocol: " << request->message.header.protocol << endl;
            cout << "As string: "
                 << string(request->message.body.data.begin(), request->message.body.data.end())
                 << endl;

            // If the protocol is not 0 close the connection.
            if (request->message.header.protocol != 0) {
                cout << "Closing connection." << endl;
                close(request->fd);
                return;
            }

            // Listen for another request.
            auto newRequest = std::make_unique<Request>();
            newRequest->fd = request->fd;
            newRequest->addr = request->addr;
            int bytesRead = recv(request->fd, &newRequest->message.header,
                                 kMessageHeaderLength, 0);
            if (bytesRead < kMessageHeaderLength) {
                perror("recv");
                close(request->fd);
                return;
            }
            newRequest->message.body.data.resize(newRequest->message.header.length);
            bytesRead = recv(request->fd, newRequest->message.body.data.data(),
                             newRequest->message.header.length, 0);
            if (bytesRead < newRequest->message.header.length) {
                perror("recv");
                close(request->fd);
                return;
            }
            request = std::move(newRequest);

        } while (true);
    };

    vector<absl::string_view> interfaces = {"lo"};

    // Create the server.
    TcpServer server(8080, defaultHandler);
    cout << "Server listening on port 8080." << endl;
    server.run();

    return 0;
}