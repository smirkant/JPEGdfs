#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H

#include "gbit.h"

enum ht_type {
	HT_LEAF,
	HT_BRANCH
};

struct ht_node {
	int depth;
	int code;
	enum gbit bits[16];
	enum ht_type type;
	struct ht_node *next;
};

extern void ht_build_tree(int *table);
extern struct ht_node *ht_get_tree_row(int row);
extern void ht_destroy_tree(void);

extern void ht_chars_to_ints(char *chars, int *ints, int n);

#endif
