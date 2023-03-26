#include "connector.h"

#include <iostream>
#include <thread>

using ostp::servercc::Request;
using ostp::servercc::connector::Connector;

/// See connector.h for documentation.
Connector::Connector(const std::function<void(const Request)> default_processor,
                     const std::function<void(int)> disconnect_handler)
    : processors(default_processor), disconnect_handler(disconnect_handler) {}

/// See connector.h for documentation.
Connector::~Connector() {}

/// See connector.h for documentation.
void Connector::add_processor(const std::string& path,
                              std::function<void(const Request)> processor) {
    processors.insert(path.c_str(), path.size(), processor);
}

/// See connector.h for documentation.
int Connector::add_client(TcpClient client) {
    client.open_socket();
    clients.insert({client.get_fd(), std::move(client)});
    run_client(client.get_fd());
    return client.get_fd();
}

/// See connector.h for documentation.
void Connector::run_client(int fd) {
    // Get the client from the map.
    TcpClient& client = clients.at(fd);

    // Run the client.
    std::thread client_thread([&client, this]() {
        // Enter a read loop.
        while (true) {
            // Read the request.
            StatusOr<std::string> message = client.receive_message();

            // Check if the client disconnected.
            if (message.failed()) {
                // Remove the client from the map.
                clients.erase(client.get_fd());

                // Call the disconnect handler.
                disconnect_handler(client.get_fd());

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

    // Add the client thread to the map.
    client_threads.insert({fd, std::move(client_thread)});
}

/// See connector.h for documentation.
void Connector::send_message(int fd, const std::string& message) {
    // Get the client from the map.
    TcpClient& client = clients.at(fd);

    // Send the message.
    client.send_message(message);
}
