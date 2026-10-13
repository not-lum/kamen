#pragma once

#include "../listing.h"
#include "../file_utils.h"
#include "../../secstack/src/secstack.h"
#include <stdint.h>

#define MAX_OP_STR_LEN 10
#define MAX_ARG_STR_LEN 30

typedef enum {
    PARSING_OK = 0,
    PARSING_CANNOT_PARSE_OPCODE,
    PARSING_UNKNOWN_OP,
    PARSING_INVALID_REGISTER_PASSED_AS_ARG
} ParsingStatus;

typedef enum {
    STK_ELEM = 0,
    REGISTER
} ArgType;

typedef enum {
    ARGS_ZERO = 0,
    ARGS_ONE
} NumbOfArgs;

typedef struct {
    OpcodeType type;
    ArgType arg_type;
    NumbOfArgs numb_of_args;
} Opcode;

typedef struct {
    Opcode opcode;
    stk_elem_t arg_value;
} Instr;

const char *parsing_error_str(ParsingStatus status);
Opcode parse_op(char op_str[MAX_OP_STR_LEN]);
RegisterCode parse_reg(char op_str[MAX_ARG_STR_LEN]);
ParsingStatus parse_line(IndexedFile *asm_file, size_t line_numb, Instr *instr);
