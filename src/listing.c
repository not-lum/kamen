#include "listing.h"
#include "spu/spu.h"

#include <stdlib.h>
#include <string.h>

// bin search: opcode, name
OpInfo OPINFO_ARR[] = {
    // INIT_CMD(push, ARG_STK_ELEM | ARG_REG)
    {ASM_HLT    , ARG_NONE                          , "hlt"    , NULL},
    {ASM_PUSH   , ARG_STK_ELEM | ARG_REG | ARG_RAM  , "push"   , &spu_push},
    {ASM_POP    , ARG_REG                           , "pop"    , &spu_pop},
    {ASM_SET    , ARG_STK_ELEM | ARG_REG | ARG_RAM  , "set"    , &spu_set},
    {ASM_DISP   , ARG_NONE                          , "disp"   , &spu_disp},
    {ASM_DISPFPS, ARG_STK_ELEM | ARG_REG | ARG_RAM  , "dispfps", &spu_dispfps},
    {ASM_DISPCLR, ARG_NONE                          , "dispclr", &spu_dispclr},
    {ASM_SLEEP  , ARG_STK_ELEM | ARG_REG | ARG_RAM  , "sleep"  , &spu_sleep},
    {ASM_MUL    , ARG_NONE                          , "mul"    , &spu_mul},
    {ASM_ADD    , ARG_NONE                          , "add"    , &spu_add},
    {ASM_SUB    , ARG_NONE                          , "sub"    , &spu_sub},
    {ASM_DIV    , ARG_NONE                          , "div"    , &spu_div},
    {ASM_OUT    , ARG_NONE                          , "out"    , &spu_out},
    {ASM_INP    , ARG_REG                           , "inp"    , &spu_inp},
    {ASM_JMP    , ARG_STK_ELEM | ARG_REG | ARG_LABEL, "jmp"    , &spu_jmp},
    {ASM_JE     , ARG_STK_ELEM | ARG_REG | ARG_LABEL, "je"     , &spu_je},
    {ASM_JA     , ARG_STK_ELEM | ARG_REG | ARG_LABEL, "ja"     , &spu_ja},
    {ASM_JB     , ARG_STK_ELEM | ARG_REG | ARG_LABEL, "jb"     , &spu_jb},
    {ASM_JAE    , ARG_STK_ELEM | ARG_REG | ARG_LABEL, "jae"    , &spu_jae},
    {ASM_JBE    , ARG_STK_ELEM | ARG_REG | ARG_LABEL, "jbe"    , &spu_jbe},
    {ASM_CALL   , ARG_STK_ELEM | ARG_REG | ARG_LABEL, "call"   , &spu_call},
    {ASM_RET    , ARG_NONE                          , "ret"    , &spu_ret},
    {ASM_SQRT   , ARG_NONE                          , "sqrt"   , &spu_sqrt},

    {ASM_UNKNOWN, ARG_NONE                          , ""       , NULL}
};

RegInfo REGINFO_ARR[] = {
    {REG_IP     , "IP"},
    {REG_AX     , "AX"},
    {REG_BX     , "BX"},
    {REG_CX     , "CX"},
    {REG_DX     , "DX"},

    {REG_UNKNOWN, "UNKNOWN"}
};


OpInfo get_opinfo_by_str(const char *str_repr) {
    for (size_t i = 0; i < sizeof(OPINFO_ARR) / sizeof(OPINFO_ARR[0]); i++) {
        if (strcasecmp(OPINFO_ARR[i].str_repr, str_repr) == 0) {
            return OPINFO_ARR[i];
        }
    }

    return (OpInfo){ASM_UNKNOWN, ARG_NONE, "", NULL};
}



OpInfo get_opinfo_by_opcode(Opcode opcode) {
    for (size_t i = 0; i < sizeof(OPINFO_ARR) / sizeof(OPINFO_ARR[0]); i++) {
        if (OPINFO_ARR[i].opcode == opcode) {
            return OPINFO_ARR[i];
        }
    }

    return (OpInfo){ASM_UNKNOWN, ARG_NONE, "", NULL};
}


RegInfo get_reginfo_by_str(const char *str_repr) {
    for (size_t i = 0; i < sizeof(REGINFO_ARR) / sizeof(REGINFO_ARR[0]); i++) {
        if (strcasecmp(REGINFO_ARR[i].str_repr, str_repr) == 0) {
            return REGINFO_ARR[i];
        }
    }

    return (RegInfo){REG_UNKNOWN, "UNKNOWN"};
}


RegInfo get_reginfo_by_code(RegisterCode regcode) {
    for (size_t i = 0; i < sizeof(REGINFO_ARR) / sizeof(REGINFO_ARR[0]); i++) {
        if (REGINFO_ARR[i].regcode == regcode) {
            return REGINFO_ARR[i];
        }
    }

    return (RegInfo){REG_UNKNOWN, "UNKNOWN"};
}


