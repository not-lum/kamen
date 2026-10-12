#include "../listing.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>


int parse_line(char *cmd, int *arg, FILE *file) {
    assert(cmd != NULL);
    assert(arg != NULL);

    char *line = NULL;
    size_t line_size = 0;

    if (getline(&line, &line_size, file) == -1)
        return -1;

    fprintf(stderr, "line: %s\n", line);

    int numb_of_args = sscanf(line, "%s %d", cmd, arg);

    free(line);
    return numb_of_args;
}

int print_op(uint8_t op, FILE *listing_stream, FILE *output_stream) {
    // TODO: variable number of args
    fprintf(listing_stream, "%d ", op);
    fwrite(&op, 1, sizeof(uint8_t), output_stream); // TODO: maybe dump ops as ints too
}

int print_arg(int arg, FILE *listing_stream, FILE *output_stream) {
    fprintf(listing_stream, "%d\n", arg);
    fwrite(&arg, sizeof(int), sizeof(uint8_t), output_stream);
}

int main(int argc, char **argv) {
    // TODO: add error handling
    FILE *input_stream = stdin;
    FILE *output_stream = stdout;
    FILE *listing_stream = fopen(".kamen_asm.log", "w");
    // TODO: STRUCT for this ^^^^^ + no bufferization

    if (argc == 2) {
        input_stream = fopen(argv[1], "rb");    
    }
    else if (argc == 3) {
        input_stream = fopen(argv[1], "rb");    
        output_stream = fopen(argv[2], "wb");    
    }

    while (true) {
        char cmd[10] = {};
        int arg = 0;

        int numb_of_args = parse_line(cmd, &arg, input_stream);

        if (numb_of_args == -1) {
            fprintf(stderr, "End of file\n");
            return 1;
        }

        if (numb_of_args == 2) {
            if (strcmp(cmd, "push") == 0) {
                print_op(ASM_PUSH, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "pushr") == 0) {
                print_op(ASM_PUSH_REG, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "popr") == 0) {
                print_op(ASM_POP_REG, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "jmp") == 0) {
                print_op(ASM_JMP, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "je") == 0) {
                print_op(ASM_JE, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "ja") == 0) {
                print_op(ASM_JA, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "jb") == 0) {
                print_op(ASM_JB, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "jae") == 0) {
                print_op(ASM_JAE, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "jbe") == 0) {
                print_op(ASM_JBE, listing_stream, output_stream);
                print_arg(arg, listing_stream, output_stream);
            }
        }
        else if(numb_of_args == 1) {
            if (strcmp(cmd, "pop") == 0) {
                print_op(ASM_POP, listing_stream, output_stream);
            }
            if (strcmp(cmd, "add") == 0) {
                print_op(ASM_ADD, listing_stream, output_stream);
            }
            if (strcmp(cmd, "mul") == 0) {
                print_op(ASM_MUL, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "div") == 0) {
                print_op(ASM_DIV, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "out") == 0) {
                print_op(ASM_OUT, listing_stream, output_stream);
            }
            else if (strcmp(cmd, "hlt") == 0) {
                print_op(ASM_HLT, listing_stream, output_stream);
                break;
            }
        }
    }

    fclose(input_stream);
    fclose(output_stream);
    fclose(listing_stream);
    return 0;
}
