#pragma once

#include "../../secstack/src/secstack.h"
#include "../file_utils.h"

// Debug settings are located in this file:
#include "config/dbg_config.h"

#ifdef KAMEN_DUMP_STACK
#   define ON_DUMP_STACK(...) __VA_ARGS__
#else
#   define ON_DUMP_STACK(...)
#endif

#ifdef KAMEN_SPU_DBG
#   define ON_SPU_DEBUG(...) __VA_ARGS__
#else
#   define ON_SPU_DEBUG(...)
#endif

#ifdef KAMEN_SPU_DBG_STDERR
#   define ON_SPU_DEBUG_STDERR(...) __VA_ARGS__
#else
#   define ON_SPU_DEBUG_STDERR(...)
#endif

#define RAM_HEIGHT 60
#define RAM_WIDTH 80
#define RAM_SIZE RAM_HEIGHT * RAM_WIDTH
#define DISP_DELAY 300000
#define REGS_COUNT 5

typedef LoadedFile Code;
typedef stk_elem_t Register;

typedef enum {
    SPU_OK = 0,
    SPU_PUSH_FAILED,
    SPU_POP_FAILED,
    SPU_INTERNAL_POP_FAILED,
    SPU_INTERNAL_PUSH_FAILED,
    SPU_DIVISION_BY_ZERO,
    SPU_INPUT_FAILED,
    SPU_BYTECODE_PREMATURE_END,
    SPU_TIME_CALC_ERROR,
    SPU_SLEEP_ERROR,
    SPU_SLEEP_ARG_LESS_THAN_ZERO,
    SPU_SET_ARG_OUT_OF_RANGE,
    SPU_INVALID_REGISTER_INDEX
} SPUStatus;


typedef struct SPU {
    Code code;
    stack_t stack;
    stack_t call_stack;
    Register regs[REGS_COUNT]; // rip, rax, rbx, rcx, rdx
    uint8_t ram[RAM_SIZE];
    uint8_t disp_fps;
    ON_SPU_DEBUG(
    FILE *_dbg_logfile;
    )
} SPU;


const char *spu_error_str(SPUStatus status);
SPUStatus spu_init(SPU *spu, LoadedFile *code, stack_t *stack, stack_t *call_stack
                   ON_SPU_DEBUG(, const char* _dbg_logfile));
SPUStatus spu_destroy(SPU *spu);
SPUStatus spu_start(SPU *spu);

SPUStatus spu_out(SPU *, stk_elem_t);
SPUStatus spu_inp(SPU *, stk_elem_t);
SPUStatus spu_pop(SPU *, stk_elem_t);
SPUStatus spu_push(SPU *, stk_elem_t);
SPUStatus spu_add(SPU *, stk_elem_t);
SPUStatus spu_sub(SPU *, stk_elem_t);
SPUStatus spu_mul(SPU *, stk_elem_t);
SPUStatus spu_div(SPU *, stk_elem_t);
SPUStatus spu_jmp(SPU *, stk_elem_t);
SPUStatus spu_je(SPU *, stk_elem_t);
SPUStatus spu_ja(SPU *, stk_elem_t);
SPUStatus spu_jb(SPU *, stk_elem_t);
SPUStatus spu_jae(SPU *, stk_elem_t);
SPUStatus spu_jbe(SPU *, stk_elem_t);
SPUStatus spu_call(SPU *, stk_elem_t);
SPUStatus spu_ret(SPU *, stk_elem_t);
SPUStatus spu_sqrt(SPU *, stk_elem_t);
SPUStatus spu_set(SPU *, stk_elem_t);
SPUStatus spu_disp(SPU *, stk_elem_t);
SPUStatus spu_dispfps(SPU *, stk_elem_t);
SPUStatus spu_dispclr(SPU *, stk_elem_t);
SPUStatus spu_sleep(SPU *, stk_elem_t);