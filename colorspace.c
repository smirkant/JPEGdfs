#include "colorspace.h"


int jc_get_R(int Y, int Cb, int Cr)
{
	double R = 0.0;

	//R = Y + 1.402 * (Cr - 128);
	R = Y + 1.402 * (Cr) + 128;

	return (int)R;
}

int jc_get_G(int Y, int Cb, int Cr)
{
	double G = 0.0;

	//G = Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128);
	G = Y - 0.34414 * (Cb) - 0.71414 * (Cr) + 128;

	return (int)G;
}

int jc_get_B(int Y, int Cb, int Cr)
{
	double B = 0.0;

	//B = Y + 1.772 * (Cb - 128);
	B = Y + 1.772 * (Cb) + 128;

	return (int)B;
}
