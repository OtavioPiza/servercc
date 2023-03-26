
#ifndef SERVERCC_SERVER_MODE_H
#define SERVERCC_SERVER_MODE_H

namespace ostp::servercc::server {

/// The mode the server will run in.
enum class ServerMode {
    /// The server will read requests synchronously.
    SYNC,

    /// The server will read requests asynchronously.
    ASYNC_THREAD,

};

}  // namespace ostp::servercc::server

#endif
