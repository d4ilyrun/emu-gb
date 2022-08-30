#pragma once

#include <stdbool.h>

#include "utils/log.h"

/// The number of expected arguments
#define GBEMU_NB_ARGS 1

struct options {
    char *args[GBEMU_NB_ARGS];
    bool trace;
    log_level log_level;
};

/**
 * \function parse_options
 * \brief Parse the program's options from its CLI arguments
 * \param argc The number of arguments
 * \param argv NULL terminated array of arguments
 */
struct options *parse_options(int argc, char **argv);

/**
 * \function get_options
 * \brief Get the program's current options
 * \return A pointer to the option struct
 */
struct options *get_options(void);
