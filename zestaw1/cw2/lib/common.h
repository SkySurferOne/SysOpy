//
// Created by damian on 16.03.17.
//

#ifndef CW1_REFACTOR_COMMON_H
#define CW1_REFACTOR_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0

enum RecordPropType {
    LASTNAME,
    BIRTH_DATE,
    EMAIL,
    PHONE_NUMBER
};
typedef enum RecordPropType RecordPropType;

struct Date {
    int day;
    int month;
    int year;
};
typedef struct Date Date;

struct Address {
    char * street;
    char * city;
    char * country;
    char * houseNumber;
    char * postalCode;
};
typedef struct Address Address;

struct Record {
    char * firstname;
    char * lastname;
    Date * birthDate;
    char * email;
    unsigned long phone;
    Address * address;
};
typedef struct Record Record;

long long generateId(Record *);
int compareCharArrays(char *, char *);
int compareBirthDates(Date *, Date *);
int compareNumbers(unsigned long, unsigned long);
int compareRecordPropTypes(Record *, Record *, RecordPropType);
Record * makeContact(char *, char *, char *, unsigned long, unsigned short, unsigned short,
                     unsigned short, char *, char *, char *, char *, char *);
void printContactRecord(Record * record);
Date * parseDate(char *, char *);

struct Stack {
    struct Stack * prev;
    void * value;
};
typedef struct Stack Stack;

Stack * stackInit();
void push(Stack **, void * value);
void * pop(Stack **);
int empty(Stack *);

#endif //CW1_REFACTOR_COMMON_H
