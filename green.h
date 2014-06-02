#ifndef GREEN_H
#define GREEN_H

#include <chrono>
#include <cmath>
#include <functional>
#include <random>

struct Vector3d {
  double x;
  double y;
  double z;
};

struct SHSample {
 Vector3d sph;
 Vector3d vec;
 double* coeff;
};

void SH_setup_spherical_samples(
  SHSample samples[],
  int sqrt_n_samples,
  int n_bands);
double P(int l,int m,double x);
double K(int l, int m);
double SH(int l, int m, double theta, double phi);

typedef std::function<double(double,double)> SH_polar_fn;
void SH_project_polar_function(
  SH_polar_fn fn,
  const SHSample samples[],
  int n_samples,
  int n_bands,
  double result[]);
void SH_product_tensor(const SHSample samples[],
  int n_samples,
  int n_bands,
  double*** result);

const double factorial[34]=
{
  1.0,
  1.0,
  2.0,
  6.0,
  24.0,
  120.0,
  720.0,
  5040.0,
  40320.0,
  362880.0,
  3628800.0,
  39916800.0,
  479001600.0,
  6227020800.0,
  87178291200.0,
  1307674368000.0,
  20922789888000.0,
  355687428096000.0,
  6.402373705728e+15,
  1.21645100408832e+17,
  2.43290200817664e+18,
  5.109094217170944e+19,
  1.1240007277776077e+21,
  2.585201673888498e+22,
  6.204484017332394e+23,
  1.5511210043330986e+25,
  4.0329146112660565e+26,
  1.0888869450418352e+28,
  3.0488834461171387e+29,
  8.841761993739702e+30,
  2.6525285981219107e+32,
  8.222838654177922e+33,
  2.631308369336935e+35,
  8.683317618811886e+36
};

#endif // GREEN_H
