#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "utils.h"

Stack * tmp;
char * rootDir = NULL;

void parse_argv(int, char **);
void search_recursive(char *, int);
void print_file_info(char *, char *, struct stat *);
void clear_tmp_stack();

int main(int argc, char **argv) {
    tmp = stackInit();
    parse_argv(argc, argv);

    return 0;
}

void clear_tmp_stack() {
    while (!empty(tmp)) {
        char * d = (char *) pop(&tmp);
        free(d);
    }
}

void print_file_info(char *fileName, char *relpath, struct stat *fileStat) {
    printf("Information for %s\n", fileName);
    printf("---------------------------\n");
    printf("Absolute path: \t\t%s%s\n", rootDir, relpath);
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
}

void search_recursive(char *dirPath, int bytes) {
    DIR *d = opendir(dirPath);
    struct dirent *dir;
    struct stat fileStat;
    char *path;
    Stack * stack = stackInit();


    int l = strlen(dirPath);
    if (l >= 2 && strcmp(&dirPath[l-1], ".") == 0) return;

    if (d == NULL) {
        fprintf(stderr, "No such dir was found\n");
        exit(EXIT_FAILURE);
    }
    printf("////////////////////////////////\n");
    printf("  Dir: %s\n", dirPath);
    printf("////////////////////////////////\n\n");

    while ((dir = readdir(d)) != NULL) {
        path = str_concat(3, dirPath, "/", dir->d_name);
        push(&tmp, path); // after procedure end this data will be deleted

        if(stat(path, &fileStat) >= 0) {
            if (S_ISDIR(fileStat.st_mode))
                push(&stack, path);
            else
                if (fileStat.st_size <= bytes)
                    print_file_info(dir->d_name, path, &fileStat);
        }
    }

    while (!empty(stack)) {
        path = (char *) pop(&stack);
        search_recursive(path, bytes);
    }

    closedir(d);
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

    printf("%s, %d\n", dirPath, bytes);
    search_recursive(dirPath, bytes);
    clear_tmp_stack(); // delete allocated memory in str_concat
    free(rootDir);
}