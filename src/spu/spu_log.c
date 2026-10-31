#include "spu_log.h"

#include "../dump_utils.h"
#include "../listing.h"
#include "../assembly/parsing_utils.h"

#include <string.h>

#define IF_TERM(to, color) ((to) == stderr || (to) == stdout) ? (color) : ("")

void log_instr(FILE *to, DecodedInstr *instr) {
    log_header(to, INFO);
    fprintf(to, "Got instr: %s%s ", IF_TERM(to, ANSI_LIGHT_BLUE), instr->op.str_repr);

    char arg_str[MAX_ARG_LEN] = {};

    if (instr->arg_mask == ARG_NONE) {
        fprintf(to, "%s\n", IF_TERM(to, ANSI_RESET));
        return;
    }

    if (instr->arg_mask & ARG_REG) {
        RegInfo reginfo = get_reginfo_by_code(instr->raw_arg);
        strcpy(arg_str, reginfo.str_repr);
    } else
        sprintf(arg_str, "%d", instr->raw_arg);

    if (instr->arg_mask & ARG_RAM)
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