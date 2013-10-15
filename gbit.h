#ifndef _GBIT_H
#define _GBIT_H

enum gbit {
	GBIT_0 = 0,
	GBIT_1 = 1
};

extern enum gbit gb_get(unsigned char *bytes, int index);
extern int gb_get_int(unsigned char *bytes, int index, int n);

extern int gb_set_1(unsigned char *bytes, int index);
extern int gb_set_0(unsigned char *bytes, int index);

extern int gb_cmp(unsigned char *bytes0, int index0,
		unsigned char *bytes1, int index1,
		int n);

#endif
