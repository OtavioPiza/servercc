#ifndef SERVERCC_CONNECTOR_H
#define SERVERCC_CONNECTOR_H

#include <functional>
#include <thread>
#include <unordered_map>

#include "default_trie.h"
#include "request.h"
#include "tcp_client.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::servercc::Request;
using ostp::servercc::client::TcpClient;
using std::unordered_map;

namespace ostp::servercc::connector {

/// A TCP connection manager that.
class Connector {
   private:
    /// Default trie for processing requests.
    DefaultTrie<char, std::function<void(const Request)>> processors;

    /// Handler for when a client disconnects.
    const std::function<void(int)> disconnect_handler;

    /// A map of the current TCP clients identified by their socket file
    /// descriptor.
    unordered_map<int, TcpClient> clients;

    /// Runs the specified client in a thread.
    ///
    /// Arguments:
    ///     fd: The file descriptor of the client.
    void run_client(int fd);

   public:
    // Constructors

    /// Constructs a new connector with the specified default processor and disconnect handler.
    ///
    /// Arguments:
    ///     default_processor: The default processor to use.
    ///     disconnect_handler: The handler to use when a client disconnects.
    Connector(const std::function<void(const Request)> default_processor,
              const std::function<void(int)> disconnect_handler);

    // Destructor
    ~Connector();

    // Methods

    /// Adds the specified processor for the specified path.
    ///
    /// Arguments:
    ///     path: The path to add the processor for.
    ///     processor: The processor to add.
    void add_processor(const std::string& path, std::function<void(const Request)> processor);

    /// Adds a TCP client to the connector.
    ///
    /// Arguments:
    ///     client: The client to add.
    /// Returns:
    ///     The file descriptor of the client.
    int add_client(TcpClient client);

    /// Send a message through the specified client.
    ///
    /// Arguments:
    ///     fd: The file descriptor of the client.
    ///     message: The message to send.
    void send_message(int fd, const std::string& message);
};

}  // namespace ostp::servercc::connector

#endif
