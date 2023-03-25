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
            StatusOr<std::string> request = client.receive_message();

            // Check if the client disconnected.
            if (request.failed()) {
                // Remove the client from the map.
                clients.erase(client.get_fd());

                // Call the disconnect handler.
                disconnect_handler(client.get_fd());

                // Exit the loop.
                break;
            }

            // Get the processor for the request.
            int space_index = request.result.find(' ');
            std::string type = request.result.substr(0, space_index);

            // Echo the request.
            std::cout << "Request in " << client.get_fd() << ": " << request.result << std::endl;

            // Create the connector request.
            // Process the request.
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
