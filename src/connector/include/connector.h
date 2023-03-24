#ifndef SERVERCC_CONNECTOR_H
#define SERVERCC_CONNECTOR_H

#include <functional>
#include <unordered_map>

#include "connector_request.h"
#include "default_trie.h"
#include "tcp_client.h"

using ostp::libcc::data_structures::DefaultTrie;
using ostp::servercc::client::TcpClient;
using std::unordered_map;

namespace ostp::servercc::connector {

/// A TCP connection manager that.
class Connector {
   private:
    /// Default trie for processing requests.
    DefaultTrie<char, std::function<void(const ConnectorRequest)>> processors;

    /// A map of the current TCP clients identified by their socket file
    /// descriptor.
    unordered_map<int, TcpClient> clients;

   public:
    // Constructors

    /// Constructs a new connector with the specified default processor.
    /// 
    /// Arguments:
    ///     default_processor: The default processor to use.
    Connector(std::function<void(const ConnectorRequest)> default_processor);

    // Destructor
    ~Connector();

    // Methods

    /// Adds the specified processor for the specified path.
    ///
    /// Arguments:
    ///     path: The path to add the processor for.
    ///     processor: The processor to add.
    void add_processor(const std::string& path,
                       std::function<void(const ConnectorRequest)> processor);

    /// Adds a TCP client to the connector.
    ///
    /// Arguments:
    ///     client: The client to add.
    void add_client(TcpClient client);
};

}  // namespace ostp::servercc::connector

#endif
