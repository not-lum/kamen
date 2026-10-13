#include "dump_utils.h"

#include <stdio.h>
#include <stdarg.h>

void ASM_OUT_print(stk_elem_t last_elem) {
    printf(ANSI_GREEN "OUTPUT: " ANSI_RESET "<" ANSI_YELLOW DBUG_PRINTF_LIT ANSI_RESET ">\n", last_elem);
}

void opcode_print(OpcodeType op, ...) {
    va_list args;
    va_start(args, op);

    const char *op_str = "";
    size_t numb_of_args = 0;
    bool reg_arg = false;

    switch (op) {
        case ASM_HLT:
            op_str = "HLT";
            break;
        case ASM_PUSH:
            op_str = "PUSH";
            numb_of_args = 1;
            break;
        case ASM_PUSH_REG:
            op_str = "PUSHr";
            reg_arg = true;
            numb_of_args = 1;
            break;
        case ASM_POP:
            op_str = "POP";
            break;
        case ASM_POP_REG:
            op_str = "POPr";
            reg_arg = true;
            numb_of_args = 1;
            break;
        case ASM_ADD:
            op_str = "ADD";
            break;
        case ASM_MUL:
            op_str = "MUL";
            break;
        case ASM_DIV:
            op_str = "DIV";
            break;
        case ASM_OUT:
            op_str = "OUT";
            break;
        case ASM_JMP:
            op_str = "JMP";
            numb_of_args = 1;
            break;
        default:
            op_str = "UNKNOWN";
    }

    fprintf(stderr, ANSI_LIGHT_BLUE "%s " ANSI_RESET, op_str);

    fprintf(stderr, ANSI_YELLOW);

    while (numb_of_args--) {
        if (reg_arg)
            fprintf(stderr, "%s ", str_reg(va_arg(args, stk_elem_t)));
        else
            fprintf(stderr, "%d ", va_arg(args, stk_elem_t));
    }

    fprintf(stderr, "\n" ANSI_RESET);

    va_end(args);
}
