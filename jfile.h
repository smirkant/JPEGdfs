#ifndef _JFILE_H_
#define _JFILE_H_

#include <stdio.h>


#define M_SOI    0xd8
#define M_APP0   0xe0
#define M_DQT    0xdb
#define M_SOF0   0xc0
#define M_DHT    0xc4
#define M_SOS    0xda
#define M_EOI    0xd9

struct jf_marker {
	unsigned char id;
	unsigned int pos;
};

struct jf_SOI {
	unsigned char id;
};

struct jf_APP0 {
	unsigned char id;
	unsigned int length;
	unsigned char identifier[5];
	unsigned int major_version;
	unsigned int minor_version;
	unsigned int xy_density_unit;
	unsigned int x_density;
	unsigned int y_density;
	unsigned int th_pixel;
	unsigned int tv_pixel;
	unsigned char thumbnail[65536];
};

struct jf_DQT {
	unsigned char id;
	unsigned int length;
	unsigned int num_of_qt;
	struct {
		unsigned int no;
		unsigned int precision;
		unsigned char table[128];
	} qt[4];
};

struct jf_SOF0 {
	unsigned char id;
	unsigned int length;
	unsigned int bpp; /*bits per pixel per color */
	unsigned int image_h;
	unsigned int image_w;
	unsigned int num_of_component;
	struct {
		unsigned int id;
		unsigned int v_factor;
		unsigned int h_factor;
		unsigned int qt_no;
	} component[4];
};

struct jf_DHT {
	unsigned char id;
	unsigned int length;
	unsigned int num_of_ht;
	struct {
		unsigned int no;
		unsigned int type;
		unsigned char table[16];
		unsigned char symbol[256];
	} ht[4];
};

struct jf_SOS {
	unsigned char id;
	unsigned int length;
	unsigned int num_of_color_component;
	struct {
		unsigned int id;
		unsigned int ac_table;
		unsigned int dc_table;
	} component[4];
};

struct jf_EOI {
};

extern int jf_init(char *fname);
extern void jf_exit(void);

extern int jf_marker_first(struct jf_marker *p);
extern int jf_marker_next(struct jf_marker *p);
extern int jf_marker_total(void);
extern int jf_marker_count(unsigned char marker_id);
extern unsigned int jf_marker_pos(unsigned char marker_id, int index);
extern unsigned int jf_segment_length(unsigned char marker_id, int index);

extern int jf_get_app0(struct jf_APP0 *p, unsigned int pos);
extern int jf_get_sof0(struct jf_SOF0 *p, unsigned int pos);
extern int jf_get_sos(struct jf_SOS *p, unsigned int pos);
extern int jf_get_dqt(struct jf_DQT *p, unsigned int pos);
extern int jf_get_dht(struct jf_DHT *p, unsigned int pos);

extern int jf_imagedata_size(void);
extern int jf_get_imagedata(unsigned char *dest);
extern unsigned int jf_scan_data_offset(void);

#endif
