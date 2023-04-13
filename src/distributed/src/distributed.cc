#include "distributed.h"

#include <arpa/inet.h>

#include <memory>
#include <random>
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
using ostp::servercc::distributed::MessageQueue;
using std::binary_semaphore;
using std::function;
using std::queue;
using std::shared_ptr;
using std::string;
using std::thread;

// Constructors.

/// See distributed.h for documentation.
DistributedServer::DistributedServer(const string interface_name, const string interface_ip,
                                     const string group, const uint16_t port,
                                     const function<void(const Request)> default_handler,
                                     const function<void(const string)> peer_connect_callback,
                                     const function<void(const string)> peer_disconnect_callback)
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
          [this](const string &ip) { this->handle_peer_disconnect(ip); }),
      multicast_client(interface_name, group, port),
      protocol_processors(default_handler),
      log_queue_semaphore(0),
      peer_connect_callback(peer_connect_callback),
      peer_disconnect_callback(peer_disconnect_callback) {
    // Add the connect request handler to the UDP server.
    udp_server.set_processor(SERVERCC_DISTRIBUTED_PROTOCOLS_CONNECT,
                             [this](const Request request) { this->handle_connect(request); });

    // Add the connect_ack request handler to the TCP server.
    tcp_server.set_processor(SERVERCC_DISTRIBUTED_PROTOCOLS_CONNECT_ACK,
                             [this](const Request request) { this->handle_connect_ack(request); });

    // Add the internal request handler to the connector.
    connector.add_processor(
        SERVERCC_DISTRIBUTED_PROTOCOLS_INTERNAL_REQUEST,
        [this](const Request request) { this->handle_internal_request(request); });

    // Add the internal response handler to the connector.
    connector.add_processor(
        SERVERCC_DISTRIBUTED_PROTOCOLS_INTERNAL_RESPONSE,
        [this](const Request request) { this->handle_internal_response(request); });

    // Add the internal response end handler to the connector.
    connector.add_processor(
        SERVERCC_DISTRIBUTED_PROTOCOLS_INTERNAL_RESPONSE_END,
        [this](const Request request) { this->handle_internal_response_end(request); });
};

// Public methods.

/// See distributed.h for documentation.
void DistributedServer::run() {
    // Run the services.
    run_tcp_server();
    run_udp_server();
    run_logger_service();

    // Send multicast requests to find peer servers.
    multicast_client.open_socket();
    multicast_client.send_message(SERVERCC_DISTRIBUTED_PROTOCOLS_CONNECT " " +
                                  std::to_string(port));
};

/// See distributed.h for documentation.
StatusOr<void> DistributedServer::add_handler(const string &protocol,
                                              const function<void(const Request)> handler) {
    // Check if the protocol is already registered.
    if (protocol_processors.contains(protocol.c_str(), protocol.length())) {
        return StatusOr<void>(Status::ERROR, "Protocol already registered.");
    }

    // Add the protocol to the protocol processors.
    protocol_processors.insert(protocol.c_str(), protocol.length(), handler);
    return StatusOr<void>(Status::OK, "Protocol registered.");
}

// Utility methods.

/// See distributed.h for documentation.
StatusOr<int> DistributedServer::multicast_message(const string &message) {
    // Send the message to the multicast group.
    return std::move(multicast_client.send_message(message));
}

/// See distributed.h for documentation.
StatusOr<int> DistributedServer::send_message(const string &address, const string &message) {
    // Generate a new random request id.
    int id;
    while (message_queues.contains(id = std::rand()))
        ;

    // Add the ID to the message queue.
    message_queues.insert({id, std::make_shared<MessageQueue>()});

    // Wrap the message in a request.
    const string request =
        SERVERCC_DISTRIBUTED_PROTOCOLS_INTERNAL_REQUEST " " + std::to_string(id) + "\r\n" + message;

    // Send the request to the peer.
    const StatusOr result = connector.send_message(address, request);
    if (result.failed()) {
        // Remove the ID from the message queue.
        message_queues.erase(id);
        return StatusOr(result.status, std::move(result.status_message), -1);
    }

    // Return the request id.
    return StatusOr(Status::OK, "Message sent.", id);
}

/// See distributed.h for documentation.
StatusOr<string> DistributedServer::receive_message(int id) {
    // Look for the message queue with the specified ID.
    auto it = message_queues.find(id);
    if (it == message_queues.end()) {
        return StatusOr<string>(Status::ERROR, "Invalid request ID.", "");
    }

    // Get the message from the queue.
    const string message = it->second->pop();

    // If the is closed and empty, remove it from the map.
    if (it->second->is_closed() && it->second->empty()) {
        message_queues.erase(it);
    }

    // Return the message.
    return StatusOr<string>(Status::OK, "Message received.", std::move(message));
}

/// See distributed.h for documentation.
void DistributedServer::log(const Status status, const string message) {
    // Add the log message to the queue and notify the logger service.
    log_queue.push(std::make_pair(status, std::move(message)));
    log_queue_semaphore.release();
}

// Sever services.

/// See distributed.h for documentation.
void DistributedServer::run_tcp_server() {
    tcp_server_thread = std::thread([this]() {
        log(Status::INFO, "Running TCP server.");
        this->tcp_server.run();
        log(Status::INFO, "TCP server stopped.");
    });
}

/// See distributed.h for documentation.
void DistributedServer::run_udp_server() {
    udp_server_thread = std::thread([this]() {
        log(Status::INFO, "Running UDP server.");
        this->udp_server.run();
        log(Status::INFO, "UDP server stopped.");
    });
}

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

// Handlers.

/// See distributed.h for documentation.
void DistributedServer::handle_connect(const Request request) {
    log(Status::INFO, "Received connect request.");

    // Find the port of the peer server that sent the request by looking after
    // the first space in the request.
    int space_index;
    for (space_index = 0;
         space_index < request.data.length() && !isspace(request.data[space_index]); space_index++)
        ;

    // Get the IP address from the connect request address.
    shared_ptr<struct sockaddr_in> addr =
        std::reinterpret_pointer_cast<struct sockaddr_in>(request.addr);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    // Get the port from the connect request data.
    const uint16_t peer_port = std::stoi(request.data.substr(space_index + 1));

    // If the ip address is the same as the interface ip then ignore the
    // request.
    if (ip == interface_ip) {
        log(Status::WARNING, "Ignoring connect from self.");
        close(request.fd);
        return;
    }

    // Create a TCP client for the peer server and try to connect to it.
    TcpClient peer_server(ip, peer_port);
    if (peer_server.open_socket().failed()) {
        // Close socket and return.
        log(Status::ERROR, "Failed to open socket for peer server'" + string(ip) + ":" +
                               std::to_string(peer_port) + "'.");
        close(peer_server.get_fd());
        close(request.fd);
        return;
    }

    // Send a connect_ack message to the peer server and wait for a connect_ack.
    if (peer_server.send_message("connect_ack").failed()) {
        // Close socket and return.
        log(Status::ERROR, "Failed to send connect_ack to peer server '" + string(ip) + ":" +
                               std::to_string(peer_port) + "'.");
        close(peer_server.get_fd());
        close(request.fd);
        return;
    }

    // If the peer server did not send a connect_ack then close the socket and
    // return.
    StatusOr<string> peer_server_request = peer_server.receive_message();
    if (peer_server_request.failed() || peer_server_request.result != "connect_ack") {
        log(Status::ERROR, "Failed to receive connect_ack from peer server '" + string(ip) + ":" +
                               std::to_string(peer_port) + "'.");
        close(peer_server.get_fd());
        close(request.fd);
        return;
    }

    // Add the peer server to the connector and mappings.
    StatusOr address = connector.add_client(peer_server);
    if (address.failed()) {
        log(Status::ERROR, "Failed to add peer server '" + string(ip) + ":" +
                               std::to_string(peer_port) + "' to connector.");
        close(peer_server.get_fd());
        close(request.fd);
        return;
    }
    peers.insert(address.result);

    // Call the peer connect callback.
    if (peer_connect_callback != nullptr) {
        peer_connect_callback(std::move(address.result));
    }

    // Return and close the socket.
    close(request.fd);
    return;
}

/// See distributed.h for documentation.
void DistributedServer::handle_connect_ack(const Request request) {
    log(Status::INFO, "Received connect_ack request.");

    // Get the ip from the connect request.
    shared_ptr<struct sockaddr_in> addr =
        std::reinterpret_pointer_cast<struct sockaddr_in>(request.addr);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    // Get the port of the peer server that sent the request.
    const uint16_t peer_port = ntohs(addr->sin_port);

    // Add the peer server to the connector and mappings.
    StatusOr address = connector.add_client(TcpClient(request.fd, ip, peer_port, request.addr));
    if (address.failed()) {
        log(Status::ERROR, "Failed to add peer server '" + string(ip) + ":" +
                               std::to_string(peer_port) + "' to connector.");
        close(request.fd);
        return;
    }

    // Send connect_ack to the peer server.
    if (connector.send_message(address.result, "connect_ack").failed()) {
        log(Status::ERROR, "Failed to send connect_ack to peer server '" + string(ip) + ":" +
                               std::to_string(peer_port) + "'.");
        close(request.fd);
        return;
    }

    // Call the peer connect callback.
    if (peer_connect_callback != nullptr) {
        peer_connect_callback(std::move(address.result));
    }

    // Return without closing the socket as it is now owned by the connector.
    return;
}

/// See distributed.h for documentation.
void DistributedServer::handle_peer_disconnect(const string &ip) {
    // Remove the peer server from the connector and mappings.
    log(Status::INFO, "Peer server: '" + ip + "' disconnected.");

    // Remove the peer server from the connector and mappings.
    peers.erase(ip);

    // Call the peer disconnect callback.
    if (peer_disconnect_callback != nullptr) {
        peer_disconnect_callback(ip);
    }
}

/// See distributed.h for documentation.
void DistributedServer::forward_request_to_protocol_processors(const Request request) {
    protocol_processors.get(request.protocol.c_str(),
                            request.protocol.length())(std::move(request));
}

/// See distributed.h for documentation.
void DistributedServer::handle_internal_request(const Request request) {
    // Get the IP from the request.
    shared_ptr<struct sockaddr_in> addr =
        std::reinterpret_pointer_cast<struct sockaddr_in>(request.addr);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    // Get the port from the request.
    const uint16_t port = ntohs(addr->sin_port);

    // Create an address for the peer server.
    const string address = string(ip) + ":" + std::to_string(port);

    // Look for the first space and first \r\n in the request.
    const int space_index = request.data.find(" ");
    const int newline_index = request.data.find("\r\n");
    if (space_index == string::npos || newline_index == string::npos) {
        log(Status::ERROR,
            "Invalid internal request from '" + address + "'. Request: '" + request.data + "'.");
        return;
    }

    // Get the message ID and body.
    const int message_id = std::stoi(request.data.substr(space_index + 1, newline_index));
    const string body = request.data.substr(newline_index + 2);

    // Create a pipe that will be used to send the response back to the
    // requesting server with the message ID header.
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        log(Status::ERROR, "Failed to create pipe.");
        return;
    }

    // Create a new mock request with the fd of the pipe and the content of the
    // body of the request.
    Request mock_request;
    mock_request.fd = pipe_fds[1];

    // Look for first space in the body.
    int i;
    for (i = 0; i < body.size() && !isspace(body[i]); i++)
        ;

    // Get the protocol and data from the body.
    mock_request.protocol = body.substr(0, i);
    mock_request.data = std::move(body);

    // Set the address of the mock request to the address of the original
    mock_request.addr = request.addr;

    // Create a new thread that will forward read the response from the pipe, add the appropriate
    // message ID header, and send the response back to the requesting server.
    std::thread([this, pipe_fds, message_id, address]() {
        // Read the response from the pipe.
        char buffer[1024];

        // Read the response from the pipe while there is data to read.
        while (true) {
            // Read the response from the pipe.
            int bytes_read = read(pipe_fds[0], buffer, 1024);

            // If there was an error reading from the pipe, log the error and return.
            if (bytes_read == -1) {
                log(Status::ERROR, "Failed to read from pipe.");
                break;
            }

            // If there is no more data to read, break.
            if (bytes_read == 0) {
                break;
            }

            // Add the message ID header to the response.
            string response = SERVERCC_DISTRIBUTED_PROTOCOLS_INTERNAL_RESPONSE " " +
                              std::to_string(message_id) + "\r\n" + string(buffer, bytes_read);

            // Send the response back to the requesting server.
            auto send_res = connector.send_message(address, response);
            if (send_res.failed()) {
                log(Status::ERROR,
                    "Failed to send response to server. " + std::string(send_res.status_message));
                break;
            }
        }

        // Send a response end message to the requesting server.
        auto send_res = connector.send_message(
            address,
            SERVERCC_DISTRIBUTED_PROTOCOLS_INTERNAL_RESPONSE_END " " + std::to_string(message_id));
        if (send_res.failed()) {
            log(Status::ERROR,
                "Failed to send response end to server. " + std::string(send_res.status_message));
        }

        // Close the pipe.
        close(pipe_fds[0]);
        close(pipe_fds[1]);
    }).detach();

    // Forward the request to the protocol processors.
    forward_request_to_protocol_processors(std::move(mock_request));
}

/// See distributed.h for documentation.
void DistributedServer::handle_internal_response(const Request request) {
    // Get the message ID from the request between the first space and the
    // first \r\n.
    const int space_index = request.data.find(" ");
    const int newline_index = request.data.find("\r\n");
    if (space_index == string::npos || newline_index == string::npos) {
        log(Status::ERROR, "Received invalid response: " + request.data);
        return;
    }

    // Get the message ID.
    const uint64_t message_id = std::stoi(request.data.substr(space_index + 1, newline_index));

    // Look up the message ID in the message ID map.
    auto message_queue = message_queues.find(message_id);
    if (message_queue == message_queues.end()) {
        log(Status::ERROR,
            "Received response for unknown message ID: " + std::to_string(message_id));
        return;
    }

    // Extract the response from the request after the first \r\n.
    const string response = request.data.substr(newline_index + 2);

    // Add the response to the message queue.
    message_queue->second.get()->push(std::move(response));
}

/// See distributed.h for documentation.
void DistributedServer::handle_internal_response_end(const Request response) {
    // Get the message ID after the first space.
    const int space_index = response.data.find(" ");
    if (space_index == string::npos) {
        log(Status::ERROR, "Received invalid response end: " + response.data);
        return;
    }

    // Get the message ID.
    const uint64_t message_id = std::stoi(response.data.substr(space_index + 1));

    // Look up the message ID in the message ID map.
    auto message_queue = message_queues.find(message_id);
    if (message_queue == message_queues.end()) {
        log(Status::ERROR,
            "Received response end for unknown message ID: " + std::to_string(message_id));
        return;
    }

    // Set the message queue to done.
    message_queue->second.get()->close();
}
