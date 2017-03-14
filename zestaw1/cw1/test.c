#include "addressbooklib.h"
#include <stdio.h>

int main () {
  //test();

  // test tree
  // Node * treeRoot = makeAddressBookOnTree(LASTNAME);
  // Record * contact1 = makeContact(
  //   "Damian", "Dowal",
  //   "damian@damian.pl",
  //   666777888,
  //   20, 2, 1996,
  //   "Dluga", "Krakow", "Polska", "8/90", "33-333"
  // );
  // Record * contact2 = makeContact(
  //   "Damian", "Zowal",
  //   "damian@damian.pl",
  //   666777888,
  //   20, 2, 1996,
  //   "Dluga", "Krakow", "Polska", "8/90", "33-333"
  // );
  // addToAddressBookOnTree(treeRoot, contact1);
  // addToAddressBookOnTree(treeRoot, contact2);
  // showAddressBookOnTree(treeRoot);
  // Node * foundCon1 = findInAddressBookOnTree(treeRoot, "Zowal");
  // printf("Found contact named 'Zowal': \n");
  // printContactNode(foundCon1);
  // deleteFromAddressBookOnTree(&treeRoot, foundCon1);
  // printf("After deletion 'Zowal': \n");
  // showAddressBookOnTree(treeRoot);
  //
  // deleteAddressBook(&treeRoot);
  // printf("After whole tree deletion: \n");
  // showAddressBookOnTree(treeRoot);
  //
  // //
  // printf("A new one: \n");
  // treeRoot = makeAddressBookOnTree(LASTNAME);
  // Record * contact3 = makeContact(
  //   "Damian", "Dowal",
  //   "damian@damian.pl",
  //   866777888,
  //   20, 2, 1996,
  //   "Dluga", "Krakow", "Polska", "8/90", "33-333"
  // );
  // Record * contact4 = makeContact(
  //   "Damian", "Zowal",
  //   "damian@damian.pl",
  //   266777888,
  //   20, 2, 1996,
  //   "Dluga", "Krakow", "Polska", "8/90", "33-333"
  // );
  // Record * contact5 = makeContact(
  //   "Damian", "Bowal",
  //   "damian@damian.pl",
  //   766777888,
  //   20, 2, 1996,
  //   "Dluga", "Krakow", "Polska", "8/90", "33-333"
  // );
  // addToAddressBookOnTree(treeRoot, contact3);
  // addToAddressBookOnTree(treeRoot, contact4);
  // addToAddressBookOnTree(treeRoot, contact5);
  // showAddressBookOnTree(treeRoot);
  //
  // rebuildAddressBookOnTree(&treeRoot, PHONE_NUMBER);
  // printf("After rebuild: \n");
  // showAddressBookOnTree(treeRoot);

  // list test
  Node * head = makeAddressBookOnLinkedList(LASTNAME);
  Record * contact1 = makeContact(
    "Damian", "Dowal",
    "damian@damian.pl",
    666777888,
    20, 2, 1996,
    "Dluga", "Krakow", "Polska", "8/90", "33-333"
  );
  Record * contact2 = makeContact(
    "Damian", "Zowal",
    "damian@damian.pl",
    666777888,
    20, 2, 1996,
    "Dluga", "Krakow", "Polska", "8/90", "33-333"
  );
  addToaddressBookOnLinkedlist(head, contact1);
  addToaddressBookOnLinkedlist(head, contact2);
  showAddressBookOnLinkedlist(head);

  return 0;
}
