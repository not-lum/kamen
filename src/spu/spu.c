#include "spu.h"
#include "../listing.h"
#include "../dump_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>


#define IP(spu) (spu)->regs[REG_IP]

#define CHECK_REG_IS_VALID(reg_ind) do { \
    if ((reg_ind) < 0 || (reg_ind) >= REGS_COUNT) \
        return SPU_INVALID_REGISTER_INDEX; \
} while (0)

#define CHECK_RAM_ADDR_IS_VALID(addr) do { \
    if ((addr) >= RAM_SIZE || (addr) < 0) \
        return SPU_SET_ARG_OUT_OF_RANGE; \
} while (0)

const char *spu_error_str(SPUStatus status) {
    switch (status) {
        case SPU_OK:
            return "SPU ok";
        case SPU_PUSH_FAILED:
            return "failed while doing 'push'";
        case SPU_POP_FAILED:
            return "failed while doing 'pop'";
        case SPU_INTERNAL_POP_FAILED:
            return "failed while doing internal 'pop'";
        case SPU_INTERNAL_PUSH_FAILED:
            return "failed while doing internal 'push'";
        case SPU_DIVISION_BY_ZERO:
            return "division by zero";
        case SPU_INPUT_FAILED:
            return "failed to read input";
        case SPU_BYTECODE_PREMATURE_END:
            return "bytecode ended prematurely";
        case SPU_TIME_CALC_ERROR:
            return "time calculation error";
        case SPU_SLEEP_ERROR:
            return "sleep error";
        case SPU_SLEEP_ARG_LESS_THAN_ZERO:
            return "sleep argument is less than zero";
        case SPU_SET_ARG_OUT_OF_RANGE:
            return "set argument out of range";
        case SPU_INVALID_REGISTER_INDEX:
            return "attempted to access non-existent register";
        default:
            return "unknown SPU error";
    }
}

SPUStatus spu_init(SPU *spu, Code *code, stack_t *stack, stack_t *call_stack
                   ON_SPU_DEBUG(, const char* _dbg_logfile)) {
    assert(spu != NULL);
    assert(code != NULL);
    assert(stack != NULL);
    assert(call_stack != NULL);

    spu->code = *code;
    spu->stack = *stack;
    spu->call_stack = *call_stack;
    spu->disp_fps = 30;

    ON_SPU_DEBUG(
    init_spu_dbg_file(spu, _dbg_logfile);
    )

    return SPU_OK;
}

SPUStatus spu_destroy(SPU *spu) {
    assert(spu != NULL);

    destroy_loaded_file(&spu->code);
    stack_destroy(&spu->stack);
    stack_destroy(&spu->call_stack);

    ON_SPU_DEBUG(
    fclose(spu->_dbg_logfile);
    )

    return SPU_OK;
}

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


SPUStatus spu_start(SPU *spu) {
    assert(spu != NULL);
    
    uint8_t *content = spu->code.content;
    size_t content_len = spu->code.content_len;

    while ((size_t)IP(spu) < content_len) {
        OpInfo opinfo = get_opinfo_by_opcode(content[IP(spu)]);

        if (opinfo.opcode == ASM_HLT)
            break;

        if (opinfo.allowed_arg_type == ARG_NONE) {
            ON_SPU_DEBUG(
            log_instr(spu->_dbg_logfile, opinfo, ARG_NONE, 0);
            )

            ON_SPU_DEBUG_STDERR(
            log_instr(stderr, opinfo, ARG_NONE, 0);
            )
    
            IP(spu) += INSTR_NO_ARG_SIZE;

            opinfo.func(spu, 0);
            continue;
        }

        // TODO: move spu funcs to shared place

        if ((size_t)IP(spu) + 2 > content_len - 1)
            return SPU_BYTECODE_PREMATURE_END;

        ArgTypeMask arg_type_mask = content[IP(spu) + 1];
        stk_elem_t arg = 0;
        memcpy(&arg, content + IP(spu) + 2, sizeof(stk_elem_t));

        ON_SPU_DEBUG(
        log_instr(spu->_dbg_logfile, opinfo, arg_type_mask, arg);
        )

        ON_SPU_DEBUG_STDERR(
        log_instr(stderr, opinfo, arg_type_mask, arg);
        )

        if ((arg_type_mask & ARG_REG) && opinfo.opcode != ASM_POP && opinfo.opcode != ASM_INP) {
            CHECK_REG_IS_VALID(arg);
            arg = spu->regs[arg];
        }

        if (arg_type_mask & ARG_RAM) {
            CHECK_RAM_ADDR_IS_VALID(arg);
            arg = spu->ram[arg];
        }

        IP(spu) += INSTR_DEFAULT_SIZE;

        SPUStatus status = opinfo.func(spu, arg);
        if (status != SPU_OK)
            return status;
    }

    return SPU_OK;
}
