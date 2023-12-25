#include "connector.h"

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "internal_request.h"

namespace ostp::servercc {

// Constructors.

// See connector.h for documentation.
Connector::Connector(handler_t defaultHandler, std::function<void(in_addr_t)> disconnectCallback)
    : defaultHandler(defaultHandler), disconnectCallback(disconnectCallback) {}

// See connector.h for documentation.
Connector::~Connector() {}

// Public methods.

// See connector.h for documentation.
absl::Status Connector::addHandler(protocol_t protocol, handler_t handler) {
    handlersMutex.lock();
    if (handlers.contains(protocol)) {
        handlersMutex.unlock();
        return absl::AlreadyExistsError("Handler already exists for protocol");
    }
    handlers.emplace(protocol, handler);
    handlersMutex.unlock();
    return absl::OkStatus();
}

// See connector.h for documentation.
absl::Status Connector::addClient(std::unique_ptr<TcpClient> client) {
    ASSERT_OK(client->openSocket(), "Failed to open socket for client");
    auto address = client->getClientInAddr();
    auto writeMutex = std::make_shared<std::mutex>();
    auto channelManager =
        std::make_shared<connector_channel_manager_t>(client->getClientFd(), writeMutex);

    clientsMutex.lock();
    if (clients.contains(address)) {
        clientsMutex.unlock();
        return absl::AlreadyExistsError("Client already exists");
    }
    clients.emplace(address, InternalClient(std::move(client), writeMutex, channelManager));
    client = nullptr;
    clientsMutex.unlock();

    // Run the client.
    return runClient(address);
}

// See connector.h for documentation.
std::pair<absl::Status, std::shared_ptr<connector_channel_manager_t::request_channel_t>>
Connector::sendRequest(in_addr_t address, std::unique_ptr<Message> message) {
    // Check if the client exists.
    clientsMutex.lock();
    auto clientIt = clients.find(address);
    if (clientIt == clients.end()) {
        clientsMutex.unlock();
        return {absl::NotFoundError("Client does not exist"), nullptr};
    }
    clientsMutex.unlock();

    // Open the channel and send the message.
    auto [status, channel] = clientIt->second.channelManager->createRequestChannel();
    if (!status.ok()) {
        return {status, nullptr};
    }
    return {channel->write(std::move(message)), channel};
}

// Private methods.

// See connector.h for documentation.
absl::Status Connector::runClient(in_addr_t address) {
    // Check if the client exists.
    clientsMutex.lock();
    auto clientIt = clients.find(address);
    if (clientIt == clients.end()) {
        clientsMutex.unlock();
        return absl::NotFoundError("Client does not exist");
    }
    clientsMutex.unlock();
    auto client = clientIt->second.client;
    auto channelManager = clientIt->second.channelManager;

    // Run the client.
    std::thread clientThread([address, client, channelManager, this]() {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address, ipStr, INET_ADDRSTRLEN);

        // Enter a read loop.
        while (true) {
            // Read the request.
            auto [rcvStatus, message] = client->receiveMessage();
            if (!rcvStatus.ok()) {
                LOG(ERROR) << "Failed to receive message from client '" << ipStr << "': "
                           << rcvStatus.message();

                if (!(rcvStatus = client->closeSocket()).ok()) {
                    LOG(ERROR) << "Failed to close socket for client '" << ipStr << "': "
                               << rcvStatus.message();
                }
                clientsMutex.lock();
                clients.erase(address);
                clientsMutex.unlock();
                disconnectCallback(address);
                break;
            }

            // If the request is internal, forward it to the appropriate channel.
            auto [fwdStatus, fwdProtocol, fwdChannel] =
                channelManager->forwardMessage(std::move(message));
            if (!fwdStatus.ok()) {
                LOG(ERROR) << "Failed to forward message from client '" << ipStr << "': "
                           << fwdStatus.message();
            }

            // If a new channel was created, create a new request with the channel ID and
            // start a new thread to process it.
            if (fwdChannel != nullptr) {
                auto request = std::make_unique<connector_request_t>(
                    fwdProtocol, client->getClientAddr(), fwdChannel);

                // Process the request.
                auto handlerIt = handlers.find(fwdProtocol);
                if (handlerIt != handlers.end()) {
                    std::thread(handlerIt->second, std::move(request)).detach();
                } else {
                    std::thread(defaultHandler, std::move(request)).detach();
                }
            }
        }
    });

    // Detach the thread.
    clientThread.detach();
    return absl::OkStatus();
}

}  // namespace ostp::servercc
