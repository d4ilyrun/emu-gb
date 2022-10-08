#pragma once

/**
 * \enum log_level
 * \brief The different log levels available
 */
typedef enum {
    LOG_INFO = 0, ///< Output message to console (default)
    LOG_TRACE,    ///< Traces of the program execution
    LOG_WARNING,  ///< Warning
    LOG_ERROR,    ///< Error
} log_level;

/**
 * \brief Log a formatted message to the console
 * \param level The log's importance level
 * \param msg The formatted message
 */
void log_print(log_level level, const char *msg, ...);

/**
 * \brief Log a formatted message to the console using a specific color code
 * \param color The message's prefix's color code
 * \param msg The formatted message
 */
void log_color(const char *color, const char *msg, ...);

#define log_info(...)  log_print(LOG_INFO, __VA_ARGS__)
#define log_trace(...) log_print(LOG_TRACE, __VA_ARGS__)
#define log_warn(...)  log_print(LOG_WARNING, __VA_ARGS__)
#define log_err(...)   log_print(LOG_ERROR, __VA_ARGS__)

#define log(...) log_info(__VA_ARGS__)
