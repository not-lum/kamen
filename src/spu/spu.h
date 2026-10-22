#pragma once

#include "../../secstack/src/secstack.h"
#include "../file_utils.h"


// #define KAMEN_SPU_DBG
// #define KAMEN_DUMP_STACK

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

#define RAM_HEIGHT 60
#define RAM_WIDTH 80
#define RAM_SIZE RAM_HEIGHT * RAM_WIDTH

typedef LoadedFile Code;
typedef stk_elem_t Register;

typedef enum {
    SPU_OK = 0,
    SPU_PUSH_FAILED,
    SPU_POP_FAILED,
    SPU_INTERNAL_POP_FAILED,
    SPU_INTERNAL_PUSH_FAILED,
    SPU_DIVISION_BY_ZERO,
    SPU_INPUT_FAILED
} SPUStatus;


typedef struct SPU {
    Code code;
    stack_t stack;
    stack_t call_stack;
    Register regs[5]; // rip, rax, rbx, rcx, rdx
    uint8_t ram[RAM_SIZE];
} SPU;


const char *spu_error_str(SPUStatus status);
SPUStatus spu_init(SPU *spu, LoadedFile *code, stack_t *stack, stack_t *call_stack);
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
SPUStatus spu_disprst(SPU *, stk_elem_t);
SPUStatus spu_dispclr(SPU *, stk_elem_t);
SPUStatus spu_sleep(SPU *, stk_elem_t);