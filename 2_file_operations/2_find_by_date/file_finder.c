//
// Created by arkadius on 16.03.19.
//
#define _XOPEN_SOURCE 700
#define  _GNU_SOURCE
#include "file_finder.h"
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <ftw.h>

int not_dot(char *name) {
    if(strcmp(name, ".") == 0|| strcmp(name, "..") == 0)
        return 0;
    return 1;
}

const char *get_file_type(mode_t mode) {
    if(mode & S_IFREG){
        return "file";
    } else if(mode & S_IFDIR) {
        return "directory";
    } else if(mode & S_IFCHR) {
        return "char dev";
    } else if(mode & S_IFBLK) {
        return "block dev";
    } else if(mode & S_IFIFO) {
        return "fifo";
    } else if(mode & S_IFLNK) {
        return "slink";
    } else if(mode & S_IFSOCK) {
        return "socket";
    }
    return "unknown";
}

void print_if(f_file file, time_t condition, int (*compare)(time_t, time_t)) {
    if(compare(file.mtime, condition)){
        printf("%s type: %s size: %d accesed: %d modified: %d \n",
                file.path, get_file_type(file.mode), file.size, file.atime, file.mtime);
    }
}

void find_recursively_dir(char *path, time_t condition, int (*compare)(time_t, time_t)){
    DIR *dir = opendir(path);
    struct dirent *dirent;
    struct stat stat;
    char next_path[256];

    while((dirent = readdir(dir)) != NULL) {
        strcpy(next_path, path);
        strcat(next_path, "/");
        strcat(next_path, dirent->d_name);
        fstatat(dirfd(dir), dirent->d_name, &stat, AT_SYMLINK_NOFOLLOW);
        if(not_dot(dirent -> d_name)) {
            if ((stat.st_mode & S_IFDIR)) {
                find_recursively_dir(next_path, condition, compare);
            }
            f_file file;
            strcpy(file.path, next_path);
            file.mode = stat.st_mode;
            file.atime = stat.st_atime;
            file.mtime = stat.st_mtime;
            file.size = (size_t) stat.st_size;
            print_if(file, condition, compare);
        }
    }

}

void find_with_dir(char *path, time_t condition, int (*compare)(time_t, time_t)) {
    find_recursively_dir(path, condition, compare);
}

time_t condition;
int (*compare)(time_t, time_t);

int tree_callback(const char *fpath, const struct stat *sb,
                  int typeflag, struct FTW *ftwbuf) {
    f_file file;
    strcpy(file.path, fpath);
    file.mode = sb->st_mode;
    file.atime = sb->st_atime;
    file.mtime = sb->st_mtime;
    file.size = (size_t) sb->st_size;
    print_if(file, condition, compare);
    return FTW_CONTINUE;
}


void find_with_nftw(char *path, time_t _condition, int (*_compare)(time_t, time_t)) {
    condition = _condition;
    compare = _compare;
    nftw(path, tree_callback, 100, 0);
}
