#ifndef SERVERCC_TYPES_H
#define SERVERCC_TYPES_H

#include <functional>

#include "include/message_header.h"
#include "include/message_body.h"
#include "include/message.h"
#include "include/request.h"

namespace ostp::servercc {

// The type of a protocol handler.
typedef std::function<void(std::unique_ptr<ostp::servercc::Request>)> handler_t;

}  // namespace ostp::servercc

#endif
