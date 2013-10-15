#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jfile.h"

#define MAX_NUM_OF_MARKER  20
#define MAX_SEGMENT_LEN   65536

static struct jf_marker marker[MAX_NUM_OF_MARKER];
static int marker_count;
static int marker_index;

#define IMAGEDATA_SIZE    (100 * 1024)

static unsigned char imagedata[IMAGEDATA_SIZE];
static int imagedata_size;
static int imagedata_pos;

static FILE *fp;

static struct {
	unsigned char ff;
	unsigned char id;
	unsigned char len[2];
	unsigned char iden[5];
	unsigned char v[2];
	unsigned char unit;
	unsigned char xd[2];
	unsigned char yd[2];
	unsigned char thp;
	unsigned char tvp;
	unsigned char tn[MAX_SEGMENT_LEN];
} meta_app0;

static struct {
	unsigned char ff;
	unsigned char id;
	unsigned char len[2];
	unsigned char t[MAX_SEGMENT_LEN];
} meta_dqt;

static struct {
	unsigned char ff;
	unsigned char id;
	unsigned char len[2];
	unsigned char p;
	unsigned char h[2];
	unsigned char w[2];
	unsigned char num;
	unsigned char component[15];
} meta_sof0;

static struct {
	unsigned char ff;
	unsigned char id;
	unsigned char len[2];
	unsigned char data[MAX_SEGMENT_LEN];
} meta_dht;

static struct {
	unsigned char ff;
	unsigned char id;
	unsigned char len[2];
	unsigned char num;
	unsigned char component[8];
} meta_sos;


extern int jf_imagedata_size(void)
{
	return imagedata_size;
}

extern int jf_get_imagedata(unsigned char *dest)
{
	memcpy (dest, imagedata, imagedata_size);
	return imagedata_size;
}

int jf_get_app0(struct jf_APP0 *p, unsigned int pos)
{
	fseek(fp, pos, SEEK_SET);
	fread(&meta_app0, 1, sizeof(meta_app0), fp);

	p->id = meta_app0.id;
	p->length = meta_app0.len[0];
	p->length = (p->length << 8) + meta_app0.len[1];
	memcpy(p->identifier, meta_app0.iden, 5);
	p->major_version = meta_app0.v[0];
	p->minor_version = meta_app0.v[1];
	p->xy_density_unit = meta_app0.unit;
	p->x_density = meta_app0.xd[0];
	p->x_density = (p->x_density << 8) + meta_app0.xd[1];
	p->y_density = meta_app0.yd[0];
	p->y_density = (p->y_density << 8) + meta_app0.yd[1];
	p->th_pixel = meta_app0.thp;
	p->tv_pixel = meta_app0.tvp;
	memcpy(p->thumbnail, meta_app0.tn, p->length - 16);

	return 0;
}

int jf_get_sof0(struct jf_SOF0 *p, unsigned int pos)
{
	int i = 0;

	fseek(fp, pos, SEEK_SET);
	fread(&meta_sof0, 1, sizeof(meta_sof0), fp);

	p->id = meta_sof0.id;
	p->length = meta_sof0.len[0];
	p->length = (p->length << 8) + meta_sof0.len[1];
	p->bpp = meta_sof0.p;
	p->image_h = meta_sof0.h[0];
	p->image_h = (p->image_h << 8) + meta_sof0.h[1];
	p->image_w = meta_sof0.w[0];
	p->image_w = (p->image_w << 8) + meta_sof0.w[1];
	p->num_of_component = meta_sof0.num;
	for (i = 0; i < meta_sof0.num; i++) {
		p->component[i].id = meta_sof0.component[i * 3 + 0];
		p->component[i].v_factor = meta_sof0.component[i * 3 + 1];
		p->component[i].v_factor = p->component[i].v_factor & 0xf;
		p->component[i].h_factor = meta_sof0.component[i * 3 + 1];
		p->component[i].h_factor = p->component[i].h_factor >> 4;
		p->component[i].qt_no = meta_sof0.component[i * 3 + 2];
	}

	return 0;
}

int jf_get_sos(struct jf_SOS *p, unsigned int pos)
{
	int i = 0;

	fseek(fp, pos, SEEK_SET);
	fread(&meta_sos, 1, sizeof(meta_sos), fp);

	p->id = meta_sos.id;
	p->length = meta_sos.len[0];
	p->length = (p->length << 8) + meta_sos.len[1];
	p->num_of_color_component = meta_sos.num;
	for (i = 0; i < p->num_of_color_component; i++) {
		p->component[i].id = meta_sos.component[i * 2 + 0];
		p->component[i].ac_table = meta_sos.component[i * 2 + 1];
		p->component[i].ac_table = p->component[i].ac_table & 0xf;
		p->component[i].dc_table = meta_sos.component[i * 2 + 1];
		p->component[i].dc_table = p->component[i].dc_table >> 4;
	}

	return 0;
}

int jf_get_dqt(struct jf_DQT *p, unsigned int pos)
{
	int i = 0, n = 0;

	fseek(fp, pos, SEEK_SET);
	fread(&meta_dqt, 1, sizeof(meta_dqt), fp);

	p->id = meta_dqt.id;
	p->length = meta_dqt.len[0];
	p->length = (p->length << 8) + meta_dqt.len[1];
	for (i = 0; i < 4; i++) {
		p->qt[i].no = meta_dqt.t[n];
		p->qt[i].no = p->qt[i].no & 0xf;
		p->qt[i].precision = meta_dqt.t[n];
		p->qt[i].precision = p->qt[i].precision >> 4;
		n = n + 1;
		if (p->qt[i].precision == 0) {
			memcpy(p->qt[i].table, &meta_dqt.t[n], 64);
			n = n + 64;
		} else {
			memcpy(p->qt[i].table, &meta_dqt.t[n], 128);
			n = n + 128;
		}
		if (n == p->length - 2)
			break;
	}
	p->num_of_qt = i + 1;

	return 0;
}

int jf_get_dht(struct jf_DHT *p, unsigned int pos)
{
	unsigned int i = 0, j = 0, n = 0;
	unsigned int count = 0;

	fseek(fp, pos, SEEK_SET);
	fread(&meta_dht, 1, sizeof(meta_dht), fp);

	p->id = meta_dht.id;
	p->length = meta_dht.len[0];
	p->length = (p->length << 8) + meta_dht.len[1];
	for (i = 0; i < 4; i++) {
		p->ht[i].no = meta_dht.data[n] & 0xf;
		p->ht[i].type = (meta_dht.data[n] >> 4) & 0x1;
		n = n + 1;
		memcpy(p->ht[i].table, &meta_dht.data[n], 16);
		n = n + 16;
		count = 0;
		for (j = 0; j < 16; j++) {
			count = count + p->ht[i].table[j];
		}
		memcpy(p->ht[i].symbol, &meta_dht.data[n], count);
		n = n + count;
		if (n + 2 == p->length) {
			break;
		} else if (n +2 > p->length) {
			return -1;
		}
	}
	p->num_of_ht = i + 1;

	return 0;
}

int is_jfif_file(void)
{
	unsigned char buf[10];
	unsigned char identifier[5] = {'J','F','I','F',0};
	int i = 0;

	fseek(fp, 2, SEEK_SET);
	fread(buf, 1, 10, fp);

	if ((buf[0] != 0xff) && (buf[1] != 0xe0))
		return 0;
	while (1) {
		if (buf[4 + i] != identifier[i])
			return 0;
		i = i + 1;
		if (i > 4)
			break;
	}

	return 1;
}

int jf_marker_first(struct jf_marker *p)
{
	p->pos = marker[0].pos;
	p->id = marker[0].id;
	marker_index = 0;
	return 0;
}

int jf_marker_next(struct jf_marker *p)
{
	marker_index = marker_index + 1;
	if (marker_index < marker_count) {
		p->pos = marker[marker_index].pos;
		p->id = marker[marker_index].id;
		return 0;
	} else {
		return -1;
	}
}

int jf_marker_total(void)
{
	return marker_count;
}

int jf_marker_count(unsigned char marker_id)
{
	int count = 0, i = 0;

	for (i = 0; i < marker_count; i++) {
		if (marker[i].id == marker_id)
			count = count + 1;
	}

	return count;
}

unsigned int jf_marker_pos(unsigned char marker_id, int index)
{
	int i = 0, n = -1;

	for (i = 0; i < marker_count; i++) {
		if (marker[i].id == marker_id) {
			n = n + 1;
			if (n == index)
				break;
		}
	}

	return marker[i].pos;
}

unsigned int jf_segment_length(unsigned char marker_id, int index)
{
	int i, n = -1;
	unsigned int length = 0;
	unsigned char buf[4];

	for (i = 0; i < marker_count; i++) {
		if (marker[i].id == marker_id) {
			n = n + 1;
			if (n == index)
				break;
		}
	}

	fseek(fp, marker[i].pos, SEEK_SET);
	fread(buf, 1, 4, fp);

	length = buf[2];
	length = (length << 8) + buf[3];

	return length;
}

unsigned int jf_scan_data_offset(void)
{
	unsigned int length = 0;
	int i;

	length = meta_sos.len[0];
	length = (length << 8) + meta_sos.len[1];

	for (i = 0; i < marker_count; i++)
		if (marker[i].id == M_SOS)
			break;

	return (marker[i].pos + 2 + length);
}

static int find_all_marker(void)
{
	unsigned int pos = 0, i = 0, size;
	unsigned char buf[4];

	fseek(fp, 0, SEEK_SET);
	marker[0].pos = 0;
	fread(buf, 1, 2, fp);
	pos = pos + 2;
	if ((buf[0] == 0xff) && (buf[1] == 0xd8))
		marker[0].id = 0xd8;
	else
		return -1;

	while (1) {
		fread(buf, 1, 4, fp);

		i = i + 1;
		marker[i].pos = pos;
		pos = pos + 4;
		if (buf[0] != 0xff)
			return -1;
		marker[i].id = buf[1];
		if (buf[1] == 0xda)
			break;

		size = buf[2];
		size = (size << 8) + buf[3];
		fseek(fp, size - 2, SEEK_CUR);
		pos = pos + size - 2;
	}

	fseek(fp, -2, SEEK_END);
	pos = ftell(fp);
	fread(buf, 1, 2, fp);
	if ((buf[0] == 0xff) && (buf[1] == 0xd9)) {
		i = i + 1;
		marker[i].pos = pos;
		marker[i].id = 0xd9;
	} else {
		return -1;
	}

	marker_count = i + 1;

	return 0;
}

int jf_init(char *fname)
{
	int sos_pos, sos_size;
	int eoi_pos;
	struct jf_SOS sos;

	fp = fopen(fname, "rb");
	if (fp == NULL) {
		printf("open file error!\n");
		return -1;
	}

	if (! is_jfif_file()) {
		printf("%s is NOT a JFIF file\n", fname);
		return -1;
	}

	if (find_all_marker() != 0) {
		printf("find all marker failed!\n");
		return -1;
	}

	sos_pos = jf_marker_pos(M_SOS, 0);
	jf_get_sos(&sos, sos_pos);
	sos_size = sos.length;
	eoi_pos = jf_marker_pos(M_EOI, 0);

	imagedata_size =  eoi_pos - sos_pos - sos_size - 2;
	imagedata_pos = sos_pos + 2 + sos_size;
	fseek(fp, imagedata_pos, SEEK_SET);
	imagedata_size = fread(imagedata, 1, imagedata_size, fp);

	return 0;
}

void jf_exit(void)
{
	fclose(fp);
}
