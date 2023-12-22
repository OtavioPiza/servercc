#ifndef SERVERCC_MESSAGE_H
#define SERVERCC_MESSAGE_H

#include <netdb.h>

#include <memory>

#include "absl/status/status.h"
#include "message_body.h"
#include "message_header.h"

namespace ostp::servercc {

// The message sent between the client and server.
struct Message {
    // The message header.
    MessageHeader header;

    // The message body.
    MessageBody body;
};

// Reads a message from the specified file descriptor.
std::pair<absl::Status, std::unique_ptr<Message>> readMessage(int fd);

// Reads a message from the specified file descriptor with a timeout.
std::pair<absl::Status, std::unique_ptr<Message>> readMessage(int fd, int timeout);

// Writes a message to the specified file descriptor.
absl::Status writeMessage(int fd, std::unique_ptr<Message> message);

// Create a wrapped message with the message ID and append the header and message buffer ID to
// the body as follows:
//
// | New header | Original body | Original header | value |
//
// This reduces the number of copies required to send the message.
//
// Note that the type of the value must be copyable.
template <typename T, protocol_t Protocol>
std::unique_ptr<Message> wrapMessage(const T& value, std::unique_ptr<Message> message) {
    message->body.data.resize(message->header.length + kMessageHeaderLength + sizeof(T));

    // Copy the original message header.
    auto offset = message->header.length;
    memcpy(message->body.data.data() + offset, &message->header, kMessageHeaderLength);

    // Copy the value.
    memcpy(message->body.data.data() + offset, &value, sizeof(T));
    offset += kMessageHeaderLength;

    // Update the header.
    message->header.protocol = Protocol;
    message->header.length = offset + sizeof(T);

    // Return the message.
    return std::move(message);
}

// Unwraps a message.
template <typename T>
std::tuple<absl::Status, std::unique_ptr<T>, std::unique_ptr<Message>> unwrapMessage(
    std::unique_ptr<Message> message) {
    // Check length.
    if (message->header.length < sizeof(T) + kMessageHeaderLength) {
        return {absl::InvalidArgumentError("Invalid message length"), nullptr, nullptr};
    }

    // Copy the last part of the body into the value.
    auto value = std::make_unique<T>();
    auto offset = message->header.length - sizeof(T);
    memcpy(value.get(), message->body.data.data() + offset, sizeof(T));

    // Copy the original message header into the message header.
    offset -= kMessageHeaderLength;
    memcpy(&message->header, message->body.data.data() + offset, kMessageHeaderLength);

    // Check the length.
    if (message->header.length != offset) {
        return {absl::InvalidArgumentError("Invalid message length"), nullptr, nullptr};
    }

    // Erase the value and header from the body.
    message->body.data.erase(message->body.data.begin() + offset, message->body.data.end());

    // Return the value and message.
    return {absl::OkStatus(), std::move(value), std::move(message)};
}

}  // namespace ostp::servercc

#endif