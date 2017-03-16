//
// Created by damian on 16.03.17.
//

#ifndef CW1_REFACTOR_BST_H
#define CW1_REFACTOR_BST_H

struct TreeNode {
    struct TreeNode * left;
    struct TreeNode * right;
    struct TreeNode * up;
    Record * value;
    RecordPropType sortBy;
    long long id;
};
typedef struct TreeNode TreeNode;

TreeNode * makeAddressBookOnTree(RecordPropType);
void addToAddressBookOnTree(TreeNode *, Record *);
TreeNode * findInAddressBookOnTree(TreeNode *, char *);
void deleteFromAddressBookOnTree(TreeNode **, TreeNode *);
void showAddressBookOnTree(TreeNode *);
void rebuildAddressBookOnTree(TreeNode **, RecordPropType);
void deleteAddressBookOnTree(TreeNode **);

TreeNode * treeMin(TreeNode *);
TreeNode * treeSucceessor(TreeNode *);
TreeNode * getDeepestTreeNode(TreeNode *);
void deleteTreeNode(TreeNode *);
void printTreeNode(TreeNode *);

#endif //CW1_REFACTOR_BST_H
