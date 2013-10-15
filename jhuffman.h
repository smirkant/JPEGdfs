#ifndef _JHUFFMAN_H
#define _JHUFFMAN_H

struct jh_symbol {
	unsigned int length;
	unsigned char bits[2];
	unsigned char code;
};

struct jh_table {
	int no;
	int type;
	int num_of_symbol;
	struct jh_symbol symbol[256];
};

extern void jh_init(void);
extern int jh_table_count(void);
extern struct jh_table *jh_get_table_from_index(int index);
/* no: 0 = Y, 1 = Cb & Cr; type: 0 = DC, 1 = AC */
extern struct jh_table *jh_get_table(int type, int no);


extern int jh_bits_to_value(unsigned char *bytes, int index, int n);

#endif
