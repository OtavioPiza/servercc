#include "message_queue.h"

using ostp::servercc::distributed::MessageQueue;
using std::string;

/// See message_queue.h for documentation.
MessageQueue::MessageQueue() : messages(), semaphore(0), closed(false) {}

/// See message_queue.h for documentation.
void MessageQueue::push(const std::string message) {
    // Check if the queue is closed.
    if (closed) {
        return;
    }

    // Push the message to the queue and signal the semaphore.
    messages.push(message);
    semaphore.release();
}

/// See message_queue.h for documentation.
string MessageQueue::pop() {
    // If the queue is closed and empty, return an empty string.
    if (closed && messages.empty()) {
        return "";
    }

    // Wait for a message to be available.
    waiting_threads++;
    semaphore.acquire();
    waiting_threads--;

    // If the queue was closed while waiting and there are no more messages, return an empty string.
    if (closed && messages.empty()) {
        return "";
    }

    string message = std::move(messages.front());
    messages.pop();
    return std::move(message);
}

/// See message_queue.h for documentation.
void MessageQueue::close() {
    // Mark the queue as closed.
    closed = true;

    // Release all waiting threads.
    for (int i = 0; i < waiting_threads; i++) {
        semaphore.release();
    }
}

/// See message_queue.h for documentation.
bool MessageQueue::is_closed() const {
    return closed;
}

/// See message_queue.h for documentation.
bool MessageQueue::empty() const {
    return messages.empty();
}
