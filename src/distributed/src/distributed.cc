#include "distributed.h"

#include <arpa/inet.h>

#include <iostream>
#include <memory>
#include <thread>

#include "multicast_client.h"
#include "status_or.h"
#include "tcp_client.h"

using ostp::libcc::utils::StatusOr;
using ostp::servercc::client::MulticastClient;
using ostp::servercc::client::TcpClient;
using ostp::servercc::distributed::DistributedServer;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;

/// See distributed.h for documentation.
DistributedServer::DistributedServer(const string interface_name, const string interface_ip,
                                     const string group, const uint16_t port)
    : interface_name(interface_name),
      interface_ip(interface_ip),
      group(group),
      port(port),
      udp_server(port, group, {interface_ip}),
      tcp_server(port),
      connector([](const Request request) { cout << request.data << endl; },
                [](int fd) { cout << "Peer server disconnected." << endl; }),
      multicast_client(interface_name, group, port) {
    // Add the connect request handler.
    udp_server.set_processor(
        "connect", [this](const Request request) { this->handle_connect_request(request); });

    // Add the connect_ack request handler.
    tcp_server.set_processor("connect_ack", [this](const Request request) {
        this->handle_connect_ack_request(request);
    });
};

void DistributedServer::run() {
    // Run the servers in the background threads.
    std::thread udp_server_thread([&]() {
        cout << "Running UDP server." << endl;
        this->udp_server.run();
        cout << "UDP server stopped." << endl;
    });
    std::thread tcp_server_thread([&]() {
        cout << "Running TCP server." << endl;
        this->tcp_server.run();
        cout << "TCP server stopped." << endl;
    });

    // Send multicast requests to find peer servers.
    multicast_client.open_socket();
    multicast_client.send_message("connect " + std::to_string(port));

    // Wait for the servers to stop.
    udp_server_thread.join();
    tcp_server_thread.join();
};

// Handlers.

/// See distributed.h for documentation.
void DistributedServer::handle_connect_request(const Request request) {
    cout << "Received connect request." << endl;

    // Find the port of the peer server that sent the request by looking after
    // the first space in the request.
    int space_index;
    for (space_index = 0; space_index < request.data.size() && !isspace(request.data[space_index]);
         space_index++)
        ;

    // Get the port from the connect request.
    const uint16_t peer_port = std::stoi(request.data.substr(space_index + 1));

    // Get the ip from the connect request.
    shared_ptr<struct sockaddr_in> addr =
        std::reinterpret_pointer_cast<struct sockaddr_in>(request.addr);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    // If the ip address is the same as the interface ip then ignore the
    // request.
    if (ip == interface_ip) {
        close(request.fd);
        return;
    }

    // Create a TCP client for the peer server and try to connect to it.
    TcpClient peer_server(ip, peer_port);
    if (peer_server.open_socket().failed()) {
        // Close socket and return.
        close(peer_server.get_fd());
        close(request.fd);
        return;
    }

    // Send a connect_ack message to the peer server and wait for a connect_ack.
    peer_server.send_message("connect_ack " + std::to_string(port));
    StatusOr<string> peer_server_request = peer_server.receive_message();

    // If the peer server did not send a connect_ack then close the socket and
    // return.
    if (peer_server_request.failed() || peer_server_request.result != "connect_ack") {
        close(peer_server.get_fd());
        close(request.fd);
        return;
    }

    // Add the peer server to the connector and mappings.
    int peer_server_fd = connector.add_client(peer_server);
    peer_ip_to_fd[ip] = peer_server_fd;
    peer_fd_to_ip[peer_server_fd] = ip;
    peer_fd_to_commands[peer_server_fd] = std::vector<string>();

    // Return.
    close(request.fd);
    return;
}

/// See distributed.h for documentation.
void DistributedServer::handle_connect_ack_request(const Request request) {
    cout << "Received connect_ack request." << endl;

    // Find the port of the peer server that sent the request by looking after
    // the first space in the request.
    int space_index;
    for (space_index = 0; space_index < request.data.size() && !isspace(request.data[space_index]);
         space_index++)
        ;

    // Get the port from the connect request.
    const uint16_t peer_port = std::stoi(request.data.substr(space_index + 1));

    // Get the ip from the connect request.
    shared_ptr<struct sockaddr_in> addr =
        std::reinterpret_pointer_cast<struct sockaddr_in>(request.addr);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    // Add the peer server to the connector and mappings.
    int peer_server_fd = connector.add_client(TcpClient(request.fd, ip, peer_port));
    peer_ip_to_fd[ip] = peer_server_fd;
    peer_fd_to_ip[peer_server_fd] = ip;
    peer_fd_to_commands[peer_server_fd] = std::vector<string>();

    // Return.
    return;
}