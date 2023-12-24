#include <arpa/inet.h>
#include <bits/stdc++.h>

#include <functional>

#include "absl/base/log_severity.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "servercc.h"

using ostp::servercc::DistributedServer;
using ostp::servercc::Message;
using ostp::servercc::Request;
using namespace std;

// The letterhead for the shell.
const string letterhead = R"(
░██████╗███████╗██████╗░██╗░░░██╗███████╗██████╗░░█████╗░░█████╗░
██╔════╝██╔════╝██╔══██╗██║░░░██║██╔════╝██╔══██╗██╔══██╗██╔══██╗
╚█████╗░█████╗░░██████╔╝╚██╗░██╔╝█████╗░░██████╔╝██║░░╚═╝██║░░╚═╝
░╚═══██╗██╔══╝░░██╔══██╗░╚████╔╝░██╔══╝░░██╔══██╗██║░░██╗██║░░██╗
██████╔╝███████╗██║░░██║░░╚██╔╝░░███████╗██║░░██║╚█████╔╝╚█████╔╝
╚═════╝░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░╚══════╝╚═╝░░╚═╝░╚════╝░░╚════╝░
)";

// The shell prompt.
const string prompt = "servercc> ";

// The main function.
int main(int argc, char *argv[]) {
    // Setup the server.
    absl::SetStderrThreshold(absl::LogSeverity::kInfo);  // TODO write to file instead of stderr.
    absl::InitializeLog();

    // Get the interface name, ip, group, and port from the command line.
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <interface> <ip> <group> <port>" << endl;
        return 0;
    }

    // The interface name.
    const string interface = argv[1];

    // The ip address of the interface.
    const string interface_ip = argv[2];

    // The multicast group.
    const string group = argv[3];

    // The port to listen on.
    const int port = stoi(argv[4]);

    // The provided_services of the server.
    set<string> provided_services = {"echo", "block"};
    for (int i = 5; i < argc; i++) {
        provided_services.insert(argv[i]);
    }

    // A map of peers to the services they provide.
    map<string, set<string>> peers_to_services;

    // A map of services to the peers that provide them.
    map<string, set<string>> services_to_peers;

    // ===================================================================== //
    // ========================== ADD CALLBACKS ============================ //
    // ===================================================================== //

    // Callback function for when a peer connects.
    function onPeerConnect = [&](in_addr_t ip, DistributedServer &server) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip, ipStr, INET_ADDRSTRLEN);
        LOG(INFO) << "Peer connected: '" << ipStr << "'";
    };

    // Callback function for when a peer disconnects.
    function onPeerDisconnect = [&](in_addr_t ip, DistributedServer &server) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip, ipStr, INET_ADDRSTRLEN);
        LOG(INFO) << "Peer disconnected: '" << ipStr << "'";
    };

    // ===================================================================== //
    // ========================== CREATE SERVER ============================ //
    // ===================================================================== //

    // Distributed server.
    DistributedServer server(
        interface, group, {interface_ip}, port,
        [](std::unique_ptr<Request> request) -> absl::Status { return absl::OkStatus(); },
        onPeerConnect, onPeerDisconnect);

    // ===================================================================== //
    // ========================== ADD HANDLERS ============================= //
    // ===================================================================== //

    // Create an echo handler that sends the message back to the peer.
    server.addHandler(0x14, [](std::unique_ptr<Request> request) -> absl::Status {
        while (true) {
            auto [rcvStatus, message] = request->receiveMessage();
            if (!rcvStatus.ok()) {
                break;
            }
            LOG(INFO) << "Received echo request: '"
                      << std::string(message->body.data.begin(), message->body.data.end()) << "'";
            ASSERT_OK(request->sendMessage(std::move(message)), "Failed to send echo reply");
        }
        return absl::OkStatus();
    });

    // Start the server.
    if (!server.run().ok()) {
        exit(1);
    }

    // ===================================================================== //
    // ========================== DEMO SHELL =============================== //
    // ===================================================================== //

    sleep(1);
    cout << letterhead << endl;
    string line = "";
    while (cout << "servercc-shell> " && getline(cin, line)) {
        if (line == "e") {
            cout << "Starting echo service. Send empty message to stop." << endl;
            cout << "Enter ip: ";
            string ip = "";
            getline(cin, ip);
            in_addr_t ipAddr;
            inet_pton(AF_INET, ip.c_str(), &ipAddr);

            while (true) {
                string message = "";
                cout << "Message: ";
                getline(cin, message);
                if (message == "") {
                    break;
                }
                auto echoMessage = std::make_unique<Message>();
                echoMessage->header.protocol = 0x14;
                echoMessage->header.length = message.size();
                echoMessage->body.data = vector<uint8_t>(message.begin(), message.end());

                auto [status, channel] = server.sendInternalRequest(ipAddr, std::move(echoMessage));
                if (!status.ok()) {
                    cout << "Failed to send echo request: " << status.message() << endl;
                    break;
                }

                auto [rcvStatus, reply] = channel->read();
                if (!rcvStatus.ok()) {
                    cout << "Failed to receive echo reply: " << rcvStatus.message() << endl;
                    channel->close();
                    break;
                }
                cout << "Echo reply: '" << string(reply->body.data.begin(), reply->body.data.end())
                     << "'" << endl;
            }

        } else if (line == "h") {
            cout << "Commands:" << endl;
            cout << "  h: help" << endl;
            cout << "  e: start echo server" << endl;
        }
    }
}
