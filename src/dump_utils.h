#pragma once

#include "../secstack/src/secstack.h"
#include "listing.h"
#include "spu/spu_internal.h"


#define ANSI_GREEN "\e[32m"
#define ANSI_RED "\e[31m"
#define ANSI_GREY "\e[90m"
#define ANSI_YELLOW "\e[33m"
#define ANSI_PURPLE "\e[35m"
#define ANSI_LIGHT_BLUE "\e[36m"
#define ANSI_BLUE "\e[34m"

#define ANSI_BG_CYAN "\e[46m"
#define ANSI_BG_WHITE "\e[47m"
#define ANSI_BG_YELLOW "\e[43m"
#define ANSI_BG_RED "\e[41m"

#define ANSI_RESET "\e[0m"

typedef enum {
    INFO = 0,
    ERROR
} LogLevel;

void log_msg(FILE *to, LogLevel level, const char *fmt, ...);
void log_header(FILE *to, LogLevel level);
