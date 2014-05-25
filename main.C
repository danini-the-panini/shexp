#include "green.h"
#include "sh_functions.h"

#include <cmath>
#include <iostream>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

double g(double t, double theta, double phi)
{
  return (theta >= M_PI-t) ? 0.0 : 1.0;
}

double q(double x)
{
  return log(x)/(x-1);
}

int main()
{
  const int N_COEFF = N_BANDS*N_BANDS;
  typedef Matrix<double,N_COEFF,N_COEFF> MatrixSH;
  typedef Matrix<double,N_COEFF,1> VectorSH;
  const int SQRT_N_SAMPLES = 1000;
  const int N_SAMPLES = SQRT_N_SAMPLES*SQRT_N_SAMPLES;
  const int N_SLICES = 10;
  const VectorSH SH_UNIT = (VectorSH() << sqrt(4.0*M_PI) , VectorXd::Zero(N_COEFF-1)).finished();

  SHSample* samples = new SHSample[N_SAMPLES];
  for (int i = 0; i < N_SAMPLES; i++)
  {
    samples[i].coeff = new double[N_COEFF];
  }
  SH_setup_spherical_samples(samples, SQRT_N_SAMPLES, N_BANDS);


  double** slices = new double*[N_SLICES];
  double* temp_matrix = new double[N_COEFF*N_COEFF];

  for (int i = 0; i < N_SLICES; i++)
  {
    double t = ((double)i/(double)N_SLICES)*M_PI;

    slices[i] = new double[N_COEFF];
    SH_project_polar_function(
        [&] (double theta, double phi)
        {
          return g(t,theta,phi);
        },
        samples, N_SAMPLES, N_BANDS, slices[i]);

    Map<VectorSH> v(slices[i]);

    SH_matrix(slices[i],temp_matrix);

    Map<MatrixSH, Unaligned, Stride<1,N_COEFF> > m(temp_matrix);

    SelfAdjointEigenSolver<MatrixSH> eigensolver(m);
    if (eigensolver.info() != Success) abort();

    // Ren et al. 2006
    double epsilon = 0.02 * eigensolver.eigenvalues().maxCoeff();

    MatrixSH d;
    d.setZero();
    auto diagonal = d.diagonal();
    for (int j =- 0; j < N_COEFF; j++)
    {
      diagonal[j] = q(max(eigensolver.eigenvalues()[j], epsilon));
    }

    // Ren et al. 2006 eq. 28
    MatrixSH mg = eigensolver.eigenvectors() * d * eigensolver.eigenvectors().transpose();
    VectorSH f = mg * (v - SH_UNIT);

    cout << "f[" << (t/M_PI)*180.0 << "] = " << f << endl;

    memcpy(slices[i],&f[0],N_COEFF);
  }

  for (int i = 0; i < N_SLICES; i++)
  {
    delete [] slices[i];
  }
  delete [] slices;
  delete [] temp_matrix;
}
