#include "spu.h"
#include "../../secstack/src/secstack.h"
#include "../file_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>


typedef enum {
    EXIT_OK = 0,
    EXIT_BAD_ARGS,
    EXIT_STACK_INIT_FAIL,
    EXIT_CALL_STACK_INIT_FAIL,
    EXIT_ERROR_WHILE_LOADING_INPUT_FILE,
    EXIT_SPU_INIT_FAIL
} ExitCode;

#define FAIL() do { goto cleanup; } while (0)

int main(int argc, char **argv) {
    ExitCode exit_code = EXIT_OK;
    SPU spu = {};

    if (argc != 2) {
        fprintf(stderr, "Correct usage:\n"
                        "%s <input_file>\n", argv[0]);
        exit_code = EXIT_BAD_ARGS;

        FAIL();
    }

    // TODO: error handling

    stack_t stack = {};
    StackStatus stk_status = STACK_INIT_DBG(&stack, 0);
    if (stk_status != STACK_OK) {
        fprintf(stderr, "Stack error: %s\n", stack_error_str(stk_status));
        exit_code = EXIT_STACK_INIT_FAIL;

        FAIL();
    }

    stack_t call_stack = {};
    stk_status = STACK_INIT_DBG(&call_stack, 0);
    if (stk_status != STACK_OK) {
        fprintf(stderr, "Stack error: %s\n", stack_error_str(stk_status));
        exit_code = EXIT_CALL_STACK_INIT_FAIL;

        FAIL();
    }

    Code code_file = {};
    FileStatus f_status = load_file_alloc(&code_file, argv[1]); 
    if (f_status != FS_OK) {
        fprintf(stderr, "File error: %s\n", fs_error_str(f_status));
        exit_code = EXIT_ERROR_WHILE_LOADING_INPUT_FILE;

        FAIL();
    }
    
    SPUStatus spu_status = spu_init(&spu, &code_file, &stack, &call_stack);
    if (spu_status != FS_OK) {
        fprintf(stderr, "File error: %s\n", spu_error_str(spu_status));
        exit_code = EXIT_SPU_INIT_FAIL;

        FAIL();
    }

    spu_status = spu_start(&spu);
    if (spu_status != SPU_OK)
        fprintf(stderr, "SPU finished with status: %s\n", spu_error_str(spu_status));

cleanup:
    spu_destroy(&spu);

    return exit_code;
}

