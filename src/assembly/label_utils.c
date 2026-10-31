#include "parsing_utils.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>

// TODO: labels array in heap
static ParsingStatus add_label(ParsingContext *ctx, Label label) {
    assert(ctx != NULL);

    ctx->labels[ctx->labels_count] = label;
    ctx->labels_count++;

    return PARSING_OK;
}


ParsingStatus parse_label(ParsingContext *ctx, Label *out_label) {
    assert(ctx != NULL);
    assert(out_label != NULL);
    // TODO: hashes

    char *line = indexed_file_get_line(&ctx->asm_file, ctx->line_index);

    int parsed_before_colon_len = 0;
    int parsed_len = 0;
    size_t sscanf_args_numb = sscanf(line, " .%" MAX_LABEL_LEN_STR "[^:]%n:%n",
                                     out_label->name, &parsed_before_colon_len, &parsed_len); 
    
    ctx->column += parsed_len;

    if (parsed_len == 0) {
        ctx->column += parsed_before_colon_len;
        return PARSING_LABEL_DOESNT_END_WITH_COLON;
    }

    if (sscanf_args_numb == 1) {
        out_label->addr = ctx->bytecode_len;

        return PARSING_OK;
    }

    return PARSING_EMPTY_LABEL;
}


static ParsingStatus _collect_labels_body(ParsingContext *ctx) {
    assert(ctx != NULL);
    // out_error_info

    while (ctx->line_index < ctx->asm_file.lines_numb) {
        char *line = indexed_file_get_line(&ctx->asm_file, ctx->line_index);

        if (is_label(line)) {
            Label label = {};
            ParsingStatus status = parse_label(ctx, &label);
            if (status != PARSING_OK)
                return status;

            if (detect_bad_char(ctx, line + ctx->column)) {
                fprintf(stderr, "ctx->column: %zu\n", ctx->column);
                return PARSING_BAD_CHAR;
            }

            add_label(ctx, label); 
        } else if (!is_comment(line) && !is_empty_line(line)) {
            OpInfo opinfo = {};
            ParsingStatus status = parse_op(ctx, &opinfo);
            if (status != PARSING_OK)
                return status;

            if (opinfo.allowed_arg_type == ARG_NONE)
                ctx->bytecode_len += INSTR_NO_ARG_SIZE;
            else
                ctx->bytecode_len += INSTR_DEFAULT_SIZE;
        }

        ctx->line_index++;
        ctx->column = 0;
    }

    return PARSING_OK;
}

ParsingStatus collect_labels(ParsingContext *ctx, ParsingErrorInfo *out_error_info) {
    assert(ctx != NULL);

    ParsingStatus status = _collect_labels_body(ctx);
    if (status != PARSING_OK) {
        if (out_error_info != NULL)
            make_error_info(ctx, status, out_error_info);

        return status;
    }

    ctx->line_index = 0;
    ctx->column = 0;
    return PARSING_OK;
}

bool find_label(ParsingContext *ctx, const char *name, stk_elem_t *out_addr) {
    assert(ctx != NULL);
    assert(name != NULL);
    assert(out_addr != NULL);

    for (size_t i = 0; i < ctx->labels_count; i++) {
        if (strcmp(ctx->labels[i].name, name) == 0) {
            *out_addr = ctx->labels[i].addr;

            return true;
        }
    }

    return false;
}


