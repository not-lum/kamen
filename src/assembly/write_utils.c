#include "write_utils.h"

#include "parsing_utils.h"
#include "../file_utils.h"

#include <assert.h>


const char *write_error_str(WriteStatus status) {
    switch (status) {
        case WRITE_OK:
            return "write ok";
        case WRITE_ERROR_WHILE_PARSING:
            return "write error while parsing";
        case WRITE_BCODE_FILE_OPEN_FAILED:
            return "write byte-code file open failed";
        case WRITE_LISTING_FILE_OPEN_FAILED:
            return "write listing file open failed";
        default:
            return "unknown write error";
    }
}

static void write_instr_to(Instr *instr, FILE *to) {
    fwrite(&instr->opcode.type, sizeof(uint8_t), 1, to);

    if (instr->opcode.numb_of_args != ARGS_ZERO)
        fwrite(&instr->arg_value, sizeof(stk_elem_t), 1, to);
}


static void list_instr_to(Instr *instr, FILE *to) {
    fprintf(to, "%d", instr->opcode.type);
    
    if (instr->opcode.numb_of_args != ARGS_ZERO)
        fprintf(to, "%d", instr->arg_value);

    fputc('\n', to);
}


WriteStatus assembly_indexed_file(IndexedFile *idx_file,
                                  const char *bcode_filename,
                                  const char *listing_filename) {

    assert(idx_file != NULL);
    assert(bcode_filename != NULL);
    assert(listing_filename != NULL);

    FILE *bcode_file = fopen(bcode_filename, "wb");
    if (bcode_file == NULL)
        return WRITE_BCODE_FILE_OPEN_FAILED;

    FILE *listing_file = fopen(listing_filename, "wb");
    if (listing_file == NULL)
        return WRITE_LISTING_FILE_OPEN_FAILED;

    for (size_t i = 0; i < idx_file->lines_numb; i++) {
        Instr instr = {};
        ParsingStatus status = parse_line(idx_file, i, &instr);
        if (status != PARSING_OK) {
            fprintf(stderr, "Error while parsing: %s\n", parsing_error_str(status));
            fprintf(stderr, "line: %zu\n", i);
            fclose(bcode_file);
            fclose(listing_file);

            return WRITE_ERROR_WHILE_PARSING; // TODO: it should be more detailed, but how?
                                              // maybe struct with each kind of statuses?
        }

        // TODO: add error handling for these:
        write_instr_to(&instr, bcode_file);
        list_instr_to(&instr, listing_file);
        
    }

    fclose(bcode_file);
    fclose(listing_file);

    return WRITE_OK;
}
