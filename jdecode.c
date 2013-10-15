#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dct.h"
#include "gbit.h"
#include "huffmantree.h"
#include "jfile.h"
#include "jhuffman.h"
#include "jlib.h"
#include "rlc.h"
#include "jdecode.h"
#include "colorspace.h"
#include "comm.h"

#define BLOCK_SIZE 64
#define MAX_IMAGE_DATA_SIZE (100 * 1024)

static unsigned char image_data[MAX_IMAGE_DATA_SIZE];
static int byte_pos;
static int bit_pos;
static int image_data_size;

static int last_dc[6];

static int R_block[64 * 6];
static int G_block[64 * 6];
static int B_block[64 * 6];
static int nblock;

static struct block_8x8 image_block_8x8;

static int mcu_index;
static int block_8x8_index;

static struct {
	int image_width;
	int image_height;
	int mcu_width;
	int mcu_height;
	int nmcu_h;
	int nmcu_v;
	int y_h_factor;
	int y_v_factor;
	int ncomponent;
	enum component_id cid[6];
	struct {
		int v_factor;
		int h_factor;
	} component[6];
} frame0;

static int init_image_data(void);
static void init_frame0_info(void);

static unsigned char get_code(enum component_id c_id, int type);
static int get_value(unsigned char code);
static int get_dc_value(enum component_id c_id);
static int get_ac_rlc_values(enum component_id c_id, int *dest);

static void to_YCbCr_block(enum component_id c_id,
			int dc_value, int *ac_rlc_values,
			int *block);
static void to_RGB_block(int *Y, int *Cb, int *Cr,
			int *R, int *G, int *B);

static void decode_one_mcu(void);


void jd_print_dct_matrix_dc(int nmcu)
{
	int i, j;
	int dc, ac[64 * 2], dct_matrix[6];
	int buf0[64], buf1[64];
	struct quantization_table *qt;

	for (i = 0; i < 6; i++)
		last_dc[i] = 0;

	bit_pos = 0;
	byte_pos = 0;

	for (i = 0; i < nmcu; i++) {
		qt = jlib_get_qt_c(1);
		for (j = 0; j < 4; j++) {
			dc = get_dc_value(1);
			get_ac_rlc_values(1, ac);
			buf0[0] = dc;
			irlc(ac, &buf0[1], 63);
			iquantization(buf0, buf1, qt->table);
			dct_matrix[j] = buf1[0];
		}

		dc = get_dc_value(2);
		get_ac_rlc_values(2, ac);
		buf0[0] = dc;
		irlc(ac, &buf0[1], 63);
		qt = jlib_get_qt_c(2);
		iquantization(buf0, buf1, qt->table);
		dct_matrix[4] = buf1[0];

		dc = get_dc_value(3);
		get_ac_rlc_values(3, ac);
		buf0[0] = dc;
		irlc(ac, &buf0[1], 63);
		qt = jlib_get_qt_c(3);
		iquantization(buf0, buf1, qt->table);
		dct_matrix[5] = buf1[0];

		printf("MCU=[%2d, %2d] (0, 0) YCbCr = [%d, %d, %d]\n",
				i % frame0.nmcu_h, i / frame0.nmcu_h,
				dct_matrix[0], dct_matrix[4], dct_matrix[5]);
		printf("MCU=[%2d, %2d] (1, 0) YCbCr = [%d, %d, %d]\n",
				i % frame0.nmcu_h, i / frame0.nmcu_h,
				dct_matrix[1], dct_matrix[4], dct_matrix[5]);
		printf("MCU=[%2d, %2d] (0, 1) YCbCr = [%d, %d, %d]\n",
				i % frame0.nmcu_h, i / frame0.nmcu_h,
				dct_matrix[2], dct_matrix[4], dct_matrix[5]);
		printf("MCU=[%2d, %2d] (1, 1) YCbCr = [%d, %d, %d]\n",
				i % frame0.nmcu_h, i / frame0.nmcu_h,
				dct_matrix[3], dct_matrix[4], dct_matrix[5]);
		printf("\n");
	}
}

void jd_print_dct_matrix(int nmcu)
{
	unsigned char code;
	int i, j;
	int dc, ac[64 * 2], dct_matrix[6];
	int buf0[64], buf1[64];
	struct quantization_table *qt;

	for (i = 0; i < 6; i++)
		last_dc[i] = 0;

	bit_pos = 0;
	byte_pos = 0;

	for (i = 0; i < nmcu; i++) {
		qt = jlib_get_qt_c(1);
		for (j = 0; j < 4; j++) {
			code = get_code(1, 0);
			dc =  get_value(code);
			get_ac_rlc_values(1, ac);
			buf0[0] = dc;
			irlc(ac, &buf0[1], 63);
			iquantization(buf0, buf1, qt->table);
			printf("MCU = [%2d, %2d]\n",
				i % frame0.nmcu_h, i / frame0.nmcu_h);
			print_8x8(buf1);
		}

		code = get_code(2, 0);
		dc = get_value(code);
		get_ac_rlc_values(2, ac);
		buf0[0] = dc;
		irlc(ac, &buf0[1], 63);
		qt = jlib_get_qt_c(2);
		iquantization(buf0, buf1, qt->table);
		printf("MCU = [%2d, %2d]\n",
			i % frame0.nmcu_h, i / frame0.nmcu_h);
		print_8x8(buf1);

		code = get_code(3, 0);
		dc = get_value(code);
		get_ac_rlc_values(3, ac);
		buf0[0] = dc;
		irlc(ac, &buf0[1], 63);
		qt = jlib_get_qt_c(3);
		iquantization(buf0, buf1, qt->table);
		printf("MCU = [%2d, %2d]\n",
			i % frame0.nmcu_h, i / frame0.nmcu_h);
		print_8x8(buf1);

		printf("\n");
	}
}


void jd_test(void)
{
	int i;
	int dc, ac[64 * 2];
	int buf[64], buf2[64];
	struct quantization_table *qt;

	for (i = 0; i < 6; i++)
		last_dc[i] = 0;
	bit_pos = 0;
	byte_pos = 0;

	for (i = 0; i < 2; i++) {
		dc = get_dc_value(1);
		get_ac_rlc_values(1, ac);
		buf[0] = dc;
		irlc(ac, &buf[1], 63);
		qt = jlib_get_qt_c(1);
		iquantization(buf, buf2, qt->table);
		print_8x8(qt->table);
		print_8x8(buf2);

		dc = get_dc_value(2);
		get_ac_rlc_values(2, ac);
		buf[0] = dc;
		irlc(ac, &buf[1], 63);
		qt = jlib_get_qt_c(2);
		iquantization(buf, buf2, qt->table);
		print_8x8(qt->table);
		print_8x8(buf2);

		dc = get_dc_value(3);
		get_ac_rlc_values(3, ac);
		buf[0] = dc;
		irlc(ac, &buf[1], 63);
		qt = jlib_get_qt_c(3);
		iquantization(buf, buf2, qt->table);
		print_8x8(qt->table);
		print_8x8(buf2);
	}
}


struct block_8x8 *jd_next_8x8_block(void)
{
	int x_off, y_off;

	if (block_8x8_index >= nblock) {
		mcu_index++;
		block_8x8_index = 0;
		if (mcu_index >= frame0.nmcu_h * frame0.nmcu_v)
			return NULL;
		else
			decode_one_mcu();
	}

	x_off = (block_8x8_index % frame0.y_h_factor) * 8;
	y_off = (block_8x8_index / frame0.y_h_factor) * 8;
	image_block_8x8.x0 = (mcu_index % frame0.nmcu_h) * frame0.mcu_width + x_off;
	image_block_8x8.y0 = (mcu_index / frame0.nmcu_h) * frame0.mcu_width + y_off;
	image_block_8x8.x0 = (mcu_index % frame0.nmcu_h) * frame0.mcu_width + x_off;
	image_block_8x8.y0 = (mcu_index / frame0.nmcu_h) * frame0.mcu_height + y_off;
	image_block_8x8.r = &R_block[block_8x8_index * 64];
	image_block_8x8.g = &G_block[block_8x8_index * 64];
	image_block_8x8.b = &B_block[block_8x8_index * 64];

	block_8x8_index++;

	return &image_block_8x8;
}

struct block_8x8 *jd_first_8x8_block(void)
{
	int i;

	for (i = 0; i < 6; i++)
		last_dc[i] = 0;
	byte_pos = 0;
	bit_pos = 0;

	mcu_index = 0;
	block_8x8_index = 0;
	decode_one_mcu();

	image_block_8x8.x0 = 0;
	image_block_8x8.y0 = 0;
	image_block_8x8.r = &R_block[0];
	image_block_8x8.g = &G_block[0];
	image_block_8x8.b = &B_block[0];

	block_8x8_index = 1;

	return &image_block_8x8;
}

int jd_image_data_size(void)
{
	return image_data_size;
}

void jd_get_image_data(unsigned char *dest)
{
	memcpy(dest, image_data, image_data_size);
}

int jd_init(char *fname)
{
	jf_init(fname);
	jh_init();
	jlib_init();

	if (init_image_data() != 0) {
		printf("We can't decode this JPEG file !\n");
		exit(1);
	}

	init_frame0_info();

	return 0;
}

void jd_exit(void)
{
	jf_exit();
}

static int init_image_data(void)
{
	int data_size = 0;
	int i, index = 0;
	unsigned char data[MAX_IMAGE_DATA_SIZE];

	data_size = jf_imagedata_size();
	jf_get_imagedata(data);

	for (i = 0; i < data_size; i++) {
		if (data[i] != 0xff) {
			image_data[index++] = data[i];
		} else {
			if (data[i + 1] == 0x0)
				image_data[index++] = data[i++];
			else
				return -1;
		}
	}

	image_data_size = index;

	return 0;
}

static void init_frame0_info(void)
{
	struct jframe frame;
	int i = 0, j, n, tmp, id;

	jlib_get_frame0_info(&frame);

	frame0.image_width = frame.width;
	frame0.image_height = frame.height;

	frame0.ncomponent = frame.num_of_component;
	for (i = 0; i < frame.num_of_component; i++) {
		id = frame.component[i].id;
		frame0.component[id].v_factor = frame.component[i].v_factor;
		frame0.component[id].h_factor = frame.component[i].h_factor;
		frame0.cid[i] = id;
	}

	n = frame0.ncomponent - 1;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n - i; j++) {
			if (frame0.cid[j] > frame0.cid[j + 1]) {
				tmp = frame0.cid[j];
				frame0.cid[j] = frame0.cid[j + 1];
				frame0.cid[j + 1] = tmp;
			}
		}
	}

	frame0.y_h_factor = frame0.component[frame0.cid[0]].h_factor;
	frame0.y_v_factor = frame0.component[frame0.cid[0]].v_factor;

	frame0.mcu_width = frame0.component[frame0.cid[0]].h_factor * 8;
	frame0.mcu_height = frame0.component[frame0.cid[0]].v_factor * 8;

	frame0.nmcu_h = frame0.image_width / frame0.mcu_width;
	if ((frame0.image_width % frame0.mcu_width) > 0)
		frame0.nmcu_h = frame0.nmcu_h + 1;
	frame0.nmcu_v = frame0.image_height / frame0.mcu_height;
	if ((frame0.image_height % frame0.mcu_height) > 0)
		frame0.nmcu_v = frame0.nmcu_v + 1;
}

static unsigned char get_code(enum component_id c_id, int type)
{
	struct jh_table *pht;
	struct jh_symbol *ps;
	int i, flag;

	pht = jlib_get_ht(c_id, type);
	ps = pht->symbol;

	for (i = 0; i < pht->num_of_symbol; i++) {
		flag = gb_cmp(&image_data[byte_pos], bit_pos,
				ps[i].bits, 0, ps[i].length);
		if (flag == 0) {
			bit_pos = bit_pos + (int)(ps[i].length);
			byte_pos = byte_pos + bit_pos / 8;
			bit_pos = bit_pos % 8;
			return ps[i].code;
		}
	}

	if (i >= pht->num_of_symbol) {
		printf("get_code() failed!\n");
		exit(1);
	}

	return 0;
}

static int get_value(unsigned char code)
{
	int length = (int)(code & 0x0f);
	int value = 0;

	value = jh_bits_to_value(&image_data[byte_pos], bit_pos, length);
	bit_pos = bit_pos + length;
	byte_pos = byte_pos + bit_pos / 8;
	bit_pos = bit_pos % 8;

	return value;
}

static int get_dc_value(enum component_id c_id)
{
	unsigned char code;
	int dc_value = last_dc[c_id];

	code = get_code(c_id, 0);

	dc_value = dc_value + get_value(code);
	last_dc[c_id] = dc_value;

	return dc_value;
}

static int get_ac_rlc_values(enum component_id c_id, int *dest)
{
	unsigned char code;
	int index = 0;
	int n = 0;

	while (1) {
		code = get_code(c_id, 1);
		if (code == 0x00) {
			dest[index++] = 0;
			dest[index++] = 0;
			break;
		}

		dest[index++] = (int)(code >> 4);
		n = n + (int)(code >> 4);
		dest[index++] = get_value(code);
		n = n + 1;

		if (n >= 63)
			break;
	}

	return index;
}

static void to_YCbCr_block(enum component_id c_id,
			int dc_value, int *ac_rlc_values,
			int *block)
{
	int tmp1[64], tmp2[64];
	struct quantization_table *qt;

	memset(tmp1, 0, 64 * sizeof(int));
	memset(tmp2, 0, 64 * sizeof(int));

	tmp1[0] = dc_value;
	irlc(ac_rlc_values, &tmp1[1], 63);

	izigzag(tmp1, tmp2);

	qt = jlib_get_qt_c(c_id);
	iquantization(tmp2, tmp1, qt->table);

	idct(tmp1, tmp2);

	memcpy(block, tmp2, 64 * sizeof(int));
}

static void to_RGB_block(int *Y, int *Cb, int *Cr,
			int *R, int *G, int *B)
{
	int i = 0;

	for (i = 0; i < 64; i++) {
		R[i] = jc_get_R(Y[i], Cb[i], Cr[i]);
		G[i] = jc_get_G(Y[i], Cb[i], Cr[i]);
		B[i] = jc_get_B(Y[i], Cb[i], Cr[i]);
	}
}

static void decode_one_mcu(void)
{
	int i, j, n;
	enum component_id c_id;
	int dc, ac_rlc[64 * 2];
	int YCbCr[64 * 8], index = 0;
	int nY, nCb, nCr;

	for (i = 0; i < frame0.ncomponent; i++) {
		c_id = frame0.cid[i];
		n = frame0.component[c_id].v_factor
			* frame0.component[c_id].h_factor;
		for (j = 0; j < n; j++) {
			dc = get_dc_value(c_id);
			get_ac_rlc_values(c_id, ac_rlc);
			to_YCbCr_block(c_id, dc, ac_rlc, YCbCr + 64 * index);
			index++;
		}
	}

	/* for 3 components: Y, Cb, Cr */
	nY = frame0.component[frame0.cid[0]].v_factor
		* frame0.component[frame0.cid[0]].h_factor;
	nCb = frame0.component[frame0.cid[1]].v_factor
		* frame0.component[frame0.cid[1]].h_factor;
	nCr = frame0.component[frame0.cid[2]].v_factor
		* frame0.component[frame0.cid[2]].h_factor;
	nblock = nY;
	for (i = 0; i < nblock; i++) {
		to_RGB_block(YCbCr + i * 64,    /* Y */
			YCbCr + (nY + (i / (nY / nCb))) * 64,  /* Cb */
			YCbCr + (nY + nCb + (i / (nY / nCr))) * 64,  /* Cr */
			&R_block[i * 64],  /* R */
			&G_block[i * 64],  /* G */
			&B_block[i * 64]);  /* B */
	}
}
