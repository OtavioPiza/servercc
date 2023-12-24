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
using ostp::servercc::Request;
using namespace std;

/// The letterhead for the shell.
const string letterhead = R"(
░██████╗███████╗██████╗░██╗░░░██╗███████╗██████╗░░█████╗░░█████╗░
██╔════╝██╔════╝██╔══██╗██║░░░██║██╔════╝██╔══██╗██╔══██╗██╔══██╗
╚█████╗░█████╗░░██████╔╝╚██╗░██╔╝█████╗░░██████╔╝██║░░╚═╝██║░░╚═╝
░╚═══██╗██╔══╝░░██╔══██╗░╚████╔╝░██╔══╝░░██╔══██╗██║░░██╗██║░░██╗
██████╔╝███████╗██║░░██║░░╚██╔╝░░███████╗██║░░██║╚█████╔╝╚█████╔╝
╚═════╝░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░╚══════╝╚═╝░░╚═╝░╚════╝░░╚════╝░
)";

/// The shell prompt.
const string prompt = "servercc> ";

/// The main function.
int main(int argc, char *argv[]) {
    // Setup the server.
    absl::SetStderrThreshold(absl::LogSeverity::kInfo);  // TODO write to file instead of stderr.
    absl::InitializeLog();

    // Get the interface name, ip, group, and port from the command line.
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <interface> <ip> <group> <port> [abilities...]" << endl
             << endl
             << "Abilities: report_temp, report_mem, sort" << endl;
        return 0;
    }

    /// The interface name.
    const string interface = argv[1];

    /// The ip address of the interface.
    const string interface_ip = argv[2];

    /// The multicast group.
    const string group = argv[3];

    /// The port to listen on.
    const int port = stoi(argv[4]);

    /// The provided_services of the server.
    set<string> provided_services = {"echo", "block"};
    for (int i = 5; i < argc; i++) {
        provided_services.insert(argv[i]);
    }

    /// A map of peers to the services they provide.
    map<string, set<string>> peers_to_services;

    /// A map of services to the peers that provide them.
    map<string, set<string>> services_to_peers;

    // ===================================================================== //
    // ========================== ADD CALLBACKS ============================ //
    // ===================================================================== //

    /// Callback function for when a peer connects.
    function on_peer_connect = [&](in_addr_t ip, DistributedServer &server) {
        LOG(INFO) << "Peer connected: '" << ip << "'";

        // peers_to_services.insert({peer_ip, set<string>()});

        // Ask the peer to announce its services.
        // auto id = server.send_message(peer_ip, "announce_services");
        // if (!id.ok()) {
        //     server.log(id.status, std::move(id.status_message));
        //     return;
        // }

        // // Read the services announced by the peer.
        // string announced_services = "";
        // while (true) {
        //     StatusOr response = server.receive_message(id.result);
        //     if (!response.ok()) {
        //         break;
        //     }
        //     announced_services += response.result;
        // }

        // Split the services by space and add to the services map.
        // int i = 0, j = 0;
        // for (; j < announced_services.size(); j++) {
        //     if (isspace(announced_services[j])) {
        //         // Extract the service.
        //         string announced_service = announced_services.substr(i, j - i);
        //         i = j + 1;

        //         // Add the peer to services_to_peers.
        //         auto peer_to_services_it = services_to_peers.find(announced_service);
        //         if (peer_to_services_it == services_to_peers.end()) {
        //             services_to_peers[announced_service] = {peer_ip};
        //         } else {
        //             peer_to_services_it->second.insert(peer_ip);
        //         }

        //         // Add the service to peers_to_services.
        //         peers_to_services[peer_ip].insert(std::move(announced_service));
        //     }
        // }
    };

    /// Callback function for when a peer disconnects.
    function on_peer_disconnect = [&](in_addr_t ip, DistributedServer &server) {
        LOG(INFO) << "Peer disconnected: '" << ip << "'";
        // Remove the peer from the services map.
        // for (const auto &peer_services : peers_to_services[peer_ip]) {
        //     services_to_peers[peer_services].erase(peer_ip);

        //     // Remove the service if it is empty.
        //     if (services_to_peers[peer_services].empty()) {
        //         services_to_peers.erase(peer_services);
        //     }
        // }

        // // Remove the peer from the peers map.
        // peers_to_services.erase(peer_ip);
    };

    // ===================================================================== //
    // ========================== CREATE SERVER ============================ //
    // ===================================================================== //

    /// Distributed server.
    DistributedServer server(
        interface, group, {interface_ip}, port,
        [](std::unique_ptr<Request> request) -> absl::Status { return absl::OkStatus(); },
        on_peer_connect, on_peer_disconnect);

    // ===================================================================== //
    // ========================== ADD HANDLERS ============================= //
    // ===================================================================== //

    /// Handler for the announce_services request.
    // server.add_handler("announce_services", [&](const Request request) {
    //     // Get the ip address of the peer from request.addr.
    //     char ip[INET_ADDRSTRLEN];
    //     inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

    //     // Log the request.
    //     server.log(Status::OK, "Received announce_services request: '" + request.data + "'");

    //     // Go through each service and send a response separated by a space.
    //     for (const auto &provided_service : provided_services) {
    //         write(request.fd, provided_service.c_str(), provided_service.size());
    //         write(request.fd, " ", 1);
    //     }

    //     // Close the connection.
    //     close(request.fd);
    // });

    /// Handler for the echo request.
    // server.add_handler("echo", [&](const Request request) {
    //     // Get the ip address of the peer from request.addr.
    //     char ip[INET_ADDRSTRLEN];
    //     inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

    //     // Log the request.
    //     server.log(Status::OK, "Received echo request: '" + request.data + "'");
    //     write(request.fd, request.data.c_str() + 5, request.data.size() - 5);

    //     // Close the connection.
    //     close(request.fd);
    // });

    /// Handler for a block request.
    ///
    /// When the server receives a block request, it will block the peer forever. This is used to
    /// test the peer disconnect callback.
    // server.add_handler("block", [&](const Request request) {
    //     // Get the ip address of the peer from request.addr.
    //     char ip[INET_ADDRSTRLEN];
    //     inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

    //     // Log the request.
    //     server.log(Status::OK, "Received block request: '" + request.data + "'");

    //     // Block the peer forever.
    //     while (true) {
    //         sleep(1);
    //     }
    // });

    /// Handler for the sort request.
    // server.add_handler("sort", [&](const Request request) {
    //     // Get the ip address of the peer from request.addr.
    //     char ip[INET_ADDRSTRLEN];
    //     inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

    //     // Log the request.
    //     server.log(Status::OK, "Received sort request: '" + request.data + "'");

    //     // Split the request by space.
    //     stringstream ss(request.data.substr(4));
    //     vector<int> numbers;
    //     int number;
    //     while (ss >> number) numbers.push_back(number);

    //     // Sort the numbers.
    //     sort(numbers.begin(), numbers.end());

    //     // Send the sorted numbers.
    //     for (int i = 0; i < numbers.size(); i++) {
    //         if (i != 0) write(request.fd, " ", 1);
    //         write(request.fd, to_string(numbers[i]).c_str(), to_string(numbers[i]).size());
    //     }

    //     // Close the connection.
    //     close(request.fd);
    // });

    /// Handler for the report_temp request.
    // server.add_handler("report_temp", [&](const Request request) {
    //     // Get the ip address of the peer from request.addr.
    //     char ip[INET_ADDRSTRLEN];
    //     inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

    //     // Log the request.
    //     server.log(Status::OK, "Received report_temp request: '" + request.data + "'");

    //     // Send the temperature by running the sensors command.
    //     FILE *fp = popen("sensors | grep 'Package id 0:' | awk '{print $4}'", "r");
    //     if (fp == nullptr) {
    //         server.log(Status::ERROR, "Could not run sensors command");
    //         close(request.fd);
    //         return;
    //     }

    //     // Read the output into the request.fd.
    //     char buffer[1024];
    //     while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
    //         write(request.fd, buffer, strlen(buffer));
    //     }

    //     // Close the pipe.
    //     pclose(fp);

    //     // Close the connection.
    //     close(request.fd);
    // });

    /// Handler to report memory usage.
    // server.add_handler("report_mem", [&](const Request request) {
    //     // Get the ip address of the peer from request.addr.
    //     char ip[INET_ADDRSTRLEN];
    //     inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

    //     // Log the request.
    //     server.log(Status::OK, "Received report_mem request: '" + request.data + "'");

    //     // Send the memory usage by running the free command.
    //     FILE *fp = popen("free -h | grep Mem | awk '{print $3}'", "r");
    //     if (fp == nullptr) {
    //         server.log(Status::ERROR, "Could not run free command");
    //         close(request.fd);
    //         return;
    //     }

    //     // Read the output into the request.fd.
    //     char buffer[1024];
    //     while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
    //         write(request.fd, buffer, strlen(buffer));
    //     }

    //     // Close the pipe.
    //     pclose(fp);

    //     // Close the connection.
    //     close(request.fd);
    // });

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
        // Process the input.
        // if (line == "peers" || line == "p") {
        //     cout << "== Peers Currently Connected ==" << endl;
        //     for (const auto &peer : peers_to_services) {
        //         cout << "- \t" << peer.first << endl;
        //     }
        //     cout << "================================" << endl;
        // }

        // // Handle the services command.
        // else if (line == "services" || line == "s") {
        //     cout << "== Services Currently Available ==" << endl;
        //     for (const auto &service : services_to_peers) {
        //         cout << "- \t" << service.first << " (" << service.second.size() << ")" << endl;
        //     }
        //     cout << "==================================" << endl;
        // }

        // // Handle the clear command.
        // else if (line == "clear") {
        //     system("clear");
        // }

        // // Handle echo command.
        // else if (line.starts_with("echo ")) {
        //     // Get the message.
        //     string message = line.substr(5);

        // Send the echo request to peers who support the service.
        //     auto peers_it = services_to_peers.find("echo");
        //     if (peers_it == services_to_peers.end() || peers_it->second.empty()) {
        //         server.log(Status::ERROR, "No peers support the echo service");
        //         continue;
        //     }

        //     // Send the echo request to peers who support the service.
        //     vector peers = vector<string>(peers_it->second.begin(), peers_it->second.end());
        //     for (const auto &peer : peers) {
        //         auto time_start = chrono::steady_clock::now();
        //         auto id = server.send_message(peer, "echo " + message);

        //         // If we could not send the message, continue.
        //         if (!id.ok()) {
        //             server.log(id.status, std::move(id.status_message));
        //             continue;
        //         }

        //         // Read all available responses.
        //         string result = "";
        //         while (true) {
        //             auto response = server.receive_message(id.result);
        //             if (!response.ok()) {
        //                 break;
        //             }
        //             result += response.result;
        //         }

        //         // Log the result.
        //         auto time_end = chrono::steady_clock::now();
        //         auto time_diff =
        //             chrono::duration_cast<chrono::milliseconds>(time_end -
        //             time_start).count();
        //         cout << "Response from " << peer << " (" << time_diff << "ms): " << result <<
        //         endl;
        //     }
        // }

        // // Handle block command.
        // else if (line.starts_with("block ")) {
        //     const string peer_ip = line.substr(6);

        //     // Check if the peer is connected.
        //     if (peers_to_services.find(peer_ip) == peers_to_services.end()) {
        //         server.log(Status::ERROR, "Peer is not connected");
        //         continue;
        //     }

        //     // Block the peer.
        //     auto res = server.send_message(peer_ip, "block");
        //     if (!res.ok()) {
        //         server.log(res.status, std::move(res.status_message));
        //         continue;
        //     }

        //     // Read the response.
        //     auto response = server.receive_message(res.result);
        //     if (!response.ok()) {
        //         server.log(response.status, std::move(response.status_message));
        //         continue;
        //     }
        // }

        // // Handle a sort request.
        // else if (line.starts_with("sort ")) {
        //     // Get the numbers.
        //     string numbers = line.substr(4);

        //     // Check if we have any peers that support the service.
        //     auto peers_it = services_to_peers.find("sort");
        //     if (peers_it == services_to_peers.end() || peers_it->second.empty()) {
        //         server.log(Status::ERROR, "No peers support the sort service");
        //         continue;
        //     }

        //     // Read the numbers into a vector.
        //     vector<int> nums;
        //     stringstream ss(numbers);
        //     int num;
        //     while (ss >> num) nums.push_back(num);

        //     // Send each peer part of the numbers.
        //     int chunk_size = ceil((double)nums.size() / (double)peers_it->second.size());
        //     int start = 0;
        //     vector<int> ids(peers_it->second.size());
        //     vector peers = vector<string>(peers_it->second.begin(), peers_it->second.end());
        //     for (auto &peer : peers) {
        //         int end = min(start + chunk_size, (int)nums.size());

        //         // Send the sort request.
        //         string message = "sort ";
        //         for (int i = start; i < end; i++) {
        //             if (i != start) message += " ";
        //             message += to_string(nums[i]);
        //         }

        //         // Attempt to send the message.
        //         while (true) {
        //             auto res = server.send_message(peer, message);
        //             if (res.ok()) {
        //                 ids.push_back(res.result);
        //                 break;
        //             }
        //         }

        //         // Update the start index.
        //         start = end;
        //     }

        //     // Read the responses.
        //     vector<vector<int>> responses;
        //     for (int i = 0; i < ids.size(); i++) {
        //         // Read all available responses.
        //         while (true) {
        //             auto response = server.receive_message(ids[i]);
        //             if (!response.ok()) {
        //                 break;
        //             }

        //             // Parse the response.
        //             stringstream ss(response.result);
        //             vector<int> nums;
        //             int num;
        //             while (ss >> num) nums.push_back(num);

        //             // Add the response to the list.
        //             responses.push_back(nums);
        //         }
        //     }

        //     // Merge the responses.
        //     function<vector<int>(int, int)> merge = [&](int start, int end) {
        //         // Base case.
        //         if (start == end) {
        //             return responses[start];
        //         }

        //         // Recursive case.
        //         int mid = (start + end) / 2;
        //         auto left = merge(start, mid);
        //         auto right = merge(mid + 1, end);

        //         // Merge the two lists.
        //         vector<int> merged;
        //         int i = 0, j = 0;
        //         while (i < left.size() && j < right.size()) {
        //             if (left[i] < right[j]) {
        //                 merged.push_back(left[i++]);
        //             } else {
        //                 merged.push_back(right[j++]);
        //             }
        //         }

        //         // Add the remaining elements.
        //         while (i < left.size()) merged.push_back(left[i++]);
        //         while (j < right.size()) merged.push_back(right[j++]);

        //         return std::move(merged);
        //     };

        //     // Print the sorted numbers.
        //     auto sorted = merge(0, responses.size() - 1);
        //     cout << "== Sorted Numbers ==" << endl;
        //     for (int i = 0; i < sorted.size(); i++) {
        //         if (i != 0) cout << " ";
        //         cout << sorted[i];
        //     }
        //     cout << endl;
        //     cout << "===================" << endl;
        // }

        // // Handle the report_temp command.
        // else if (line == "report_temp" || line == "rt") {
        //     // Check if we have any peers that support the service.
        //     auto peers_it = services_to_peers.find("report_temp");
        //     if (peers_it == services_to_peers.end() || peers_it->second.empty()) {
        //         server.log(Status::ERROR, "No peers support the report_temp service");
        //         continue;
        //     }

        //     // Send the report_temp request to peers who support the service.
        //     cout << "== Temperature Reported by Peers ==" << endl;
        //     for (const auto &peer : peers_it->second) {
        //         auto id = server.send_message(peer, "report_temp");

        //         // If we could not send the message, continue.
        //         if (!id.ok()) {
        //             server.log(id.status, std::move(id.status_message));
        //             continue;
        //         }

        //         // Read from response.
        //         string announced_temp = "";
        //         while (true) {
        //             auto response = server.receive_message(id.result);
        //             if (!response.ok()) {
        //                 break;
        //             }
        //             // Append the response to the announced_temp.
        //             announced_temp += response.result;
        //         }

        //         // Print the temperature from the peer.
        //         cout << peer << " is at " << announced_temp;
        //     }
        //     cout << "==================================" << endl;
        // }

        // // Handle report memory command.
        // else if (line == "report_mem" || line == "rm") {
        //     // Check if we have any peers that support the service.
        //     auto peers_it = services_to_peers.find("report_mem");
        //     if (peers_it == services_to_peers.end() || peers_it->second.empty()) {
        //         server.log(Status::ERROR, "No peers support the report_mem service");
        //         continue;
        //     }

        //     // Send the report_mem request to peers who support the service.
        //     cout << "== Memory Usage Reported by Peers ==" << endl;
        //     for (const auto &peer : peers_it->second) {
        //         auto id = server.send_message(peer, "report_mem");

        //         // If we could not send the message, continue.
        //         if (!id.ok()) {
        //             server.log(id.status, std::move(id.status_message));
        //             continue;
        //         }

        //         // Read from response.
        //         string announced_mem = "";
        //         while (true) {
        //             auto response = server.receive_message(id.result);
        //             if (!response.ok()) {
        //                 break;
        //             }
        //             // Append the response to the announced_mem.
        //             announced_mem += response.result;
        //         }

        //         // Print the memory usage from the peer.
        //         cout << peer << " is using " << announced_mem;
        //     }
        //     cout << "====================================" << endl;
        // }

        // // Send connect message.
        // else if (line == "sc") {
        //     auto res = server.send_connect_message();
        //     if (!res.ok()) {
        //         server.log(res.status, std::move(res.status_message));
        //     }
        // }

        // Print supported commands.
        // else if (line == "help" || line == "h") {
        // cout << "== Supported Commands ==" << endl
        //      << "peers\t - List peers currently connected" << endl
        //      << "services\t - List services currently available" << endl
        //      << "echo <message>\t - Send a message to peers who support the echo service" << endl
        //      << "report_temp\t - Report the temperature of peers who support the report_temp "
        //         "service"
        //      << "report_mem\t - Report the memory usage of peers who support the report_mem "
        //         "service"
        //      << endl
        //      << "sort\t - Sort a list of numbers using peers who support the sort service" <<
        //      endl
        //      << "clear\t - Clear the screen" << endl
        //      << "help\t - Print this message" << endl
        //      << "========================" << endl;
        // }
    }
}
