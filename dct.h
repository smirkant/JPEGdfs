#ifndef DCT_H
#define DTC_H

#define PI 3.1415926


extern void fdct(int *src, int *dest);
extern void idct(int *src, int *dest);

extern void fzigzag(int *src, int *dest);
extern void izigzag(int *src, int *dest);

extern void fquantization(int *src, int *dest, int *qt_table);
extern void iquantization(int *src, int *dest, int *qt_table);

#endif
