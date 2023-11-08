#include "message.h"

namespace ostp::servercc {

// See message.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> readMessage(int fd) {
    std::unique_ptr<Message> message = std::make_unique<Message>();

    // Read the header.
    auto bytesRead = recv(fd, &message->header, kMessageHeaderLength, 0);
    if (bytesRead < kMessageHeaderLength) {
        return {absl::InvalidArgumentError("Error reading message header"), nullptr};
    }

    if (message->header.length > 0) {
        message->body.data.resize(message->header.length);

        // Read the body.
        bytesRead = recv(fd, message->body.data.data(), message->header.length, 0);
        if (bytesRead < message->header.length) {
            return {absl::InvalidArgumentError("Error reading message body"), nullptr};
        }
    }

    // Return the message.
    return {absl::OkStatus(), std::move(message)};
}

// See message.h for documentation.
absl::Status writeMessage(int fd, std::unique_ptr<Message> message) {
    if (fd < 0) {
        return absl::InvalidArgumentError("Invalid file descriptor");
    }
    message->header.length = message->body.data.size();

    // Write the header.
    auto bytesWritten = send(fd, &message->header, kMessageHeaderLength, 0);
    if (bytesWritten < kMessageHeaderLength) {
        return absl::InvalidArgumentError("Error writing message header");
    }

    // Write the body.
    bytesWritten = send(fd, message->body.data.data(), message->header.length, 0);
    if (bytesWritten < message->header.length) {
        return absl::InvalidArgumentError("Error writing message body");
    }
    fsync(fd);

    // Return.
    return absl::OkStatus();
}

}  // namespace ostp::servercc
