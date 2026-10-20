#include "spu.h"
#include "../listing.h"
#include "../dump_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>


#define IP spu->regs[REG_IP]

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
        case SPU_DIVISION_BY_ZERO:
            return "SPU division by zero";
        case SPU_INPUT_FAILED:
            return "SPU failed to read input";
        default:
            return "unknown SPU error";
    }
}

SPUStatus spu_init(SPU *spu, Code *code, stack_t *stack, stack_t *call_stack) {
    assert(spu != NULL);
    assert(code != NULL);
    assert(stack != NULL);
    assert(call_stack != NULL);

    spu->code = *code;
    spu->stack = *stack;
    spu->call_stack = *call_stack;
    memset(spu->regs, 0, sizeof(spu->regs));

    return SPU_OK;
}

SPUStatus spu_destroy(SPU *spu) {
    assert(spu != NULL);

    destroy_loaded_file(&spu->code);
    stack_destroy(&spu->stack);
    stack_destroy(&spu->call_stack);

    return SPU_OK;
}

SPUStatus spu_call(SPU *spu, stk_elem_t addr) {
    assert(spu != NULL);

    StackStatus status = 0;
    stack_push(&spu->call_stack, IP, &status);
    if (status != STACK_OK) {
        return SPU_INTERNAL_PUSH_FAILED;
    }

    IP = addr;

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )    

    return SPU_OK;
}

SPUStatus spu_ret(SPU *spu, stk_elem_t elem) {
    assert(spu != NULL);

    StackStatus status = 0;
    IP = stack_pop(&spu->call_stack, &status);
    if (status != STACK_OK) {
        return SPU_INTERNAL_POP_FAILED;
    }

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )

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
        return SPU_PUSH_FAILED;
    }
    return SPU_OK;
}

SPUStatus spu_pop(SPU *spu, stk_elem_t res) {
    assert(spu != NULL);

    StackStatus status = 0;
    spu->regs[res] = stack_pop(&spu->stack, &status);

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )

    if (status != STACK_OK) {
        return SPU_POP_FAILED;
    }

    return SPU_OK;
}

SPUStatus spu_inp(SPU *spu, stk_elem_t res) {
    assert(spu != NULL);

    stk_elem_t input = 0;
    if (scanf("%d", &input) != 1)
        return SPU_INPUT_FAILED;
    
    spu->regs[res] = input;

    return SPU_OK;
}

SPUStatus spu_out(SPU *spu, stk_elem_t res) {
    assert(spu != NULL);

    StackStatus status = 0;
    stk_elem_t last_elem = stack_pop(&spu->stack, &status);
    if (status != STACK_OK) {
        return SPU_INTERNAL_POP_FAILED;
    }

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )

    ASM_OUT_print(last_elem);

    return SPU_OK;
}


static SPUStatus spu_bin_op(SPU *spu, Opcode opcode) {
    assert(spu != NULL);

    stk_elem_t first_elem = 0;
    stk_elem_t second_elem = 0;

    StackStatus status = stack_pop_n(&spu->stack, 2, &second_elem, &first_elem);
    if (status != STACK_OK)
        return SPU_INTERNAL_POP_FAILED;

    stk_elem_t result = 0;

    switch (opcode) {
        case ASM_ADD:
            result = first_elem + second_elem;
            break;
        case ASM_SUB:
            result = first_elem - second_elem;
            break;
        case ASM_MUL:
            result = first_elem * second_elem;
            break;
        case ASM_DIV:
            if (second_elem == 0)
                return SPU_DIVISION_BY_ZERO;

            result = first_elem / second_elem;
            break;
        default:
            break;
    }

    stack_push(&spu->stack, result, &status);
    if (status != STACK_OK)
        return SPU_INTERNAL_PUSH_FAILED;

    return SPU_OK;
}

SPUStatus spu_add(SPU *spu, stk_elem_t res) {
    return spu_bin_op(spu, ASM_ADD);
}

SPUStatus spu_sub(SPU *spu, stk_elem_t res) {
    return spu_bin_op(spu, ASM_SUB);
}

SPUStatus spu_mul(SPU *spu, stk_elem_t res) {
    return spu_bin_op(spu, ASM_MUL);
}

SPUStatus spu_div(SPU *spu, stk_elem_t res) {
    return spu_bin_op(spu, ASM_DIV);
}


SPUStatus spu_jmp(SPU *spu, stk_elem_t res) {
    assert(spu != NULL);

    IP = res;

    return SPU_OK;
}


SPUStatus spu_sqrt(SPU *spu, stk_elem_t elem) {
    assert(spu != NULL);

    StackStatus status = 0;
    int numb = stack_pop(&spu->stack, &status);
    if (status != STACK_OK)
        return SPU_INTERNAL_POP_FAILED;

    stk_elem_t root = lround(sqrt(numb));
    // fprintf(stderr, "orig_elem: %d\n", numb);
    // fprintf(stderr, "sqrt_root: %d\n", root);

    stack_push(&spu->stack, root, &status);
    if (status != STACK_OK)
        return SPU_INTERNAL_POP_FAILED;

    return SPU_OK;
}

static SPUStatus spu_cond_jmp(SPU *spu, stk_elem_t res, Opcode opcode) {
    assert(spu != NULL);

    stk_elem_t first_elem = 0;
    stk_elem_t second_elem = 0;

    StackStatus status = stack_pop_n(&spu->stack, 2, &second_elem, &first_elem);
    if (status != STACK_OK)
        return SPU_INTERNAL_POP_FAILED;

    bool should_jmp = false;

    switch (opcode) {
        case ASM_JE:
            should_jmp = first_elem == second_elem;
            break;
        case ASM_JA:
            should_jmp = first_elem > second_elem;
            break;
        case ASM_JB:
            should_jmp = first_elem < second_elem;
            break;
        case ASM_JAE:
            should_jmp = first_elem >= second_elem;
            break;
        case ASM_JBE:
            should_jmp = first_elem <= second_elem;
            break;
        default:
            break;
    }

    if (should_jmp)
        IP = res;

    return SPU_OK;
}

SPUStatus spu_je(SPU *spu, stk_elem_t res) {
    return spu_cond_jmp(spu, res, ASM_JE);
}

SPUStatus spu_ja(SPU *spu, stk_elem_t res) {
    return spu_cond_jmp(spu, res, ASM_JA);
}

SPUStatus spu_jb(SPU *spu, stk_elem_t res) {
    return spu_cond_jmp(spu, res, ASM_JB);
}

SPUStatus spu_jae(SPU *spu, stk_elem_t res) {
    return spu_cond_jmp(spu, res, ASM_JAE);
}

SPUStatus spu_jbe(SPU *spu, stk_elem_t res) {
    return spu_cond_jmp(spu, res, ASM_JBE);
}

SPUStatus spu_start(SPU *spu) {
    assert(spu != NULL);
    
    uint8_t *content = spu->code.content;
    size_t content_len = spu->code.content_len;

    while (IP < content_len) {
        OpInfo opinfo = get_opinfo_by_opcode(content[IP]);
        ON_SPU_DEBUG(
        opcode_print(opinfo.opcode);
        )

        ArgTypeMask arg_type_mask = content[IP + 1];
        stk_elem_t arg = 0; // TODO: remove for ops without args
        memcpy(&arg, content + IP + 2, sizeof(stk_elem_t));

        if (opinfo.opcode == ASM_HLT)
            break;

        if ((arg_type_mask & ARG_REG) && opinfo.opcode != ASM_POP && opinfo.opcode != ASM_INP)
            arg = spu->regs[arg];

        IP += 6;

        SPUStatus status = opinfo.func(spu, arg);
        if (status != SPU_OK)
            return status;
    }

    return SPU_OK;
}
