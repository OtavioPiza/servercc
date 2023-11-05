#include <bits/stdc++.h>

#include "absl/strings/string_view.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/status/internal/statusor_internal.h"
#include "servercc.h"

using namespace std;
using ostp::servercc::handler_t;
using ostp::servercc::Message;
using ostp::servercc::protocol_t;
using ostp::servercc::TcpClient;


int main() {
    // Create a client to connect to the server on port 8080 locally.
    TcpClient client("localhost", 8080);
    if (!client.openSocket().ok()) {
        cout << "Could not open socket." << endl;
        return 1;
    }

    while (true) {
        // Read a line from stdin and convert to a vector of bytes (uint8_t).
        uint32_t protocol;
        cout << "Enter protocol: ";
        cin >> protocol;
        cout << "Enter message: ";
        string line;
        cin >> line;
        vector<uint8_t> bytes(line.begin(), line.end());

        // Create a message to send to the server.
        Message message;
        message.header.length = bytes.size();
        message.header.protocol = protocol;
        message.body.data = move(bytes);

        cout << "Sending " << message.header.length << " bytes to server." << endl;

        // Send the message.
        if (!client.sendMessage(message).ok()) {
            cout << "Could not send message." << endl;
            return 1;
        }
    }

    return 0;
}