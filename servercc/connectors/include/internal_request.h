#ifndef SERVERCC_INTERNAL_REQUEST_H
#define SERVERCC_INTERNAL_REQUEST_H

#include <netinet/in.h>

#include <memory>

#include "types.h"

namespace ostp::servercc {

// A request between servercc processes.
template <protocol_t ResponseProtocol, protocol_t ResponseEndProtocol>
struct InternalRequest {
    // The channel to read and write messages.
    std::shared_ptr<InternalChannel<ResponseProtocol, ResponseEndProtocol>> channel;

    // The protocol of the first message.
    protocol_t protocol;
};

// The type of a protocol handler for internal requests.
template <protocol_t ResponseProtocol, protocol_t ResponseEndProtocol>
using internal_handler_t =
    std::function<void(std::unique_ptr<ostp::servercc::InternalRequest<ResponseProtocol,
                                                                       ResponseEndProtocol>>)>;

}  // namespace ostp::servercc

#endif
