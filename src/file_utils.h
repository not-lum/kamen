#pragma once

#include <stdint.h>
#include <stdio.h>

typedef struct String {
    size_t len;
    char *addr;
} String;

typedef struct LoadedFile {
    uint8_t *content;
    size_t content_len;
} LoadedFile;

typedef enum {
    FS_OK,
    FS_OPEN_ERROR,
    FS_READ_ERROR,
    FS_ALLOC_ERROR,
    FS_CLOSE_ERROR,
    FS_WRITE_ERROR,
    FS_STAT_ERROR,
} FileStatus;

const char *fs_error_str(FileStatus error);
void destroy_loaded_file(LoadedFile *out_l_file);
FileStatus get_file_size(const char *filename, size_t *out_file_size);
FileStatus load_file_alloc(LoadedFile *out_l_file, const char *filename);
