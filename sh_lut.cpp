#include "sh_lut.h"
#include "green.h"
#include "sh_functions.h"

#include <cmath>
#include <iostream>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

double g(double t, double theta, double)
{
  return (theta >= M_PI-t) ? 0.0 : 1.0;
}

// Ren et al. 2006 eq. 31
double q(double x)
{
  return log(x)/(x-1);
}

void SH_make_lut(double* data, int n_points)
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

  // each slice corresponds to a set of SH coeffs for a sample point on the LUT
  double** slices = new double*[n_points];
  double* temp_matrix = new double[N_COEFFS*N_COEFFS];

  for (int i = 0; i < n_points; i++)
  {
    double t = ((double)i/(double)n_points)*(M_PI*0.5);

    slices[i] = new double[N_COEFFS];
    SH_project_polar_function(
        [&] (double theta, double phi)
        {
          return g(t,theta,phi);
        },
        samples, N_SAMPLES, N_BANDS, slices[i]);

    Map<VectorSH> v(slices[i]);

    SH_matrix(slices[i],temp_matrix);

    Map<MatrixSH, Unaligned, Stride<1,N_COEFFS> > m(temp_matrix);

    SelfAdjointEigenSolver<MatrixSH> eigensolver(m);
    if (eigensolver.info() != Success) abort();

    // Ren et al. 2006
    double epsilon = 0.02 * eigensolver.eigenvalues().maxCoeff();

    MatrixSH d;
    d.setZero();
    auto diagonal = d.diagonal();
    for (int j =- 0; j < N_COEFFS; j++)
    {
      diagonal[j] = q(max(eigensolver.eigenvalues()[j], epsilon));
    }

    // Ren et al. 2006 eq. 28
    MatrixSH mg = eigensolver.eigenvectors() * d * eigensolver.eigenvectors().transpose();
    VectorSH f = mg * (v - SH_UNIT);

    //cout << "f[" << (t/M_PI)*180.0 << "] = " << f << endl;

    // copy the result into the data array
    memcpy(data+i*N_COEFFS, &f[0], N_COEFFS);
  }

  for (int i = 0; i < n_points; i++)
  {
    delete [] slices[i];
  }
  delete [] slices;
  delete [] temp_matrix;
}

int main(int argc, char** argv)
{
  int lut_size = argc > 1 ? atoi(argv[1]) : 10;
  int sh_lut_size = N_COEFFS*lut_size;

  double* sh_logs = new double[sh_lut_size];

  SH_make_lut(sh_logs, lut_size);

  cout << lut_size << endl;

  for (int i = 0; i < sh_lut_size; i++)
  {
    cout << sh_logs[i] << " ";
  }

  delete [] sh_logs;

  return 0;
}
