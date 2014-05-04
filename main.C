#include "eigen.h"
#include "sh_code_gen/green.h"
#include "sh_functions.h"

#include <cmath>
#include <iostream>

using namespace std;

double g(double t, double theta, double phi)
{
  return (theta >= M_PI-t) ? 0.0 : 1.0;
}

int main()
{
  const int N_COEFF = N_BANDS*N_BANDS;
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
  double*** matrices2D = new double**[N_SLICES];
  double*** eigenbasis = new double**[N_SLICES];

  int s = N_SLICES/2;

  cout << "Examples:" << endl;
  for (int i = 0; i < N_SLICES; i++)
  {
    double t = ((double)i/(double)N_SLICES)*M_PI;

    slices[i] = new double[N_COEFF];
    SH_project_polar_function([&] (double theta, double phi) { return g(t,theta,phi); },
      samples, N_SAMPLES, N_BANDS, slices[i]);

    matrices[i] = new double[N_COEFF*N_COEFF];
    SH_matrix(slices[i],matrices[i]);

    matrices2D[i] = new double*[N_COEFF];
    eigenbasis[i] = new double*[N_COEFF];
    for (int j = 0; j < N_COEFF; j++)
    {
      matrices2D[i][j] = matrices[i]+j*N_COEFF;
      eigenbasis[i][j] = new double[N_COEFF];
    }

    if (i == s)
    {
      cout << "g -> "; printvector(slices[s],N_COEFF);
      cout << "Mg:" << endl;
      printmatrix(matrices2D[s],N_COEFF);
    }

    tridiagonalize(matrices2D[i], N_COEFF);

    gram_schmidt_s(matrices2D[i],N_COEFF,eigenbasis[i],100,true);
  }

  for (int i = 0; i < N_SAMPLES; i++)
    delete [] samples[i].coeff;
  delete [] samples;
  samples = 0;


  cout << "Dg:" << endl;
  printmatrix(matrices2D[s],N_COEFF);
  cout << "Rg:" << endl;
  printmatrix(eigenbasis[s],N_COEFF);

  double** Rg = new double*[N_COEFF];
  for (int i = 0; i < N_COEFF; i++)
    Rg[i] = new double[N_COEFF];
  copymatrix(Rg,eigenbasis[s],N_COEFF);
  transpose(eigenbasis[s],N_COEFF);
  multiply(Rg,eigenbasis[s],N_COEFF);
  cout << "Rg*RgT:" << endl;
  printmatrix(Rg,N_COEFF);

  for (int i = 0; i < N_SLICES; i++)
  {
    delete [] slices[i];
    delete [] matrices[i];
    delete [] matrices2D[i];
    for (int j = 0; j < N_COEFF; j++)
      delete [] eigenbasis[i][j];
    delete [] eigenbasis[i];
  }
  for (int i = 0; i < N_COEFF; i++)
    delete [] Rg[i];
  delete [] slices;
  delete [] matrices;
  delete [] matrices2D;
  delete [] eigenbasis;
  delete [] Rg;
}