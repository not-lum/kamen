#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "./secstack/src/secstack.h"


int parse_line(char *cmd, int *arg) {
    assert(cmd != NULL);
    assert(arg != NULL);

    char *line = NULL;
    size_t line_size = 0;

    getline(&line, &line_size, stdin);

    int numb_of_args = sscanf(line, "%s %d", cmd, arg);

    free(line);
    return numb_of_args;
}

int main() {
    stack_t stack = {};
    StackStatus status = stack_init(&stack, 0
                                    ON_DEBUG(,
                                    VAR_NAME(stk1),
                                    __func__,
                                    __FILE__,
                                    __LINE__));
    char cmd[5] = "";
    int arg = 0;

    while (true) {
        int numb_of_args = parse_line(cmd, &arg);

//        printf("cmd: <%s>\n", cmd);
//        printf("arg: %d\n", arg);
//        printf("numb_of_args: %d\n", numb_of_args);

        if (numb_of_args == 2) {
            if (strcmp(cmd, "push") == 0) {
                stack_push(&stack, arg, &status);
            }
        } else if(numb_of_args == 1) {
            if (strcmp(cmd, "add") == 0) {
                int first_arg = stack_pop(&stack, &status);
                int second_arg = stack_pop(&stack, &status);

                stack_push(&stack, first_arg + second_arg, &status);
            }
            else if (strcmp(cmd, "div") == 0) {
                int first_arg = stack_pop(&stack, &status);
                int second_arg = stack_pop(&stack, &status);

                stack_push(&stack, second_arg / first_arg, &status);
            }
            else if (strcmp(cmd, "out") == 0) {
                int result = stack_pop(&stack, &status);
                printf("RESULT: %d\n", result);
                break;
            }
        }
    }

    stack_destroy(&stack);
    return 0;
}

