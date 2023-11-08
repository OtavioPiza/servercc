#include "connector.h"

namespace ostp::servercc {

// Constructors.

// See connector.h for documentation.
Connector::Connector(handler_t defaultHandler,
                     std::function<void(absl::string_view)> disconnectCallback)
    : defaultHandler(defaultHandler), disconnectCallback(disconnectCallback) {}

// See connector.h for documentation.
Connector::~Connector() {}

// Public methods.

// See connector.h for documentation.
absl::Status Connector::addHandler(protocol_t protocol, handler_t handler) {
    if (handlers.contains(protocol)) {
        return absl::AlreadyExistsError("Handler already exists for protocol.");
    }
    handlers.emplace(protocol, handler);
    return absl::OkStatus();
}

// See connector.h for documentation.
absl::Status Connector::addClient(std::unique_ptr<TcpClient> client) {
    // Open the socket and run the client.
    if (!client->isOpen() && !client->openSocket().ok()) {
        return absl::InternalError("Failed to open socket.");
    }
    auto address = client->getAddress();
    clients.emplace(client->getAddress(), std::move(client));
    return runClient(address);
}

// See connector.h for documentation.
absl::Status Connector::sendMessage(absl::string_view address, std::unique_ptr<Message> message) {
    // Check if the client exists.
    auto client = clients.find(address);
    if (client == clients.end()) {
        return absl::NotFoundError("Client does not exist.");
    }
    return client->second->sendMessage(std::move(message));
}

// Private methods.

// See connector.h for documentation.
absl::Status Connector::runClient(absl::string_view address) {
    // Check if the client exists.
    auto clientIt = clients.find(address);
    if (clientIt == clients.end()) {
        return absl::NotFoundError("Client does not exist.");
    }
    auto client = clientIt->second;

    // Run the client.
    std::thread clientThread([address, client, this]() {
        // Enter a read loop.
        while (true) {
            // Read the request.
            auto [status, message] = client->receiveMessage();
            if (!status.ok()) {
                if (!client->closeSocket().ok()) {
                    // TODO: Log error.
                    perror("Failed to close socket.");
                }
                clients.erase(address);
                disconnectCallback(address);
                break;
            }

            // Get the processor for the request.
            auto request = std::make_unique<Request>();
            request->addr = client->getClientAddr();
            request->fd = client->getClientFd();
            request->message = std::move(message);

            // Process the request.
            auto handlerIt = handlers.find(request->message->header.protocol);
            if (handlerIt == handlers.end()) {
                defaultHandler(std::move(request));
            } else {
                handlerIt->second(std::move(request));
            }
        }
    });

    // Detach the thread.
    clientThread.detach();
    return absl::OkStatus();
}

}  // namespace ostp::servercc
