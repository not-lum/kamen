#include "spu.h"
#include "spu_internal.h"
#include "spu_log.h"
#include "../dump_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// TODO: cool dump and logs
// TODO: binary signature


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
        case SPU_INVALID_RAM_ADDR:
            return "attempted to access invalid RAM address";
        case SPU_INVALID_REGISTER_INDEX:
            return "attempted to access non-existent register";
        case SPU_UNKNOWN_OPCODE:
            return "unknown opcode provided";
        case SPU_INVALID_SIGNATURE:
            return "invalid binary file signature";
        case SPU_INVALID_HEADER:
            return "invalid binary file header";
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

static SPUStatus resolve_arg(SPU *spu, const DecodedInstr *instr,
                             stk_elem_t *out_arg) {
    assert(spu != NULL);
    assert(instr != NULL);
    
    *out_arg = instr->raw_arg;

    if ((instr->arg_mask & ARG_REG) && instr->op.opcode != ASM_POP
                                    && instr->op.opcode != ASM_INP) {
        CHECK_REG_IS_VALID(*out_arg);
        *out_arg = spu->regs[*out_arg];
    }

    if (instr->arg_mask & ARG_RAM) {
        CHECK_RAM_ADDR_IS_VALID(*out_arg);
        *out_arg = spu->ram[*out_arg];
    }

    return SPU_OK;
}

static SPUStatus parse_header(SPU *spu) {
    assert(spu != NULL);

    if (spu->code.content_len < FULL_HEADER_SIZE)
        return SPU_INVALID_HEADER;

    if (memcmp(spu->code.content, BIN_SIGNATURE,
                BIN_SIGNATURE_SIZE) != 0)
        return SPU_INVALID_SIGNATURE;

    size_t declared_len = 0;
    memcpy(&declared_len, spu->code.content + BIN_SIGNATURE_SIZE,
           FILESIZE_HEADER_SIZE);

    if (declared_len != spu->code.content_len - FULL_HEADER_SIZE)
        return SPU_INVALID_HEADER;

    spu->code.content_len = declared_len;

    return SPU_OK;
}

SPUStatus decode_instr(SPU *spu, DecodedInstr *instr) {
    assert(spu != NULL);
    assert(instr != NULL);

    uint8_t *content = spu->code.content + FULL_HEADER_SIZE;
    size_t content_len = spu->code.content_len;

    Opcode opcode = content[IP(spu)];
    if (opcode >= ASM_UNKNOWN)
        return SPU_UNKNOWN_OPCODE;

    instr->op = get_opinfo_by_opcode(opcode);

    if (instr->op.allowed_arg_type == ARG_NONE) {
        instr->arg_mask = ARG_NONE;
        instr->size = INSTR_NO_ARG_SIZE;

        return SPU_OK;
    }

    if ((size_t)IP(spu) + INSTR_DEFAULT_SIZE > content_len)
        return SPU_BYTECODE_PREMATURE_END;

    instr->arg_mask = content[IP(spu) + 1];
    memcpy(&instr->raw_arg, content + IP(spu) + 2, sizeof(stk_elem_t));
    instr->size = INSTR_DEFAULT_SIZE;

    return SPU_OK;
}

SPUStatus exec_instr(SPU *spu, DecodedInstr *instr) {
    assert(spu != NULL);
    assert(instr != NULL);

    stk_elem_t value = 0;
    SPUStatus status = resolve_arg(spu, instr, &value);
    if (status != SPU_OK)
        return status;

    IP(spu) += instr->size;

    return instr->op.func(spu, value);
}

SPUStatus spu_start(SPU *spu) {
    assert(spu != NULL);

    // TODO: add binary signature
    // TODO: separate this into funcs
    // TODO: arg types in byte with opcode

    SPUStatus status = parse_header(spu);
    if (status != SPU_OK)
        return status;

    while ((size_t)IP(spu) < spu->code.content_len) {
        DecodedInstr instr = {};
        status = decode_instr(spu, &instr);
        if (status != SPU_OK)
            return status;

        if (instr.op.opcode == ASM_HLT) 
            break;

        ON_SPU_DEBUG(
        log_instr(spu->_dbg_logfile, &instr);
        )

        ON_SPU_DEBUG_STDERR(
        log_instr(stderr, &instr);
        )

        status = exec_instr(spu, &instr);
        if (status != SPU_OK)
            return status;
    }

    return SPU_OK;
}
