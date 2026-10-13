#pragma once

typedef enum {
    IP = 0,
    AX,
    BX,
    CX,
    DX,

    REG_UNKNOWN
} RegisterCode;

typedef enum {
    ASM_HLT = 0,
    ASM_PUSH,
    ASM_PUSH_REG,
    ASM_POP,
    ASM_POP_REG,
    ASM_MUL,
    ASM_ADD,
    ASM_DIV,
    ASM_OUT,
    ASM_JMP,
    ASM_JE,
    ASM_JA,
    ASM_JB,
    ASM_JAE,
    ASM_JBE,

    ASM_UNKNOWN,
} OpcodeType;

static inline const char *str_reg(RegisterCode reg) {
    switch (reg) {
        case IP:
            return "IP";
        case AX:
            return "AX";
        case BX:
            return "BX";
        case CX:
            return "CX";
        case DX:
            return "DX";
        default:
            return "UNKNOWN";
    }
}
