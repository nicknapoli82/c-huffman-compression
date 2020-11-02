#ifndef MINTREE
#define MINTREE

#include <stdint.h>

#include "huff.h"

#define TREEBITS (256 / 8)

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

uint32_t minTree_toBits(uint32_t seeking, minBTree *root);

struct valueNode_tuple minTree_fromBit(uint8_t direction, minBTree *current);

void minTree_printTree(minBTree *start);

#endif
