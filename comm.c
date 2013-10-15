#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "colorspace.h"
#include "dct.h"
#include "gbit.h"
#include "huffmantree.h"
#include "jdecode.h"
#include "jfile.h"
#include "jhuffman.h"
#include "jlib.h"
#include "rlc.h"
#include "comm.h"

void print_hts(void)
{
	int n, i, j, k, count;
	struct jh_table *ht;

	n = jh_table_count();
	for (i = 0; i < n; i++) {
		ht = jh_get_table_from_index(i);

		printf("no = %d ", ht->no);
		if (ht->no == 0)
			printf("(Y)   ,  ");
		if (ht->no == 1)
			printf("(CbCr),  ");
		printf("type = %d ", ht->type);
		if (ht->type == 0)
			printf("(DC),  ");
		if (ht->type == 1)
			printf("(AC),  ");

		printf("Total number of codes: %d\n", ht->num_of_symbol);

		for (j = 1; j <= 16; j++) {
			printf("%2d bits: ", j);
			count = 0;
			for (k = 0; k < ht->num_of_symbol; k++) {
				if (ht->symbol[k].length == j)
					count++;
			}
			printf("(%3d ): ", count);

			for (k = 0; k < ht->num_of_symbol; k++) {
				if (ht->symbol[k].length == j)
					printf("%2x ", ht->symbol[k].code);
			}
			printf("\n");
		}
		printf("\n");
	}
}

void print_all_marker_addr(void)
{
	struct jf_marker marker;
	int flag = 0;

	printf("There are %d markers.\n", jf_marker_total());
	flag = jf_marker_first(&marker);
	while (flag == 0) {
		switch (marker.id) {
		case 0xd8:
			printf("SOI, %2x, %8x\n", marker.id, marker.pos);
			break;
		case 0xe0:
			printf("APP0, %2x, %8x\n", marker.id, marker.pos);
			break;
		case 0xdb:
			printf("DQT, %2x, %8x\n", marker.id, marker.pos);
			break;
		case 0xc0:
			printf("SOF0, %2x, %8x\n", marker.id, marker.pos);
			break;
		case 0xc4:
			printf("DHT, %2x, %8x\n", marker.id, marker.pos);
			break;
		case 0xda:
			printf("SOS, %2x, %8x\n", marker.id, marker.pos);
			break;
		case 0xd9:
			printf("EOI, %2x, %8x\n", marker.id, marker.pos);
			break;
		default:
			printf("%2x, %8x\n", marker.id, marker.pos);
		}
		flag = jf_marker_next(&marker);
	}
}

void print_qts(void)
{
	int i, j, k, n;
	struct quantization_table *qt;

	n = jlib_qt_total();
	for (i = 0; i < n; i++) {
		qt = jlib_get_qt_from_index(i);
		printf("no = %d ", qt->no);
		if (qt->no == 0)
			printf("(Luminance)\n");
		else
			printf("(Chrominance)\n");
		printf("precision = %d bits\n", (qt->precision + 1) * 8);
		print_8x8(qt->table);
	}
}


void print_8x8(int *block)
{
	int i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			printf("%d, ", block[i * 8 + j]);
		}
		printf("\n");
	}
	printf("\n");
}
