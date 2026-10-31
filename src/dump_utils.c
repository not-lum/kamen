#include "dump_utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <string.h>


static const char *log_level_str(LogLevel level) {
    switch (level) {
        case INFO:
            return "INFO";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void log_header(FILE *to, LogLevel level) {
    assert(to != NULL);

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char buff[64] = {};
    strftime(buff, 64, "%a %b %d %T", tm);

    fprintf(to, "[%s] [%s]: ", buff, log_level_str(level));
}

void log_msg(FILE *to, LogLevel level, const char *fmt, ...) {
    assert(to != NULL);
    assert(fmt != NULL);

    va_list args;
    va_start(args, fmt);

    log_header(to, level);
    vfprintf(to, fmt, args);
    fprintf(to, "\n");

    va_end(args);
}
