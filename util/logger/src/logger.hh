/**
 * @file logger.hh
 * @author Otavio Piza (you@domain.com)
 * @brief Set of functions to log messages to console
 * @version 0.1
 * @date 2022-04-30
 *
 * @copyright Copyright (c) 2022
 */

#ifndef RESTPP_LOGGER_HH
#define RESTPP_LOGGER_HH

#ifndef INFO_STREAM
#define INFO_STREAM stdout
#endif

#ifndef ERROR_STREAM
#define ERROR_STREAM stderr
#endif

#include <string>

namespace restpp
{
    /**
     * @brief logs a message to INFO_STREAM
     *
     * @param msg message to log
     * @param sender sender of the message
     */
    void log_info(const std::string &msg, const std::string &sender = "");

    /**
     * @brief logs a warning warning message to ERROR_STREAM
     *
     * @param msg message to log
     * @param sender sender of the message
     */
    void log_warn(const std::string &msg, const std::string &sender);

    /**
     * @brief logs an error to ERROR_STREAM
     *
     * @param msg message to log
     * @param sender sender of the message
     */
    void log_error(const std::string &msg, const std::string &sender = "");
}

#endif