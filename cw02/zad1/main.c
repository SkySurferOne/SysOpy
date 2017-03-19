#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/times.h>
#include <fcntl.h>
#include <unistd.h>

#define RAND_GEN_DIR "/dev/random"
#define FAST_RAND_GEN_DIR "/dev/urandom"
#define TESTS_ON 1


enum RandGenMode {
    FAST_RAND_GEN_MODE,
    SLOW_RAND_GEN_MODE
};
typedef enum RandGenMode RandGenMode;

// lib api
void lib_read(char *, size_t, FILE *);
void lib_write(char *, size_t, FILE *);
void lib_swap_records(FILE *, int, int, size_t);
void lib_shuffle(char *, size_t, size_t);
void lib_sort(char *, size_t, size_t);
int lib_compare_records(FILE *, int, int, size_t);

// sys api
void sys_read(char *, size_t, int);
void sys_write(char *, size_t, int);
void sys_swap_records(int, int, int, size_t);
void sys_shuffle(char *, size_t, size_t);
void sys_sort(char *, size_t, size_t);
void sys_seek(int, int, int);
int sys_compare_records(int, int, int, size_t);

// file generation
void generate_file(char *, size_t, size_t, RandGenMode);
void get_rand_bytes(char *, size_t, RandGenMode);
void trans_buff_stream(char *, size_t);

// arg parsing
void parse_args(int, char **);
size_t parse_to_size_t(int, char**);

// timing
void init_time(void);
void get_and_print_time();

// globals
double realStart;
double userStart;
double systemStart;

void init_time() {
    struct tms buffer;
    times(&buffer);
    realStart = clock() / (double)CLOCKS_PER_SEC;
    userStart = buffer.tms_utime / (double)CLOCKS_PER_SEC;
    systemStart = buffer.tms_stime / (double)CLOCKS_PER_SEC;
}

void get_and_print_time() {
    struct tms buffer;
    times(&buffer);

    double real = clock() / (double)CLOCKS_PER_SEC;
    double user = buffer.tms_utime / (double)CLOCKS_PER_SEC;
    double sys = buffer.tms_stime / (double)CLOCKS_PER_SEC;

    printf("Times: real = %f, user = %f, system = %f\n\n", real - realStart, user - userStart, sys - systemStart);
}

inline size_t parse_to_size_t(int i, char ** array) {
    return (size_t) strtol(array[i], (char **)NULL, 10);
}

void testMsg(char *msg, size_t recNum, size_t recSize) {
    printf("%s (rec_num: %d, rec_size: %d)\n", msg, (int) recNum, (int) recSize);
}

void parse_args(int argc, char **argv) {
    if(argc < 4) {
        printf("Invalid command. Write --help to more information.\n");
        exit(1);
    }

    if(TESTS_ON) init_time();
    if (strcmp(argv[1], "lib") == 0) {
        size_t recNum = parse_to_size_t(4, argv);
        size_t recSize = parse_to_size_t(5, argv);
        if(strcmp(argv[2], "shuffle") == 0) { lib_shuffle(argv[3], recNum, recSize);
            if(TESTS_ON) testMsg("lib_shuffle testing", recNum, recSize); }
        if(strcmp(argv[2], "sort") == 0) { lib_sort(argv[3], recNum, recSize);
            if(TESTS_ON) testMsg("lib_sort testing", recNum, recSize); }
    } else if (strcmp(argv[1], "sys") == 0) {
        size_t recNum = parse_to_size_t(4, argv);
        size_t recSize = parse_to_size_t(5, argv);
        if(strcmp(argv[2], "shuffle") == 0) { sys_shuffle(argv[3], recNum, recSize);
            if(TESTS_ON) testMsg("sys_shuffle testing", recNum, recSize); }
        if(strcmp(argv[2], "sort") == 0) { sys_sort(argv[3], recNum, recSize);
            if(TESTS_ON) testMsg("sys_sort testing", recNum, recSize); }
    } else {
        if (strcmp(argv[1], "generate") == 0) {
            if (strcmp(argv[2], "-f") == 0) {
                size_t recNum = parse_to_size_t(4, argv);
                size_t recSize = parse_to_size_t(5, argv);
                generate_file(argv[3],  recNum, recSize, FAST_RAND_GEN_MODE);
            } else {
                size_t recNum = parse_to_size_t(3, argv);
                size_t recSize = parse_to_size_t(4, argv);
                generate_file(argv[2],  recNum, recSize, SLOW_RAND_GEN_MODE);
            }
        } else if (strcmp(argv[1], "--help") == 0) {
            char *help = "Syntax: generate [-f] <file_path> <rec_num> <rec_size>\n"
                         "         or\n"
                         "        lib|sys shuffle|sort <file_path> <rec_num> <rec_size>\n";

            printf("%s\n", help);
        } else {
            printf("Invalid command. Write --help to more information.\n");
            exit(1);
        }
    }
    if(TESTS_ON) get_and_print_time();
}

void lib_read(char *buffer, size_t size, FILE *fptr) {
    size_t res = fread(buffer, sizeof(char), size, fptr);
    if (res != size) {
        printf("Something went wrong on lib_read function");
        exit(1);
    }
}

void lib_write(char *buffer, size_t size, FILE *fptr) {
    size_t res = fwrite(buffer, sizeof(char), size, fptr);
    if (res != size) {
        printf("Something went wrong on lib_write function");
        exit(1);
    }
}

void get_rand_bytes(char *buffer, size_t recSize, RandGenMode mode) {
    FILE *fptr;
    switch (mode) {
        case SLOW_RAND_GEN_MODE:
            fptr = fopen(RAND_GEN_DIR, "r");
            break;
        case FAST_RAND_GEN_MODE:
            fptr = fopen(FAST_RAND_GEN_DIR, "r");
            break;
        default:
            printf("There is not such a mode\n");
            exit(1);
    }

    if (fptr == NULL) {
        printf("Problem with random data generator has occurred\n");
        exit(1);
    }
    lib_read(buffer, recSize, fptr);
    fclose(fptr);
}

void trans_buff_stream(char *buffer, size_t size) {
    for (int i=0; i<size; i++) {
        buffer[i] = (char) (abs(buffer[i]) % 25 + 97);
   }
}

void generate_file(char *filename, size_t recNum, size_t recSize, RandGenMode mode) {
    FILE *fptr;
    fptr = fopen(filename, "w+");
    if (fptr == NULL) {
        printf("Problem with creating file has occurred\n");
        exit(1);
    }

    char *buffer = (char *) malloc(sizeof(char) * recSize + 1);

    for (int i=0; i<recNum; i++) {
        get_rand_bytes(buffer, recSize, mode);
        trans_buff_stream(buffer, recSize);
        buffer[recSize] = '\n';
        lib_write(buffer, recSize + 1, fptr);
    }
    lib_write("\0", 1, fptr);

    free(buffer);
    fclose(fptr);
}

void lib_swap_records(FILE *fptr, int i, int j, size_t size) {
    char *buff1 = (char *) malloc(sizeof(char) * size);
    char *buff2 = (char *) malloc(sizeof(char) * size);

    fseek(fptr, i * size, SEEK_SET);
    lib_read(buff1, size, fptr);
    fseek(fptr, j * size, SEEK_SET);
    lib_read(buff2, size, fptr);

    fseek(fptr, i * size, SEEK_SET);
    lib_write(buff2, size, fptr);
    fseek(fptr, j * size, SEEK_SET);
    lib_write(buff1, size, fptr);

    free(buff1);
    free(buff2);
}

void lib_shuffle(char *filename, size_t recNum, size_t recSize) {
    FILE *fptr;
    fptr = fopen(filename, "r+");
    if (fptr == NULL) {
        printf("Connot find %s file\n", filename);
        exit(1);
    }

    srand(time(NULL));

    int i = recNum, j = 0;
    while (i > 1) {
        --i;
        j = rand() % i;
        lib_swap_records(fptr, i, j, recSize + 1);
    }
    fclose(fptr);
}

int lib_compare_records(FILE *fptr, int i, int j, size_t size) {
    char *buff1 = (char *) malloc(sizeof(char));
    char *buff2 = (char *) malloc(sizeof(char));

    fseek(fptr, i * size, SEEK_SET);
    lib_read(buff1, 1, fptr);
    fseek(fptr, j * size, SEEK_SET);
    lib_read(buff2, 1, fptr);

    int res = (int) (*buff1 - *buff2);

    free(buff1);
    free(buff2);

    return res;
}

void lib_sort(char *filename, size_t recNum, size_t recSize) {
    FILE *fptr;
    fptr = fopen(filename, "r+");
    if (fptr == NULL) {
        printf("Connot find %s file\n", filename);
        exit(1);
    }

    int f;
    for (int i = 0; i < recNum; i++) {
        f = 0;
        for (int j = 0; j < recNum - i - 1; j++) {
            if (lib_compare_records(fptr, j, j+1, recSize + 1) > 1) {
                lib_swap_records(fptr, j, j+1, recSize + 1);
                f = 1;
            }
        }
        if (!f) break;
    }
    fclose(fptr);
}

void sys_read(char *buffer, size_t size, int fileDesc) {
    int res = (int) read(fileDesc, buffer, size);
    if(res != size){
        printf("Something went wrong on lib_read function");
        exit(1);
    }
}

void sys_write(char *buffer, size_t size, int fileDesc) {
    int res = (int) write(fileDesc, buffer, size);
    if(res != size){
        printf("Something went wrong on lib_write function\n");
        exit(1);
    }
}

void sys_swap_records(int fileDesc, int i, int j, size_t size) {
    char *buff1 = (char *) malloc(sizeof(char) * size);
    char *buff2 = (char *) malloc(sizeof(char) * size);

    sys_seek(fileDesc, i * size, SEEK_SET);
    sys_read(buff1, size, fileDesc);
    sys_seek(fileDesc, j * size, SEEK_SET);
    sys_read(buff2, size, fileDesc);

    sys_seek(fileDesc, i * size, SEEK_SET);
    sys_write(buff2, size, fileDesc);
    sys_seek(fileDesc, j * size, SEEK_SET);
    sys_write(buff1, size, fileDesc);

    free(buff1);
    free(buff2);
}

void sys_seek(int fileDesc, int offset, int whence) {
    int res = (int)lseek(fileDesc, offset, whence);
    if(res == -1){
        printf("Something went wrong on sys_seek function\n");
        exit(1);
    }
}

void sys_shuffle(char * filename, size_t recNum, size_t recSize) {
    int desc = open(filename, O_RDWR);
    if (desc == -1) {
        printf("Connot find %s file\n", filename);
        exit(1);
    }

    srand(time(NULL));

    int i = recNum, j = 0;
    while (i > 1) {
        --i;
        j = rand() % i;
        sys_swap_records(desc, i, j, recSize + 1);
    }

    if(close(desc) != 0){
        printf("Closing file error occurred\n");
        exit(1);
    }
}

int sys_compare_records(int fileDesc, int i, int j, size_t size) {
    char *buff1 = (char *) malloc(sizeof(char));
    char *buff2 = (char *) malloc(sizeof(char));

    sys_seek(fileDesc, i * size, SEEK_SET);
    sys_read(buff1, 1, fileDesc);
    sys_seek(fileDesc, j * size, SEEK_SET);
    sys_read(buff2, 1, fileDesc);

    int res = (int) (*buff1 - *buff2);

    free(buff1);
    free(buff2);

    return res;
}

void sys_sort(char * filename, size_t recNum, size_t recSize) {
    int fileDesc = open(filename, O_RDWR);
    if (fileDesc == -1) {
        printf("Connot find %s file\n", filename);
        exit(1);
    }

    int f;
    for (int i = 0; i < recNum; i++) {
        f = 0;
        for (int j = 0; j < recNum - i - 1; j++) {
            if (sys_compare_records(fileDesc, j, j+1, recSize + 1) > 1) {
                sys_swap_records(fileDesc, j, j+1, recSize + 1);
                f = 1;
            }
        }
        if (!f) break;
    }

    if(close(fileDesc) != 0){
        printf("Closing file error occurred\n");
        exit(1);
    }
}

int main(int argc, char **argv) {
    parse_args(argc, argv);
    return 0;
}

