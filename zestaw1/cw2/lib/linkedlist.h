//
// Created by damian on 16.03.17.
//

#ifndef CW1_REFACTOR_LINKEDLIST_H
#define CW1_REFACTOR_LINKEDLIST_H

struct ListNode {
    struct ListNode * prev;
    struct ListNode * next;
    Record * value;
    RecordPropType sortBy;
    long long id;
};
typedef struct ListNode ListNode;

ListNode * makeAddressBookOnLinkedList(RecordPropType);
void addToaddressBookOnLinkedlist(ListNode *, Record *);
ListNode * findInAddressBookOnLinkedlist(ListNode *, char *);
void deleteFromAddressBookOnLinkedlist(ListNode *);
void showAddressBookOnLinkedlist(ListNode *);
void rebuildAddressBookOnLinkedlist(ListNode **, RecordPropType);
void deleteAddressBookOnLinkedList(ListNode **);

void deleteListNode(ListNode *);
void printListNode(ListNode *);

#endif //CW1_REFACTOR_LINKEDLIST_H
