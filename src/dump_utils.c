#include "dump_utils.h"

#include <stdio.h>
#include <stdarg.h>

void ASM_OUT_print(stk_elem_t last_elem) {
    printf(ANSI_GREEN "OUTPUT: " ANSI_RESET "<" ANSI_YELLOW DBUG_PRINTF_LIT ANSI_RESET ">\n", last_elem);
}

void opcode_print(Opcode opcode) {
    OpInfo opinfo = get_opinfo_by_opcode(opcode);

    fprintf(stderr, ANSI_LIGHT_BLUE "%s " ANSI_RESET, opinfo.str_repr);
    fprintf(stderr, "\n" ANSI_RESET);
}

