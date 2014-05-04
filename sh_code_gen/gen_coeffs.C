#include "green.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

// double light(double theta, double phi);

using namespace std;

// double light(double theta, double phi)
// {
//   return max(0.0,5.0*cos(theta) - 4.0) +
//           max(0.0, -4.0 * sin(theta - M_PI) * cos(phi - 2.5) - 3.0);
// }

inline bool is_zero(double x) { return (x>0 ? x : -x) < 0.00001; }

int main(int argc, char** argv)
{
  const int n_bands = (argc > 1) ? atoi(argv[1]) : 3;
  const int n_coeff = n_bands*n_bands;
  const int sqrt_n_samples = 1000;
  const int n_samples = sqrt_n_samples*sqrt_n_samples;

  SHSample* samples = new SHSample[n_samples];
  for (int i = 0; i < n_samples; i++)
  {
    samples[i].coeff = new double[n_coeff];
  }
  SH_setup_spherical_samples(samples, sqrt_n_samples, n_bands);
  double* coeffs = new double[n_coeff];
  memset(coeffs, 0.0, n_coeff*sizeof(double)); 

  // SH_project_polar_function(light, samples, n_samples, n_bands, coeffs);

  // int n = 0;
  // for (int l = 0; l < n_bands; l++)
  // {
  //   for (int m = -l; m <= l; m++)
  //   {
  //     cout << coeffs[n] << ", ";
  //     ++n;
  //   }
  //   cout << endl;
  // }

  double*** gamma = new double**[n_coeff];
  for (int i = 0; i < n_coeff; ++i)
  {
    gamma[i] = new double*[n_coeff];
    for (int j = 0; j < n_coeff; ++j)
    {
      gamma[i][j] = new double[n_coeff];
      memset(gamma[i][j], 0.0, n_coeff*sizeof(double));
    }
  }

  SH_product_tensor(samples, n_samples, n_bands, gamma);

  cout << "{";
  int m = 0;
  for (int i = 0; i < n_coeff; ++i)
    for (int j = i; j < n_coeff; ++j)
      for (int k = j; k < n_coeff; ++k)
        if (!is_zero(gamma[i][j][k]))
        {
          if (m++>0) cout << ",";
          cout << "["<<i<<","<<j<<","<<k<<"] => "<<gamma[i][j][k];
        }
  cout << "}";

  cout << endl;

  delete [] coeffs;
  for (int i = 0; i < n_samples; i++)
  {
    delete [] samples[i].coeff;
  }
  delete [] samples;
  for (int i = 0; i < n_coeff; ++i)
  {
    for (int j = 0; j < n_coeff; ++j)
    {
      delete [] gamma[i][j];
    }
    delete [] gamma[i];
  }
  delete [] gamma;
}