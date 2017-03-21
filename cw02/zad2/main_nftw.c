//
// Created by damian on 21.03.17.
//
#include "utils.h"
#define _XOPEN_SOURCE 1			/* Required under GLIBC for nftw() */
#define _XOPEN_SOURCE_EXTENDED 1
#include <ftw.h>

char * rootDir = NULL;
int MAX_SIZE = 0;

void list_files(char *, int);
int nftw(const char *dirpath, int (*fn) (const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf), int nopenfd, int flags);

int main(int argc, char **argv) {
    parse_argv(argc, argv);

    return 0;
}

void fix_root_dir() {
    if (rootDir == NULL) return;
    int l = strlen(rootDir);
    int i;
    for (i=l-1; i>0; i--) {
        if (rootDir[i] == '/')
            break;
    }
    int newSize = i + 1;
    char *fixed = (char *) malloc(sizeof(char) * newSize + 1);
    strncpy(fixed, rootDir, newSize);
    fixed[newSize] = '\0';
    rootDir = fixed;
}

void parse_argv(int argc, char ** argv) {
    char *dirPath, *endptr, *str;
    long val;
    int bytes;
    str = argv[2];
    dirPath = argv[1];
    rootDir = argv[0];
    fix_root_dir();

    if (argc < 3) { printf("%s\n", "To less arguments"); exit(EXIT_FAILURE); }

    errno = 0;
    val = strtol(str, &endptr, 10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0) || (val > INT_MAX || val < INT_MIN)) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }
    bytes = (int) val;

    if (endptr == str) {
        fprintf(stderr, "No digits were found\n");
        exit(EXIT_FAILURE);
    }

    list_files(dirPath, bytes);
    free(rootDir);
}

int print_file_info(const char *path, const struct stat *fileStat, int typeflag, struct FTW *sftw) {
    if(!S_ISREG(fileStat->st_mode) || (int) fileStat->st_size > MAX_SIZE) {
        return 0;
    }

    printf("---------------------------\n");
    printf("Absolute path: \t\t%s\n", path);
    printf("File Size: \t\t%d bytes\n", fileStat->st_size);
    printf("Number of Links: \t%d\n", fileStat->st_nlink);
    printf("File inode: \t\t%d\n", fileStat->st_ino);
    printf("Last modification: \t\t%s", ctime(&fileStat->st_mtime));

    printf("File Permissions: \t");
    printf( (S_ISDIR(fileStat->st_mode)) ? "d" : "-");
    printf( (fileStat->st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat->st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat->st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXOTH) ? "x" : "-");
    printf("\n\n");

    return 0;
}

void list_files(char *dirPath, int bytes) {
    MAX_SIZE = bytes;
    char *dir = str_concat(2, rootDir, dirPath);
    nftw(dir, print_file_info, 10, 0);
}