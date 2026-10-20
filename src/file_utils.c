#include "file_utils.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>


const char *fs_error_str(FileStatus error) {
    switch (error) {
        case FS_OK:
            return "ok";
        case FS_ALLOC_ERROR:
            return "cannot allocate memory";
        case FS_OPEN_ERROR:
            return "cannot open file";
        case FS_STAT_ERROR:
            return "cannot obtain file info";
        case FS_READ_ERROR:
            return "cannot read file";
        case FS_WRITE_ERROR:
            return "cannot write to a file";
        case FS_CLOSE_ERROR:
            return "cannot close file";
        default:
            return "unknown error";
    }
}

FileStatus get_file_size(const char *filename, size_t *out_file_size) {
    assert(filename != NULL);
    assert(out_file_size != NULL);

    struct stat stat_buff = {};

    if (lstat(filename, &stat_buff) == -1) {
        return FS_STAT_ERROR;
    }

    *out_file_size = stat_buff.st_size;

    return FS_OK;
}

static size_t str_count_char(char *str, char ch) {
    size_t res = 0;

    while (*str != '\0') {
        if (*str == ch)
            res++;

        str++;
    }

    return res;
}

FileStatus load_file_alloc(LoadedFile *out_l_file, const char *filename) {
    assert(filename != NULL);
    assert(out_l_file != NULL);

    FILE *file_ptr = fopen(filename, "rb");
    if (file_ptr == NULL)
        return FS_OPEN_ERROR;

    size_t file_size = 0;

    FileStatus status_gfs = get_file_size(filename, &file_size);
    if (status_gfs != FS_OK) {
        fclose(file_ptr);
        return status_gfs;
    }

    uint8_t *file_buff = calloc(file_size + 1, sizeof(out_l_file->content[0]));
    if (file_buff == NULL) {
        fclose(file_ptr);
        return FS_ALLOC_ERROR;
    }

    size_t file_buff_len = fread(file_buff, sizeof(out_l_file->content[0]), file_size, file_ptr);
    if (ferror(file_ptr)) {
        free(file_buff);
        fclose(file_ptr);
        return FS_READ_ERROR;
    }

    if (fclose(file_ptr) == EOF) {
        free(file_buff);
        return FS_CLOSE_ERROR;
    }

    out_l_file->content = file_buff;
    out_l_file->content_len = file_buff_len;

    return FS_OK;
}

static FileStatus index_lines(IndexedFile *idx_file) {
    assert(idx_file != NULL);

    char *content = (char *)idx_file->l_file.content;
    size_t content_len = idx_file->l_file.content_len;

    idx_file->index = calloc(str_count_char(content, '\n') + 1, sizeof(idx_file->index[0]));
    if (idx_file->index == NULL)
        return FS_ALLOC_ERROR;

    String *index = idx_file->index;

    index[0].addr = content;

    idx_file->lines_numb = 1;

    while (true) {
        char *addr = strchr(index[idx_file->lines_numb - 1].addr, '\n'); // load_file_alloc adds '\0' at the end of written data, so strchr won't search for '\n' infinitely
        if (addr == NULL)
            break;

        if (addr == content + content_len - 1) {
            index[idx_file->lines_numb - 1].len = addr - index[idx_file->lines_numb - 1].addr;
            *addr = '\0';
            break;
        }

        *addr = '\0';
        index[idx_file->lines_numb - 1].len = addr - index[idx_file->lines_numb - 1].addr;
        index[idx_file->lines_numb].addr = addr + 1;
        
        idx_file->lines_numb++;
    }

    return FS_OK;
}


FileStatus create_indexed_file(IndexedFile *out_idx_file, const char *filename) {
    assert(filename != NULL);
    assert(out_idx_file != NULL);

    FileStatus status = load_file_alloc(&out_idx_file->l_file, filename);

    if (status != FS_OK) {
        return status; 
    }

    status = index_lines(out_idx_file);

    return status;
}


void destroy_loaded_file(LoadedFile *out_l_file) {
    free(out_l_file->content);
    out_l_file->content_len = 0;
}


void destroy_indexed_file(IndexedFile *out_idx_file) {
    destroy_loaded_file(&out_idx_file->l_file);

    free(out_idx_file->index);
}


