#include <math.h>
#include "dct.h"

static int zigzag_table[] = {
        0,  1,  5,  6, 14, 15, 27, 28,
        2,  4,  7, 13, 16, 26, 29, 42,
        3,  8, 12, 17, 25, 30, 41, 43,
        9, 11, 18, 24, 31, 40, 44, 53,
        10,19, 23, 32, 39, 45, 52, 54,
        20,22, 33, 38, 46, 51, 55, 60,
        21,34, 37, 47, 50, 56, 59, 61,
        35,36, 48, 49, 57, 58, 62, 63};

static int dct_round(double value);

extern void fquantization(int *src, int *dest, int *qt_table)
{
	int i;
	int *psrc = src, *pdest = dest;

	for (i = 0; i < 64; i++) {
		*pdest++ = (int)(*psrc++ / qt_table[i] + 0.5);
	}
}

extern void iquantization(int *src, int *dest, int *qt_table)
{
	int i;
	int *psrc = src, *pdest = dest;

	for (i = 0; i < 64; i++) {
		*pdest++ = (int)(*psrc++ * qt_table[i]);
	}
}

extern void fzigzag(int *src, int *dest)
{
	int i;
	int *psrc = src, *pdest = dest;

	for (i = 0; i < 64; i++) {
		*(pdest + zigzag_table[i]) = *psrc++;
	}
}

extern void izigzag(int *src, int *dest)
{
	int i;
	int *psrc = src, *pdest = dest;

	for (i = 0; i < 64; i++) {
		*pdest++ = *(psrc + zigzag_table[i]);
	}
}

extern void fdct(int *src, int *dest)
{
	int u, v, x, y, i, j;
	double alpha_u, alpha_v;
	double tmp, tmp1, tmp2;
	int *psrc, *pdest;

	pdest = dest;
	for (i = 0; i < 64; i++) {
		u = i % 8;
		v = i / 8;
		if (u == 0)
			alpha_u = sqrt(1.0 / 8.0);
		else
			alpha_u = sqrt(2.0 / 8.0);
		if (v == 0)
			alpha_v = sqrt(1.0 / 8.0);
		else
			alpha_v = sqrt(2.0 / 8.0);

		tmp = 0.0;
		psrc = src;
		for (j = 0; j < 64; j++) {
			x = j % 8;
			y = j / 8;
			tmp1 = ((2.0 * x + 1.0) * u * PI) / 16.0;
			tmp2 = ((2.0 * y + 1.0) * v * PI) / 16.0;
			tmp = tmp + (*psrc++) * cos(tmp1) * cos(tmp2);
		}

		*pdest++ = dct_round(alpha_u * alpha_v * tmp);
	}
}

extern void idct(int *src, int *dest)
{
	int u, v, x, y, i, j;
	double alpha_u, alpha_v;
	double tmp, tmp1, tmp2;
	int *psrc, *pdest;

	pdest = dest;
	for (i = 0; i < 64; i++) {
		x = i % 8;
		y = i / 8;

		tmp = 0.0;
		psrc = src;
		for (j = 0; j < 64; j++) {
			u = j % 8;
			v = j / 8;
			if (u == 0)
				alpha_u = sqrt(1.0 /8.0);
			else
				alpha_u = sqrt(2.0 / 8.0);
			if (v == 0)
				alpha_v = sqrt(1.0 / 8.0);
			else
				alpha_v = sqrt(2.0 / 8.0);

			tmp1 = ((2.0 * x + 1.0) * u * PI) / 16.0;
			tmp2 = ((2.0 * y + 1.0) * v * PI) / 16.0;
			tmp = tmp + alpha_u * alpha_v * (*psrc++) * cos(tmp1) * cos(tmp2);
		}

		*pdest++ = dct_round(tmp);
	}
}

static int dct_round(double value)
{
	if (value >= 0.0)
		return (int)(value + 0.5);
	else
		return (int)(value - 0.5);
}
