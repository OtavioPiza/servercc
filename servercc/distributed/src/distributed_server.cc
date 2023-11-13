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
#include "internal_channel_manager.h"

namespace ostp::servercc {

using ostp::libcc::data_structures::MessageBuffer;
using ostp::servercc::kMessageHeaderLength;

namespace {}  // namespace

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
std::pair<absl::Status, channel_id_t> DistributedServer::sendInternalRequest(
    absl::string_view address, std::unique_ptr<Message> message) {}

// See distributed.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> DistributedServer::receiveInternalMessage(
    const channel_id_t id) {}

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
    // auto messageIdsIt = peersToMessageIds.find(ip);
    // if (messageIdsIt != peersToMessageIds.end()) {
    //     LOG(WARNING) << "Peer server '" << ip << "' disconnected with pending messages.";

    //     // Close the message queues of the pending messages remove the message IDs from the
    //     // messageIds_to_peers map.
    //     for (const auto id : messageIdsIt->second) {
    //         messageBuffers[id]->close();
    //     }
    // }

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
void DistributedServer::handleInternalRequest(std::unique_ptr<Request> request) {}

// See distributed.h for documentation.
void DistributedServer::handleInternalResponse(std::unique_ptr<Request> request) {}

// See distributed.h for documentation.
void DistributedServer::handleInternalResponseEnd(std::unique_ptr<Request> request) {}

}  // namespace ostp::servercc
