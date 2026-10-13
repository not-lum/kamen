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
    EXIT_BAD_ARGS
} ExitCode;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Correct usage:\n"
                        "%s <input_file>\n", argv[0]);
        return EXIT_BAD_ARGS;
    }

    // TODO: error handling

    stack_t stack = {};
    StackStatus stk_status = STACK_INIT_DBG(&stack, 0);

    Code code_file = {};
    FileStatus f_status = load_file_alloc(&code_file, argv[1]); 
    
    SPU spu = {};
    SPUStatus spu_status = spu_init(&spu, &code_file, &stack);

    spu_status = spu_start(&spu, 0);

    fprintf(stderr, "SPU finished with status: %s\n", spu_error_str(spu_status));

    spu_destroy(&spu);
    return EXIT_OK;
}

