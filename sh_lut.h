#ifndef SH_LUT_H
#define SH_LUT_H

// visibility function to be mapped onto SH
double g(double t, double theta, double);

// log function applied to the diagonal matrix
// for computing SH log
double q(double x);

// makes a look-up-table of SH log coeffs
// takes n_points 'samples' from the function
// and stores them in data
// data should be of size n_points*N_COEFF
void SH_make_lut(double* data, int n_points);

#endif // SH_LUT_H
