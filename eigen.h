#pragma once

void printmatrix(double** A, int m);
void printvector(double* x, int m);
void copymatrix(double** dest, double **source, int n);
void transpose(double** A, int n);
void multiply(double** A, double** B, int m);
void multiply_into(double** A, double** B, int m, double** C);
void multiply_into_tri(double** A, double** B, int m);
void tridiagonalize(double** A, int n);
void qr_decomp(double** A, int n, double** Q, double** R);
void qr_decomp_tri(double** A, int n, double** Q, double** R);
void gram_schmidt(double** A, int n, int iterations = 1, bool tri = false);
void gram_schmidt_s(double** A, int n, double** S, int iterations = 1, bool tri = false);