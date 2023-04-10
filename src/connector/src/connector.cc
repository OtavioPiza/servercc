#include "connector.h"

using ostp::libcc::utils::Status;
using ostp::libcc::utils::StatusOr;
using ostp::servercc::Request;
using ostp::servercc::connector::Connector;
using std::string;

/// See connector.h for documentation.
Connector::Connector(const std::function<void(const Request)> default_processor,
                     const std::function<void(const string)> disconnect_handler)
    : processors(default_processor), disconnect_handler(disconnect_handler) {}

/// See connector.h for documentation.
Connector::~Connector() {}

/// See connector.h for documentation.
void Connector::add_processor(const std::string& path,
                              std::function<void(const Request)> processor) {
    processors.insert(path.c_str(), path.size(), processor);
}

/// See connector.h for documentation.
string Connector::add_client(TcpClient client) {
    // Open the socket and run the client.
    client.open_socket();
    run_client(client);

    // Add the client to the map and return the address.
    const string address = client.get_address() + ":" + std::to_string(client.get_port());
    clients.insert({address, std::move(client)});
    return std::move(address);
}

/// See connector.h for documentation.
void Connector::run_client(TcpClient& client) {
    // Run the client.
    std::thread client_thread([&client, this]() {
        // Enter a read loop.
        while (true) {
            // Read the request.
            StatusOr<std::string> message = client.receive_message();

            // Check if the client disconnected.
            if (message.failed()) {
                // Remove the client from the map.
                clients.erase(client.get_address() + ":" + std::to_string(client.get_port()));

                // Call the disconnect handler.
                disconnect_handler(client.get_address());

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

            // Process the request.
            processors.get(&message.result[0], i)(std::move(request));
        }
    });

    // Detach the thread.
    client_thread.detach();
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
