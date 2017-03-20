//
// Created by damian on 16.03.17.
//
#include "common.h"
#include "linkedlist.h"

ListNode * makeAddressBookOnLinkedList(RecordPropType sortBy) {
    ListNode * node = (ListNode *) malloc(sizeof(ListNode));
    node->value = NULL;
    node->prev = NULL;
    node->next = NULL;
    node->sortBy = sortBy;
    node->id = -1;
    return node;
}

void addToaddressBookOnLinkedlist(ListNode * head, Record * record) {
    if (head == NULL) { printf("%s\n", "head is NULL"); return; }

    ListNode * cp = head;
    ListNode * newNode = (ListNode *) malloc(sizeof(ListNode));
    newNode->id = generateId(record);
    newNode->sortBy = head->sortBy;
    newNode->value = record;
    if(cp->next == NULL) {
        head->next = newNode;
        newNode->prev = head;
        newNode->next = NULL;
    } else {
        RecordPropType sortBy = head->sortBy;
        while (cp->next != NULL &&
               compareRecordPropTypes(record, cp->next->value, sortBy) > 0) {
            cp = cp->next;
        }
        newNode->next = cp->next;
        if (cp->next != NULL) cp->next->prev = newNode;
        newNode->prev = cp;
        cp->next = newNode;
    }
}

ListNode * findInAddressBookOnLinkedlist(ListNode * head, char * searchBy) {
    head = head->next;
    while (head != NULL) {
        switch (head->sortBy) {
            case LASTNAME:
                if (compareCharArrays(head->value->lastname, searchBy) == 0)
                    return head;
                break;
            case EMAIL:
                if (compareCharArrays(head->value->email, searchBy) == 0)
                    return head;
                break;
            case BIRTH_DATE:
                if (compareBirthDates(head->value->birthDate, parseDate(searchBy, "/")) == 0)
                    return head;
                break;
            case PHONE_NUMBER:
                if (compareNumbers(head->value->phone, (unsigned long) atol(searchBy)) == 0)
                    return head;
                break;
        }
        head = head->next;
    }
    return NULL;
}

void deleteFromAddressBookOnLinkedlist(ListNode * node) {
    ListNode * toTrash = node;
    if (node != NULL && node->next != NULL)
        node->next->prev = node->prev;

    if(node != NULL && node->prev != NULL)
        node->prev->next = node->next;

    deleteListNode(toTrash);
}

void showAddressBookOnLinkedlist(ListNode * head) {
    if (head == NULL) { printf("%s\n", "List is empty"); return; }
    head = head->next;
    while (head != NULL) {
        printListNode(head);
        head = head->next;
    }
}

void rebuildAddressBookOnLinkedlist(ListNode ** head, RecordPropType sortBy) {
    if (*head == NULL) { printf("%s\n", "head is NULL"); return; }
    ListNode * newHead = makeAddressBookOnLinkedList(sortBy);

    ListNode * cp = (*head)->next;
    while (cp != NULL) {
        ListNode * toDel = cp;
        addToaddressBookOnLinkedlist(newHead, cp->value);
        cp = cp->next;
        free(toDel);
    }
    *head = newHead;
}

// locale procedure
void deleteAddressBookOnLinkedlistPostOrder(ListNode * node) {
    if (node != NULL) {
        deleteAddressBookOnLinkedlistPostOrder(node->next);
        deleteListNode(node);
    }
}

void deleteAddressBookOnLinkedList(ListNode ** root) {
    if (*root == NULL) return;
    deleteAddressBookOnLinkedlistPostOrder((*root)->next);
    *root = NULL;
}

void deleteListNode(ListNode * node) {
    node->prev = NULL;
    node->next = NULL;

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

void printListNode(ListNode * node) {
    if(node == NULL) printf("%s\n", "Node is NULL");
    if(node->value == NULL) printf("%s\n", "Record is NULL");
    printf("id:%lld\n", node->id);
    printContactRecord(node->value);
}
