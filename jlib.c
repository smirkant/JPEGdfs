#include <string.h>
#include <stdlib.h>
#include "jfile.h"
#include "jlib.h"
#include "jhuffman.h"

static int zigzag_table[] = {
        0,  1,  5,  6, 14, 15, 27, 28,
        2,  4,  7, 13, 16, 26, 29, 42,
        3,  8, 12, 17, 25, 30, 41, 43,
        9, 11, 18, 24, 31, 40, 44, 53,
        10,19, 23, 32, 39, 45, 52, 54,
        20,22, 33, 38, 46, 51, 55, 60,
        21,34, 37, 47, 50, 56, 59, 61,
        35,36, 48, 49, 57, 58, 62, 63};

static struct jframe frame_sof0;
static struct jscan sos_info;
static struct quantization_table q_table[4];
static int num_of_qt;

static struct {
	int v_factor;
	int h_factor;

	int qt_no;
	struct quantization_table *qt;

	int AC_ht_no;
	struct jh_table *AC_ht;

	int DC_ht_no;
	struct jh_table *DC_ht;
} component_info[6];

static void init_component_info(void);


int jlib_get_v_factor_c(enum component_id c_id)
{
	return component_info[c_id].v_factor;
}

int jlib_get_h_factor_c(enum component_id c_id)
{
	return component_info[c_id].h_factor;
}

struct quantization_table *jlib_get_qt_c(enum component_id c_id)
{
	return component_info[c_id].qt;
}

struct jh_table *jlib_get_ac_ht_c(enum component_id c_id)
{
	return component_info[c_id].AC_ht;
}

struct jh_table *jlib_get_dc_ht_c(enum component_id c_id)
{
	return component_info[c_id].DC_ht;
}

int jlib_qt_total(void)
{
	return num_of_qt;
}

struct jh_table *jlib_get_ht(enum component_id c_id, int type)
{
	int no = 0;

	if (c_id == COMPONENT_Y)
		no = 0;
	else
		no = 1;

	return jh_get_table(type, no);
}

struct quantization_table *jlib_get_qt_from_index(int index)
{
	if ((index >= 0) && (index < num_of_qt))
		return &q_table[index];
	else
		return NULL;
}

struct quantization_table *jlib_get_qt(int no)
{
	int i;

	for (i = 0; i < num_of_qt; i++) {
		if (q_table[i].no == no)
			return &q_table[i];
	}

	return NULL;
}

void jlib_get_sos_info(struct jscan *info)
{
	memcpy(info, &sos_info, sizeof(struct jscan));
}


int jlib_frame0_height(void)
{
	return frame_sof0.height;
}

int jlib_frame0_width(void)
{
	return frame_sof0.width;
}

int jlib_frame0_component_total(void)
{
	return frame_sof0.num_of_component;
}

void jlib_get_frame0_info(struct jframe *frame0)
{
	memcpy(frame0, &frame_sof0, sizeof(struct jframe));
}

void jlib_init(void)
{
	int pos, i, j, k, num;
	int index;
	struct jf_SOF0 sof0;
	struct jf_SOS sos;
	struct jf_DQT dqt;

	/* frame0 info */
	pos = jf_marker_pos(M_SOF0, 0);
	jf_get_sof0(&sof0, pos);

	frame_sof0.bpp = sof0.bpp;
	frame_sof0.height = sof0.image_h;
	frame_sof0.width = sof0.image_w;
	frame_sof0.num_of_component = sof0.num_of_component;
	for (i = 0; i < frame_sof0.num_of_component; i++) {
		frame_sof0.component[i].id = sof0.component[i].id;
		frame_sof0.component[i].v_factor = sof0.component[i].v_factor;
		frame_sof0.component[i].h_factor = sof0.component[i].h_factor;
		frame_sof0.component[i].qt_no = sof0.component[i].qt_no;
	}


	/* start os scan */
	pos = jf_marker_pos(M_SOS, 0);
	jf_get_sos(&sos, pos);

	sos_info.num_of_component = sos.num_of_color_component;
	for (i = 0; i < sos_info.num_of_component; i++) {
		sos_info.component[i].id = sos.component[i].id;
		sos_info.component[i].AC_ht_no = sos.component[i].ac_table;
		sos_info.component[i].DC_ht_no = sos.component[i].dc_table;
	}


	/* quantization table */
	num_of_qt = 0;
	index = 0;
	num = jf_marker_count(M_DQT);
	for (i = 0; i < num; i++) {
		pos = jf_marker_pos(M_DQT, i);
		jf_get_dqt(&dqt, pos);

		num_of_qt = num_of_qt + dqt.num_of_qt;

		for (j = 0; j < dqt.num_of_qt; j++) {
			q_table[index].no = dqt.qt[j].no;
			q_table[index].precision = dqt.qt[j].precision;
			for (k = 0; k < 64; k++) {
				q_table[index].table[k] = (int)dqt.qt[j].table[zigzag_table[k]];
			}
			index = index + 1;
		}
	}
	if (index != num_of_qt) {
		printf("Extract quantization table failed!\n");
		exit(1);
	}

	init_component_info();
}

static void init_component_info(void)
{
	int i = 0, j = 0;
	int id = 1;

	for (i = 0; i < frame_sof0.num_of_component; i++) {
		id = frame_sof0.component[i].id;
		component_info[id].v_factor = frame_sof0.component[i].v_factor;
		component_info[id].h_factor = frame_sof0.component[i].h_factor;
		component_info[id].qt_no = frame_sof0.component[i].qt_no;
		for (j = 0; j < num_of_qt; j++) {
			if (q_table[j].no == component_info[id].qt_no) {
				component_info[id].qt = &q_table[j];
				break;
			}
		}
		if (j >= num_of_qt) {
			printf("Initialize Quantization Table failed !\n");
			exit(1);
		}
	}

	for (i = 0; i < sos_info.num_of_component; i++) {
		id = sos_info.component[i].id;
		component_info[id].AC_ht_no = sos_info.component[i].AC_ht_no;
		component_info[id].DC_ht_no = sos_info.component[i].DC_ht_no;
		component_info[id].AC_ht = jh_get_table(1, component_info[id].AC_ht_no);
		component_info[id].DC_ht = jh_get_table(0, component_info[id].DC_ht_no);
	}
}
