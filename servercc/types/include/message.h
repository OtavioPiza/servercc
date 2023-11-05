#ifndef SERVERCC_MESSAGE_H
#define SERVERCC_MESSAGE_H

#include "message_body.h"
#include "message_header.h"

namespace ostp::servercc {

struct Message {
    MessageHeader header;
    MessageBody body;
};

}  // namespace ostp::servercc

#endif