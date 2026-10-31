#pragma once

#include "spu/spu.h"
#include <stdint.h>
#include <math.h>

#define BIN_SIGNATURE "KMN'"
#define BIN_SIGNATURE_SIZE 4
#define FILESIZE_HEADER_SIZE 8
#define FULL_HEADER_SIZE (BIN_SIGNATURE_SIZE + FILESIZE_HEADER_SIZE)

#define INSTR_DEFAULT_SIZE 1 + 1 + 4
#define INSTR_NO_ARG_SIZE 1

typedef enum {
    REG_IP = 0,
    REG_AX,
    REG_BX,
    REG_CX,
    REG_DX,

    REG_UNKNOWN
} RegisterCode;

typedef struct {
    RegisterCode regcode;
    const char* str_repr;
} RegInfo;

typedef enum {
    ASM_HLT = 0,
    ASM_PUSH,
    ASM_POP,
    ASM_SET,
    ASM_DISP,
    ASM_DISPFPS,
    ASM_DISPCLR,
    ASM_SLEEP,
    ASM_MUL,
    ASM_ADD,
    ASM_SUB,
    ASM_DIV,
    ASM_OUT,
    ASM_INP,
    ASM_JMP,
    ASM_JE,
    ASM_JA,
    ASM_JB,
    ASM_JAE,
    ASM_JBE,
    ASM_CALL,
    ASM_RET,
    ASM_SQRT,
    ASM_UNKNOWN,
} Opcode;


typedef enum {
    ARG_NONE = 1,
    ARG_STK_ELEM = 1 << 1,
    ARG_REG = 1 << 2,
    ARG_RAM = 1 << 3,
    ARG_LABEL = 1 << 4
} ArgType;

typedef uint8_t ArgTypeMask; 

typedef struct {
    Opcode opcode;
    ArgTypeMask allowed_arg_type;
    const char *str_repr;
    SPUStatus (*func)(SPU *, stk_elem_t);
} OpInfo;

OpInfo get_opinfo_by_str(const char *str_repr);
RegInfo get_reginfo_by_code(RegisterCode regcode);
OpInfo get_opinfo_by_opcode(Opcode opcode);
RegInfo get_reginfo_by_str(const char *str_repr);
