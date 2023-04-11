#ifndef SERVERCC_DISTRIBUTED_MESSAGE_QUEUE_H
#define SERVERCC_DISTRIBUTED_MESSAGE_QUEUE_H

#include <queue>
#include <semaphore>
#include <string>

using std::binary_semaphore;
using std::queue;
using std::string;

namespace ostp::servercc::distributed {

class MessageQueue {
   public:
    /// Creates a new MessageQueue.
    MessageQueue();

    /// Pushes a message to the queue.
    void push(const std::string message);

    /// Pops a message from the queue.
    string pop();

    /// Close the queue.
    void close();

    /// Whether the queue is closed.
    bool is_closed() const;

    /// Whether the queue is empty.
    bool empty() const;

   private:
    // Attributes.

    /// The queue of messages.
    queue<string> messages;

    /// The semaphore used to synchronize the queue.
    binary_semaphore semaphore;

    /// Whether the queue is closed.
    bool closed;
};

}  // namespace ostp::servercc::distributed

#endif
