#ifndef SH_FUNCTIONS_H
#define SH_FUNCTIONS_H

#define N_BANDS 3
#define N_COEFFS 9

void SH_product(const double *a, const double *b, double *c);

void SH_square(const double *a, double *c);

void SH_matrix(const double *a, double *M);

#endif
