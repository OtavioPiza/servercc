#ifndef SERVERCC_MESSAGE_H
#define SERVERCC_MESSAGE_H

#include <memory>

#include "absl/status/status.h"
#include "message_body.h"
#include "message_header.h"

namespace ostp::servercc {

struct Message {
    MessageHeader header;
    MessageBody body;
};

// Reads a message from the specified file descriptor.
std::pair<absl::Status, std::unique_ptr<Message>> readMessage(int);

}  // namespace ostp::servercc

#endif