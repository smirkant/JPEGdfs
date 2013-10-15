#ifndef JDECODE_H
#define JDECODE_H

struct block_8x8 {
	int x0;
	int y0;
	int *r;
	int *g;
	int *b;
};

extern int jd_init(char *fname);
extern void jd_exit(void);

extern int jd_image_data_size(void);
extern void jd_get_image_data(unsigned char *dest);

extern struct block_8x8 *jd_first_8x8_block(void);
extern struct block_8x8 *jd_next_8x8_block(void);

extern void jd_test(void);
extern void jd_print_dct_matirx_dc(int nmcu);
extern void jd_print_dct_matirx(int nmcu);

#endif
