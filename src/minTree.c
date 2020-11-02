#include "minTree.h"
#include "huff.h"

#include <stdlib.h>
#include <string.h>
// TESTING
#include <stdio.h>

minBTree *createNode(uint32_t value, minBTree *left, minBTree *right) {
    minBTree *node = calloc(1, sizeof(minBTree));
    if (node == NULL)
	return NULL;
    node->value = value;
    node->leaf_value = -1;
    node->left = left;
    node->right = right;
    return node;
}

minBTree *minTree_createTree(unsigned int table_length, huff_table table[]) {
    // Just take the entire table at first and make a bunch of nodes to start
    // inserting into the tree
    minBTree **t_list = calloc(table_length, sizeof(minBTree *));
    if (t_list == NULL) 
	return NULL;

    for (unsigned int i = 0; i < table_length; i++) {
	t_list[i] = createNode(table[i].freq, NULL, NULL);
	t_list[i]->leaf_value = table[i].value;
    }

    // combine nodes in the tree until the list is length 1
    while(table_length > 1) {
	minBTree *temp = createNode(t_list[0]->value + t_list[1]->value, t_list[0], t_list[1]);
	if (temp == NULL)
	    return NULL;

	// shift all nodes in the list into their correct places in the list
	unsigned int sh_down = 2;
	unsigned int place = 0;
	// Shift until new tree node should be inserted into list
	while(temp->value > t_list[sh_down]->value && sh_down < table_length) {
	    t_list[place] = t_list[sh_down];
	    sh_down++; place++;
	}
	// Insert temp node into the list
	t_list[place] = temp;
	place++;
	    
	// Pull down the remaining nodes in the list
	while(sh_down < table_length) {
	    t_list[place] = t_list[sh_down];
	    sh_down++; place++;
	}

	table_length--;
    }

    minBTree *result = t_list[0];
    free(t_list);

    return result;
}

void minTree_destroyTree(minBTree *current) {
    if (current->left != NULL) {
	minTree_destroyTree(current->left);
    }
    if (current->right != NULL) {
	minTree_destroyTree(current->right);
    }
    free(current);
}

unsigned int minTree_traverseToValue(uint32_t seeking, minBTree *current, uint32_t depth) {
    // Check the value
    if(current->leaf_value == (int32_t) seeking) {
	return depth;
    }

    uint32_t check;
    // Check left
    if(current->left != NULL) {
	if ((check = minTree_traverseToValue(seeking, current->left, depth + 1))) {
	    // No bit to set. So we just carry on.
	    return check;
	}
    }
    
    // Check right
    if(current->right != NULL) {
	if ((check = minTree_traverseToValue(seeking, current->right, depth + 1))) {
	    // Set the bit at this depth
	    treeBits[depth / 8] |= 1 << (7 - (depth % 8));
	    return check;
	}
    }

    return 0;
}

uint32_t minTree_toBits(uint32_t seeking, minBTree *current) {
    memset(treeBits, 0, TREEBITS);
    return minTree_traverseToValue(seeking, current, 0);
}

struct valueNode_tuple minTree_fromBit(uint8_t direction, minBTree *current) {
    if (direction) current = current->right;
    else current = current->left;
    if (current->leaf_value != -1)
	return (struct valueNode_tuple) {.value = current->leaf_value, .node = NULL };
    else
	return (struct valueNode_tuple) {.value = -1, .node = current };
}

int traverseAndPrint(int depth_seeking, int current_depth, minBTree *at) {
    if (depth_seeking == current_depth) {
	printf("Val = %i, Leaf = %i | ", at->value, at->leaf_value);
	return 1;
    }

    int check_l = 0;
    int check_r = 0;
    if (at->left != NULL && current_depth < depth_seeking) {
	check_l = traverseAndPrint(depth_seeking, current_depth + 1, at->left);
    }
    if (at->right != NULL && current_depth < depth_seeking) {
	check_r = traverseAndPrint(depth_seeking, current_depth + 1, at->right);
    }
    if (check_l || check_r)
	return 1;
    return 0;
}

void minTree_printTree(minBTree *start) {
    for (int i = 1, c = 1; c; i++) {
	printf("Depth = %i | ", i);
	c = traverseAndPrint(i, 1, start);
	printf("\n");
    }
}
