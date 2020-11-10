#ifndef MINTREE
#define MINTREE

/*********************************************************************/
/* A note to anyone reading through this code.			     */
/* I am not a personal fan of the coupling of the tree code and the  */
/* huff code. The only thing that ties the two directly together is  */
/* the treeBits array. I could separate these, but alas I don't feel */
/* to much need. This tree code could be generalized much better.    */
/*********************************************************************/

#include <stdint.h>

#include "huff.h"

// The idea of treeBits is that this tree operates directly with the huffman algorithm
// The worst case depth of the tree itself should be no more than 256
// Or the POSSIBLE_VALUES defined 
#define TREEBITS (POSSIBLE_VALUES / 8)

char treeBits[TREEBITS];

typedef struct minBTree {
    int32_t value;
    int32_t leaf_value;
    struct minBTree *left;
    struct minBTree *right;
} minBTree;

struct valueNode_tuple {
    uint8_t value;
    minBTree *node;
};

minBTree *minTree_createTree(unsigned int table_length, huff_table table[]);

void minTree_destroyTree(minBTree *current);

// Directly reads bits from table into treeBits
uint32_t minTree_toBits(uint32_t seeking, minBTree *root);

// Traverse tree depth 1 and return the current position in the tree
struct valueNode_tuple minTree_fromBit(uint8_t direction, minBTree *current);

// Debugging only. Will print out the huffman tree generated in an unpretty format
void minTree_printTree(minBTree *start);

#endif
