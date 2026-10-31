#include "spu.h"
#include "spu_internal.h"
#include "../listing.h"
#include "../dump_utils.h"
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>


SPUStatus spu_call(SPU *spu, stk_elem_t addr) {
    assert(spu != NULL);

    StackStatus status = 0;
    stack_push(&spu->call_stack, IP(spu), &status);
    if (status != STACK_OK) {
        return SPU_INTERNAL_PUSH_FAILED;
    }

    IP(spu) = addr;

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )    

    return SPU_OK;
}

SPUStatus spu_ret(SPU *spu, stk_elem_t _) {
    assert(spu != NULL);

    StackStatus status = 0;
    IP(spu) = stack_pop(&spu->call_stack, &status);
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

SPUStatus spu_pop(SPU *spu, stk_elem_t reg_ind) {
    assert(spu != NULL);
    CHECK_REG_IS_VALID(reg_ind);

    StackStatus status = 0;
    spu->regs[reg_ind] = stack_pop(&spu->stack, &status);

    ON_DUMP_STACK(
    dump_stack(&spu->stack, status);
    )

    if (status != STACK_OK) {
        return SPU_POP_FAILED;
    }

    return SPU_OK;
}

SPUStatus spu_set(SPU *spu, stk_elem_t ram_addr) {
    assert(spu != NULL);
    CHECK_RAM_ADDR_IS_VALID(ram_addr);

    StackStatus status = 0;
    stk_elem_t value = stack_pop(&spu->stack, &status);
    if (status != STACK_OK)
        return SPU_INTERNAL_PUSH_FAILED;

    spu->ram[ram_addr] = value;

    return SPU_OK;
}

SPUStatus spu_disp(SPU *spu, stk_elem_t _) {
    assert(spu != NULL);

    struct timeval start_tv, end_tv = {};

    if (gettimeofday(&start_tv, NULL) != 0)
        return SPU_TIME_CALC_ERROR;

    printf("\033[?2026h\033[H");

    for (size_t i = 0; i < RAM_SIZE; i++) {
        if (spu->ram[i] == 0)
            printf("  ");
        else
            printf("██");

        if ((i + 1) % RAM_WIDTH == 0)
            putchar('\n');
    }

    printf("\033[?2026l");

    if (gettimeofday(&end_tv, NULL) != 0)
        return SPU_TIME_CALC_ERROR;

    // TODO: int overflow prevention checks
    int64_t time_to_sleep = 1000000000 / spu->disp_fps -
                           (end_tv.tv_sec - start_tv.tv_sec) * 1000000000 -
                           (end_tv.tv_usec - start_tv.tv_usec) * 1000 - DISP_DELAY;

    struct timespec delay = {
        .tv_sec = time_to_sleep / 1000000000,
        .tv_nsec = time_to_sleep % 1000000000
    };

    if (nanosleep(&delay, NULL) != 0)
        return SPU_SLEEP_ERROR;

    return SPU_OK;
}

SPUStatus spu_dispfps(SPU *spu, stk_elem_t fps) {
    assert(spu != NULL);

    spu->disp_fps = fps;

    return SPU_OK;
}

SPUStatus spu_dispclr(SPU *spu, stk_elem_t _) {
    assert(spu != NULL);

    printf("\033[2J");

    return SPU_OK;
}

SPUStatus spu_sleep(SPU* spu, stk_elem_t microsecs) {
    assert(spu != NULL);

    if (microsecs < 0)
        return SPU_SLEEP_ARG_LESS_THAN_ZERO;

    struct timespec delay = {
        .tv_sec = microsecs / 1000000,
        .tv_nsec = (microsecs % 1000000) * 1000
    };

    if (nanosleep(&delay, NULL) != 0)
        return SPU_SLEEP_ERROR;

    return SPU_OK;
}

SPUStatus spu_inp(SPU *spu, stk_elem_t reg_ind) {
    assert(spu != NULL);
    CHECK_REG_IS_VALID(reg_ind);

    stk_elem_t input = 0;
    if (scanf(STK_ELEM_SCANF_FMT, &input) != 1)
        return SPU_INPUT_FAILED;
    
    spu->regs[reg_ind] = input;

    return SPU_OK;
}

static void ASM_OUT_print(stk_elem_t last_elem) {
    printf(ANSI_GREEN "OUTPUT: " ANSI_RESET "<" ANSI_YELLOW STK_ELEM_PRINTF_FMT ANSI_RESET ">\n", last_elem);
}


SPUStatus spu_out(SPU *spu, stk_elem_t _) {
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

SPUStatus spu_add(SPU *spu, stk_elem_t _) {
    return spu_bin_op(spu, ASM_ADD);
}

SPUStatus spu_sub(SPU *spu, stk_elem_t _) {
    return spu_bin_op(spu, ASM_SUB);
}

SPUStatus spu_mul(SPU *spu, stk_elem_t _) {
    return spu_bin_op(spu, ASM_MUL);
}

SPUStatus spu_div(SPU *spu, stk_elem_t _) {
    return spu_bin_op(spu, ASM_DIV);
}

SPUStatus spu_jmp(SPU *spu, stk_elem_t res) {
    assert(spu != NULL);

    IP(spu) = res;

    return SPU_OK;
}

SPUStatus spu_sqrt(SPU *spu, stk_elem_t _) {
    assert(spu != NULL);

    StackStatus status = 0;
    int numb = stack_pop(&spu->stack, &status);
    if (status != STACK_OK)
        return SPU_INTERNAL_POP_FAILED;

    stk_elem_t root = lround(sqrt(numb));

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
        IP(spu) = res;

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
