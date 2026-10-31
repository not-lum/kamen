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
#include <strings.h>


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
        case PARSING_UNABLE_TO_PARSE_ARG:
            return "unable to parse operation argument";
        case PARSING_EMPTY_LABEL:
            return "the label is empty";
        case PARSING_SQUARE_BRACKET_NOT_CLOSED:
            return "square bracket is not closed";
        case PARSING_UNKNOWN_LABEL:
            return "reference to undefined label";
        case PARSING_NOT_A_NUMBER:
            return "given argument is not a number";
        case PARSING_BAD_NUMBER:
            return "argument contains non-numeric characters, but supposed to be a number";
        case PARSING_LABEL_DOESNT_END_WITH_COLON:
            return "label definition must end with colon";
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

bool is_comment(const char *str) {
    return str_starts_with(str, COMMENT_CHAR);
}

bool is_label(const char *str) {
    return str_starts_with(str, '.');
}

bool is_empty_line(const char *str) {
    return str_starts_with(str, '\0');
}

// TODO: todo -> list of features in README.md

void print_parsing_error(ParsingErrorInfo error_info) {
    fprintf(stderr, ANSI_RED "Syntax Error " ANSI_RESET "(" ANSI_GREEN "%zu:%zu" ANSI_RESET "): ("
            ANSI_LIGHT_BLUE "%s" ANSI_RESET ")\n",
            error_info.line_index + 1, error_info.column + 1, error_info.msg);

    fputs(ANSI_LIGHT_BLUE, stderr);
    size_t prefix_len = fprintf(stderr, "%zu: ", error_info.line_index + 1);
    fputs(ANSI_RESET, stderr);

    fprintf(stderr, ANSI_YELLOW "%s\n" ANSI_RESET, error_info.line);

    size_t arrow_indent_len = prefix_len + error_info.column + 1;
    for (size_t i = 0; i < arrow_indent_len ; i++) {
        if (i == arrow_indent_len - 1)
            fputc('^', stderr);
        else
            fputc(' ', stderr);
    }

    fputc('\n', stderr);
}

void make_error_info(ParsingContext *ctx, ParsingStatus status,
                     ParsingErrorInfo *out_error_info) {
    assert(ctx != NULL);
    assert(out_error_info != NULL);

    out_error_info->msg = parsing_error_str(status);
    out_error_info->line = indexed_file_get_line(&ctx->asm_file, ctx->line_index);
    out_error_info->line_index = ctx->line_index;
    out_error_info->column = ctx->column;
}


ParsingStatus parse_op(ParsingContext *ctx, OpInfo *out_opinfo) {
    assert(ctx != NULL);
    assert(out_opinfo != NULL);

    char *line = indexed_file_get_line(&ctx->asm_file, ctx->line_index);

    char op_str[MAX_OP_LEN] = {};
    int parsed_len = 0;
    int indent_len = 0;
    size_t sscanf_args_numb = sscanf(line, " %n%" MAX_OP_LEN_STR "s %n",
                                     &indent_len, op_str, &parsed_len);
    ctx->column += indent_len;

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
    
    ctx->column += parsed_len - indent_len;

    *out_opinfo = opinfo;

    return PARSING_OK;
}

ParsingStatus parse_arg_str(ParsingContext *ctx, ArgStr *arg_str) {
    assert(ctx != NULL);
    assert(arg_str != NULL);

    char *line = indexed_file_get_line(&ctx->asm_file, ctx->line_index);

    if (line[ctx->column] == '\0') {
        return PARSING_UNABLE_TO_PARSE_ARG;
    } else {
        size_t sscanf_args_numb = sscanf(line + ctx->column,
                                         "%" MAX_ARG_LEN_STR "s%n",
                                         arg_str->str, &arg_str->len);

        if (sscanf_args_numb == 0)
            return PARSING_UNABLE_TO_PARSE_ARG;
    }

    // fprintf(stderr, "arg_str: %s\n", arg_str);
    // fprintf(stderr, "valid_chars: %zu\n", *valid_chars);
    
    ctx->column += arg_str->len;

    return PARSING_OK;
}

bool detect_bad_char(ParsingContext *ctx, char *str) {
    while (isspace(*str)) {
        str++;
        ctx->column++;
    }

    if (*str != COMMENT_CHAR && *str != '\0') {
        return true;
    }

    return false;
}


static ParsingStatus handle_arg_none(ParsingContext *ctx,
                              Instr* out_instr) {
    assert(ctx != NULL);
    assert(out_instr != NULL);

    out_instr->arg_type_mask |= ARG_NONE;
    out_instr->arg_value = 0;

    return PARSING_OK;
}


static ParsingStatus handle_arg_label(ParsingContext *ctx,
                               ArgStr *arg_str,
                               Instr *out_instr) {
    assert(ctx != NULL);
    assert(arg_str != NULL);
    assert(out_instr != NULL);

    stk_elem_t code_addr = 0;

    if (find_label(ctx, arg_str->str + 1, &code_addr)) {
        out_instr->arg_type_mask = ARG_STK_ELEM;
        out_instr->arg_value = code_addr;

        return PARSING_OK;
    }

    ctx->column -= arg_str->len;
    return PARSING_UNKNOWN_LABEL;
}

static ParsingStatus handle_arg_stk_elem(ParsingContext *ctx,
                                  ArgStr *arg_str,
                                  Instr *out_instr) {
    assert(ctx != NULL);
    assert(arg_str != NULL);
    assert(out_instr != NULL);

    char *end_ptr = NULL;
    errno = 0;
    long long parsed_numb = strtoll(arg_str->str, &end_ptr, 10);

    if (errno == ERANGE || parsed_numb > STK_ELEM_MAX || parsed_numb < STK_ELEM_MIN) {
        ctx->column -= arg_str->len;
        
        return PARSING_ARG_OUT_OF_RANGE;
    }

    if (*end_ptr == '\0') {
        out_instr->arg_type_mask |= ARG_STK_ELEM;
        out_instr->arg_value = (stk_elem_t)parsed_numb;

        return PARSING_OK;
    }

    if (*end_ptr == *arg_str->str)
        return PARSING_NOT_A_NUMBER;

    ctx->column -= arg_str->len;
    return PARSING_BAD_NUMBER;
}

static ParsingStatus handle_arg_reg(ParsingContext *ctx,
                             const ArgStr *arg_str,
                             Instr *out_instr) {
    assert(ctx != NULL);
    assert(arg_str != NULL);
    assert(out_instr != NULL);

    RegInfo reginfo = get_reginfo_by_str(arg_str->str);
        
    if (reginfo.regcode == REG_UNKNOWN) {
        ctx->column -= arg_str->len;

        return PARSING_UNABLE_TO_PARSE_ARG;
    }

    out_instr->arg_type_mask |= ARG_REG;
    out_instr->arg_value = (stk_elem_t)reginfo.regcode;

    return PARSING_OK;
}


static ParsingStatus enable_arg_ram(ParsingContext *ctx, ArgStr *arg_str,
                             Instr *out_instr) {
    assert(ctx != NULL);
    assert(arg_str != NULL);
    assert(out_instr != NULL);

    out_instr->arg_type_mask |= ARG_RAM;
    arg_str->str[arg_str->len - 1] = '\0';
    arg_str->str++;

    return PARSING_OK;
}


static ParsingStatus parse_arg(ParsingContext *ctx, ArgTypeMask allowed_arg_type,
                               Instr* out_instr) {
    assert(ctx != NULL);
    assert(out_instr != NULL);

    ArgStr arg_str = {
        .str = (char [MAX_ARG_LEN]){}, // compound literal !!!
        .len = 0
    };
    ParsingStatus status = parse_arg_str(ctx, &arg_str);

    if (status != PARSING_OK) {
        if (allowed_arg_type & ARG_NONE)
            return handle_arg_none(ctx, out_instr);
        else
            return status;
    }

    if ((allowed_arg_type & ARG_LABEL) && arg_str.str[0] == '.')
        return handle_arg_label(ctx, &arg_str, out_instr);

    if ((allowed_arg_type & ARG_RAM) && (arg_str.str[0] == '[')) {
        if (arg_str.str[arg_str.len - 1] != ']')
            return PARSING_SQUARE_BRACKET_NOT_CLOSED;

        enable_arg_ram(ctx, &arg_str, out_instr);
    }

    if (allowed_arg_type & ARG_STK_ELEM) {
        status = handle_arg_stk_elem(ctx, &arg_str, out_instr);
        if (status == PARSING_OK || status == PARSING_BAD_NUMBER ||
            status == PARSING_ARG_OUT_OF_RANGE)
            return status;
    }

    if (allowed_arg_type & ARG_REG)
        return handle_arg_reg(ctx, &arg_str, out_instr);
    
    if (!is_comment(arg_str.str)) {
        ctx->column -= arg_str.len;
        return PARSING_UNABLE_TO_PARSE_ARG;
    } else {
        ctx->column = indexed_file_get_line_len(&ctx->asm_file,
                                                ctx->line_index);
        return PARSING_OK;
    }
}


static ParsingStatus _parse_line_body(ParsingContext *ctx, Instr *instr) {
    assert(ctx != NULL);
    assert(instr != NULL);
    // TODO: separate into funcs, improve code quality - DONE
    // TODO: add add_label func - DONE

    char *line = indexed_file_get_line(&ctx->asm_file, ctx->line_index);

    if (is_comment(line) || is_label(line) || is_empty_line(line)) {
        instr->is_blank = true;
        return PARSING_OK;
    }

    OpInfo opinfo = {};
    ParsingStatus status = parse_op(ctx, &opinfo);
    if (status != PARSING_OK)
        return status;

    instr->is_blank = false;
    instr->opcode = opinfo.opcode;

    return parse_arg(ctx, opinfo.allowed_arg_type, instr);
}


ParsingStatus parse_line(ParsingContext *ctx, Instr *out_instr,
                         ParsingErrorInfo *out_error_info) {
    assert(ctx != NULL);
    assert(out_instr != NULL);
    // out_error_info could be NULL - in that case no error info is written

    char *line = indexed_file_get_line(&ctx->asm_file, ctx->line_index);

    ParsingStatus status = _parse_line_body(ctx, out_instr);
    if (status == PARSING_OK && !out_instr->is_blank && detect_bad_char(ctx, line + ctx->column))
        status = PARSING_BAD_CHAR;

    if (status != PARSING_OK) {
        if (out_error_info != NULL)
            make_error_info(ctx, status, out_error_info);

        return status;
    }

    ctx->line_index++;
    ctx->column = 0;
    return PARSING_OK;
}