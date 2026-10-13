#pragma once

#include "../../secstack/src/secstack.h"
#include "../file_utils.h"
#include "../listing.h"


#define KAMEN_SPU_DBG
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
    SPU_INTERNAL_PUSH_FAILED
} SPUStatus;


typedef struct SPU {
    Code code;
    stack_t stack;
    Register regs[5]; // rip, rax, rbx, rcx, rdx
} SPU;

const char *spu_error_str(SPUStatus status);
SPUStatus spu_init(SPU *spu, LoadedFile *code, stack_t *stack);
SPUStatus spu_destroy(SPU *spu);
SPUStatus spu_start(SPU *spu, size_t start);
SPUStatus spu_pop(SPU *spu, bool reg, RegisterCode r_code);
SPUStatus spu_push(SPU *spu, stk_elem_t arg);

