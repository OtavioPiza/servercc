#include <string>

#include "logger.hh"

/**
 * @brief logs a message to INFO_STREAM
 *
 * @param msg message to log
 * @param sender sender of the message
 */
inline void restpp::log_info(const std::string &msg, const std::string &sender)
{
    fprintf(INFO_STREAM, "INFO [%s]: %s\n", sender.c_str(), msg.c_str());
}

/**
 * @brief logs an error to ERROR_STREAM
 *
 * @param msg message to log
 * @param sender sender of the message
 */
inline void restpp::log_error(const std::string &msg, const std::string &sender)
{
    fprintf(ERROR_STREAM, "ERROR [%s]: %s\n", sender.c_str(), msg.c_str());
}