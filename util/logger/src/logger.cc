#include <string>

#include "logger.hh"

/**
 * @brief logs a message to INFO_STREAM
 *
 * @param msg message to log
 * @param sender sender of the message
 */
void restpp::log_info(const std::string &msg, const std::string &sender)
{
    fprintf(INFO_STREAM,
            "\033[0;32m"
            "INFO [%s]: %s\n"
            "\033[0m",
            sender.c_str(), msg.c_str());
}

/**
 * @brief logs a warning warning message to ERROR_STREAM
 * 
 * @param msg 
 * @param sender 
 */
void restpp::log_warn(const std::string &msg, const std::string &sender)
{
    fprintf(INFO_STREAM,
            "\033[0;33m"
            "WARN [%s]: %s\n"
            "\033[0m",
            sender.c_str(), msg.c_str());
}

/**
 * @brief logs an error to ERROR_STREAM
 *
 * @param msg message to log
 * @param sender sender of the message
 */
void restpp::log_error(const std::string &msg, const std::string &sender)
{
    fprintf(ERROR_STREAM,
            "\033[0;31m"
            "ERROR [%s]: %s\n"
            "\033[0m",
            sender.c_str(), msg.c_str());
}