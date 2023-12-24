#ifndef SERVERCC_PROTOCOLS_H
#define SERVERCC_PROTOCOLS_H

#include "inttypes.h"

namespace ostp::servercc {

// The type of the message protocol.
typedef uint32_t protocol_t;

// Starts a new connection UDP request.
//
// | header | body -------------------- |
// | header | port to send response to  |
constexpr protocol_t kConnectRequestProtocol = 0x00;

// Acknowledges a connection UDP request.
//
// | header | body ---- |
// | header |           |
constexpr protocol_t kConnectAckRequestProtocol = 0x01;

// Responds to a connection UDP request.
//
// | header | body ---- |
// | header |           |
constexpr protocol_t kConnectAckResponseProtocol = 0x02;

// Starts a new internal request channel.
//
// | header | body --------------------------------------- |                                         
// | header | original body | original header | channel ID |
constexpr protocol_t kInternalRequestProtocol = 0x10;

// Ends an internal request channel.
//
// | header | body ----- |
// | header | channel ID |
constexpr protocol_t kInternalRequestEndProtocol = 0x11;

// Starts a new internal response channel.
//
// | header | body --------------------------------------- |
// | header | original body | original header | channel ID |
constexpr protocol_t kInternalResponseProtocol = 0x13;

// Ends an internal response channel.
//
// | header | body ----- |
// | header | channel ID |
constexpr protocol_t kInternalResponseEndProtocol = 0x14;

// Represents an error in the internal channel.
//
// | header | body --------------------- |
// | header | channel ID | error message |
constexpr protocol_t kInternalErrorProtocol = 0x15;

}  // namespace ostp::servercc

#endif
