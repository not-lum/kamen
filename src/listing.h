#pragma once

typedef enum {
    RIP = 0,
    RAX,
    RBX,
    RCX,
    RDX
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
    ASM_JBE
} Opcode;
