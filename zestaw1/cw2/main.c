#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lib/addressbooklib.h"

Record *makeRecord(char *pString[9]);

void addRecordToDataStr(Record *pRecord);

Node * bookOnTree = NULL;
Node * bookOnList = NULL;

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
        char * recStrings[9];
        int j = 0;

        tok = strtok(tmp, ";");
        while(tok != NULL) {
            if(i != 0)
                recStrings[j] = tok;
            tok = strtok( NULL,  ";");
            ++j;
        }

        if (i != 0) {
            // make record here
            Record *record = makeRecord(recStrings);
            addRecordToDataStr(record);
        }
        ++i;
    }

    return fclose(fp);
}

// TODO timing
void addRecordToDataStr(Record *pRecord) {
    clock_t begin = clock();

    addToAddressBookOnTree(bookOnTree, pRecord);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("%lf\n", time_spent);

    addToaddressBookOnLinkedlist(bookOnList, pRecord);
}

Address *makeAddress(char *streetAddress, char *city, char *country, char *postalCode) {
    Address * address = (Address *) malloc(sizeof(Address));
    address->street = streetAddress;
    address->city = city;
    address->country = country;
    address->postalCode = postalCode;
    address->houseNumber = "";
    return address;
}

Record *makeRecord(char *pString[9]) {
    Record * record = (Record *)  malloc(sizeof(Record));
    record->firstname = pString[0];
    record->lastname = pString[1];
    record->birthDate = parseDate(pString[2]);
    record->email = pString[3];
    record->phone = (unsigned int) atol(pString[4]);
    record->address = makeAddress(pString[5], pString[6], pString[7], pString[9]);
    return record;
}

// TODO timing
void makeBooks() {
    bookOnTree = makeAddressBookOnTree(LASTNAME);
    bookOnList = makeAddressBookOnLinkedList(LASTNAME);
}

int main() {
    makeBooks();

    readFile("../data.csv");

    return 0;
}