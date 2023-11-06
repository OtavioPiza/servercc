#ifndef SERVERCC_MESSAGE_BODY_H
#define SERVERCC_MESSAGE_BODY_H

#include <inttypes.h>

#include <memory>
#include <vector>

namespace ostp::servercc {

// The body of a message consists of a unique pointer to a vector of bytes.
struct MessageBody {
    // The data of the message.
    std::vector<uint8_t> data;
};

}  // namespace ostp::servercc

#endif