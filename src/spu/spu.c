#include "spu.h"
#include "../dump_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>


const char *spu_error_str(SPUStatus status) {
    switch (status) {
        case SPU_OK:
            return "SPU ok";
        case SPU_PUSH_FAILED:
            return "SPU failed while doing 'push'";
        case SPU_POP_FAILED:
            return "SPU failed while doing 'pop'";
        case SPU_INTERNAL_POP_FAILED:
            return "SPU failed while doing internal 'pop'";
        case SPU_INTERNAL_PUSH_FAILED:
            return "SPU failed while doing internal 'push'"; 
        default:
            return "uknown SPU error";
    }
}

SPUStatus spu_init(SPU *spu, Code *code, stack_t *stack) {
    assert(spu != NULL);
    assert(code != NULL);
    assert(stack != NULL);

    spu->code = *code;
    spu->stack = *stack;
    memset(spu->regs, 0, sizeof(spu->regs)); // TODO: add option to set custom regs

    return SPU_OK;
}

SPUStatus spu_destroy(SPU *spu) {
    assert(spu != NULL);

    destroy_loaded_file(&spu->code);
    stack_destroy(&spu->stack);

    return SPU_OK;
}

SPUStatus spu_push(SPU *spu, stk_elem_t elem) {
    assert(spu != NULL);

    StackStatus status = 0;
    stack_push(&spu->stack, elem, &status);

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )

    if (status != STACK_OK) {
        // TODO: more detailed error handling
        return SPU_PUSH_FAILED;
    }
    return SPU_OK;
}

SPUStatus spu_pop(SPU *spu, bool reg, RegisterCode r_code) {
    assert(spu != NULL);

    StackStatus status = 0;

    if (reg)
        spu->regs[r_code] = stack_pop(&spu->stack, &status);
    else
        stack_pop(&spu->stack, &status);

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )

    if (status != STACK_OK) {
        // TODO: more detailed error handling
        return SPU_POP_FAILED;
    }

    return SPU_OK;
}

SPUStatus spu_out(SPU *spu) {
    assert(spu != NULL);

    StackStatus status = 0;
    stk_elem_t last_elem = stack_pop(&spu->stack, &status);

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )

    if (status != STACK_OK) {
        // TODO: more detailed error handling
        return SPU_INTERNAL_POP_FAILED;
    }

    ASM_OUT_print(last_elem);

    return SPU_OK;
}

SPUStatus spu_start(SPU *spu, size_t start) {
    assert(spu != NULL);
    assert(start <= spu->code.content_len);
    
    uint8_t *content = spu->code.content;
    size_t content_len = spu->code.content_len;
    size_t i = start; // TODO: replace with just start. replace with RIP

    while (i < content_len) {
        OpcodeType opcode = content[i];

        if (opcode == ASM_PUSH || opcode == ASM_PUSH_REG) {
            // TODO: check if there are sizeof(stk_elem_t) bytes in the end for our arg
            stk_elem_t arg = 0;
            memcpy(&arg, content + i + 1, sizeof(stk_elem_t));

            ON_SPU_DEBUG(
            opcode_print(opcode, arg);
            )

            if (opcode == ASM_PUSH_REG)
                arg = spu->regs[arg]; // TODO: think about this: user can access RIP and segfault possible

            SPUStatus status = spu_push(spu, arg);
            
            if (status != SPU_OK) 
                return status;

            i += 1 + sizeof(arg);
        }
        else if (opcode == ASM_POP || opcode == ASM_POP_REG) {
            // TODO: check if there are sizeof(stk_elem_t) bytes in the end for our arg
            stk_elem_t arg = 0;

            if (opcode == ASM_POP_REG) {
                arg = 0;
                memcpy(&arg, content + i + 1, sizeof(stk_elem_t));
            }

            ON_SPU_DEBUG(
            opcode_print(opcode, arg);
            )


            SPUStatus status = spu_pop(spu, (opcode == ASM_POP_REG), arg);

            if (status != SPU_OK) 
                return status;

            i += 1;
            if (opcode == ASM_POP_REG)
                i += sizeof(stk_elem_t);
        }
        else if (opcode == ASM_OUT) {
            ON_SPU_DEBUG(
            opcode_print(opcode);
            )

            SPUStatus status = spu_out(spu);
            getc(stdin);
        
            if (status != SPU_OK) 
                return status;

            i += 1;
        }
        else if (opcode == ASM_JMP || opcode == ASM_JE ||
                 opcode == ASM_JA || opcode == ASM_JB ||
                 opcode == ASM_JAE || opcode == ASM_JBE) {
            // TODO: check if there are sizeof(stk_elem_t) bytes in the end for our arg
            // TODO: solve alignment problem
            stk_elem_t arg = 0;
            memcpy(&arg, content + i + 1, sizeof(stk_elem_t));

            ON_SPU_DEBUG(
            opcode_print(opcode, arg);
            );

            if (opcode == ASM_JMP)
                return spu_start(spu, arg);

            stk_elem_t first_elem = 0;
            stk_elem_t second_elem = 0;

            StackStatus status = stack_pop_n(&spu->stack, 2, &second_elem, &first_elem);
            if (status != STACK_OK)
                return SPU_INTERNAL_POP_FAILED;

            if (opcode == ASM_JE && first_elem == second_elem)
                return spu_start(spu, arg);

            if (opcode == ASM_JA && first_elem > second_elem)
                return spu_start(spu, arg);

            if (opcode == ASM_JB && first_elem < second_elem)
                return spu_start(spu, arg);

            if (opcode == ASM_JAE && first_elem >= second_elem)
                return spu_start(spu, arg);

            if (opcode == ASM_JBE && first_elem >= second_elem)
                return spu_start(spu, arg);

            i += 1 + sizeof(stk_elem_t);
        }
        else if (opcode == ASM_ADD || opcode == ASM_MUL) {
            ON_SPU_DEBUG(
            opcode_print(opcode);
            )

            stk_elem_t first_elem = 0;
            stk_elem_t second_elem = 0;

            StackStatus status = stack_pop_n(&spu->stack, 2, &second_elem, &first_elem);
            if (status != STACK_OK)
                return SPU_INTERNAL_POP_FAILED;

            if (opcode == ASM_MUL)
                stack_push(&spu->stack, first_elem * second_elem, &status);
            else if (opcode == ASM_ADD)
                stack_push(&spu->stack, first_elem + second_elem, &status);

            if (status != STACK_OK)
                return SPU_INTERNAL_PUSH_FAILED;

            i += 1;
        }
        else if (opcode == ASM_HLT) {
            ON_SPU_DEBUG(
            opcode_print(opcode);
            )

            break;
        }
    }

    return SPU_OK;
}

