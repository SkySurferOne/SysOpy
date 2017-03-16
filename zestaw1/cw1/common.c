//
// Created by damian on 16.03.17.
//
#include "common.h"

int compareRecordPropTypes(Record * a, Record * b, RecordPropType propType) {
    switch (propType) {
        case LASTNAME:
            return compareCharArrays(a->lastname, b->lastname);
        case EMAIL:
            return compareCharArrays(a->email, b->email);
        case BIRTH_DATE:
            return compareBirthDates(a->birthDate, b->birthDate);
        case PHONE_NUMBER:
            return compareNumbers(a->phone, b->phone);
    }
    return -2;
}

int compareCharArrays(char * a, char * b) {
    int eval = strcmp(a, b);
    if (eval < 0)
        return -1;
    else if (eval >0)
        return 1;

    return 0;
}

int compareBirthDates(Date * a, Date * b) {
    if (a->year > b->year) {
        return 1;
    } else if (a->year < b->year) {
        return -1;
    } else {
        if (a->month > b->month) {
            return 1;
        } else if (a->month < b->month) {
            return -1;
        } else {
            if (a->day > b->day) {
                return 1;
            } else if (a->day < b->day) {
                return -1;
            }
        }
    }

    return 0;
}

int compareNumbers(unsigned long a, unsigned long b) {
    long eval = (a - b);
    if (eval < 0)
        return -1;
    else if (eval > 0)
        return 1;
    else
        return 0;
}

long long generateId(Record * record) {
    long long weight = 709;
    long long sum = 0;
    long long mod = 104729;
    for (int i=0; record->lastname[i] != 0; i++)
        sum = sum%mod * weight + record->lastname[i];
    for (int i=0; record->firstname[i] != 0; i++)
        sum = sum%mod * weight + record->firstname[i];
    for (int i=0; record->email[i] != 0; i++)
        sum = sum%mod * weight + record->email[i];
    for (int i=0; record->address->street[i] != 0; i++)
        sum = sum%mod * weight + record->address->street[i];
    for (int i=0; record->address->city[i] != 0; i++)
        sum = sum%mod * weight + record->address->city[i];
    for (int i=0; record->address->country[i] != 0; i++)
        sum = sum%mod * weight + record->address->country[i];
    for (int i=0; record->address->houseNumber[i] != 0; i++)
        sum = sum%mod * weight + record->address->houseNumber[i];
    for (int i=0; record->address->postalCode[i] != 0; i++)
        sum = sum%mod * weight + record->address->postalCode[i];
    sum = sum%mod * weight + record->phone % 709;
    sum = sum%mod * weight + record->birthDate->year;
    sum = sum%mod * weight + record->birthDate->month;
    sum = sum%mod * weight + record->birthDate->day;
    return sum%mod;
}

Record * makeContact(
        char * firstname, char * lastname,
        char * email,
        unsigned long phone,
        unsigned short birthDay,
        unsigned short birthMonth,
        unsigned short birthYear,
        char * street,
        char * city,
        char * country,
        char * houseNumber,
        char * postalCode
) {
    Record * record = (Record *) malloc(sizeof(Record));
    record->firstname = firstname;
    record->lastname = lastname;
    record->email = email;
    record->phone = phone;

    record->birthDate = (Date *) malloc(sizeof(Date));
    record->birthDate->day = birthDay;
    record->birthDate->month = birthMonth;
    record->birthDate->year = birthYear;

    record->address = (Address *) malloc(sizeof(Address));
    record->address->street = street;
    record->address->city = city;
    record->address->country = country;
    record->address->houseNumber = houseNumber;
    record->address->postalCode = postalCode;

    return record;
}

void printContactRecord(Record * record) {
    if(record == NULL) printf("%s\n", "Record is NULL");
    printf("%s %s \nemail: %s \nphone: %zu\nbirth date: %d.%d.%d\nAddress: %s %s, %s %s, %s\n\n",
           record->firstname, record->lastname, record->email, record->phone,
           record->birthDate->day, record->birthDate->month, record->birthDate->year,
           record->address->street,
           record->address->houseNumber,
           record->address->postalCode,
           record->address->city,
           record->address->country);
}

Date * parseDate(char * dateStr, char * del) {
    int d, m, y;
    char format[50];
    snprintf(format, sizeof(format), "%s%s%s%s%s", "%d", del, "%d", del, "%d");
    sscanf(dateStr, format, &d, &m, &y);
    Date * date = (Date *) malloc(sizeof(Date));
    date->day = d;
    date->month = m;
    date->year = y;
    return date;
}

Stack * stackInit() {
    return NULL;
}

void push(Stack ** stack, void * value) {
    if (*stack == NULL) {
        *stack = (Stack *) malloc(sizeof(Stack));
        (*stack)->prev = NULL;
        (*stack)->value = value;
    } else {
        Stack * newItem = (Stack *) malloc(sizeof(Stack));
        newItem->value = value;
        newItem->prev = (*stack);
        *stack = newItem;
    }
}

void * pop(Stack ** stack) {
    if (*stack == NULL) return NULL;
    Stack * tmp = (*stack);
    void * item = tmp->value;
    (*stack) = (*stack)->prev;
    free(tmp);
    return item;
};

int empty(Stack * stack) {
    return (stack == NULL);
}

