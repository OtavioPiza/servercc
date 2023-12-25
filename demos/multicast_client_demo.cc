#include <bits/stdc++.h>

#include "absl/strings/string_view.h"
#include "servercc.h"

using namespace std;
using ostp::servercc::handler_t;
using ostp::servercc::kMessageHeaderLength;
using ostp::servercc::Message;
using ostp::servercc::MulticastClient;
using ostp::servercc::protocol_t;
using ostp::servercc::Request;

int main() {
    MulticastClient client("lo", "224.0.0.1", 6000, 1);
    if (!client.openSocket().ok()) {
        cout << "Failed to open socket." << endl;
        return 1;
    }

    // Send a message from the user input.
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
        auto message = make_unique<Message>();
        message->header.length = bytes.size();
        message->header.protocol = protocol;
        message->body.data = move(bytes);

        cout << "Sending " << message->header.length << " bytes to server." << endl;

        // Send the message.
        if (!client.sendMessage(std::move(message)).ok()) {
            cout << "Could not send message." << endl;
            return 1;
        }
    }

    return 0;
}