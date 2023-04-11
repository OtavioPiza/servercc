#ifndef SERVERCC_CONNECTOR_H
#define SERVERCC_CONNECTOR_H

#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

#include "default_trie.h"
#include "request.h"
#include "tcp_client.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::servercc::Request;
using ostp::servercc::client::TcpClient;
using std::function;
using std::string;
using std::unordered_map;

namespace ostp::servercc::connector {

/// A TCP connection manager that.
class Connector {
   public:
    // Constructors

    /// Constructs a new connector with the specified default processor and disconnect handler.
    ///
    /// Arguments:
    ///     default_processor: The default processor to use.
    ///     disconnect_handler: The handler to use when a client disconnects.
    Connector(const function<void(const Request)> default_processor,
              const function<void(const string)> disconnect_handler);

    // Destructor
    ~Connector();

    // Methods

    /// Adds the specified processor for the specified path.
    ///
    /// Arguments:
    ///     path: The path to add the processor for.
    ///     processor: The processor to add.
    void add_processor(const string& path, function<void(const Request)> processor);

    /// Adds a TCP client to the connector.
    ///
    /// Arguments:
    ///     client: The client to add.
    /// Returns:
    ///     The address of the client if successful, otherwise an error.
    StatusOr<string> add_client(TcpClient client);

    /// Send a message through the specified client.
    ///
    /// Arguments:
    ///     address: The address of the client to send the message to.
    ///     message: The message to send.
    ///
    /// Returns:
    ///     The number of bytes sent if successful, otherwise an error.
    StatusOr<int> send_message(const string& address, const string& message);

   private:
    /// Default trie for processing requests.
    DefaultTrie<char, function<void(const Request)>> processors;

    /// Handler for when a client disconnects.
    const function<void(const string)> disconnect_handler;

    /// A map of the current TCP clients identified by their socket file
    /// descriptor.
    unordered_map<string, TcpClient> clients;

    /// Runs the specified client in a thread.
    ///
    /// Arguments:
    ///     client: The client to run.
    void run_client(const string address);
};

}  // namespace ostp::servercc::connector

#endif
