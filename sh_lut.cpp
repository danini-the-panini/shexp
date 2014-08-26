#include "sh_lut.h"
#include "green.h"
#include "sh_functions.h"

#include <cmath>
#include <iostream>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

SH_polar_fn g_polar(const double t)
{
  return [t](double theta, double phi)
  {
    return (theta < t) ? 0.0 : 1.0;
  };
}

SH_cart_fn g_cart(const double t)
{
  const double cos_t = cos(t);
  return [cos_t](double x, double y, double z)
  {
    return z >= cos_t ? 0.0 : 1.0;
  };
}

// Ren et al. 2006 eq. 31
double q(double x)
{
  return log(x)/(x-1.0);
}

double shdot(double *x, double *y)
{
  double z = 0;
  for (int i = 0; i < N_COEFFS; i++)
    z += x[i]*y[i];
  return z;
}

void SH_make_lut(double* data, double* a, double* b, int n_points)
{
  typedef Matrix<double,N_COEFFS,N_COEFFS> MatrixSH;
  typedef Matrix<double,N_COEFFS,1> VectorSH;
  const int SQRT_N_SAMPLES = 1000;
  const int N_SAMPLES = SQRT_N_SAMPLES*SQRT_N_SAMPLES;
  const VectorSH SH_UNIT = (VectorSH() << sqrt(4.0*M_PI) , VectorXd::Zero(N_COEFFS-1)).finished();

  SHSample* samples = new SHSample[N_SAMPLES];
  for (int i = 0; i < N_SAMPLES; i++)
  {
    samples[i].coeff = new double[N_COEFFS];
  }
  SH_setup_spherical_samples(samples, SQRT_N_SAMPLES, N_BANDS);

  // the slice corresponds to a set of SH coeffs for a sample point on the LUT
  double* slice = new double[N_COEFFS];
  double* temp_matrix = new double[N_COEFFS*N_COEFFS];

  double *f_hat, *g_hat;
  f_hat = new double[N_COEFFS];
  g_hat = new double[N_COEFFS];

  for (int i = 0; i < n_points; i++)
  {
    cerr << "======================" << endl;

    double t = ((double)i/(double)(n_points-1))*(M_PI*0.5);

    SH_project_polar_function(g_polar(t), samples, N_SAMPLES, N_BANDS, slice);

    for(int l=0; l<N_BANDS; ++l) {
      for(int m=-l; m<=l; ++m) {
        int index = l*(l+1)+m;
        if (m != 0 && l != 0) slice[index] = 0.0;
      }
    }

    Map<VectorSH> v(slice);

    SH_matrix(slice,temp_matrix);

    Map<MatrixSH, Unaligned, Stride<1,N_COEFFS> > m(temp_matrix);

    SelfAdjointEigenSolver<MatrixSH> eigensolver(m);
    if (eigensolver.info() != Success) abort();

    // Ren et al. 2006
    double epsilon = 0.02 * eigensolver.eigenvalues().maxCoeff();

    MatrixSH d;
    d.setZero();
    auto diagonal = d.diagonal();
    for (int j = 0; j < N_COEFFS; j++)
    {
      diagonal[j] = q(max(eigensolver.eigenvalues()[j], epsilon));
    }

    // Ren et al. 2006 eq. 28
    VectorSH f = eigensolver.eigenvectors() * d * eigensolver.eigenvectors().transpose() * (v - SH_UNIT);

    cerr << f << endl;
    cerr << "----------------------" << endl;

    // copy the result into the data array
    for(int l=0; l<N_BANDS; ++l) {
      for(int m=-l; m<=l; ++m) {
        int j = l*(l+1)+m;
        data[i*N_COEFFS+j] = ((m!=0 && l!=0) ? 0.0 : f[j]);
      }
    }

    // calculate a and b
    f_hat[0] = 0;
    for (int j = 1; j < N_COEFFS; j++)
    {
      f_hat[j] = f[j];
    }

    for (int j = 0; j < N_COEFFS; j++)
    {
      g_hat[j] = slice[j] * exp(-f[0]/sqrt(4.0*M_PI));
    }

    a[i] = g_hat[0]/sqrt(4.0*M_PI);
    b[i] = shdot(g_hat, f_hat)/shdot(f_hat,f_hat);
  }

  delete [] slice;
  delete [] temp_matrix;
  delete [] f_hat;
  delete [] g_hat;
}

int main(int argc, char** argv)
{
  int lut_size = argc > 1 ? atoi(argv[1]) : 16;
  int sh_lut_size = N_COEFFS*lut_size;

  double* sh_logs = new double[sh_lut_size];
  double* a = new double[lut_size];
  double* b = new double[lut_size];

  SH_make_lut(sh_logs, a, b, lut_size);

  cout << lut_size << endl;

  for (int i = 0; i < sh_lut_size; i++)
  {
    cout << sh_logs[i];

    if ((i+1)%N_COEFFS == 0) cout << endl;
    else cout << " ";
  }
  for (int i= 0; i < lut_size; i++) cout << a[i] << " ";
  cout << endl;
  for (int i= 0; i < lut_size; i++) cout << b[i] << " ";

  delete [] sh_logs;

  return 0;
}
