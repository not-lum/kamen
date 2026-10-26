#pragma once

#include "../listing.h"
#include "../file_utils.h"
#include "../../secstack/src/secstack.h"
#include <stdint.h>

#define MAX_OP_LEN 10
#define MAX_ARG_LEN 30
#define MAX_LABEL_LEN 30
#define MAX_LABELS_NUMB 1024
#define MAX_OP_LEN_STR "9"   // 10-1=9 (null byte doesn't count)
#define MAX_ARG_LEN_STR "29" // 30-1=29 (null byte doesn't count)
#define MAX_LABEL_LEN_STR "29" // 30-1=29 (null byte doesn't count)

typedef enum {
    PARSING_OK = 0,
    PARSING_CANNOT_PARSE_OP,
    PARSING_UNKNOWN_OP,
    PARSING_BAD_CHAR,
    PARSING_ARG_OUT_OF_RANGE,
    PARSING_BAD_OP_ARG,
    PARSING_UNABLE_TO_PARSE_ARG,
    PARSING_EMPTY_LABEL,
    PARSING_SQUARE_BRACKET_NOT_CLOSED
} ParsingStatus;


typedef struct {
    const char* msg;
    size_t line;
    size_t numb_of_bad_char;
} ParsingErrorInfo;

typedef struct {
    char name[MAX_LABEL_LEN];
    stk_elem_t addr;
} Label;

typedef struct {
    IndexedFile asm_file;
    Label labels[MAX_LABELS_NUMB];
    size_t labels_count;
    size_t bytecode_len;
    size_t curr_line_numb;
    size_t curr_line_valid_chars;
} ParsingContext;

typedef struct {
    bool is_not_code;
    Opcode opcode;
    ArgTypeMask arg_type_mask;
    stk_elem_t arg_value;
} Instr;

const char *parsing_error_str(ParsingStatus status);
ParsingStatus parse_line(ParsingContext *context, Instr *instr, ParsingErrorInfo *out_error_info);
void print_parsing_error(ParsingErrorInfo error_info, IndexedFile asm_file);
ParsingStatus collect_labels(ParsingContext *context, ParsingErrorInfo *error_info);
