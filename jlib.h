#ifndef JLIB_H
#define JLIB_H

enum component_id {
	COMPONENT_Y = 1,
	COMPONENT_Cb = 2,
	COMPONENT_Cr = 3,
	COMPONENT_I = 4,
	COMPONENT_Q = 5
};

struct jframe {
	int bpp; /* bits per pixel per color component */
	int height;
	int width;
	int num_of_component;
	struct {
		enum component_id id;
		int v_factor;
		int h_factor;
		int qt_no;
	} component[4];
};

struct jscan {
	int num_of_component;
	struct {
		enum component_id id;
		int AC_ht_no;
		int DC_ht_no;
	} component[4];
};

struct quantization_table {
	int no;
	int precision;
	int table[128];
};


extern void jlib_init(void);

extern 	void jlib_get_frame0_info(struct jframe *frame0);
extern int jlib_frame0_height(void);
extern int jlib_frame0_width(void);
extern int jlib_frame0_component_total(void);


extern void jlib_get_sos_info(struct jscan *info);


extern int jlib_qt_total(void);
extern struct quantization_table *jlib_get_qt_from_index(int index);
extern struct quantization_table *jlib_get_qt(int no);


/* type: 0 = DC, 1 = AC */
extern struct jh_table *jlib_get_ht(enum component_id c_id, int type);


/* get xxx from component */
extern int jlib_get_v_factor_c(enum component_id c_id);
extern int jlib_get_h_factor_c(enum component_id c_id);
extern struct quantization_table *jlib_get_qt_c(enum component_id c_id);
extern struct jh_table *jlib_get_ac_ht_c(enum component_id c_id);
extern struct jh_table *jlib_get_dc_ht_c(enum component_id c_id);

#endif
