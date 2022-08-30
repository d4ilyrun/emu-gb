/**
 * \file options.c
 * \creator Léo DUBOIN <leo@duboin.com>
 *
 * Parse the program's CLI arguments using glibc's argp library.
 *
 * Documentation: https://girishjoshi.io/post/glibc-argument-parsing-argp/
 */

#include "options.h"

#include <argp.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

inline struct options *get_options(void)
{
    static struct options options = {
        .log_level = LOG_INFO,
        .trace = false,
    };

    return &options;
}

#define STR_EQ(str1, str2) !strcmp((str1), (str2))

static error_t parse_opt(int key, char *value, struct argp_state *state)
{
    struct options *arguments = state->input;

    switch (key) {
    case 't':
        arguments->trace = true;
        break;

    case 'l':
        if (STR_EQ(value, "TRACE"))
            arguments->log_level = LOG_TRACE;
        else if (STR_EQ(value, "WARNING"))
            arguments->log_level = LOG_WARNING;
        else if (STR_EQ(value, "ERROR"))
            arguments->log_level = LOG_ERROR;
        else {
            log_warn("Invalid argument for option -l / --log-level: %s", value);
            arguments->log_level = LOG_INFO;
        }
        break;

    case ARGP_KEY_ARG:
        // Too many arguments, if your program expects only one argument.
        if (state->arg_num > GBEMU_NB_ARGS)
            argp_usage(state);
        arguments->args[state->arg_num] = value;
        break;

    case ARGP_KEY_END:
        if (state->arg_num < GBEMU_NB_ARGS)
            argp_usage(state);
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static char doc[] = "GB-EMU: Yet another gameboy emulator written in C";
static char args_doc[] = "CARTRIDGE";

static struct argp_option long_options[] = {
    // Log related
    {"trace", 't', 0, 0, "Output traces during execution"},
    {"log-level", 'l', "LEVEL", 0, "Do not output logs of lower importance"},

    {0},
};

struct options *parse_options(int argc, char **argv)
{
    static struct argp argp = {long_options, parse_opt, args_doc, doc};
    struct options *arguments = get_options();

    argp_parse(&argp, argc, argv, 0, 0, arguments);

    return get_options();
}
