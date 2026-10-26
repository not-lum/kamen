#include "write_utils.h"

#include "parsing_utils.h"
#include "../file_utils.h"

#include <assert.h>


const char *write_error_str(WriteStatus status) {
    switch (status) {
        case WRITE_OK:
            return "write ok";
        case WRITE_ERROR_WHILE_PARSING:
            return "error while parsing";
        case WRITE_BCODE_FILE_OPEN_FAILED:
            return "bytecode file open failed";
        case WRITE_LISTING_FILE_OPEN_FAILED:
            return "error while openning listing file";
        default:
            return "unknown write error";
    }
}

static void write_instr_to(Instr *instr, FILE *to) {
    fwrite(&instr->opcode, sizeof(uint8_t), 1, to);

    if (instr->arg_type_mask != ARG_NONE) {
        fwrite(&instr->arg_type_mask, sizeof(uint8_t), 1, to);
        fwrite(&instr->arg_value, sizeof(stk_elem_t), 1, to);
    }
}


static void list_instr_to(Instr *instr, FILE *to) {
    fprintf(to, STK_ELEM_PRINTF_FMT, instr->opcode);

    if (instr->arg_type_mask != ARG_NONE) {
        fprintf(to, STK_ELEM_PRINTF_FMT, instr->arg_type_mask);
        fprintf(to, STK_ELEM_PRINTF_FMT, instr->arg_value);
    }

    fputc('\n', to);
}


WriteStatus assembly_indexed_file(IndexedFile *idx_file,
                                  const char *bcode_filename,
                                  const char *listing_filename,
                                  ParsingErrorInfo *error_info) {
    assert(idx_file != NULL);
    assert(bcode_filename != NULL);
    assert(listing_filename != NULL);

    FILE *bcode_file = fopen(bcode_filename, "wb");
    if (bcode_file == NULL)
        return WRITE_BCODE_FILE_OPEN_FAILED;

    FILE *listing_file = fopen(listing_filename, "w");
    if (listing_file == NULL)
        return WRITE_LISTING_FILE_OPEN_FAILED;

    ParsingContext parsing_context = {.asm_file = *idx_file};
    ParsingStatus status = collect_labels(&parsing_context, error_info);
    if (status != PARSING_OK) {
        // fprintf(stderr, "collect error\n");
        fclose(bcode_file);
        fclose(listing_file);

        return WRITE_ERROR_WHILE_PARSING;
    }

    // for (size_t i = 0; i < 10; i++)
    //     fprintf(stderr, "(label_name, addr): (%s, %d)\n",
    //             parsing_context.labels[i].label_name, parsing_context.labels[i].addr);

    for (size_t line = 0; line < idx_file->lines_numb; line++) {
        Instr instr = {};

        status = parse_line(&parsing_context, &instr, error_info);
        if (status != PARSING_OK) {
            fclose(bcode_file);
            fclose(listing_file);

            return WRITE_ERROR_WHILE_PARSING;
        }

        // fprintf(stderr, "curr_line: %zu\n", parsing_context.curr_line_numb);
        // fprintf(stderr, "curr_line: %zu\n", parsing_context.curr_line_numb);
        // TODO: add error handling for these

        if (!instr.is_not_code) {
            write_instr_to(&instr, bcode_file);
            list_instr_to(&instr, listing_file);
        }
    }

    fclose(bcode_file);
    fclose(listing_file);

    return WRITE_OK;
}
