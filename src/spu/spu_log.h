#pragma once

#include "spu_internal.h"

#include "../../secstack/src/secstack.h"

void log_instr(FILE *to, DecodedInstr *instr);
void init_spu_dbg_file(SPU* spu, const char *filename);
