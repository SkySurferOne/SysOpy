#ifndef ADDRESSBOOKLIB_H
#define ADDRESSBOOKLIB_H
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

struct Node {
  struct Node * left;
  struct Node * right;
  struct Node * up; // used only in tree impl
  Record * value;
  RecordPropType sortBy;
  long long id;
};
typedef struct Node Node;

extern void test (void);

// tree api
// implemented
extern Node * makeAddressBookOnTree(RecordPropType sortBy);
extern void addToAddressBookOnTree(Node *, Record *);
extern Node * findInAddressBookOnTree(Node * root, char * searchBy);
extern void deleteFromAddressBookOnTree(Node ** root, Node * node);
extern void showAddressBookOnTree(Node *);
extern void rebuildAddressBookOnTree(Node ** root, RecordPropType sortBy);

// linked list
extern Node * makeAddressBookOnLinkedList(RecordPropType sortBy);
extern void addToaddressBookOnLinkedlist(Node *, Record *);
extern Record * findInAddressBookOnLinkedlist(Node * head, char * searchBy);
extern void deleteFromAddressBookOnLinkedlist(Node ** head, Node * node);
extern void showAddressBookOnLinkedlist(Node *);
extern void rebuildAddressBookOnLinkedlist(Node ** head, RecordPropType sortBy);


// utils
// potrzeba 2 ?
extern void deleteAddressBook(Node ** root);

extern long long generateId(Record *);
extern int compareCharArrays(char * a, char * b);
extern int compareBirthDates(Date * a, Date * b);
extern int compareNumbers(unsigned long a, unsigned long b);
extern int compareRecordPropTypes(Record * a, Record * b, RecordPropType propType);
extern Record * makeContact(
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
);
extern void printContactNode(Node * node);
extern void printContactRecord(Record * record);
extern Node * treeMin(Node * node);
extern Node * treeSucceessor(Node * node);
extern void deleteNode(Node * node);
extern Date * parseDate(char * dateStr);

#endif
