#include "internal_channel.h"

namespace ostp::servercc {

InternalChannel::InternalChannel(channel_id_t id, int writeFd)
    : id(id), writeFd(writeFd), messageBuffer() {}

}  // namespace ostp::servercc