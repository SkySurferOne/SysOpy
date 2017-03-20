#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <dlfcn.h>
#include "addressbook.h"

#define FILE_SEP ","
#define FILE_COL_NUM 10

void *lib;
char *error;

Date * (*dparseDate)(char *, char *);

TreeNode * (*dmakeAddressBookOnTree)(RecordPropType);
void (*daddToAddressBookOnTree)(TreeNode *, Record *);
TreeNode * (*dfindInAddressBookOnTree)(TreeNode *, char *);
void (*ddeleteFromAddressBookOnTree)(TreeNode **, TreeNode *);
void (*dshowAddressBookOnTree)(TreeNode *);
void (*drebuildAddressBookOnTree)(TreeNode **, RecordPropType);
void (*ddeleteAddressBookOnTree)(TreeNode **);
TreeNode * (*dgetDeepestTreeNode)(TreeNode *);

ListNode * (*dmakeAddressBookOnLinkedList)(RecordPropType);
void (*daddToaddressBookOnLinkedlist)(ListNode *, Record *);
ListNode * (*dfindInAddressBookOnLinkedlist)(ListNode *, char *);
void (*ddeleteFromAddressBookOnLinkedlist)(ListNode *);
void (*dshowAddressBookOnLinkedlist)(ListNode *);
void (*drebuildAddressBookOnLinkedlist)(ListNode **, RecordPropType);
void (*ddeleteAddressBookOnLinkedList)(ListNode **);

Record *makeRecord(char *pString[FILE_COL_NUM]);
void addRecordToDataStrTest(Record *pRecord);

TreeNode * bookOnTree = NULL;
ListNode * bookOnList = NULL;
double real_start;
double real_previous;
double user_start;
double user_previous;
double system_start;
double system_previus;


int readFile(const char * path) {
    FILE * fp = fopen(path, "r");

    if (fp == NULL) {
        printf ("File doesn't exist, errno = %d\n", errno);
        return 1;
    }

    char line[1024];
    char * tok;
    int i = 0;
    while (fgets(line, sizeof line, fp) != NULL) {
        char * tmp = strdup(line);
        char * recStrings[FILE_COL_NUM];
        int j = 0;

        tok = strtok(tmp, FILE_SEP);
        while(tok != NULL && i != 0) {
            recStrings[j] = tok;
            tok = strtok(NULL, FILE_SEP);
            ++j;
        }

        if (i != 0) {
            Record *record = makeRecord(recStrings);
            addRecordToDataStrTest(record);
        }
        ++i;
    }

    return fclose(fp);
}

Address *makeAddress(char *street, char *houseNumber, char *city, char *country, char *postalCode) {
    Address * address = (Address *) malloc(sizeof(Address));
    address->street = street;
    address->city = city;
    address->country = country;
    address->postalCode = postalCode;
    address->houseNumber = houseNumber;
    return address;
}

char * removeNewLineSign(char * str) {
    if (str == NULL || strlen(str) == 0) return "";
    if(str[strlen(str) - 1] != '\n')
        return str;
    str[strlen(str) - 1] = '\0';
    return str;
}

Record *makeRecord(char *pString[FILE_COL_NUM]) {
    Record * record = (Record *) malloc(sizeof(Record));
    record->firstname = pString[0];
    record->lastname = pString[1];
    record->birthDate = dparseDate(pString[2], "/");
    record->email = pString[3];
    record->phone = (unsigned int) atol(pString[4]);
    record->address = makeAddress(pString[5], pString[6], pString[7], pString[8], removeNewLineSign(pString[9]));
    return record;
}

void initTime() {
    struct tms buffer;
    times(&buffer);
    real_start = real_previous = clock() / (double)CLOCKS_PER_SEC;
    user_start = user_previous = buffer.tms_utime / (double)CLOCKS_PER_SEC;
    system_start = system_previus = buffer.tms_stime / (double)CLOCKS_PER_SEC;
}

void getAndPrintTime(char *info){
    struct tms buffer;
    times(&buffer);

    double real = clock() / (double)CLOCKS_PER_SEC;
    double user = buffer.tms_utime / (double)CLOCKS_PER_SEC;
    double sys = buffer.tms_stime / (double)CLOCKS_PER_SEC;
    if(strcmp(info, "--") != 0 ){
        printf("%s:\n", info);
        printf("Times: real = %f, user = %f, system = %f\n", real - real_previous, user - user_previous, sys - system_previus);
        printf("Times from start: real = %f, user = %f, system = %f\n\n", real - real_start, user - user_start, sys - system_start);
    }
    real_previous = real;
    user_previous = user;
    system_previus = sys;
}

// making books
void makeBooksTest() {
    bookOnTree = dmakeAddressBookOnTree(LASTNAME);
    getAndPrintTime("Make book on tree");
    bookOnList = dmakeAddressBookOnLinkedList(LASTNAME);
    getAndPrintTime("Make book on linked list");
}

// adding records
void addRecordToDataStrTest(Record *pRecord) {
    daddToAddressBookOnTree(bookOnTree, pRecord);
    getAndPrintTime("Add record to address book on tree");
    daddToaddressBookOnLinkedlist(bookOnList, pRecord);
    getAndPrintTime("Add record to address book on list");
}

// rebuilding
void rebuildDataStrTest() {
    drebuildAddressBookOnTree(&bookOnTree, PHONE_NUMBER);
    getAndPrintTime("Rebuild book on tree");
    drebuildAddressBookOnLinkedlist(&bookOnList, PHONE_NUMBER);
    getAndPrintTime("Rebuild book on list");
}

// finding
void findAndDeleteRecordTest() {
    unsigned long phone = bookOnTree->value->phone;
    char searchKey[10];
    snprintf(searchKey, 10, "%zu", phone);
    printf("Search key: %s\n", searchKey);

    TreeNode * foundTreeNode = dfindInAddressBookOnTree(bookOnTree, searchKey);
    getAndPrintTime("Find record by search key in tree (optimistic)");

    phone = dgetDeepestTreeNode(bookOnTree)->value->phone;
    snprintf(searchKey, 10, "%zu", phone);
    dfindInAddressBookOnTree(bookOnTree, searchKey);
    getAndPrintTime("Find record by search in tree (pessimistic)");

    phone = bookOnList->next->value->phone;
    snprintf(searchKey, 10, "%zu", phone);
    ListNode * foundListNode = dfindInAddressBookOnLinkedlist(bookOnList, searchKey);
    getAndPrintTime("Find record by search in linked list (optimistic)");

    ListNode * cp = bookOnList->next;
    while (cp != NULL && cp->next != NULL) cp = cp->next;
    phone = cp->value->phone;
    snprintf(searchKey, 10, "%zu", phone);
    dfindInAddressBookOnLinkedlist(bookOnList, searchKey);
    getAndPrintTime("Find record by search in linked list (pessimistic)");

    ddeleteFromAddressBookOnTree(&bookOnTree, foundTreeNode);
    getAndPrintTime("Delete node on tree");

    ddeleteFromAddressBookOnLinkedlist(foundListNode);
    getAndPrintTime("Delete node on linked list");
}

void deleteAddressBooks() {
    ddeleteAddressBookOnTree(&bookOnTree);
    getAndPrintTime("Delete whole address book on tree");

    ddeleteAddressBookOnLinkedList(&bookOnList);
    getAndPrintTime("Delete whole address book on linked list");
}

void checkDLErr() {
    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        exit(1);
    }
}

void loadDynApi() {
    #pragma GCC diagnostic push    //Save actual diagnostics state
    #pragma GCC diagnostic ignored "-Wpedantic"    //Disable pedantic
    dparseDate = dlsym(lib, "parseDate"); checkDLErr();
    dmakeAddressBookOnTree = dlsym(lib, "makeAddressBookOnTree"); checkDLErr();

    daddToAddressBookOnTree = dlsym(lib, "addToAddressBookOnTree"); checkDLErr();
    dfindInAddressBookOnTree = dlsym(lib, "findInAddressBookOnTree"); checkDLErr();
    ddeleteFromAddressBookOnTree = dlsym(lib, "deleteFromAddressBookOnTree"); checkDLErr();
    dshowAddressBookOnTree = dlsym(lib, "showAddressBookOnTree"); checkDLErr();
    drebuildAddressBookOnTree = dlsym(lib, "rebuildAddressBookOnTree"); checkDLErr();
    ddeleteAddressBookOnTree = dlsym(lib, "deleteAddressBookOnTree"); checkDLErr();
    dgetDeepestTreeNode = dlsym(lib, "getDeepestTreeNode"); checkDLErr();

    dmakeAddressBookOnLinkedList = dlsym(lib, "makeAddressBookOnLinkedList"); checkDLErr();
    daddToaddressBookOnLinkedlist = dlsym(lib, "addToaddressBookOnLinkedlist"); checkDLErr();
    dfindInAddressBookOnLinkedlist = dlsym(lib, "findInAddressBookOnLinkedlist"); checkDLErr();
    ddeleteFromAddressBookOnLinkedlist = dlsym(lib, "makeAddressBookOnTree"); checkDLErr();
    dshowAddressBookOnLinkedlist = dlsym(lib, "makeAddressBookOnTree"); checkDLErr();
    drebuildAddressBookOnLinkedlist = dlsym(lib, "makeAddressBookOnTree"); checkDLErr();
    ddeleteAddressBookOnLinkedList = dlsym(lib, "makeAddressBookOnTree"); checkDLErr();
    #pragma GCC diagnostic pop    //Restore diagnostics state
}

int main() {
    if (!access("../../zad1/build/libaddressbookShared.so", F_OK ))
        lib = dlopen ("../../zad1/build/libaddressbookShared.so", RTLD_LAZY);
    else if (!access("../zad1/build/libaddressbookShared.so", F_OK )) {
        lib = dlopen ("../zad1/build/libaddressbookShared.so", RTLD_LAZY);
    } else {
        printf("Cannot find the 'libaddressbookShared.so' file");
        exit(1);
    }

    if (!lib) { fputs(dlerror(), stderr); exit(1); }
    loadDynApi();

    initTime(); // init time variables
    makeBooksTest(); // test book making
    if (!access("../data.csv", F_OK ))
        readFile("../data.csv"); // read data and test adding records
    else if (!access("data.csv", F_OK ))
        readFile("data.csv");
    else if (!access("../../zad2/data.csv", F_OK ))
        readFile("../../zad2/data.csv");
    else if (!access("../zad2/data.csv", F_OK ))
        readFile("../zad2/data.csv");
    else {
        printf("Cannot reach the file 'data.csv'");
        exit(1);
    }
    rebuildDataStrTest(); // test rebuild data structures
    findAndDeleteRecordTest(); // test finding and deleting
    deleteAddressBooks(); // test deleting whole books

    return 0;
}