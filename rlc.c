#include "rlc.h"

void frlc(int *block, int *rlc, int n)
{
	int i, zeroes = 0;
	int index = 0;

	for (i = 0; i < n; i++) {
		if (block[i] == 0) {
			zeroes = zeroes + 1;
			if (zeroes >= 15) {
				rlc[index++] = zeroes;
				rlc[index++] = block[++i];
				zeroes = 0;
			}
		} else {
			rlc[index++] = zeroes;
			rlc[index++] = block[i];
			zeroes = 0;
		}
	}

	for (i = index / 2 - 1; i >= 0; i--) {
		if (rlc[i * 2 + 1] != 0)
			break;
	}
	i = i + 1;
	rlc[i * 2 + 0] = 0;
}

void irlc(int *rlc, int *block, int n)
{
	int i = 0, j, index = 0;
	int zeroes;

	while (1) {
		if (index >= n)
			break;

		if ((rlc[i * 2 + 0] == 0) && (rlc[i * 2 + 1] == 0)) {
			for ( ; index < n; )
				block[index++] = 0;
		} else {
			zeroes = rlc[i * 2 + 0];
			for (j = 0; j < zeroes; j++)
				block[index++] = 0;
			block[index++] = rlc[i * 2 + 1];
		}

		i = i + 1;
	}
}
