
#ifndef SERVERCC_SERVER_MODE_H
#define SERVERCC_SERVER_MODE_H

#include <stdint.h>

namespace ostp::severcc
{

    /// The mode the server will run in.
    enum class ServerMode
    {
        /// The server will run in a single thread.
        SINGLE_THREAD,

        /// The server will run in multiple threads.
        MULTI_THREAD,

        /// The server will run in a single process.
        SINGLE_PROCESS,

        /// The server will run in multiple processes.
        MULTI_PROCESS
    };

} // namespace ostp::severcc

#endif
