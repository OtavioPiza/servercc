#include "connector.h"

using ostp::libcc::utils::Status;
using ostp::libcc::utils::StatusOr;
using ostp::servercc::Request;
using ostp::servercc::connector::Connector;
using std::function;
using std::string;

// Constructors.

/// See connector.h for documentation.
Connector::Connector(const function<void(const Request)> default_processor,
                     const function<void(const string)> disconnect_handler)
    : processors(default_processor), disconnect_handler(disconnect_handler) {}

/// See connector.h for documentation.
Connector::~Connector() {}

// Public methods.

/// See connector.h for documentation.
void Connector::add_processor(const std::string& path, function<void(const Request)> processor) {
    processors.insert(path.c_str(), path.size(), processor);
}

/// See connector.h for documentation.
StatusOr<string> Connector::add_client(TcpClient client) {
    const string address = client.get_address() + ":" + std::to_string(client.get_port());

    // Open the socket and run the client.
    if (client.open_socket().failed()) {
        return StatusOr(Status::ERROR, "Failed to open socket.", std::move(address));
    }

    // Add the client to the map and run it.
    clients.insert({address, std::move(client)});
    run_client(address);

    // Return the address.
    return StatusOr(Status::OK, "Successfully added client.", std::move(address));
}

/// See connector.h for documentation.
StatusOr<int> Connector::send_message(const string& address, const std::string& message) {
    // Check if the client exists.
    auto client = clients.find(address);
    if (client == clients.end()) {
        return StatusOr(Status::ERROR, "Client does not exist.", 0);
    }

    // Send the message.
    return std::move(client->second.send_message(message));
}

// Private methods.

/// See connector.h for documentation.
void Connector::run_client(const string address) {
    // Run the client.
    std::thread client_thread([address, this]() {
        TcpClient& client = clients.at(address);

        // Enter a read loop.
        while (true) {
            // Read the request.
            StatusOr message = client.receive_message();
            if (message.failed()) {
                // Remove the client from the map.
                clients.erase(address);

                // Call the disconnect handler.
                disconnect_handler(address);

                // Close the socket.
                client.close_socket();

                // Exit the loop.
                break;
            }

            // Get the processor for the request.
            int i;
            for (i = 0; i < message.result.size() && !isspace(message.result[i]); i++)
                ;

            // Create the connector request.
            Request request(client.get_fd(), client.get_addr());
            request.data = message.result;
            request.protocol = std::string(&message.result[0], i);
            request.addr = client.get_addr();

            // Process the request.
            processors.get(&message.result[0], i)(std::move(request));
        }
    });

    // Detach the thread.
    client_thread.detach();
}
