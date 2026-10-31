#pragma once

#include "../listing.h"

#define IP(spu) (spu)->regs[REG_IP]

#define CHECK_REG_IS_VALID(reg_ind) do { \
    if ((reg_ind) < 0 || (reg_ind) >= REGS_COUNT) \
        return SPU_INVALID_REGISTER_INDEX; \
} while (0)

#define CHECK_RAM_ADDR_IS_VALID(addr) do { \
    if ((addr) >= RAM_SIZE || (addr) < 0) \
        return SPU_INVALID_RAM_ADDR; \
} while (0)


typedef struct {
    OpInfo op;
    ArgTypeMask arg_mask;
    stk_elem_t raw_arg;
    size_t size;
} DecodedInstr;
