#include "parsing_utils.h"
#include "../listing.h"
#include "../dump_utils.h"
#include "../file_utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>


const char *parsing_error_str(ParsingStatus status) {
    switch (status) {
        case PARSING_OK:
            return "parsing ok";
        case PARSING_CANNOT_PARSE_OP:
            return "cannot parse operation";
        case PARSING_UNKNOWN_OP:
            return "invalid operation";
        case PARSING_BAD_CHAR:
            return "bad char while parsing line";
        case PARSING_ARG_OUT_OF_RANGE:
            return "argument out of range";
        case PARSING_BAD_OP_ARG:
            return "bad argument for operation provided";
        case PARSING_UNABLE_TO_PARSE_ARG:
            return "unable to parse operation argument";
        case PARSING_EMPTY_LABEL:
            return "the label is empty";
        case PARSING_SQUARE_BRACKET_NOT_CLOSED:
            return "square bracket is not closed";
        default:
            return "unknown parsing error";
    }
}

static bool str_starts_with(const char *str, char ch) {
    while (isspace(*str)) {
        str++;
    }

    if (*str == ch)
        return true;

    return false;
}

static bool is_comment(const char *str) {
    return str_starts_with(str, ';');
}

static bool is_label(const char *str) {
    return str_starts_with(str, '.');
}

static bool is_empty_line(const char *str) {
    return str_starts_with(str, '\0');
}

void print_parsing_error(ParsingErrorInfo error_info, IndexedFile asm_file) {
    fprintf(stderr, ANSI_RED "Syntax Error " ANSI_RESET "(" ANSI_GREEN "%zu:%zu" ANSI_RESET "): ("
            ANSI_LIGHT_BLUE "%s" ANSI_RESET ")\n",
            error_info.line + 1, error_info.numb_of_bad_char + 1, error_info.msg);

    fputs(ANSI_LIGHT_BLUE, stderr);
    size_t len_of_prefix = fprintf(stderr, "%zu: ", error_info.line + 1);
    fputs(ANSI_RESET, stderr);

    fprintf(stderr, ANSI_YELLOW "%s\n" ANSI_RESET, asm_file.index[error_info.line].addr);

    size_t spaces_numb = len_of_prefix + error_info.numb_of_bad_char + 1;
    for (size_t i = 0; i < spaces_numb ; i++) {
        if (i == spaces_numb - 1)
            fputc('^', stderr);
        else
            fputc(' ', stderr);
    }

    fputc('\n', stderr);
}

static void make_error_info(ParsingContext *context, ParsingStatus status,
                            ParsingErrorInfo *out_error_info) {
    assert(context != NULL);
    assert(out_error_info != NULL);

    out_error_info->msg = parsing_error_str(status);
    out_error_info->line = context->curr_line_numb;
    out_error_info->numb_of_bad_char = context->curr_line_valid_chars;
}


static bool check_for_bad_char(char bad_char) {
    if (bad_char != '\0' && bad_char != ';')
        return true;

    return false;
}

static ParsingStatus parse_label(ParsingContext *context) {
    size_t *valid_chars = &context->curr_line_valid_chars;
    char *line = context->asm_file.index[context->curr_line_numb].addr;

    char bad_char = '\0';
    int bytes_read = 0;
    char label_name[MAX_LABEL_LEN];

    size_t sscanf_args_numb = sscanf(line, " .%" MAX_LABEL_LEN_STR "[^:]: %c%n",
                                     label_name, &bad_char, &bytes_read); 

    *valid_chars += bytes_read;
    
    // fprintf(stderr, "sscanf_args_numb: %zu\n", sscanf_args_numb);
    // fprintf(stderr, "bad_label_char: <%c>\n", bad_char);

    if (!isspace(bad_char) && bad_char != ';' && bad_char != '\0') {
        *valid_chars -= 1;

        return PARSING_BAD_CHAR;
    }

    if (sscanf_args_numb >= 1) {
        Label label = {.addr = context->bytecode_len};
        strncpy(label.name, label_name, MAX_LABEL_LEN);
        context->labels[context->labels_count] = label;
        context->labels_count++;

        return PARSING_OK;
    }

    return PARSING_EMPTY_LABEL;
}


static ParsingStatus parse_op(ParsingContext *context, OpInfo *out_opinfo) {
    assert(context != NULL);
    assert(out_opinfo != NULL);

    size_t *valid_chars = &context->curr_line_valid_chars;
    char *line = context->asm_file.index[context->curr_line_numb].addr;

    char op_str[MAX_OP_LEN] = {};
    int bytes_read_op = 0;

    size_t sscanf_args_numb = sscanf(line, " %" MAX_OP_LEN_STR "s %n",
                                     op_str, &bytes_read_op);

    if (sscanf_args_numb != 1)
        return PARSING_CANNOT_PARSE_OP;

    OpInfo opinfo = get_opinfo_by_str(op_str);
    // fprintf(stderr, "opinfo.op_str: %s\n", op_str);
    // fprintf(stderr, "opinfo.str_repr: %s\n", opinfo.str_repr);
    // fprintf(stderr, "====================\n"
    //                 "opcode parsed:\n"
    //                 "opcode.type: %d\n"
    //                 "opcode.arg_type: %d\n"
    //                 "opcode.numb_of_args: %d\n"
    //                 "====================\n",
    //                 opcode.type, opcode.arg_type, opcode.numb_of_args);

    if (opinfo.opcode == ASM_UNKNOWN)
        return PARSING_UNKNOWN_OP;

    *valid_chars += bytes_read_op;

    *out_opinfo = opinfo;

    return PARSING_OK;
}


ParsingStatus collect_labels(ParsingContext *context, ParsingErrorInfo *error_info) {
    assert(context != NULL);
    assert(error_info != NULL);

    while (context->curr_line_numb < context->asm_file.lines_numb) {
        char *line = context->asm_file.index[context->curr_line_numb].addr;

        if (is_label(line)) {
            ParsingStatus status = parse_label(context);
            if (status != PARSING_OK) {
                make_error_info(context, status, error_info);

                return status;
            }
        } else if (!is_comment(line) && !is_empty_line(line)) {
            OpInfo opinfo = {};
            ParsingStatus status = parse_op(context, &opinfo);
            if (status != PARSING_OK)
                return status;

            if (opinfo.allowed_arg_type == ARG_NONE)
                context->bytecode_len += INSTR_NO_ARG_SIZE;
            else
                context->bytecode_len += INSTR_DEFAULT_SIZE;
        }

        context->curr_line_numb++;
        context->curr_line_valid_chars = 0;
    }

    context->curr_line_numb = 0;
    context->curr_line_valid_chars = 0;

    return PARSING_OK;
}

static bool find_label(ParsingContext *context, const char *name, stk_elem_t *out_addr) {
    assert(context != NULL);
    assert(name != NULL);
    assert(out_addr != NULL);

    for (size_t i = 0; i < context->labels_count; i++) {
        if (strcmp(context->labels[i].name, name) == 0) {
            *out_addr = context->labels[i].addr;

            return true;
        }
    }

    return false;
}

static ParsingStatus parse_arg(ParsingContext *context, char arg_str[],
                               int *arg_len, char *bad_char,
                               int *bytes_parsed_arg) {
    assert(context != NULL);
    assert(arg_len != NULL);
    assert(arg_str != NULL);
    assert(bad_char != NULL);
    assert(bytes_parsed_arg != NULL);

    size_t *valid_chars = &context->curr_line_valid_chars;
    char *line = context->asm_file.index[context->curr_line_numb].addr;

    if (line[*valid_chars] == '\0') {
        return PARSING_UNABLE_TO_PARSE_ARG;
    } else {
        size_t sscanf_args_numb = sscanf(line + *valid_chars, "%" MAX_ARG_LEN_STR "s%n %c%n",
                                         arg_str, arg_len, bad_char, bytes_parsed_arg);

        if (sscanf_args_numb == 0)
            return PARSING_UNABLE_TO_PARSE_ARG;
    }

    // fprintf(stderr, "arg_str: %s\n", arg_str);
    // fprintf(stderr, "valid_chars: %zu\n", *valid_chars);
    
    *valid_chars += *bytes_parsed_arg;

    return PARSING_OK;
}

#define RETURN_ERROR(status) do { \
    if (out_error_info != NULL) \
        make_error_info(context, (status), out_error_info); \
    \
    return status; \
} while (0)

#define RETURN_OK() do { \
    context->curr_line_numb++; \
    context->curr_line_valid_chars = 0; \
    return PARSING_OK; \
} while (0)

ParsingStatus parse_line(ParsingContext *context, Instr *instr, ParsingErrorInfo *out_error_info) {
    assert(context != NULL);
    assert(instr != NULL);
    // out_error_info could be NULL - in that case no error info is written

    size_t *valid_chars = &context->curr_line_valid_chars;
    char *line = context->asm_file.index[context->curr_line_numb].addr;

    if (is_comment(line) || is_label(line) || is_empty_line(line)) {
        instr->is_comment_or_label = true; // TODO: change field name
        RETURN_OK();
    }

    OpInfo opinfo = {};
    ParsingStatus status = parse_op(context, &opinfo);
    if (status != PARSING_OK)
        RETURN_ERROR(status);

    char bad_char = '\0';
    char arg_str_arr[MAX_ARG_LEN] = {};
    char *arg_str = arg_str_arr;
    int arg_len = 0;
    int bytes_parsed_arg = 0;
    status = parse_arg(context, arg_str_arr, &arg_len, &bad_char, &bytes_parsed_arg);
    if (status != PARSING_OK) {
        if (opinfo.allowed_arg_type & ARG_NONE) {
            if (check_for_bad_char(bad_char)) {
                *valid_chars -= 1;
    
                RETURN_ERROR(PARSING_BAD_CHAR);
            }

            instr->is_comment_or_label = false;
            instr->opcode = opinfo.opcode;
            instr->arg_type_mask |= ARG_NONE;
            instr->arg_value = 0;

            RETURN_OK();
        } else {
            RETURN_ERROR(status);
        }
    }

    if ((opinfo.allowed_arg_type & ARG_LABEL) && arg_str[0] == '.') {
        stk_elem_t addr = 0;
        if (find_label(context, arg_str + 1, &addr)) {
            if (check_for_bad_char(bad_char)) {
                *valid_chars -= 1;

                RETURN_ERROR(PARSING_BAD_CHAR);
            }

            instr->is_comment_or_label = false;
            instr->opcode = opinfo.opcode;
            instr->arg_type_mask = ARG_STK_ELEM;
            instr->arg_value = addr;

            RETURN_OK();
        }  
    }

    if ((opinfo.allowed_arg_type & ARG_RAM) && (arg_str[0] == '[') && (arg_str[arg_len - 1] == ']')) {
        if (arg_str[arg_len - 1] != ']')
            RETURN_ERROR(PARSING_SQUARE_BRACKET_NOT_CLOSED);
        // fprintf(stderr, "[] used\n");
        instr->arg_type_mask |= ARG_RAM;
        arg_str[arg_len - 1] = '\0';
        arg_str++;
    }

    if (opinfo.allowed_arg_type & ARG_STK_ELEM) {
        char *end_ptr = NULL;
        errno = 0;
        long long parsed_numb = strtoll(arg_str, &end_ptr, 10);

        if (errno == ERANGE || parsed_numb > STK_ELEM_MAX || parsed_numb < STK_ELEM_MIN) {
            *valid_chars -= bytes_parsed_arg;
            
            RETURN_ERROR(PARSING_ARG_OUT_OF_RANGE);
        }

        if (*end_ptr == '\0') {
            if (check_for_bad_char(bad_char)) {
                *valid_chars -= 1;

                RETURN_ERROR(PARSING_BAD_CHAR);
            }
            
            instr->is_comment_or_label = false;
            instr->opcode = opinfo.opcode;
            instr->arg_type_mask |= ARG_STK_ELEM;
            instr->arg_value = (stk_elem_t)parsed_numb;

            RETURN_OK();
        }
    }

    if (opinfo.allowed_arg_type & ARG_REG) {
        RegInfo reginfo = get_reginfo_by_str(arg_str);
        
        if (reginfo.regcode == REG_UNKNOWN) {
            *valid_chars -= bytes_parsed_arg;

            RETURN_ERROR(PARSING_BAD_OP_ARG);
        }

        if (check_for_bad_char(bad_char)) {
            *valid_chars -= 1;

            RETURN_ERROR(PARSING_BAD_CHAR);
        }

        instr->is_comment_or_label = false;
        instr->opcode = opinfo.opcode;
        instr->arg_type_mask |= ARG_REG;
        instr->arg_value = (stk_elem_t)reginfo.regcode;

        RETURN_OK();
    }
    
    RETURN_ERROR(PARSING_UNABLE_TO_PARSE_ARG);
}

