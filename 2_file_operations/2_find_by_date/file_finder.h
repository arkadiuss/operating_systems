//
// Created by arkadius on 16.03.19.
//

#ifndef INC_2_FIND_BY_DATE_FILE_FINDER_H
#define INC_2_FIND_BY_DATE_FILE_FINDER_H

#include <wchar.h>
#include <fcntl.h>
#include <time.h>

typedef struct f_file {
    char path[256];
    mode_t mode;
    size_t size;
    time_t atime, mtime;
} f_file;

void find_with_dir(char *path, time_t condition, int (*compare)(time_t a, time_t b));
void find_with_nftw(char *path, time_t condition, int (*compare)(time_t a, time_t b));

#endif //INC_2_FIND_BY_DATE_FILE_FINDER_H
