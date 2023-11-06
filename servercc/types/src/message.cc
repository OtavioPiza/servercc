#include "message.h"

#include <netdb.h>

#include <memory>

#include "absl/status/status.h"

namespace ostp::servercc {

// See message.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> readMessage(int fd) {
    std::unique_ptr<Message> message = std::make_unique<Message>();
    auto bytesRead = recv(fd, &message->header, kMessageHeaderLength, 0);
    if (bytesRead < kMessageHeaderLength) {
        return {absl::InvalidArgumentError("Error reading message header"), nullptr};
    }
    message->body.data.resize(message->header.length);
    bytesRead = recv(fd, message->body.data.data(), message->header.length, 0);
    if (bytesRead < message->header.length) {
        return {absl::InvalidArgumentError("Error reading message body"), nullptr};
    }
    return {absl::OkStatus(), std::move(message)};
}

}  // namespace ostp::servercc
