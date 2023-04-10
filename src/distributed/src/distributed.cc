#include "distributed.h"

#include <arpa/inet.h>

#include <memory>
#include <thread>

#include "distributed_protocols.h"
#include "logger.h"
#include "multicast_client.h"
#include "status.h"
#include "status_or.h"
#include "tcp_client.h"

using ostp::libcc::utils::log_error;
using ostp::libcc::utils::log_info;
using ostp::libcc::utils::log_ok;
using ostp::libcc::utils::log_warn;
using ostp::libcc::utils::Status;
using ostp::libcc::utils::StatusOr;
using ostp::servercc::client::MulticastClient;
using ostp::servercc::client::TcpClient;
using ostp::servercc::distributed::DistributedServer;
using std::endl;
using std::shared_ptr;
using std::string;

/// See distributed.h for documentation.
DistributedServer::DistributedServer(const string interface_name, const string interface_ip,
                                     const string group, const uint16_t port,
                                     std::function<void(const Request)> default_handler)
    : interface_name(interface_name),
      interface_ip(interface_ip),
      group(group),
      port(port),
      udp_server(port, group, {interface_ip},
                 [this](const Request request) {
                     this->forward_request_to_protocol_processors(std::move(request));
                 }),
      tcp_server(port,
                 [this](const Request request) {
                     this->forward_request_to_protocol_processors(std::move(request));
                 }),
      connector(
          [this](const Request request) {
              this->forward_request_to_protocol_processors(std::move(request));
          },
          [this](int fd) { this->handle_peer_disconnect(fd); }),
      multicast_client(interface_name, group, port),
      protocol_processors(default_handler),
      log_queue_semaphore(0) {
    // Add the connect request handler to the UDP server.
    udp_server.set_processor(SERVERCC_DISTRIBUTED_PROTOCOLS_CONNECT, [this](const Request request) {
        this->handle_connect_request(request);
    });

    // Add the connect_ack request handler to the TCP server.
    tcp_server.set_processor(
        SERVERCC_DISTRIBUTED_PROTOCOLS_CONNECT_ACK,
        [this](const Request request) { this->handle_connect_ack_request(request); });
};

// Handlers.

/// See distributed.h for documentation.
void DistributedServer::handle_connect_request(const Request request) {
    log(Status::INFO, "Received connect request.");

    // Find the port of the peer server that sent the request by looking after
    // the first space in the request.
    int space_index;
    for (space_index = 0;
         space_index < request.data.length() && !isspace(request.data[space_index]); space_index++)
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
        log(Status::WARNING, "Received connect request from self.");
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
    log(Status::INFO, "Received connect_ack request.");

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

    // Send connect_ack to the peer server.
    connector.send_message(peer_server_fd, "connect_ack");

    // Return.
    return;
}

/// See distributed.h for documentation.
void DistributedServer::forward_request_to_protocol_processors(const Request request) {
    protocol_processors.get(request.protocol.c_str(),
                            request.protocol.length())(std::move(request));
}

/// See distributed.h for documentation.
void DistributedServer::handle_peer_disconnect(int fd) {
    // Remove the peer server from the connector and mappings.
    log(Status::INFO,
        "Peer server disconnected. ip: " + peer_fd_to_ip[fd] + " fd: " + std::to_string(fd));
}

// Public methods.

/// See distributed.h for documentation.
void DistributedServer::run() {
    // Run the servers in the background threads.
    std::thread udp_server_thread([&]() {
        log(Status::INFO, "Running UDP server.");
        this->udp_server.run();
        log(Status::INFO, "UDP server stopped.");
    });
    std::thread tcp_server_thread([&]() {
        log(Status::INFO, "Running TCP server.");
        this->tcp_server.run();
        log(Status::INFO, "TCP server stopped.");
    });

    run_logger_service();

    // Send multicast requests to find peer servers.
    multicast_client.open_socket();
    multicast_client.send_message(SERVERCC_DISTRIBUTED_PROTOCOLS_CONNECT " " +
                                  std::to_string(port));

    // Wait for the servers to stop.
    udp_server_thread.join();
    tcp_server_thread.join();
};

/// See distributed.h for documentation.
StatusOr<bool> DistributedServer::add_handler(const string protocol,
                                              const std::function<void(const Request)> handler) {
    // Check if the protocol is already registered.
    if (protocol_processors.contains(protocol.c_str(), protocol.length())) {
        return StatusOr<bool>(Status::OK, "Protocol already registered.", false);
    }

    // Add the protocol to the protocol processors.
    protocol_processors.insert(protocol.c_str(), protocol.length(), handler);
    return StatusOr<bool>(Status::OK, nullptr, false);
}

// Logging methods.

/// See distributed.h for documentation.
void DistributedServer::run_logger_service() {
    // Create a thread for the logger service.
    logger_service_thread = std::thread([&]() {
        while (true) {
            const string sender = "DistributedServerLoggerService";

            // Wait for a log message and then log it depending on the log level.
            log_queue_semaphore.acquire();
            std::pair<const Status, const string> log_message = std::move(log_queue.front());
            log_queue.pop();

            // Log the message.
            switch (log_message.first) {
                case Status::OK:
                    log_ok(log_message.second, sender);
                    break;

                case Status::ERROR:
                    log_error(log_message.second, sender);
                    break;

                case Status::WARNING:
                    log_warn(log_message.second, sender);
                    break;

                default:
                    log_info(log_message.second, sender);
                    break;
            }
        }
    });
}

/// See distributed.h for documentation.
void DistributedServer::log(const Status status, const string message) {
    // Add the log message to the queue and notify the logger service.
    log_queue.push(std::make_pair(status, message));
    log_queue_semaphore.release();
}
