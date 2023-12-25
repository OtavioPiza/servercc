#include "distributed_server.h"

#include <arpa/inet.h>

#include <iostream>
#include <memory>
#include <random>
#include <thread>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "internal_channel_manager.h"

namespace ostp::servercc {

using ostp::libcc::data_structures::MessageBuffer;
using ostp::servercc::kMessageHeaderLength;

namespace {}  // namespace

// See distributed.h for documentation.
DistributedServer::DistributedServer(
    absl::string_view interfaceName, absl::string_view group,
    std::vector<absl::string_view> interfaces, const uint16_t port, handler_t default_handler,
    const std::function<void(in_addr_t, DistributedServer &server)> peerConnectCallback,
    const std::function<void(in_addr_t, DistributedServer &server)> peerDisconnectCallback)
    : interfaceName(interfaceName),  // TODO: allow multiple interfaces.
      interfaces(std::move(interfaces)),
      group(group),
      port(port),
      udpServer(port, group, this->interfaces,
                [this](std::unique_ptr<Request> request) -> absl::Status {
                    return this->forwardRequestToHandler(std::move(request));
                }),
      tcpServer(port,
                [this](std::unique_ptr<Request> request) -> absl::Status {
                    return this->forwardRequestToHandler(std::move(request));
                }),
      connector(
          [this](std::unique_ptr<Request> request) -> absl::Status {
              return this->forwardRequestToHandler(std::move(request));
          },
          [this](in_addr_t peerIp) { this->onConnectorDisconnect(peerIp); }),
      multicastClient(interfaceName, group, port, 1),  // TODO: Make TTL configurable.
      defaultHandler(defaultHandler),
      peerConnectCallback(peerConnectCallback),
      peerDisconnectCallback(peerDisconnectCallback) {
    // TODO define types and return stats from setting handlers.
    // Add the connect request handler to the UDP server.
    absl::Status status;
    if (!(status = udpServer.addHandler(kConnectRequestProtocol,
                                        [this](std::unique_ptr<Request> request) -> absl::Status {
                                            return this->handleConnect(std::move(request));
                                        }))
             .ok()) {
        LOG(FATAL) << "Failed to add connect request handler to UDP server: " << status.message();
    }

    // Add the connectAck request handler to the TCP server.
    if (!(status = tcpServer.addHandler(kConnectAckRequestProtocol,
                                        [this](std::unique_ptr<Request> request) -> absl::Status {
                                            return this->handleConnectAck(std::move(request));
                                        }))
             .ok()) {
        LOG(FATAL) << "Failed to add connectAck request handler to TCP server: "
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
    return absl::InternalError("Failed to connect to multicast group");
}

// See distributed.h for documentation.
absl::Status DistributedServer::addHandler(protocol_t protocol, handler_t handler) {
    // Check if the protocol is already registered.
    if (handlers.contains(protocol)) {
        return absl::AlreadyExistsError("Handler already exists for protocol");
    }
    handlers.insert({protocol, handler});
    return absl::OkStatus();
}

// See distributed.h for documentation.
absl::Status DistributedServer::multicastMessage(std::unique_ptr<Message> message) {
    if (!multicastClient.isOpen() && !multicastClient.openSocket().ok()) {
        return absl::InternalError("Failed to open socket");
    }
    return std::move(multicastClient.sendMessage(std::move(message)));
}

// See distributed.h for documentation.
absl::Status DistributedServer::sendConnectMessage() {
    auto message = std::make_unique<Message>();
    message->header.protocol = kConnectRequestProtocol;
    message->header.length = sizeof(uint16_t);
    message->body.data.resize(message->header.length);
    memcpy(message->body.data.data(), &port, message->header.length);
    return multicastMessage(std::move(message));
}

// See distributed.h for documentation.
std::pair<absl::Status, std::shared_ptr<connector_channel_manager_t::request_channel_t>>
DistributedServer::sendInternalRequest(in_addr_t address, std::unique_ptr<Message> message) {
    return connector.sendRequest(address, std::move(message));
}

// See distributed.h for documentation.
absl::Status DistributedServer::runTcpServer() {
    // TODO Create setup phase to catch errors early
    tcpServerThread = std::thread([this]() {
        LOG(INFO) << "Running TCP server";
        this->tcpServer.run();
    });
    return absl::OkStatus();
}

// See distributed.h for documentation.
absl::Status DistributedServer::runUdpServer() {
    // TODO create setup phase to catch errors early
    udpServerThread = std::thread([this]() {
        LOG(INFO) << "Running UDP server";
        this->udpServer.run();
    });
    return absl::OkStatus();
}

// See distributed.h for documentation.
void DistributedServer::onConnectorDisconnect(in_addr_t ip) {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip, ipStr, INET_ADDRSTRLEN);

    // Remove from the peers list.
    peers.erase(ip);

    // Call the user-specified disconnect callback.
    if (peerDisconnectCallback != nullptr) {
        peerDisconnectCallback(ip, *this);
    }
}

// See distributed.h for documentation.
absl::Status DistributedServer::forwardRequestToHandler(std::unique_ptr<Request> request) {
    auto handler = handlers.find(request->getProtocol());
    if (handler == handlers.end()) {
        return defaultHandler(std::move(request));

    } else {
        return handler->second(std::move(request));
    }
}

// See distributed.h for documentation.
absl::Status DistributedServer::handleConnect(std::unique_ptr<Request> request) {
    ASSERT_OK_AND_ASSIGN(message, request->receiveMessage(), "Failed to receive connect request");
    if (message->header.length != sizeof(uint16_t)) {
        return absl::InternalError("Invalid connect request length");
    }

    // Get the port and ip from the connect request.
    uint16_t peerPort;
    memcpy(&peerPort, message->body.data.data(), sizeof(uint16_t));

    // Get the IP address from the connect request address.
    auto addr = request->getAddr();
    in_addr_t peerIp = ((sockaddr_in *)&addr)->sin_addr.s_addr;
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peerIp, ipStr, INET_ADDRSTRLEN);

    // If the ip address is the same as the interface ip then ignore the
    // request or if the peer server is already connected.
    if (interfaces[0] == ipStr || peers.contains(peerIp)) {
        return absl::OkStatus();
    }

    // Create a TCP client for the peer server and try to connect to it.
    auto peerServer = std::make_unique<TcpClient>(ipStr, peerPort);
    auto openStatus = peerServer->openSocket();
    if (!openStatus.ok()) {
        close(peerServer->getClientFd());
        return absl::InternalError(absl::StrCat("Failed to open socket to peer server '", ipStr,
                                                "': ", openStatus.message()));
    }

    // Send a connectAck message to the peer server and wait for a connectAck.
    auto connectAckMessage = std::make_unique<Message>();
    connectAckMessage->header.protocol = kConnectAckRequestProtocol;
    connectAckMessage->header.length = 0;
    auto sendStatus = peerServer->sendMessage(std::move(connectAckMessage));
    if (!sendStatus.ok()) {
        close(peerServer->getClientFd());
        return absl::InternalError(absl::StrCat("Failed to send connectAck to peer server '", ipStr,
                                                "': ", sendStatus.message()));
    }

    // If the peer server did not send a connectAck then close the socket and return.
    auto [receiveStatus, ackResponse] = peerServer->receiveMessage();
    if (!receiveStatus.ok() || ackResponse->header.protocol != kConnectAckResponseProtocol) {
        close(peerServer->getClientFd());
        return absl::InternalError(absl::StrCat("Failed to receive connect end from peer server '",
                                                ipStr, "': ", receiveStatus.message()));
    }

    // Add the peer server to the connector and mappings.
    auto connectorStatus = connector.addClient(std::move(peerServer));
    if (!connectorStatus.ok()) {
        close(peerServer->getClientFd());
        return absl::InternalError(absl::StrCat("Failed to add peer server '", ipStr,
                                                "' to connector: ", connectorStatus.message()));
    }

    // Call the peer connect callback.
    if (peerConnectCallback != nullptr) {
        peerConnectCallback(peerIp, *this);
    }
    return absl::OkStatus();
}

// See distributed.h for documentation.
absl::Status DistributedServer::handleConnectAck(std::unique_ptr<Request> request) {
    // Get the port and ip from the connect request.
    sockaddr addr = request->getAddr();
    auto *addr_in = (sockaddr_in *)&addr;
    uint16_t peerPort = ntohs(addr_in->sin_port);
    in_addr_t peerIp = addr_in->sin_addr.s_addr;
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peerIp, ipStr, INET_ADDRSTRLEN);

    // Add the peer server to the connector and mappings.
    ASSERT_OK_AND_ASSIGN(message, request->receiveMessage(),
                         "Failed to receive connectAck request");

    // Send connectAck to the peer server.
    auto connectAckMessage = std::make_unique<Message>();
    connectAckMessage->header.protocol = kConnectAckResponseProtocol;
    connectAckMessage->header.length = 0;
    ASSERT_OK(request->sendMessage(std::move(connectAckMessage)),
              "Failed to send connectAck to peer server");

    // Try to cast request to TcpRequest.
    auto tcpRequest = std::unique_ptr<TcpRequest>(dynamic_cast<TcpRequest *>(request.release()));
    if (tcpRequest == nullptr) {
        return absl::InternalError("Failed to cast request to TcpRequest");
    }

    // Add the peer server to the connector and mappings.
    auto connectorStatus = connector.addClient(
        std::make_unique<TcpClient>(tcpRequest->setKeepAlive(), ipStr, peerPort, addr));
    if (!connectorStatus.ok()) {
        LOG(ERROR) << "Failed to add peer server '" << ipStr
                   << "' to connector: " << connectorStatus.message();
        close(tcpRequest->setKeepAlive());
        return std::move(connectorStatus);
    }

    // Call the peer connect callback.
    if (peerConnectCallback != nullptr) {
        peerConnectCallback(peerIp, *this);
    }
    return absl::OkStatus();
}

}  // namespace ostp::servercc
