#include "connector.h"

#include <thread>

using ostp::servercc::connector::Connector;

/// See connector.h for documentation.
Connector::Connector(std::function<void(const ConnectorRequest)> default_processor)
    : processors(default_processor) {}

/// See connector.h for documentation.
Connector::~Connector() {}

/// See connector.h for documentation.
void Connector::add_processor(const std::string& path,
                              std::function<void(const ConnectorRequest)> processor) {
    processors.insert(path.c_str(), path.size(), processor);
}

/// See connector.h for documentation.
void Connector::add_client(TcpClient client) {
    // Start a thread that reads from the client and processes the requests.
    std::thread([this, client]() mutable {
        // While possible, read from the client.
        while (true) {
            StatusOr<std::string> read_result = client.receive_message();

            // If the client has disconnected, remove it from the map and
            // return.
            if (read_result.failed()) {
                return;
            }

            // Find the request type by looking for the first space.
            const size_t space_index = read_result.result.find(' ');

            // Create a request object and process it.
            ConnectorRequest request(client.get_fd(), read_result.result.substr(0, space_index),
                                     std::move(read_result.result));
        }
    }).detach();
}
