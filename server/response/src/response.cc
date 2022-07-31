#include "response.hh"

/**
 * @brief Construct a new Response object
 *
 * @param socket socket file descriptor
 */
restpp::Response::Response(int socket)
{
    this->_sent = false;
    this->socket = socket;
}

/**
 * @brief checks if a response was already sent
 *
 * @return true if a reaspose was already sent
 * @return false if a response was not yet sent
 */
bool restpp::Response::is_sent()
{
    return this->_sent;
}