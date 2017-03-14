#include "addressbooklib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
  ====================================
    Tree implementation
  ====================================
*/
Node * makeAddressBookOnLinkedList(RecordPropType sortBy) {
  return makeAddressBookOnTree(sortBy);
}
//void addToaddressBookOnLinkedlist(Node *, Record *);
// Record * findInAddressBookOnLinkedlist(Node * head, char * searchBy);
// void deleteFromAddressBookOnLinkedlist(Node ** head, Node * node);
// void showAddressBookOnLinkedlist(Node *);
// void rebuildAddressBookOnLinkedlist(Node ** head, RecordPropType sortBy);

// tested
Node * makeAddressBookOnTree(RecordPropType sortBy) {
  Node * node = (Node *) malloc(sizeof(Node));
  node->value = NULL;
  node->left = NULL;
  node->right = NULL;
  node->up = NULL;
  node->sortBy = sortBy;
  node->id = -1;
  return node;
}

// tested
void addToAddressBookOnTree(Node * root, Record * record) {
  if(root == NULL) {printf("%s\n", "root is NULL. You can't add item to empty root."); return; }
  if (root->value == NULL) {
    root->value = record;
    root->id = generateId(record);
  } else {

    while(true) {
      if (compareRecordPropTypes(record, root->value, root->sortBy) <= 0) {
        if(root->left == NULL) break;
        root = root->left;
      } else {
        if(root->right == NULL) break;
        root = root->right;
      }
    }

    Node * node = (Node *) malloc(sizeof(Node));
    node->left = NULL;
    node->right = NULL;
    node->up = root;
    node->sortBy = root->sortBy;
    node->id = generateId(record);
    node->value = record;
    if (compareRecordPropTypes(record, root->value, root->sortBy) <= 0) {
      root->left = node;
    } else {
      root->right = node;
    }
  }
}

Node * findInAddressBookOnTree(Node * root, char * searchBy) {
  if (root == NULL) return NULL;
  switch (root->sortBy) {
    case LASTNAME:
      if (compareCharArrays(searchBy, root->value->lastname) == 0)
        return root;
      else if (compareCharArrays(searchBy, root->value->lastname) < 0)
        return findInAddressBookOnTree(root->left, searchBy);
      else
        return findInAddressBookOnTree(root->right, searchBy);
    case EMAIL:
      if (compareCharArrays(searchBy, root->value->email) == 0)
        return root;
      else if (compareCharArrays(searchBy, root->value->email) < 0)
        return findInAddressBookOnTree(root->left, searchBy);
      else
        return findInAddressBookOnTree(root->right, searchBy);
    case BIRTH_DATE:
      if (compareBirthDates(parseDate(searchBy), root->value->birthDate) == 0)
        return root;
      else if (compareBirthDates(parseDate(searchBy), root->value->birthDate) < 0)
        return findInAddressBookOnTree(root->left, searchBy);
      else
        return findInAddressBookOnTree(root->right, searchBy);
    case PHONE_NUMBER:
    if (compareNumbers(atol(searchBy), root->value->phone) == 0)
      return root;
    else if (compareNumbers(atol(searchBy), root->value->phone) < 0)
      return findInAddressBookOnTree(root->left, searchBy);
    else
      return findInAddressBookOnTree(root->right, searchBy);
  }
}

Date * parseDate(char * dateStr) {
  int d, m, y;
  sscanf(dateStr, "%d-%d-%d", &d, &m, &y);
  Date * date = (Date *) malloc(sizeof(Date));
  date->day = d;
  date->month = m;
  date->year = y;
  return date;
}

void addToNewTree(Node * node, Node * newRoot) {
  if (node != NULL) {
    addToNewTree(node->left, newRoot);
    addToNewTree(node->right, newRoot);
    node->up = NULL;
    node->left = NULL;
    node->right = NULL;
    addToAddressBookOnTree(newRoot, node->value);
  }
}

void rebuildAddressBookOnTree(Node ** root, RecordPropType sortBy) {
  Node * newRoot = makeAddressBookOnTree(sortBy);

  addToNewTree(*root, newRoot);

  *root = newRoot;
}

// tested
void showAddressBookOnTree(Node * root) {
  if(root != NULL) {
    showAddressBookOnTree(root->left);
    printContactNode(root);
    showAddressBookOnTree(root->right);
  }
}

Node * treeMin(Node * node) {
  if(node == NULL) return NULL;
  while(node->left != NULL) {
    node = node->left;
  }
  return node;
}

Node * treeSucceessor(Node * node) {
  if (node->right != NULL)
    return treeMin(node->right);

  Node * y = node->up;
  while (y != NULL && node == y->right) {
    node = y;
    y = y->up;
  }
  return y;
}

void deleteFromAddressBookOnTree(Node ** root, Node * node) {
  Node * y, * x;
  if (node->left == NULL || node->right == NULL)
    y = node;
  else
    y = treeSucceessor(node);

  if(y->left != NULL)
    x = y->left;
  else
    x = y->right;

  if(x != NULL)
    x->up = y->up;

  if(y->up == NULL)
    *root = x;
  else if (y == y->up->left)
    y->up->left = x;
  else
    y->up->right = x;

  if(y != node) {
    node->value = y->value;
    node->id = y->id;
  }

  deleteNode(y);
}

void deleteNode(Node * node) {
  node->left = NULL;
  node->right = NULL;
  node->up = NULL;

  Record * record = node->value;
  Date * date = record->birthDate;
  Address * address = record->address;

  free(address);
  free(date);
  free(record);
  free(node);

  record = NULL;
  date = NULL;
  address = NULL;
}

void deleteAddressBookPostOrder(Node * node) {
  if (node != NULL) {
    deleteAddressBookPostOrder(node->left);
    deleteAddressBookPostOrder(node->right);
    deleteNode(node);
  }
}

void deleteAddressBook(Node ** root) {
  deleteAddressBookPostOrder(*root);
  *root = NULL;
}

/*
  ====================================
    Linked list implementation
  ====================================
*/

/*
  ================================
    Utils
  ================================
*/
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
}

// a > b -> 1
// a < b -> -1
// a == b -> 0
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
  int eval = (a - b);
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

void printContactNode(Node * node) {
  if(node == NULL) printf("%s\n", "Node is NULL");
  if(node->value == NULL) printf("%s\n", "Record is NULL");
  printf("id:%Ld\n", node->id);
  printContactRecord(node->value);
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

void test (void) {
  printf("%s\n", "Test!");
}
