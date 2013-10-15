#include <stdlib.h>
#include "huffmantree.h"
#include "gbit.h"

static struct ht_node *tree[17];
static int depth;

#define L_VALUE   GBIT_0
#define R_VALUE   GBIT_1

static struct ht_node *new_left_child(struct ht_node *parent);
static struct ht_node *new_right_child(struct ht_node *parent);
static struct ht_node *new_tree(void);
static void new_row(int cur_depth);
static void set_leaf_flag(int cur_depth, int length);

extern void ht_chars_to_ints(char *chars, int *ints, int n)
{
	int i = 0, *pi = ints;
	char ch, *pc = chars;

	for (i = 0; i < n; i++) {
		ch = *pc++;
		*pi++ = (int)ch;
	}
}

extern struct ht_node *ht_get_tree_row(int row)
{
	return tree[row];
}

extern void ht_build_tree(int *table)
{
	int i = 0;
	struct ht_node *p = NULL;

	p = new_tree();
	tree[0] = p;

	depth = 1;
	for (i = 1; i < 17; i++) {
		new_row(i - 1);
		set_leaf_flag(i, table[i - 1]);
		depth = depth + 1;
	}
}

extern void ht_destroy_tree(void)
{
	int i;
	struct ht_node *p, *p1;

	for (i = 0; i < 17; i++) {
		p = tree[i];
		while (p != NULL) {
			p1 = p;
			p = p->next;
			free(p1);
		}
	}
}

static void set_leaf_flag(int cur_depth, int length)
{
	struct ht_node *p = NULL;
	int i = 0;

	p = tree[cur_depth];

	for (i = 0; i < length; i++) {
		p->type = HT_LEAF;
		p = p->next;
	}
}

static void new_row(int cur_depth)
{
	struct ht_node *p1 = NULL, *p2 = NULL, *p;

	p1 = tree[cur_depth];

	while (p1 != NULL) {
		if (p1->type == HT_BRANCH)
			break;
		p1 = p1->next;
	}

	if (p1 != NULL) {
		p = new_left_child(p1);
		tree[cur_depth + 1] = p;
		p2 = new_right_child(p1);
		p->next = p2;
		p = p->next;
		p1 = p1->next;
	}

	while (p1 != NULL) {
		p2 = new_left_child(p1);
		p->next = p2;
		p = p->next;
		p2 = new_right_child(p1);
		p->next = p2;
		p = p->next;
		p1 = p1->next;
	}
}

static struct ht_node *new_tree(void)
{
	struct ht_node *root = NULL;
	int i = 0;

	for (i = 0; i < 16; i++)
		tree[i] = NULL;
	depth = 0;

	root = (struct ht_node *)malloc(sizeof(struct ht_node));
	if (root != NULL) {
		root->depth = 0;
		root->code = 0;
		root->type = HT_BRANCH;
		root->next = NULL;
	}

	return root;
}

static struct ht_node *new_left_child(struct ht_node *parent)
{
	struct ht_node *node = NULL;
	int i = 0;

	node = (struct ht_node *)malloc(sizeof(struct ht_node));
	if (node != NULL) {
		node->depth = depth;
		node->code = 0;
		for (i = 0; i < parent->depth; i++)
			node->bits[i] = parent->bits[i];
		node->bits[i] = L_VALUE;
		node->type = HT_BRANCH;
		node->next = NULL;
	}

	return node;
}

static struct ht_node *new_right_child(struct ht_node *parent)
{
	struct ht_node *node = NULL;
	int i = 0;

	node = (struct ht_node *)malloc(sizeof(struct ht_node));
	if (node != NULL) {
		node->depth = depth;
		node->code = 0;
		for (i = 0; i < parent->depth; i++)
			node->bits[i] = parent->bits[i];
		node->bits[i] = R_VALUE;
		node->type = HT_BRANCH;
		node->next = NULL;
	}

	return node;
}
