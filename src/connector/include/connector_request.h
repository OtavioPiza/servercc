#ifndef SERVERCC_CONNECTOR_REQUEST_H
#define SERVERCC_CONNECTOR_REQUEST_H

#include <string>

namespace ostp::servercc::connector {

/// A request from a connector.
struct ConnectorRequest {
    /// The file descriptor of the connector.
    const int fd;

    /// The request type.
    const std::string type;

    /// The request.
    const std::string request;
};

}  // namespace ostp::servercc::connector

#endif
