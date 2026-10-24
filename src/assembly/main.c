#include "../dump_utils.h"
#include "../file_utils.h"
#include "parsing_utils.h"
#include "write_utils.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>


#define LISTING_FILENAME ".kamen_listing.log"
#define FAIL() do { goto cleanup; } while (0)

typedef enum {
    EXIT_OK = 0,
    EXIT_BAD_ARGS,
    EXIT_CANNOT_OPEN_INPUT_FILE,
    EXIT_CANNOT_ASSEMBLE_FILE
} ExitCode;


int main(int argc, char **argv) {
    // TODO: add error handling
    // TODO: disable listing bufferisation: setvbuf 

    ExitCode exit_code = EXIT_OK;
    IndexedFile asm_file = {};

    if (argc != 3) {
        fprintf(stderr, "Correct usage:\n"
                        "%s: <input_file> <output_file>\n", argv[0]);
        
        exit_code = EXIT_BAD_ARGS;
        FAIL();
    }


    FileStatus f_status = create_indexed_file(&asm_file, argv[1]);
    if (f_status != 0) {
        exit_code = EXIT_CANNOT_OPEN_INPUT_FILE;
        fprintf(stderr, "Error while indexing file '%s': %s\n",
                argv[1], fs_error_str(f_status));
        FAIL();
    }

    ParsingErrorInfo error_info = {};
    WriteStatus w_status = assembly_indexed_file(&asm_file, argv[2], LISTING_FILENAME, &error_info);
    if (w_status != WRITE_OK) {
        exit_code = EXIT_CANNOT_ASSEMBLE_FILE;
        fprintf(stderr, "Error while assembling: %s\n", write_error_str(w_status));

        if (w_status == WRITE_ERROR_WHILE_PARSING) {
            print_parsing_error(error_info, asm_file);
        }


        FAIL();
    }

    fprintf(stderr, "Success\n");

cleanup:
    destroy_indexed_file(&asm_file);

    return exit_code;
}

