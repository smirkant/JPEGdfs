#include <stdio.h>
#include <string.h>
#include "gbit.h"

int gb_get_int(unsigned char *bytes, int index, int n)
{
	int i = 0;
	unsigned int value = 0;
	enum gbit ggbit;

	for (i = 0; i < n; i++) {
		ggbit = gb_get(bytes, index + i);
		value = (value << 1) + ggbit;
	}

	return (int)value;
}

enum gbit gb_get(unsigned char *bytes, int index)
{
	int i = 0, j = 0;
	char ch = 0;

	i = index / 8;
	j = 7 - (index % 8);
	ch = *(bytes + i);
	ch = (ch >> j) & 0x1;
	if (ch)
		return GBIT_1;
	else
		return GBIT_0;
}

int gb_set_1(unsigned char *bytes, int index)
{
	int i = 0, j = 0;
	char ch = 0;

	i = index / 8;
	j = 7 - (index % 8);
	ch = *(bytes + i);
	ch = ch | (1 << j);
	*(bytes + i) = ch;

	return 0;
}

int gb_set_0(unsigned char *bytes, int index)
{
	int i = 0, j = 0;
	char ch = 0;

	i = index / 8;
	j = 7 - (index % 8);
	ch = *(bytes + i);
	ch = ch & (~(1 << j));
	*(bytes + i) = ch;

	return 0;
}

int gb_cmp(unsigned char *bytes0, int index0,
		unsigned char *bytes1, int index1,
		int n)
{
	int i = 0;
	enum gbit bit0, bit1;

	for (i = 0; i < n; i++) {
		bit0 = gb_get(bytes0, index0 + i);
		bit1 = gb_get(bytes1, index1 + i);
		if (bit0 != bit1)
			break;
	}
	if (i == n)
		return 0;
	else
		return -1;
}
