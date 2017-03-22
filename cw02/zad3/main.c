#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


struct ParsedArgs {
    char *exePath;
    char *filePath;
};
typedef struct ParsedArgs ParsedArgs;

void parse_args(int, char **, ParsedArgs *);
void show_help();
void main_loop(ParsedArgs *);

// api
void set_lock_read_nblock(int, off_t);
void set_lock_write_nblock(int, off_t);
void set_lock_read_block(int, off_t);
void set_lock_write_block(int, off_t);

void show_locked_chars(int, off_t);
void release_lock(int, off_t);
void read_char(int, off_t);
void write_char(int, off_t, char);

int perform_lock_action(int, int, short, short, off_t, off_t);
struct flock get_lock(int, off_t, off_t);

int main(int argc, char ** argv) {
    ParsedArgs *parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv, parsedArgs);
    main_loop(parsedArgs);

    free(parsedArgs);
    return 0;
}

int read_char_num(off_t *charNum, off_t maxNum) {
    printf("Write number between 0 to %ld: \n", maxNum);
    scanf("%ld", charNum);
    if(*charNum < 0 || *charNum > maxNum) {
        printf("Char number is out of range\n");
        return -1;
    }
    return 0;
}

void main_loop(ParsedArgs *parsedArgs) {
    int fileDesc, q = 0;
    off_t fileEnd, charNum;
    char cmd, c;

    fileDesc = open(parsedArgs->filePath, O_RDWR);
    if (fileDesc == -1) { printf("Connot find %s file\n", parsedArgs->filePath); exit(1); }
    fileEnd = lseek(fileDesc, 0, SEEK_END);
    if (fileEnd == -1) { printf("Something went wrong on lseek function\n"); exit(1);}

    printf("What do you want to do with: %s?\n"
                   "Choose numbers form 1 to 8. For more information write 'h'. "
                   "For quit write 'q'\n", parsedArgs->filePath);
    while (!q) {
        printf("> "); scanf(" %c", &cmd);

        switch (cmd) {
            case '1':
                printf("Selected set_lock_read_nblock(char_number)\n"); if (read_char_num(&charNum, fileEnd) == -1) continue;
                set_lock_read_nblock(fileDesc, charNum);
                break;
            case '2':
                printf("Selected set_lock_write_nblock(char_number)\n"); if (read_char_num(&charNum, fileEnd) == -1) continue;
                set_lock_write_nblock(fileDesc, charNum);
                break;
            case '3':
                printf("Selected set_lock_read_block(char_number)\n"); if (read_char_num(&charNum, fileEnd) == -1) continue;
                set_lock_read_block(fileDesc, charNum);
                break;
            case '4':
                printf("Selected set_lock_write_block(char_number)\n"); if (read_char_num(&charNum, fileEnd) == -1) continue;
                set_lock_write_block(fileDesc, charNum);
                break;
            case '5':
                printf("Selected show_locked_chars\n");
                show_locked_chars(fileDesc, fileEnd);
                break;
            case '6':
                printf("Selected release_lock(char_number)\n"); if (read_char_num(&charNum, fileEnd) == -1) continue;
                release_lock(fileDesc, charNum);
                break;
            case '7':
                printf("Selected read_char(char_number)\n"); if (read_char_num(&charNum, fileEnd) == -1) continue;
                read_char(fileDesc, charNum);
                break;
            case '8':
                printf("Selected write_char(char_number, new_char)\n"); if (read_char_num(&charNum, fileEnd) == -1) continue;
                printf("Write new char which be place at %ld byte: \n", charNum);
                scanf(" %c", &c);
                write_char(fileDesc, charNum, c);
                break;
            case 'h':
                show_help();
                break;
            case 'q':
                q = 1;
                break;
            default:
                printf("Wrong cmd, please write 'h' to more information.\n");
        }
    }
    close(fileDesc);
}

void show_help() {
    char *helpMsg = "Syntax: <file_path>\n"
                    "\tCommand description:\n"
                    "\t\t1 - set_lock_read_nblock(char_number)\n"
                    "\t\t2 - set_lock_write_nblock(char_number)\n"
                    "\t\t3 - set_lock_read_block(char_number)\n"
                    "\t\t4 - set_lock_write_block(char_number)\n"
                    "\t\t5 - show_locked_chars\n"
                    "\t\t6 - release_lock(char_number)\n"
                    "\t\t7 - read_char(char_number)\n"
                    "\t\t8 - write_char(char_number, new_char)\n"
                    "\t\th - show help\n"
                    "\t\tq - quit the program\n";
    printf("%s", helpMsg);
}

void parse_args(int argc, char ** argv, ParsedArgs *parsedArgs) {
    if (argc < 2) { printf("%s\n", "Too less arguments"); show_help(); exit(1); }
    parsedArgs->exePath = argv[0];
    parsedArgs->filePath = argv[1];
}

// api
int perform_lock_action(int fileDesc, int cmd, short l_type, short l_whence, off_t l_start, off_t l_len) {
    struct flock flock;

    flock.l_type = l_type;
    flock.l_whence = l_whence;
    flock.l_start = l_start;
    flock.l_len = l_len;
    flock.l_pid = getpid();

    return (fcntl(fileDesc, cmd, &flock));
}

struct flock get_lock(int fileDesc, off_t l_start, off_t l_len) {
    struct flock flock;
    off_t f;
    f = lseek(fileDesc, l_start, SEEK_SET);
    if (f == -1) { printf("Something went wrong on lseek function\n"); exit(1); }

    flock.l_type = F_WRLCK;
    flock.l_start = l_start;
    flock.l_whence = SEEK_SET;
    flock.l_len = l_len;
    if (fcntl(fileDesc, F_GETLK, &flock) < 0) {
        printf("Get lock failed\n");
        exit(1);
    }
    return flock;
}

void set_lock_read_nblock(int fileDesc, off_t charNum) {
    if (perform_lock_action(fileDesc, F_SETLK, F_RDLCK, SEEK_SET, charNum, 1) == -1) {
        printf("Cannot acquire read lock on file %d on %ld byte\n", fileDesc, charNum);
    }
}

void set_lock_write_nblock(int fileDesc, off_t charNum) {
    if (perform_lock_action(fileDesc, F_SETLK, F_WRLCK, SEEK_SET, charNum, 1) == -1) {
        printf("Cannot acquire write lock on file %d on %ld byte\n", fileDesc, charNum);
    }
}

void set_lock_read_block(int fileDesc, off_t charNum) {
    if (perform_lock_action(fileDesc, F_SETLKW, F_RDLCK, SEEK_SET, charNum, 1) == -1) {
        printf("Cannot acquire read lock on file %d on %ld byte\n", fileDesc, charNum);
    }
}

void set_lock_write_block(int fileDesc, off_t charNum) {
    if (perform_lock_action(fileDesc, F_SETLKW, F_WRLCK, SEEK_SET, charNum, 1) == -1) {
        printf("Cannot acquire read lock on file %d on %ld byte\n", fileDesc, charNum);
    }
}

void show_locked_chars(int fileDesc, off_t fileEnd) {
    struct flock flock;

    for (off_t i=0; i<fileEnd; i++) {
        flock = get_lock(fileDesc, i, 1);
        if(flock.l_type == F_WRLCK)
            printf("Write lock on %ld byte; PID: %d\n", i, flock.l_pid);

        if(flock.l_type == F_RDLCK)
            printf("Read lock on %ld byte; PID: %d\n", i, flock.l_pid);
    }
}

void release_lock(int fileDesc, off_t charNum) {
    if(perform_lock_action(fileDesc, F_SETLK, F_UNLCK, SEEK_SET, charNum, 1) == -1){
        printf("Cannot unlock %ld byte on file %d\n", charNum, fileDesc);
    }
}

void read_char(int fileDesc, off_t charNum) {
    char res;
    off_t f;
    struct flock lock = get_lock(fileDesc, charNum, 1);

    if(lock.l_type == F_WRLCK){ printf("%s\n", "Cannot read this character; it is locked."); return; }
    f = lseek(fileDesc, charNum, SEEK_SET);
    if (f == -1) { printf("Something went wrong on lseek function\n"); exit(1); }

    (read(fileDesc, &res, sizeof(char)) == 1) ?
        printf("Char on %ld byte: '%c'\n", charNum, res)
    :
        printf("%s\n", "Cannot read character");
}

void write_char(int fileDesc, off_t charNum, char c) {
    off_t f;
    struct flock lock = get_lock(fileDesc, charNum, 1);

    if(lock.l_type != F_UNLCK){ printf("%s\n", "Cannot write character; area is locked."); return; }
    f = lseek(fileDesc, charNum, SEEK_SET);
    if (f == -1) { printf("Something went wrong on lseek function\n"); exit(1); }

    if(write(fileDesc, &c, sizeof(char)) != 1)
        printf("%s\n", "Cannot write character\n");
}