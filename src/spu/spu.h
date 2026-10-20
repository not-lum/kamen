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
} SPU;


const char *spu_error_str(SPUStatus status);
SPUStatus spu_init(SPU *spu, LoadedFile *code, stack_t *stack, stack_t *call_stack);
SPUStatus spu_destroy(SPU *spu);
SPUStatus spu_start(SPU *spu);

SPUStatus spu_out(SPU *spu, stk_elem_t res);
SPUStatus spu_inp(SPU *spu, stk_elem_t res);
SPUStatus spu_pop(SPU *spu, stk_elem_t res);
SPUStatus spu_push(SPU *spu, stk_elem_t res);
SPUStatus spu_add(SPU *spu, stk_elem_t res);
SPUStatus spu_sub(SPU *spu, stk_elem_t res);
SPUStatus spu_mul(SPU *spu, stk_elem_t res);
SPUStatus spu_div(SPU *spu, stk_elem_t res);
SPUStatus spu_jmp(SPU *spu, stk_elem_t res);
SPUStatus spu_je(SPU *spu, stk_elem_t res);
SPUStatus spu_ja(SPU *spu, stk_elem_t res);
SPUStatus spu_jb(SPU *spu, stk_elem_t res);
SPUStatus spu_jae(SPU *spu, stk_elem_t res);
SPUStatus spu_jbe(SPU *spu, stk_elem_t res);
SPUStatus spu_call(SPU *spu, stk_elem_t res);
SPUStatus spu_ret(SPU *spu, stk_elem_t res);
SPUStatus spu_sqrt(SPU *spu, stk_elem_t elem);