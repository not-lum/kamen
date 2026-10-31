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

typedef struct IndexedFile {
    LoadedFile l_file;
    String *index;
    size_t lines_numb; 
} IndexedFile;

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
FileStatus load_file_alloc(LoadedFile *out_l_file, const char *filename);
FileStatus create_indexed_file(IndexedFile *out_idx_file, const char *filename);
void destroy_loaded_file(LoadedFile *out_l_file);
void destroy_indexed_file(IndexedFile *out_idx_file);
FileStatus get_file_size(const char *filename, size_t *out_file_size);
char *indexed_file_get_line(const IndexedFile *idx_file, size_t line_index);
size_t indexed_file_get_line_len(const IndexedFile *idx_file, size_t line_index);
