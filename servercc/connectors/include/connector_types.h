#ifndef SERVERCC_CONNECTOR_TYPES_H
#define SERVERCC_CONNECTOR_TYPES_H

#include <inttypes.h>

#include "channel_types.h"
#include "internal_channel_manager.h"
#include "internal_request.h"
#include "types.h"

namespace ostp::servercc {

// The type of the connector channel manager.
typedef InternalChannelManager<kInternalRequestProtocol, kInternalRequestEndProtocol,
                               kInternalResponseProtocol, kInternalResponseEndProtocol,
                               kInternalResponseEndProtocol, 1024>
    connector_channel_manager_t;

// The type of the connector request.
typedef InternalRequest<kInternalResponseProtocol, kInternalResponseEndProtocol>
    connector_request_t;

// The type of the connector handler.
typedef internal_handler_t<kInternalResponseProtocol, kInternalResponseEndProtocol>
    connector_handler_t;

}  // namespace ostp::servercc

#endif
