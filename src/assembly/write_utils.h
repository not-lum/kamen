#pragma once

#include "../file_utils.h"
#include "parsing_utils.h"


typedef enum {
    WRITE_OK = 0,
    WRITE_ERROR_WHILE_PARSING,
    WRITE_BCODE_FILE_OPEN_FAILED,
    WRITE_LISTING_FILE_OPEN_FAILED,
} WriteStatus;

const char *write_error_str(WriteStatus status);
WriteStatus assembly_indexed_file(IndexedFile *idx_file,
                                  const char *bcode_filename,
                                  const char *listing_filename,
                                  ParsingErrorInfo *error_info);