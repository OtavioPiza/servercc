#include "internal_channel.h"

namespace ostp::servercc {

namespace {


}  // namespace

// See internal_channel.h for documentation.
InternalChannel::InternalChannel(channel_id_t id, int writeFd)
    : id(id), writeFd(writeFd), messageBuffer() {}

// See internal_channel.h for documentation.
InternalChannel::~InternalChannel() {
    // TODO send close message to close the channel.
}

// See internal_channel.h for documentation.
std::pair<absl::Status, std::unique_ptr<Message>> InternalChannel::read() {
    return std::move(messageBuffer.pop());
}

// See internal_channel.h for documentation.
absl::Status write(std::unique_ptr<Message> message) {
}

}  // namespace ostp::servercc