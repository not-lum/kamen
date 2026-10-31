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

#define COMMENT_CHAR ';' 

typedef enum {
    PARSING_OK = 0,
    PARSING_CANNOT_PARSE_OP,
    PARSING_UNKNOWN_OP,
    PARSING_BAD_CHAR,
    PARSING_ARG_OUT_OF_RANGE,
    PARSING_UNABLE_TO_PARSE_ARG,
    PARSING_EMPTY_LABEL,
    PARSING_SQUARE_BRACKET_NOT_CLOSED,
    PARSING_UNKNOWN_LABEL,
    PARSING_NOT_A_NUMBER,
    PARSING_BAD_NUMBER,
    PARSING_LABEL_DOESNT_END_WITH_COLON
} ParsingStatus;


typedef struct {
    const char* msg;
    char *line;
    size_t line_index;
    size_t column;
} ParsingErrorInfo;

typedef struct {
    char name[MAX_LABEL_LEN];
    stk_elem_t addr;
} Label;

// TODO: move some of these to separate files

typedef struct {
    char *str;
    int len;
} ArgStr;

typedef struct {
    IndexedFile asm_file;
    Label labels[MAX_LABELS_NUMB];
    size_t labels_count;
    size_t bytecode_len;

    size_t line_index;
    size_t column;
} ParsingContext;

typedef struct {
    bool is_blank;
    Opcode opcode;
    ArgTypeMask arg_type_mask;
    stk_elem_t arg_value;
} Instr;

const char *parsing_error_str(ParsingStatus status);
void print_parsing_error(ParsingErrorInfo error_info);
void make_error_info(ParsingContext *, ParsingStatus, ParsingErrorInfo *);

ParsingStatus parse_line(ParsingContext *context, Instr *instr, ParsingErrorInfo *out_error_info);
ParsingStatus parse_op(ParsingContext *, OpInfo *);

ParsingStatus collect_labels(ParsingContext *context, ParsingErrorInfo *error_info);
bool find_label(ParsingContext *, const char *, stk_elem_t *);

bool is_comment(const char *);
bool is_label(const char *);
bool is_empty_line(const char *);

bool detect_bad_char(ParsingContext *ctx, char *str);
