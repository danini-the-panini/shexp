#include "eigen.h"

#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>

using namespace std;


void printvector(double* x, int m)
{
  cout << "[ ";
  for (int i = 0; i < m; i++)
    cout << setprecision(4) << setw(10) << x[i] << " ";
  cout << "]" << endl;
}

void printmatrix(double** A, int m)
{
  for (int i = 0; i < m; i++)
  {
    printvector(A[i],m);
  }
}

void copymatrix(double** dest, double **source, int n)
{
  for (int i = 0; i < n; i++)
    memcpy(dest[i],source[i],n*sizeof(double));
}

void transpose(double** A, int n)
{
  double tmp;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      tmp = A[i][j];
      A[i][j] = A[j][i];
      A[j][i] = tmp;
    }
}

void multiply(double** A, double** B, int m)
{
  double* tmp = new double[m];
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < m; j++)
    {
      tmp[j] = 0.0;
      for (int k = 0; k < m; k++)
        tmp[j] += A[i][k] * B[k][j];
    }
    memcpy(A[i], tmp, m*sizeof(double));
  }
  delete [] tmp;
}

inline int ab(double x) { return (x<0)?-x:x; }

void multiply_into(double** A, double** B, int m, double** C)
{
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < m; j++)
    {
      C[i][j] = 0.0;
      for (int k = 0; k < m; k++)
        C[i][j] += A[i][k] * B[k][j];
    }
  }
}

void multiply_into_tri(double** A, double** B, int m, double** C)
{
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < m; j++)
    {
      C[i][j] = 0.0;
      for (int k = 0; k < m; k++)
        C[i][j] += (i>k || k-j > 1)
          ? 0
          : A[i][k] * B[k][j];
    }
  }
}

void tridiagonalize(double** matrix, int n)
{
  double* x = new double[n];
  double** A = matrix, ** H = new double*[n], ** tmp_m = NULL;
  for (int i = 0; i < n; i++)
    H[i] = new double[n];
  double tmp;

  for (int col = 0; col < n-2; col++)
  {
    tmp = 0.0;
    memset(x, 0.0, (col+1)*sizeof(double)); // first 'col' values of x <- 0
    for (int i = col+1; i < n; i++) // the rest <- A[i,j]
    {
      x[i] = A[i][col];
      tmp += x[i]*x[i];
    }
    tmp = sqrt(tmp);
    x[col+1] -= (A[col+1][col] < 0 ? 1 : -1) * tmp; // subtract 'y', making 'u' 

    // normalize 'u' ...
    tmp = 0.0;
    for (int i = 0; i < n; i++)
      tmp += x[i]*x[i];
    tmp = sqrt(tmp);
    for (int i = 0; i < n; i++)
      x[i] /= tmp;

    // H <- I - 2u*uT
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < n; j++)
        H[i][j] = (i==j ? 1.0 : 0.0) - 2.0*(x[i]*x[j]);
    }

    // calculate HAH
    multiply(A,H,n); // A <- AH
    multiply(H,A,n); // H <- H(AH)

    // A <-> H
    tmp_m = A;
    A = H;
    H = tmp_m;
  }

  // matrix <- A (if necessary)
  if (A != matrix)
  {
    for (int i = 0; i < n; i++)
      memcpy(matrix[i],A[i],n*sizeof(4));
    H = A;
  }

  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      if (j-i > 1 || j-i < -1)
        A[i][j] = 0.0;
    }

  delete [] x;
  for (int i = 0; i < n; i++)
    delete [] H[i];
  delete [] H;
}

void qr_decomp(double** A, int n, double** Q, double** R)
{
  // Q <- A
  // R <- 0
  for (int i = 0; i < n; i++)
  {
    memcpy(Q[i],A[i],n*sizeof(double));
    memset(R[i],0.0,n*sizeof(double));
  }

  double tmp;
  for (int i = 0; i < n; i++) // each column in A
  {

    for (int j = 0; j < i; j++) // each column before i
    {
      tmp = 0.0;
      for (int r = 0; r < n; r++) // <e[j],a[i]>
        tmp += Q[r][j] * A[r][i];

      for (int r = 0; r < n; r++)
      {
        Q[r][i] -= tmp * Q[r][j]; // Q[i] = a[i] - proj(e[j],a[i]) - ...
      }
    }

    // normalize
    tmp = 0.0;
    for (int r = 0; r < n; r++)
      tmp += Q[r][i] * Q[r][i];
    tmp = sqrt(tmp);
    for (int r = 0; r < n; r++)
      Q[r][i] /= tmp;

    for (int r = 0; r <= i; r++) // rows in R
    {
      for (int s = 0; s < n; s++)
        R[r][i] += Q[s][r] * A[s][i]; // <e[r],a[i]>
    }

  }
}

void qr_decomp_tri(double** A, int n, double** Q, double** R)
{
  // Q <- A
  // R <- 0
  for (int i = 0; i < n; i++)
  {
    memcpy(Q[i],A[i],n*sizeof(double));
    memset(R[i],0.0,n*sizeof(double));
  }

  double tmp;
  for (int i = 0; i < n; i++) // each column in A
  {

    for (int j = 0; j < i; j++) // each column before i
    {
      tmp = 0.0;
      for (int r = max(0,j-1); r <= min(n-1,i+1); r++) // <e[j],a[i]>
        tmp += Q[r][j] * A[r][i];

      for (int r = max(0,j-1); r <= min(n-1,i+1); r++)
      {
        Q[r][i] -= tmp * Q[r][j]; // Q[i] = a[i] - proj(e[j],a[i]) - ...
      }
    }

    // printmatrix(Q,n);
    // cout << "..." << endl;

    // normalize
    tmp = 0.0;
    for (int r = 0; r < n; r++)
      tmp += Q[r][i] * Q[r][i];
    tmp = sqrt(tmp);
    for (int r = 0; r < n; r++)
      Q[r][i] /= tmp;

    for (int r = 0; r <= i; r++) // rows in R
    {
      for (int s = 0; s < n; s++)
        R[r][i] += Q[s][r] * A[s][i]; // <e[r],a[i]>
    }

  }
}

void gram_schmidt(double** A, int n, int iterations, bool tri)
{
  double** Q = new double*[n];
  double** R = new double*[n];
  for (int i = 0; i < n; i++)
  {
    Q[i] = new double[n];
    R[i] = new double[n];
  }

  for (int i = 0; i < iterations; i++)
  {
    if (tri)
    {
      qr_decomp_tri(A,n,Q,R);
      multiply_into_tri(R,Q,n,A);
    } else
    {
      qr_decomp(A,n,Q,R);
      multiply_into(R,Q,n,A);
    }

  }

  for (int i = 0; i < n; i++)
  {
    delete [] Q[i];
    delete [] R[i];
  }
  delete [] Q;
  delete [] R;
}

void gram_schmidt_s(double** A, int n, double** S, int iterations, bool tri)
{
  double** Q = new double*[n];
  double** R = new double*[n];
  for (int i = 0; i < n; i++)
  {
    Q[i] = new double[n];
    R[i] = new double[n];
  }

  if (tri)
  {
    qr_decomp_tri(A,n,Q,R);
    multiply_into_tri(R,Q,n,A);
  } else
  {
    qr_decomp(A,n,Q,R);
    multiply_into(R,Q,n,A);
  }

  // S0 <- Q0
  for (int i = 0; i < n; i++)
    memcpy(S[i],Q[i],n*sizeof(double));

  for (int i = 1; i < iterations; i++)
  {
    if (tri)
    {
      qr_decomp_tri(A,n,Q,R);
      multiply_into_tri(R,Q,n,A);
    } else
    {
      qr_decomp(A,n,Q,R);
      multiply_into(R,Q,n,A);
    }

    multiply(S,Q,n); // Sk <- Sk-1 Q
  }

  for (int i = 0; i < n; i++)
  {
    delete [] Q[i];
    delete [] R[i];
  }
  delete [] Q;
  delete [] R;
}