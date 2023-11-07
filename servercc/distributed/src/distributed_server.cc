#include "distributed_server.h"

#include <arpa/inet.h>

#include <iostream>
#include <memory>
#include <random>
#include <thread>

#include "absl/strings/str_cat.h"

namespace ostp::servercc {

using ostp::libcc::data_structures::MessageBuffer;
using ostp::servercc::kMessageHeaderLength;

// See distributed.h for documentation.
DistributedServer::DistributedServer(
    absl::string_view interfaceName, absl::string_view group,
    std::vector<absl::string_view> interfaces, const uint16_t port, handler_t default_handler,
    const std::function<void(absl::string_view, DistributedServer &server)> peerConnectCallback,
    const std::function<void(absl::string_view string, DistributedServer &server)>
        peerDisconnectCallback)
    : interfaceName(interfaceName),  // TODO: allow multiple interfaces.
      interfaces(std::move(interfaces)),
      group(group),
      port(port),
      udpServer(port, group, this->interfaces,
                [this](std::unique_ptr<Request> request) {
                    this->forwardRequestToHandler(std::move(request));
                }),
      tcpServer(port,
                [this](std::unique_ptr<Request> request) {
                    this->forwardRequestToHandler(std::move(request));
                }),
      connector(
          [this](std::unique_ptr<Request> request) {
              this->forwardRequestToHandler(std::move(request));
          },
          [this](absl::string_view peerIp) { this->onConnectorDisconnect(peerIp); }),
      multicastClient(interfaceName, group, port, 1),  // TODO: Make TTL configurable.
      defaultHandler(defaultHandler),
      logQueueSemaphore(0),
      peerConnectCallback(peerConnectCallback),
      peerDisconnectCallback(peerDisconnectCallback) {
    // TODO define types and return stats from setting handlers.

    // Add the connect request handler to the UDP server.
    if (!udpServer
             .addHandler(0x10,
                         [this](std::unique_ptr<Request> request) {
                             this->handleConnect(std::move(request));
                         })
             .ok()) {
        throw std::runtime_error("Failed to add connect request handler to UDP server.");
    }

    // Add the connect_ack request handler to the TCP server.
    if (!tcpServer
             .addHandler(0x11,
                         [this](std::unique_ptr<Request> request) {
                             this->handleConnectAck(std::move(request));
                         })
             .ok()) {
        throw std::runtime_error("Failed to add connect_ack request handler to TCP server.");
    }

    // Add the internal request handler to the connector.
    if (!connector
             .addHandler(0x12,
                         [this](std::unique_ptr<Request> request) {
                             this->handleInternalRequest(std::move(request));
                         })
             .ok()) {
        throw std::runtime_error("Failed to add internal request handler to connector.");
    }

    // Add the internal response handler to the connector.
    if (!connector
             .addHandler(0x13,
                         [this](std::unique_ptr<Request> request) {
                             this->handleInternalResponse(std::move(request));
                         })
             .ok()) {
        throw std::runtime_error("Failed to add internal response handler to connector.");
    }

    // Add the internal response end handler to the connector.
    if (!connector
             .addHandler(0x14,
                         [this](std::unique_ptr<Request> request) {
                             this->handleInternalResponseEnd(std::move(request));
                         })
             .ok()) {
        throw std::runtime_error("Failed to add internal response end handler to connector.");
    }
}

// See distributed.h for documentation.
absl::Status DistributedServer::run() {
    // Run the services.
    absl::Status status;
    if (!(status = runTcpServer()).ok()) {
        return status;
    }
    if (!(status = runUdpServer()).ok()) {
        return status;
    }
    runLoggerService();

    // TODO make this configurable.
    // Try to connect to the multicast group.
    int retries = 5;
    while (true) {
        // Send the connect message.
        const auto result = sendConnectMessage();
        if (!result.ok()) {
            // TODO log

        } else {
            return absl::OkStatus();
        }
    }
    return absl::InternalError("Failed to connect to multicast group.");
}

// See distributed.h for documentation.
absl::Status DistributedServer::addHandler(protocol_t protocol, handler_t handler) {
    // Check if the protocol is already registered.
    if (handlers.contains(protocol)) {
        return absl::AlreadyExistsError("Handler already exists for protocol.");
    }
    handlers.insert({protocol, handler});
    return absl::OkStatus();
}

// See distributed.h for documentation.
absl::Status DistributedServer::multicastMessage(const Message &message) {
    if (!multicastClient.isOpen() && !multicastClient.openSocket().ok()) {
        return absl::InternalError("Failed to open socket.");
    }
    return std::move(multicastClient.sendMessage(message));
}

// See distributed.h for documentation.
absl::Status DistributedServer::sendConnectMessage() {
    Message message;
    message.header.protocol = 0x10;
    message.header.length = sizeof(uint16_t);
    message.body.data.resize(message.header.length);
    memcpy(message.body.data.data(), &port, message.header.length);
    return multicastMessage(message);
}

// See distributed.h for documentation.
std::pair<absl::Status, uint32_t> DistributedServer::sendMessage(absl::string_view address,
                                                                 const Message &message) {
    // Generate a new random request id.
    uint32_t id;
    while (messageBuffers.contains(id = std::rand()))
        ;

    // Add the ID to the message queue.
    auto messageBuffer = std::make_shared<MessageBuffer<std::unique_ptr<Message>>>();
    messageBuffers.insert({id, messageBuffer});

    // Create a wrapper message.
    Message request;
    request.header.protocol = 0x12;
    request.header.length = sizeof(uint32_t) + kMessageHeaderLength + message.header.length;
    request.body.data.resize(request.header.length);

    // Copy the message ID, message header, and message body into the wrapper message body.
    memcpy(request.body.data.data(), &id, sizeof(uint32_t));
    memcpy(request.body.data.data() + sizeof(uint32_t), &message.header, kMessageHeaderLength);
    memcpy(request.body.data.data() + sizeof(uint32_t) + kMessageHeaderLength,
           message.body.data.data(), message.header.length);

    // Send the request to the peer.
    const auto result = connector.sendMessage(address, request);
    if (!result.ok()) {
        // Remove the ID from the message queue.
        messageBuffers.erase(id);
        return {result, -1};
    }

    // Add the message ID to the peers_message_ids map.
    messageIdsToPeers.insert({id, address});

    // Add the message ID to the peers_message_ids map.
    const auto messageIdsIt = peersToMessageIds.find(address);
    if (messageIdsIt == peersToMessageIds.end()) {
        peersToMessageIds.insert({address, {id}});
    } else {
        messageIdsIt->second.insert(id);
    }

    // Return the request id.
    return {absl::OkStatus(), id};
}

// See distributed.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> DistributedServer::receiveMessage(
    const uint32_t id) {
    // Look for the message queue with the specified ID.
    auto it = messageBuffers.find(id);
    if (it == messageBuffers.end()) {
        return {absl::NotFoundError("Invalid request ID."), nullptr};
    }

    // Get the message from the queue.
    auto res = it->second->pop();

    // If the is closed and empty, remove it from the messages_queues map.
    if (it->second->is_closed() && it->second->empty()) {
        // Erase the message queue.
        messageBuffers.erase(it);

        // Erase the message ID from the peers_to_message_ids and remove the peer if it is empty.
        auto message_ids_it = peersToMessageIds.find(messageIdsToPeers[id]);
        if (message_ids_it != peersToMessageIds.end()) {
            message_ids_it->second.erase(id);
            if (message_ids_it->second.empty()) {
                peersToMessageIds.erase(message_ids_it);
            }
        }

        // Erase the message ID from the message_ids_to_peers map.
        messageIdsToPeers.erase(id);
    }

    // Return the message.
    return std::move(res);
}

// See distributed.h for documentation.
void DistributedServer::log(absl::Status status, absl::string_view message) {
    // Add the log message to the queue and notify the logger service.
    logQueue.push(std::make_pair(status, message));
    logQueueSemaphore.release();
}

// See distributed.h for documentation.
absl::Status DistributedServer::runTcpServer() {
    // TODO Create setup phase to catch errors early
    tcpServerThread = std::thread([this]() {
        log(absl::OkStatus(), "Running TCP server.");
        this->tcpServer.run();
        log(absl::OkStatus(), "TCP server stopped.");
    });
    return absl::OkStatus();
}

// See distributed.h for documentation.
absl::Status DistributedServer::runUdpServer() {
    // TODO create setup phase to catch errors early
    udpServerThread = std::thread([this]() {
        log(absl::OkStatus(), "Running UDP server.");
        this->udpServer.run();
        log(absl::OkStatus(), "UDP server stopped.");
    });
    return absl::OkStatus();
}

// See distributed.h for documentation.
void DistributedServer::runLoggerService() {
    // Create a thread for the logger service.
    loggerServiceThread = std::thread([&]() {
        while (true) {
            const string sender = "DistributedServerLoggerService";

            // Wait for a log message and then log it depending on the log level.
            logQueueSemaphore.acquire();
            auto logMessage = logQueue.front();
            logQueue.pop();

            // Log the message.
            if (logMessage.first.ok()) {
                // Print in green if the log level is INFO.
                std::cout << "\033[1;32m" << sender << ": " << logMessage.second << "\033[0m"
                          << std::endl;
            } else {
                // Print in red if the log level is ERROR.
                std::cout << "\033[1;31m" << sender << ": " << logMessage.second << "\033[0m"
                          << std::endl;
            }
        }
    });
}

// See distributed.h for documentation.
void DistributedServer::onConnectorDisconnect(absl::string_view ip) {
    log(absl::OkStatus(), absl::StrCat("Peer disconnected: ", ip));

    // Look for pending messages from the disconnected peer.
    auto messageIdsIt = peersToMessageIds.find(ip);
    if (messageIdsIt != peersToMessageIds.end()) {
        // Log an error.
        log(absl::InternalError("Peer disconnected without sending a response."),
            absl::StrCat("Peer disconnected without sending a response. Missing messages: ",
                         messageIdsIt->second.size(), "."));

        // Close the message queues of the pending messages remove the message IDs from the
        // message_ids_to_peers map.
        for (const auto id : messageIdsIt->second) {
            messageBuffers[id]->close();
        }
    }

    // Remove from the peers list.
    peers.erase(ip);

    // Call the user-specified disconnect callback.
    if (peerDisconnectCallback) peerDisconnectCallback(ip, *this);
}

}  // namespace ostp::servercc

// // Handlers.

// // See distributed.h for documentation.
// void DistributedServer::handle_connect(const Request request) {
//     // Find the port of the peer server that sent the request by looking after
//     // the first space in the request.
//     int space_index;
//     for (space_index = 0;
//          space_index < request.data.length() && !isspace(request.data[space_index]);
//          space_index++)
//         ;

//     // Get the IP address from the connect request address.
//     char ip[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, &((sockaddr_in *)&request.addr)->sin_addr, ip, INET_ADDRSTRLEN);

//     // Get the port from the connect request data.
//     const uint16_t peer_port = std::stoi(request.data.substr(space_index + 1));

//     // Log the connect request.
//     log(Status::INFO, "Received connect request from peer server '" + string(ip) + "' on port " +
//                           std::to_string(peer_port) + ".");

//     // If the ip address is the same as the interface ip then ignore the
//     // request or if the peer server is already connected.
//     if (ip == interfaces || peers.contains(ip)) {
//         log(Status::WARNING, "Ignoring connect request from peer server '" + string(ip) + "'.");
//         close(request.fd);
//         return;
//     }

//     // Create a TCP client for the peer server and try to connect to it.
//     TcpClient peerServer(ip, peer_port);
//     if (peerServer.open_socket().failed()) {
//         log(Status::ERROR, "Failed to open socket to peer server '" + string(ip) + "'.");
//         // Close socket and return.
//         close(peerServer.get_fd());
//         close(request.fd);
//         return;
//     }

//     // Send a connect_ack message to the peer server and wait for a connect_ack.
//     if (peerServer.send_message("connect_ack").failed()) {
//         // Close socket and return.
//         log(Status::ERROR, "Failed to send connect_ack to peer server '" + string(ip) + "'.");
//         close(peerServer.get_fd());
//         close(request.fd);
//         return;
//     }

//     // If the peer server did not send a connect_ack then close the socket and
//     // return.
//     StatusOr<string> peerServer_request = peerServer.receive_message();
//     if (peerServer_request.failed() || peerServer_request.result != "connect_ack") {
//         log(Status::ERROR, "Failed to receive connect_ack from peer server '" + string(ip) +
//         "'."); close(peerServer.get_fd()); close(request.fd); return;
//     }

//     // Add the peer server to the connector and mappings.
//     StatusOr address = connector.addClient(peerServer);
//     if (address.failed()) {
//         log(Status::ERROR, "Failed to add peer server '" + string(ip) + "' to connector.");
//         close(peerServer.get_fd());
//         close(request.fd);
//         return;
//     }
//     peers.insert(address.result);

//     // Call the peer connect callback.
//     if (peerConnectCallback != nullptr) {
//         peerConnectCallback(std::move(address.result), *this);
//     }

//     // Return and close the socket.
//     close(request.fd);
//     return;
// }

// // See distributed.h for documentation.
// void DistributedServer::handle_connect_ack(const Request request) {
//     // Get the port and ip from the connect request.
//     sockaddr_in *addr = (sockaddr_in *)&request.addr;
//     uint16_t peer_port = ntohs(addr->sin_port);
//     char ip[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

//     // Log the connect_ack request.
//     log(Status::INFO, "Received connect_ack from peer server '" + string(ip) + "' on port " +
//                           std::to_string(peer_port) + ".");

//     // Add the peer server to the connector and mappings.
//     StatusOr address = connector.addClient(TcpClient(request.fd, ip, peer_port, request.addr));
//     if (address.failed()) {
//         log(Status::ERROR, "Failed to add peer server '" + string(ip) + "' to connector.");
//         close(request.fd);
//         return;
//     }

//     // Send connect_ack to the peer server.
//     if (connector.send_message(address.result, "connect_ack").failed()) {
//         log(Status::ERROR, "Failed to send connect_ack to peer server '" + string(ip) + "'.");
//         close(request.fd);
//         return;
//     }

//     // Call the peer connect callback.
//     if (peerConnectCallback != nullptr) {
//         peerConnectCallback(std::move(address.result), *this);
//     }

//     // Return without closing the socket as it is now owned by the connector.
//     return;
// }

// // See distributed.h for documentation.
// void DistributedServer::forwardRequestToHandler(const Request request) {
//     protocol_processors.get(request.protocol.c_str(),
//                             request.protocol.length())(std::move(request));
// }

// // See distributed.h for documentation.
// void DistributedServer::handle_internal_request(const Request request) {
//     // Get the port and ip from the connect request.
//     sockaddr_in *addr = (sockaddr_in *)&request.addr;
//     uint16_t peer_port = ntohs(addr->sin_port);
//     char ip[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

//     // Create an address for the peer server.
//     const string address = string(ip);

//     // Look for the first space and first \r\n in the request.
//     const int space_index = request.data.find(" ");
//     const int newline_index = request.data.find("\r\n");
//     if (space_index == string::npos || newline_index == string::npos) {
//         log(Status::ERROR,
//             "Invalid internal request from '" + address + "'. Request: '" + request.data + "'.");
//         return;
//     }

//     // Get the message ID and body.
//     const int message_id = std::stoi(request.data.substr(space_index + 1, newline_index));
//     const string body = request.data.substr(newline_index + 2);

//     // Create a pipe that will be used to send the response back to the
//     // requesting server with the message ID header.
//     int pipe_fds[2];
//     if (pipe(pipe_fds) == -1) {
//         log(Status::ERROR, "Failed to create pipe.");
//         return;
//     }

//     // Create a new mock request with the fd of the pipe and the content of the
//     // body of the request.
//     Request mock_request;
//     mock_request.fd = pipe_fds[1];

//     // Look for first space in the body.
//     int i;
//     for (i = 0; i < body.size() && !isspace(body[i]); i++)
//         ;

//     // Get the protocol and data from the body.
//     mock_request.protocol = body.substr(0, i);
//     mock_request.data = std::move(body);

//     // Set the address of the mock request to the address of the original
//     mock_request.addr = request.addr;

//     // Create a new thread that will forward read the response from the pipe, add the appropriate
//     // message ID header, and send the response back to the requesting server.
//     std::thread([this, pipe_fds, message_id, address]() {
//         // Read the response from the pipe.
//         char buffer[1024];

//         // Read the response from the pipe while there is data to read.
//         while (true) {
//             // Read the response from the pipe.
//             int bytes_read = read(pipe_fds[0], buffer, 1024);

//             // If there was an error reading from the pipe, log the error and return.
//             if (bytes_read == -1) {
//                 log(Status::ERROR, "Failed to read from pipe.");
//                 break;
//             }

//             // If there is no more data to read, break.
//             if (bytes_read == 0) {
//                 break;
//             }

//             // Add the message ID header to the response.
//             string response = SERVERCC_DISTRIBUTED_PROTOCOLS_INTERNAL_RESPONSE " " +
//                               std::to_string(message_id) + "\r\n" + string(buffer, bytes_read);

//             // Send the response back to the requesting server.
//             auto send_res = connector.send_message(address, response);
//             if (send_res.failed()) {
//                 log(Status::ERROR,
//                     "Failed to send response to server. " +
//                     std::string(send_res.status_message));
//                 break;
//             }
//         }

//         // Send a response end message to the requesting server.
//         auto send_res = connector.send_message(
//             address,
//             SERVERCC_DISTRIBUTED_PROTOCOLS_INTERNAL_RESPONSE_END " " +
//             std::to_string(message_id));
//         if (send_res.failed()) {
//             log(Status::ERROR,
//                 "Failed to send response end to server. " +
//                 std::string(send_res.status_message));
//         }

//         // Close the pipe.
//         close(pipe_fds[0]);
//         close(pipe_fds[1]);
//     }).detach();

//     // Forward the request to the protocol processors.
//     forwardRequestToHandler(std::move(mock_request));
// }

// // See distributed.h for documentation.
// void DistributedServer::handle_internal_response(const Request request) {
//     // Get the message ID from the request between the first space and the
//     // first \r\n.
//     const int space_index = request.data.find(" ");
//     const int newline_index = request.data.find("\r\n");
//     if (space_index == string::npos || newline_index == string::npos) {
//         log(Status::ERROR, "Received invalid response: " + request.data);
//         return;
//     }

//     // Get the message ID.
//     const uint64_t message_id = std::stoi(request.data.substr(space_index + 1, newline_index));

//     // Look up the message ID in the message ID map.
//     auto message_buffer = message_buffers.find(message_id);
//     if (message_buffer == message_buffers.end()) {
//         log(Status::ERROR,
//             "Received response for unknown message ID: " + std::to_string(message_id));
//         return;
//     }

//     // Extract the response from the request after the first \r\n.
//     const string response = request.data.substr(newline_index + 2);

//     // Add the response to the message queue.
//     message_buffer->second.get()->push(std::move(response));
// }

// // See distributed.h for documentation.
// void DistributedServer::handle_internal_response_end(const Request response) {
//     // Get the message ID after the first space.
//     const int space_index = response.data.find(" ");
//     if (space_index == string::npos) {
//         log(Status::ERROR, "Received invalid response end: " + response.data);
//         return;
//     }

//     // Get the message ID.
//     const uint64_t message_id = std::stoi(response.data.substr(space_index + 1));

//     // Look up the message ID in the message ID map.
//     auto message_buffer = message_buffers.find(message_id);
//     if (message_buffer == message_buffers.end()) {
//         log(Status::ERROR,
//             "Received response end for unknown message ID: " + std::to_string(message_id));
//         return;
//     }

//     // Set the message queue to done.
//     message_buffer->second.get()->close();
// }
