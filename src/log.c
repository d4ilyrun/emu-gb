#include "utils/log.h"

#define _GNU_SOURCE // needed for asprintf

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "options.h"

#define COLOR_RESET "\033[0m"

const char *level_colors[] = {
    [LOG_INFO] = "\033[1;34m",    // cyan
    [LOG_TRACE] = "\033[1;30m",   // dark grey
    [LOG_WARNING] = "\033[1;30m", // yellow
    [LOG_ERROR] = "\033[1;31m",   // red
};

void log_print(log_level level, const char *fmt, ...)
{
    const struct options *opt = get_options();

    // Don't output if the importance level is lower than the one requested
    if (level < opt->log_level)
        return;

    va_list args;
    va_start(args, fmt);

    FILE *stream = (level) >= LOG_WARNING ? stderr : stdout;

    char *log = NULL;
    asprintf(&log, "%s| %s%s\n", level_colors[level], COLOR_RESET, fmt);

    vfprintf(stream, log, args);
    free(log);
}

void log_color(const char *color, const char *fmt, ...)
{
    // Don't output if the importance level is lower than the one requested.
    // As the level is not specified, default to LOG_INFO
    if (get_options()->log_level > LOG_INFO)
        return;

    va_list args;
    va_start(args, fmt);

    char *log = NULL;
    asprintf(&log, "%s| %s%s\n", color, COLOR_RESET, fmt);

    vfprintf(stdout, log, args);
    free(log);
}
