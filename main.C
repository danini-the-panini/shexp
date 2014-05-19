#include "sh_code_gen/green.h"
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

int main()
{
  const int N_COEFF = N_BANDS*N_BANDS;
  typedef Matrix<double,N_COEFF,N_COEFF> MatrixSH;
  const int SQRT_N_SAMPLES = 1000;
  const int N_SAMPLES = SQRT_N_SAMPLES*SQRT_N_SAMPLES;

  SHSample* samples = new SHSample[N_SAMPLES];
  for (int i = 0; i < N_SAMPLES; i++)
  {
    samples[i].coeff = new double[N_COEFF];
  }
  SH_setup_spherical_samples(samples, SQRT_N_SAMPLES, N_BANDS);

  const int N_SLICES = 10;

  double** slices = new double*[N_SLICES];
  double** matrices = new double*[N_SLICES];

  int s = N_SLICES/2;

  cout << "Examples:" << endl;
  for (int i = 0; i < N_SLICES; i++)
  {
    double t = ((double)i/(double)N_SLICES)*M_PI;

    slices[i] = new double[N_COEFF];
    SH_project_polar_function([&] (double theta, double phi) { return g(t,theta,phi); }, samples, N_SAMPLES, N_BANDS, slices[i]);

    matrices[i] = new double[N_COEFF*N_COEFF];
    SH_matrix(slices[i],matrices[i]);

    Map<MatrixSH, Unaligned, Stride<1,N_COEFF> > m(matrices[i]);


    SelfAdjointEigenSolver<MatrixSH> eigensolver(m);
    if (eigensolver.info() != Success) abort();

    if ( i == N_COEFF/2 )
    {
      cout << "Original Matrix\n"
           << m << endl;

      cout << "The eigenvalues of A are:\n" << eigensolver.eigenvalues() << endl;
      cout << "Here's a matrix whose columns are eigenvectors of A \n"
           << "corresponding to these eigenvalues:\n"
           << eigensolver.eigenvectors() << endl;

      MatrixSH d;
      d.setZero();
      d.diagonal() = eigensolver.eigenvalues();

      cout << "Here's something:\n"
           << eigensolver.eigenvectors().transpose() * d * eigensolver.eigenvectors() << endl;
    }



  }

  for (int i = 0; i < N_SLICES; i++)
  {
    delete [] slices[i];
    delete [] matrices[i];
  }
  delete [] slices;
  delete [] matrices;
}
