#include "dump_utils.h"
#include "assembly/parsing_utils.h"
#include "listing.h"
#include "spu/spu.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define IF_TERM(to, color) ((to) == stderr || (to) == stdout) ? (color) : ("")

void ASM_OUT_print(stk_elem_t last_elem) {
    printf(ANSI_GREEN "OUTPUT: " ANSI_RESET "<" ANSI_YELLOW STK_ELEM_PRINTF_FMT ANSI_RESET ">\n", last_elem);
}

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

void log_instr(FILE *to, OpInfo opinfo, ArgTypeMask arg_mask, stk_elem_t arg) {
    log_header(to, INFO);
    fprintf(to, "Got instr: %s%s ", IF_TERM(to, ANSI_LIGHT_BLUE), opinfo.str_repr);

    char arg_str[MAX_ARG_LEN] = {};

    if (arg_mask == ARG_NONE) {
        fprintf(to, "%s\n", IF_TERM(to, ANSI_RESET));
        return;
    }

    if (arg_mask & ARG_REG) {
        RegInfo reginfo = get_reginfo_by_code(arg);
        strcpy(arg_str, reginfo.str_repr);
    } else
        sprintf(arg_str, "%d", arg);

    if (arg_mask & ARG_RAM)
        fprintf(to, "%s[%s]%s\n",
                IF_TERM(to, ANSI_YELLOW), arg_str, IF_TERM(to, ANSI_RESET));
    else
        fprintf(to, "%s%s%s\n",
                IF_TERM(to, ANSI_YELLOW), arg_str, IF_TERM(to, ANSI_RESET));
}

void init_spu_dbg_file(SPU* spu, const char *filename) {
    ON_SPU_DEBUG(
    spu->_dbg_logfile = fopen(filename, "a");
    setvbuf(spu->_dbg_logfile, NULL, _IONBF, 0);
    )
}