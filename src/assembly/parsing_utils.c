#include "parsing_utils.h"
#include "../listing.h"
#include "../file_utils.h"
#include "../../secstack/src/secstack.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


const char *parsing_error_str(ParsingStatus status) {
    switch (status) {
        case PARSING_OK:
            return "parsing ok";
        case PARSING_CANNOT_PARSE_OPCODE:
            return "parsing cannot parse opcode";
        case PARSING_UNKNOWN_OP:
            return "invalid opcode while parsing";
        case PARSING_INVALID_REGISTER_PASSED_AS_ARG:
            return "invalid register passsed as operation argument";
        default:
            return "unknown parsing error";
    }
}

Opcode parse_op(char op_str[MAX_OP_STR_LEN]) {
    if (strcasecmp(op_str, "hlt") == 0)
        return (Opcode){ASM_HLT, 0, 0};
    else if (strcasecmp(op_str, "push") == 0)
        return (Opcode){ASM_PUSH, STK_ELEM, ARGS_ONE};
    else if (strcasecmp(op_str, "pushr") == 0)
        return (Opcode){ASM_PUSH_REG, REGISTER, ARGS_ONE};
    else if (strcasecmp(op_str, "pop") == 0)
        return (Opcode){ASM_POP, 0, ARGS_ZERO};
    else if (strcasecmp(op_str, "popr") == 0)
        return (Opcode){ASM_POP_REG, REGISTER, ARGS_ONE};
    else if (strcasecmp(op_str, "mul") == 0)
        return (Opcode){ASM_MUL, 0, ARGS_ZERO};
    else if (strcasecmp(op_str, "add") == 0)
        return (Opcode){ASM_ADD, 0, ARGS_ZERO};
    else if (strcasecmp(op_str, "div") == 0)
        return (Opcode){ASM_DIV, 0, ARGS_ZERO};
    else if (strcasecmp(op_str, "out") == 0)
        return (Opcode){ASM_OUT, 0, ARGS_ZERO};
    else if (strcasecmp(op_str, "jmp") == 0)
        return (Opcode){ASM_JMP, STK_ELEM, ARGS_ONE};
    else if (strcasecmp(op_str, "ja") == 0)
        return (Opcode){ASM_JA, STK_ELEM, ARGS_ONE};
    else if (strcasecmp(op_str, "jb") == 0)
        return (Opcode){ASM_JB, STK_ELEM, ARGS_ONE};
    else if (strcasecmp(op_str, "jae") == 0)
        return (Opcode){ASM_JAE, STK_ELEM, ARGS_ONE};
    else if (strcasecmp(op_str, "jbe") == 0)
        return (Opcode){ASM_JBE, STK_ELEM, ARGS_ONE};
    else
        return (Opcode){ASM_UNKNOWN, 0, 0};
}


RegisterCode parse_reg(char op_str[MAX_ARG_STR_LEN]) {
    if (strcasecmp(op_str, "ip") == 0)
        return IP;
    else if (strcasecmp(op_str, "ax") == 0)
        return AX;
    else if (strcasecmp(op_str, "bx") == 0)
        return BX;
    else if (strcasecmp(op_str, "cx") == 0)
        return CX;
    else if (strcasecmp(op_str, "dx") == 0)
        return DX;
    else
        return REG_UNKNOWN;
}


ParsingStatus parse_line(IndexedFile *asm_file, size_t line_numb,
                        Instr *instr) {
    assert(asm_file != NULL);
    assert(instr != NULL);

    char *line = asm_file->index[line_numb].addr;

    char op_str[MAX_OP_STR_LEN] = {};
    int bytes_read = 0;
    size_t numb_of_args = sscanf(line, "%s %n", op_str, &bytes_read);
    // fprintf(stderr, "opcode read: %s\n", op_str);

    if (numb_of_args != 1)
        return PARSING_CANNOT_PARSE_OPCODE;

    Opcode opcode = parse_op(op_str);
    // fprintf(stderr, "====================\n"
    //                 "opcode parsed:\n"
    //                 "opcode.type: %d\n"
    //                 "opcode.arg_type: %d\n"
    //                 "opcode.numb_of_args: %d\n"
    //                 "====================\n",
    //                 opcode.type, opcode.arg_type, opcode.numb_of_args);

    if (opcode.type == ASM_UNKNOWN)
        return PARSING_UNKNOWN_OP;

    stk_elem_t arg_value = 0;

    if (opcode.numb_of_args == ARGS_ONE) {
        switch (opcode.arg_type) {
            case (STK_ELEM):
                sscanf(line + bytes_read, DBUG_PRINTF_LIT, &arg_value);
                break;
            case (REGISTER): {
                char arg_str[MAX_ARG_STR_LEN] = {};
                sscanf(line + bytes_read, "%s", arg_str);

                arg_value = parse_reg(arg_str);

                if (arg_value == REG_UNKNOWN)
                    return PARSING_INVALID_REGISTER_PASSED_AS_ARG;
                break;
            }
            default:
                // TODO: maybe add something here. return with error?
                break;
        }
    }

    instr->opcode = opcode;
    instr->arg_value = arg_value;
    
    return PARSING_OK;
}

