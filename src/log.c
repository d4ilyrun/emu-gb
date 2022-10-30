#include "utils/log.h"

#define _GNU_SOURCE // needed for asprintf

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "options.h"

#define COLOR_RESET "\033[0m"

const char *g_level_colors[] = {
    [LOG_INFO] = "\033[1;34m",    // cyan
    [LOG_TRACE] = "\033[1;30m",   // dark grey
    [LOG_WARNING] = "\033[1;33m", // yellow
    [LOG_ERROR] = "\033[1;31m",   // red
};

void log_print(log_level level, const char *fmt, ...)
{
    const struct options *opt_ptr = get_options();

    // Don't output if the importance level is lower than the one requested
    // Except if expecially specified (traces)
    if (level == LOG_TRACE) {
        if (!get_options()->trace)
            return;
    } else if (level < opt_ptr->log_level) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    FILE *stream_ptr = (level) >= LOG_WARNING ? stderr : stdout;

    char *log_ptr = NULL;
    asprintf(&log_ptr, "%s| %s%s\n", g_level_colors[level], COLOR_RESET, fmt);

    vfprintf(stream_ptr, log_ptr, args);
    free(log_ptr);
}

void log_color(const char *color, const char *fmt, ...)
{
    // Don't output if the importance level is lower than the one requested.
    // As the level is not specified, default to LOG_INFO
    if (get_options()->log_level > LOG_INFO)
        return;

    va_list args;
    va_start(args, fmt);

    char *log_ptr = NULL;
    asprintf(&log_ptr, "%s| %s%s\n", color, COLOR_RESET, fmt);

    vfprintf(stdout, log_ptr, args);
    free(log_ptr);
}
