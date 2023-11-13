#ifndef SERVERCC_MESSAGE_HEADER_H
#define SERVERCC_MESSAGE_HEADER_H

#include "protocols.h"
#include <inttypes.h>

namespace ostp::servercc {

// Header for a message. Has the following format:
//
// | 0x00 | 0x01 | 0x02 | 0x03 | 0x04 | 0x05 | 0x06 | 0x07 |
// |-------------------------------------------------------|
// | Length of message         | Protocol of message       |
// |-------------------------------------------------------|
//
// We use a fixed length header to make it easier to parse messages. The header is represented as a
// a struct packed with a length of 8 bytes.
struct MessageHeader {
    // The length of the data in the message.
    u_int32_t length;

    // The protocol of the message.
    protocol_t protocol;
} __attribute__((packed));

// The length of the message header.
constexpr uint32_t kMessageHeaderLength = sizeof(MessageHeader);


}  // namespace ostp::servercc

#endif