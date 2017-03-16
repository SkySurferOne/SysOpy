//
// Created by damian on 16.03.17.
//
#include "common.h"
#include "bst.h"

TreeNode * makeAddressBookOnTree(RecordPropType sortBy) {
    TreeNode * node = (TreeNode *) malloc(sizeof(TreeNode));
    node->value = NULL;
    node->left = NULL;
    node->right = NULL;
    node->up = NULL;
    node->sortBy = sortBy;
    node->id = -1;
    return node;
}

void addToAddressBookOnTree(TreeNode * root, Record * record) {
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

        TreeNode * node = (TreeNode *) malloc(sizeof(TreeNode));
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

TreeNode * findInAddressBookOnTree(TreeNode * root, char * searchBy) {
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
            if (compareBirthDates(parseDate(searchBy, "/"), root->value->birthDate) == 0)
                return root;
            else if (compareBirthDates(parseDate(searchBy, "/"), root->value->birthDate) < 0)
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
    return NULL;
}

void deleteFromAddressBookOnTree(TreeNode ** root, TreeNode * node) {
    TreeNode * y, * x;
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

    deleteTreeNode(y);
}

void showAddressBookOnTree(TreeNode * root) {
    if(root != NULL) {
        showAddressBookOnTree(root->left);
        printTreeNode(root);
        showAddressBookOnTree(root->right);
    }
}

// locale procedure
void addToNewTree(TreeNode * node, TreeNode * newRoot) {
    if (node != NULL) {
        addToNewTree(node->left, newRoot);
        addToNewTree(node->right, newRoot);
        node->up = NULL;
        node->left = NULL;
        node->right = NULL;
        addToAddressBookOnTree(newRoot, node->value);
        free(node);
    }
}

void rebuildAddressBookOnTree(TreeNode ** root, RecordPropType sortBy) {
    TreeNode * newRoot = makeAddressBookOnTree(sortBy);
    addToNewTree(*root, newRoot);
    *root = newRoot;
}

// locale procedure
void deleteAddressBookOnTreePostOrder(TreeNode * node) {
    if (node != NULL) {
        deleteAddressBookOnTreePostOrder(node->left);
        deleteAddressBookOnTreePostOrder(node->right);
        deleteTreeNode(node);
    }
}

void deleteAddressBookOnTree(TreeNode ** root) {
    deleteAddressBookOnTreePostOrder(*root);
    *root = NULL;
}

TreeNode * treeMin(TreeNode * node) {
    if(node == NULL) return NULL;
    while(node->left != NULL) {
        node = node->left;
    }
    return node;
}

TreeNode * treeSucceessor(TreeNode * node) {
    if (node->right != NULL)
        return treeMin(node->right);

    TreeNode * y = node->up;
    while (y != NULL && node == y->right) {
        node = y;
        y = y->up;
    }
    return y;
}

TreeNode * getDeepestTreeNode(TreeNode * root) {
    Stack * stack = stackInit();
    push(&stack, root);
    TreeNode * lastNode = NULL;

    while (!empty(stack)) {
        lastNode = (TreeNode *) pop(&stack);
        if (lastNode->left != NULL)
            push(&stack, lastNode->left);
        if(lastNode->right != NULL)
            push(&stack, lastNode->right);
    }

    return lastNode;
}

void deleteTreeNode(TreeNode * node) {
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

void printTreeNode(TreeNode * node) {
    if(node == NULL) printf("%s\n", "TreeNode is NULL");
    if(node->value == NULL) printf("%s\n", "Record is NULL");
    printf("id:%lld\n", node->id);
    printContactRecord(node->value);
}


