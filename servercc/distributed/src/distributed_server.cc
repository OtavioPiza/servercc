#include "distributed_server.h"

#include <arpa/inet.h>

#include <iostream>
#include <memory>
#include <random>
#include <thread>

#include "absl/base/log_severity.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

namespace ostp::servercc {

using ostp::libcc::data_structures::MessageBuffer;
using ostp::servercc::kMessageHeaderLength;

namespace {



}  // namespace

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
      peerConnectCallback(peerConnectCallback),
      peerDisconnectCallback(peerDisconnectCallback) {
    absl::SetStderrThreshold(absl::LogSeverity::kInfo);  // TODO write to file instead of stderr.
    absl::InitializeLog();

    // TODO define types and return stats from setting handlers.
    // Add the connect request handler to the UDP server.
    absl::Status status;
    if (!(status = udpServer.addHandler(0x10, [this](std::unique_ptr<Request> request) {
             this->handleConnect(std::move(request));
         })).ok()) {
        LOG(FATAL) << "Failed to add connect request handler to UDP server: " << status.message();
    }

    // Add the connectAck request handler to the TCP server.
    if (!(status = tcpServer.addHandler(0x11, [this](std::unique_ptr<Request> request) {
             this->handleConnectAck(std::move(request));
         })).ok()) {
        LOG(FATAL) << "Failed to add connectAck request handler to TCP server: "
                   << status.message();
    }

    // Add the internal request handler to the connector.
    if (!(status = connector.addHandler(0x12, [this](std::unique_ptr<Request> request) {
             this->handleInternalRequest(std::move(request));
         })).ok()) {
        LOG(FATAL) << "Failed to add internal request handler to connector: " << status.message();
    }

    // Add the internal response handler to the connector.
    if (!(status = connector.addHandler(0x13, [this](std::unique_ptr<Request> request) {
             this->handleInternalResponse(std::move(request));
         })).ok()) {
        LOG(FATAL) << "Failed to add internal response handler to connector: " << status.message();
    }

    // Add the internal response end handler to the connector.
    if (!(status = connector.addHandler(0x14, [this](std::unique_ptr<Request> request) {
             this->handleInternalResponseEnd(std::move(request));
         })).ok()) {
        LOG(FATAL) << "Failed to add internal response end handler to connector: "
                   << status.message();
    }
}

// See distributed.h for documentation.
absl::Status DistributedServer::run() {
    // Run the services.
    absl::Status status;
    if (!(status = runTcpServer()).ok()) {
        LOG(ERROR) << "Failed to run TCP server: " << status.message();
        return status;
    }
    if (!(status = runUdpServer()).ok()) {
        LOG(ERROR) << "Failed to run UDP server: " << status.message();
        return status;
    }

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
absl::Status DistributedServer::multicastMessage(std::unique_ptr<Message> message) {
    if (!multicastClient.isOpen() && !multicastClient.openSocket().ok()) {
        return absl::InternalError("Failed to open socket.");
    }
    return std::move(multicastClient.sendMessage(std::move(message)));
}

// See distributed.h for documentation.
absl::Status DistributedServer::sendConnectMessage() {
    auto message = std::make_unique<Message>();
    message->header.protocol = 0x10;
    message->header.length = sizeof(uint16_t);
    message->body.data.resize(message->header.length);
    memcpy(message->body.data.data(), &port, message->header.length);
    return multicastMessage(std::move(message));
}

// See distributed.h for documentation.
std::pair<absl::Status, uint32_t> DistributedServer::sendInternalRequest(
    absl::string_view address, std::unique_ptr<Message> message) {
    // Generate a new random request id.
    uint32_t id;
    while (messageBuffers.contains(id = std::rand()))
        ;

    // Add the ID to the message queue.
    auto messageBuffer = std::make_shared<MessageBuffer<std::unique_ptr<Message>>>();
    messageBuffers.insert({id, messageBuffer});

    // Send the request to the peer.
    const auto result = connector.sendMessage(address, wrapMessage(0x12, id, std::move(message)));
    message = nullptr;
    if (!result.ok()) {
        // Remove the ID from the message queue.
        messageBuffers.erase(id);
        return {result, -1};
    }

    // Add the message ID to the peers_messageIds map.
    messageIdsToPeers.insert({id, address});

    // Add the message ID to the peers_messageIds map.
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
std::pair<absl::Status, std::unique_ptr<Message>> DistributedServer::receiveInternalMessage(
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

        // Erase the message ID from the peers_to_messageIds and remove the peer if it is empty.
        auto messageIds_it = peersToMessageIds.find(messageIdsToPeers[id]);
        if (messageIds_it != peersToMessageIds.end()) {
            messageIds_it->second.erase(id);
            if (messageIds_it->second.empty()) {
                peersToMessageIds.erase(messageIds_it);
            }
        }

        // Erase the message ID from the messageIds_to_peers map.
        messageIdsToPeers.erase(id);
    }

    // Return the message.
    return std::move(res);
}

// See distributed.h for documentation.
absl::Status DistributedServer::runTcpServer() {
    // TODO Create setup phase to catch errors early
    tcpServerThread = std::thread([this]() {
        LOG(INFO) << "Running TCP server.";
        this->tcpServer.run();
    });
    return absl::OkStatus();
}

// See distributed.h for documentation.
absl::Status DistributedServer::runUdpServer() {
    // TODO create setup phase to catch errors early
    udpServerThread = std::thread([this]() {
        LOG(INFO) << "Running UDP server.";
        this->udpServer.run();
    });
    return absl::OkStatus();
}

// See distributed.h for documentation.
void DistributedServer::onConnectorDisconnect(absl::string_view ip) {
    LOG(INFO) << "Peer server '" << ip << "' disconnected.";

    // Look for pending messages from the disconnected peer.
    auto messageIdsIt = peersToMessageIds.find(ip);
    if (messageIdsIt != peersToMessageIds.end()) {
        LOG(WARNING) << "Peer server '" << ip << "' disconnected with pending messages.";

        // Close the message queues of the pending messages remove the message IDs from the
        // messageIds_to_peers map.
        for (const auto id : messageIdsIt->second) {
            messageBuffers[id]->close();
        }
    }

    // Remove from the peers list.
    peers.erase(ip);

    // Call the user-specified disconnect callback.
    if (peerDisconnectCallback != nullptr) {
        peerDisconnectCallback(ip, *this);
    }
}

// See distributed.h for documentation.
void DistributedServer::handleConnect(std::unique_ptr<Request> request) {
    // Get the port and ip from the connect request.
    uint16_t peerPort;
    memcpy(&peerPort, request->message->body.data.data(), sizeof(uint16_t));

    // Get the IP address from the connect request address.
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((sockaddr_in *)&request->addr)->sin_addr, ip, INET_ADDRSTRLEN);

    // Log the connect request.
    LOG(INFO) << "Received connect request from peer server '" << ip << "' on port "
              << std::to_string(peerPort) << ".";

    // If the ip address is the same as the interface ip then ignore the
    // request or if the peer server is already connected.
    if (interfaces[0] == ip || peers.contains(ip)) {
        LOG(INFO) << "Ignoring connect request from peer server '" << ip << "'.";
        return;
    }

    // Create a TCP client for the peer server and try to connect to it.
    auto peerServer = std::make_unique<TcpClient>(ip, peerPort);
    if (!peerServer->openSocket().ok()) {
        LOG(ERROR) << "Failed to open socket to peer server '" << ip << "'.";
        // Close socket and return.
        close(peerServer->getClientFd());
        return;
    }

    LOG(INFO) << "Opened socket to peer server '" << ip << "'.";

    // Send a connectAck message to the peer server and wait for a connectAck.
    auto connectAckMessage = std::make_unique<Message>();
    connectAckMessage->header.protocol = 0x11;
    connectAckMessage->header.length = 0;
    if (!peerServer->sendMessage(std::move(connectAckMessage)).ok()) {
        // Close socket and return.
        LOG(ERROR) << "Failed to send connectAck to peer server '" << ip << "'.";
        close(peerServer->getClientFd());
        return;
    }
    connectAckMessage = nullptr;

    LOG(INFO) << "Sent connectAck to peer server '" << ip << "'.";

    // If the peer server did not send a connectAck then close the socket and return.
    auto [status, ackResponse] = peerServer->receiveMessage();
    if (!status.ok() || ackResponse->header.protocol != 0x11) {
        LOG(ERROR) << "Failed to receive connectAck from peer server '" << ip << "'.";
        close(peerServer->getClientFd());
        return;
    }
    ackResponse = nullptr;

    LOG(INFO) << "Received connectAck from peer server '" << ip << "'.";

    // Add the peer server to the connector and mappings.
    if (!connector.addClient(std::move(peerServer)).ok()) {
        LOG(ERROR) << "Failed to add peer server '" << ip << "' to connector.";
        close(peerServer->getClientFd());
        return;
    }
    peerServer = nullptr;

    LOG(INFO) << "Added peer server '" << ip << "' to connector.";

    // Call the peer connect callback.
    if (peerConnectCallback != nullptr) {
        peerConnectCallback(ip, *this);
    }

    // Return and close the socket.
    return;
}

// See distributed.h for documentation.
void DistributedServer::handleConnectAck(std::unique_ptr<Request> request) {
    // Get the port and ip from the connect request.
    auto *addr = (sockaddr_in *)&request->addr;
    auto peerPort = ntohs(addr->sin_port);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    // Log the connectAck request.
    LOG(INFO) << "Received connectAck from peer server '" << ip << "' on port "
              << std::to_string(peerPort) << ".";

    // Add the peer server to the connector and mappings.
    if (!connector.addClient(std::make_unique<TcpClient>(request->fd, ip, peerPort, request->addr))
             .ok()) {
        LOG(ERROR) << "Failed to add peer server '" << ip << "' to connector.";
        close(request->fd);
        return;
    }

    LOG(INFO) << "Added peer server '" << ip << "' to connector.";

    // Send connectAck to the peer server.
    auto connectAckMessage = std::make_unique<Message>();
    connectAckMessage->header.protocol = 0x11;
    connectAckMessage->header.length = 0;
    if (!connector.sendMessage(ip, std::move(connectAckMessage)).ok()) {
        LOG(ERROR) << "Failed to send connectAck to peer server '" << ip << "'.";
        close(request->fd);
        return;
    }
    connectAckMessage = nullptr;

    LOG(INFO) << "Sent connectAck to peer server '" << ip << "'.";

    // Call the peer connect callback.
    if (peerConnectCallback != nullptr) {
        peerConnectCallback(ip, *this);
    }

    // Return without closing the socket as it is now owned by the connector.
    return;
}

// See distributed.h for documentation.
void DistributedServer::forwardRequestToHandler(std::unique_ptr<Request> request) {
    auto handler = handlers.find(request->message->header.protocol);
    if (handler == handlers.end()) {
        defaultHandler(std::move(request));

    } else {
        handler->second(std::move(request));
    }
}

// See distributed.h for documentation.
void DistributedServer::handleInternalRequest(std::unique_ptr<Request> request) {
    // Get the port and ip from the connect request.
    auto *addr = (sockaddr_in *)&request->addr;
    auto port = ntohs(addr->sin_port);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    // Get the message ID from the end of the message body.
    uint32_t messageId;
    auto offset = request->message->header.length - sizeof(uint32_t);
    memcpy(&messageId, request->message->body.data.data() + offset, sizeof(uint32_t));

    // Create a pipe that will be used to send the response back to the
    // requesting server with the message ID header.
    int pipeFds[2];
    if (pipe(pipeFds) == -1) {
        LOG(ERROR) << "Failed to create pipe.";
        return;
    }

    // Create a new mock request with the fd of the pipe and the content of the
    // body of the request.
    request->fd = pipeFds[1];

    // Copy header from the end of the body to the header.
    offset -= kMessageHeaderLength;
    memcpy(&request->message->header, request->message->body.data.data() + offset,
           kMessageHeaderLength);
    request->message->body.data.erase(request->message->body.data.begin() + offset,
                                      request->message->body.data.end());

    // Create a new thread that will forward read the response from the pipe, add the appropriate
    // message ID header, and send the response back to the requesting server.
    std::thread([this, pipeFds, messageId, ip]() {
        // Read the response from the pipe while there is data to read.
        while (true) {
            // Read the message from the pipe.
            auto [status, message] = readMessage(pipeFds[0]);
            if (!status.ok()) {
                LOG(ERROR) << "Failed to read message from pipe.";
                break;
            }

            // Send the response back to the requesting server.
            if (!connector
                     .sendMessage(ip, std::move(wrapMessage(0x13, messageId, std::move(message))))
                     .ok()) {
                LOG(ERROR) << "Failed to send response to server '" << ip << "'.";
                break;
            }
            message = nullptr;
        }

        // Send a response end message to the requesting server.
        auto responseEnd = std::make_unique<Message>();
        responseEnd->header.protocol = 0x14;
        responseEnd->header.length = sizeof(uint32_t);
        responseEnd->body.data.resize(responseEnd->header.length);
        memcpy(responseEnd->body.data.data(), &messageId, sizeof(uint32_t));
        if (!connector.sendMessage(ip, std::move(responseEnd)).ok()) {
            LOG(ERROR) << "Failed to send response end to server '" << ip << "'.";
        }
        responseEnd = nullptr;

        // Close the pipe.
        close(pipeFds[0]);
        close(pipeFds[1]);
    }).detach();

    // Forward the request to the protocol processors.
    forwardRequestToHandler(std::move(request));
}

// See distributed.h for documentation.
void DistributedServer::handleInternalResponse(std::unique_ptr<Request> request) {
    // Unwrap the message.
    auto [messageId, message] = unwrapMessage(std::move(request->message));
    request->message = nullptr;

    // Look up the message ID in the message ID map.
    auto messageBuffer = messageBuffers.find(messageId);
    if (messageBuffer == messageBuffers.end()) {
        LOG(ERROR) << "Received response for unknown message ID: " << std::to_string(messageId);
        return;
    }

    // Add the response to the message queue.
    if (!messageBuffer->second.get()->push(std::move(message)).ok()) {
        LOG(ERROR) << "Message queue full for message ID: " << std::to_string(messageId);
    }
    // TODO send response end message to peer.
}

// See distributed.h for documentation.
void DistributedServer::handleInternalResponseEnd(std::unique_ptr<Request> request) {
    // Get the message ID after the first space.
    uint32_t messageId;
    memcpy(&messageId, request->message->body.data.data(), sizeof(uint32_t));

    // Look up the message ID in the message ID map.
    auto messageBuffer = messageBuffers.find(messageId);
    if (messageBuffer == messageBuffers.end()) {
        LOG(ERROR) << "Received response end for unknown message ID: " << std::to_string(messageId);
        return;
    }
    // Set the message queue to done.
    messageBuffer->second.get()->close();
}

}  // namespace ostp::servercc
