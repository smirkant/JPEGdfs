#include <stdlib.h>
#include <string.h>
#include "jfile.h"
#include "jhuffman.h"
#include "huffmantree.h"
#include "gbit.h"


static struct jh_table table[8];
static int num_of_table;

static int get_symbol(struct jh_symbol *symbol,
		unsigned char *table,
		unsigned char *symtable);

int jh_bits_to_value(unsigned char *bytes, int index, int n)
{
	unsigned int value = 0;
	int ret = 0;
	int i;
	enum gbit ggbit;

	if (gb_get(bytes, index) == GBIT_1) {
		for (i = 0; i < n; i++) {
			ggbit = gb_get(bytes, index + i);
			value = value << 1;
			value = value | ggbit;
		}
		ret = (int)value;
	} else {
		for (i = 0; i < n; i++) {
			ggbit = gb_get(bytes, index + i);
			value = value << 1;
			value = value | (! ggbit);
		}
		ret = 0 - (int)value;
	}

	return ret;
}

struct jh_table *jh_get_table_from_index(int index)
{
	if ((index >= 0) && (index < num_of_table))
		return &table[index];
	else
		return NULL;
}

struct jh_table *jh_get_table(int type, int no)
{
	int i;

	for (i = 0; i < num_of_table; i++) {
		if ((table[i].type == type) && (table[i].no == no))
			return &table[i];
	}

	return NULL;
}

void jh_init(void)
{
	struct {
		unsigned int no;
		unsigned int type;
		unsigned char table[16];
		unsigned char symbol[256];
	} ht[8];
	struct jf_DHT dht;
	int num_of_dht;
	int i = 0, num = 0, j;
	unsigned int pos = 0;
	int symbol_count;

	num_of_dht = jf_marker_count(M_DHT);
	num_of_table = 0;

	for (i = 0; i < num_of_dht; i++) {
		pos = jf_marker_pos(M_DHT, i);
		jf_get_dht(&dht, pos);

		num = dht.num_of_ht;
		num_of_table = num_of_table + num;

		for (j = 0; j < num; j++)
			memcpy(&ht[num_of_table - num + j], &dht.ht[j], sizeof(ht[0]));
	}

	for (i = 0; i < num_of_table; i++) {
		table[i].no = ht[i].no;
		table[i].type = ht[i].type;
		table[i].num_of_symbol = 0;
		for (j = 0; j < 16; j++)
			table[i].num_of_symbol += ht[i].table[j];

		symbol_count = get_symbol(table[i].symbol, ht[i].table, ht[i].symbol);
		if (table[i].num_of_symbol != symbol_count) {
			printf("Build binary tree FAILED!\n");
			exit(1);
		}
	}
}

int jh_table_count(void)
{
	return num_of_table;
}

static int get_symbol(struct jh_symbol *symbol,
		unsigned char *table,
		unsigned char *symtable)
{
	int i = 0, j = 0, index = 0;
	int tmp[16];
	struct ht_node *p = NULL;

	ht_chars_to_ints((char *)table, tmp, 16);
	ht_build_tree(tmp);

	for (i = 0; i < 17; i++) {
		p = ht_get_tree_row(i);
		while (p != NULL) {
			if (p->type == HT_BRANCH) {
				p = p->next;
				continue;
			}

			symbol[index].length = p->depth;
			symbol[index].code = symtable[index];
			for (j = 0; j < p->depth; j++) {
				if (p->bits[j] == GBIT_1)
					gb_set_1(symbol[index].bits, j);
				else
					gb_set_0(symbol[index].bits, j);
			}

			index = index + 1;
			p = p->next;
		}
	}

	ht_destroy_tree();
	return index;
}
