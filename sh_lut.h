#ifndef SH_LUT_H
#define SH_LUT_H

// log function applied to the diagonal matrix
// for computing SH log
double q(double x);

// makes a look-up-table of SH log coeffs
// takes n_points 'samples' from the function
// and stores them in data
// data should be of size n_points*N_COEFF
void SH_make_lut(double* data, double* a, double* b, int n_points);

#endif // SH_LUT_H
