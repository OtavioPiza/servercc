#include "connector.h"

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
        return absl::AlreadyExistsError("Handler already exists for protocol.");
    }
    handlers.emplace(protocol, handler);
    handlersMutex.unlock();
    return absl::OkStatus();
}

// See connector.h for documentation.
absl::Status Connector::addClient(std::unique_ptr<TcpClient> client) {
    // Open the socket and run the client.
    if (!client->isOpen() && !client->openSocket().ok()) {
        clientsMutex.unlock();
        return absl::InternalError("Failed to open socket.");
    }
    auto address = client->getClientInAddr();
    auto writeMutex = std::make_shared<std::mutex>();

    clientsMutex.lock();
    clients.emplace(address, InternalClient{
                                 .client = std::move(client),
                                 .channelManager = std::make_shared<connector_channel_manager_t>(
                                     client->getClientFd(), writeMutex),
                                 .writeMutex = writeMutex,
                             });
    clientsMutex.unlock();

    writeMutex = nullptr;

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
    auto client = clientIt->second;
    clientsMutex.unlock();

    // Open the channel and send the message.
    auto [status, channel] = client.channelManager->createRequestChannel();
    if (!status.ok()) {
        return {status, nullptr};
    }
    return {channel->write(std::move(message)), channel};
}

// Private methods.

// See connector.h for documentation.
absl::Status Connector::runClient(in_addr_t address) {
    // Check if the client exists.
    auto clientIt = clients.find(address);
    if (clientIt == clients.end()) {
        return absl::NotFoundError("Client does not exist.");
    }
    auto client = clientIt->second.client;
    auto channelManager = clientIt->second.channelManager;

    // Run the client.
    std::thread clientThread([address, client, channelManager, this]() {
        // Enter a read loop.
        while (true) {
            // Read the request.
            auto [status, message] = client->receiveMessage();
            if (!status.ok()) {
                LOG(ERROR) << "Failed to receive message from client " << address << ": "
                           << status.message();

                if (!(status = client->closeSocket()).ok()) {
                    LOG(ERROR) << "Failed to close socket for client " << address << ": "
                               << status.message();
                }
                clients.erase(address);
                disconnectCallback(address);
                break;
            }

            // If the request is internal, forward it to the appropriate channel.
            auto [fStatus, fProtocol, fChannel] =
                channelManager->forwardMessage(std::move(message));
            if (!fStatus.ok()) {
                LOG(ERROR) << "Failed to forward message from client " << address << ": "
                           << status.message();
            }

            // If a new channel was created, create a new request with the channel ID and
            // start a new thread to process it.
            if (fChannel != nullptr) {
                auto request = std::make_unique<Request>();
                request->channel = fChannel;
                request->addr = client->getClientAddr();
                request->fd = -1;

                // Process the request.
                auto handlerIt = handlers.find(protocol);
                if (handlerIt == handlers.end()) {
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
