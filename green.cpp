#include "green.h"

using namespace std;

void SH_setup_spherical_samples(
  SHSample samples[],
  int sqrt_n_samples,
  int n_bands)
{
  long seed = chrono::system_clock::now().time_since_epoch().count();
  default_random_engine gen (seed);

  uniform_real_distribution<double> random (0, 1);
  // fill an N*N*2 array with uniformly distributed
  // samples across the sphere using jittered stratification
  int i=0; // array index
  double oneoverN = 1.0/sqrt_n_samples;
  for(int a=0; a<sqrt_n_samples; a++) {
    for(int b=0; b<sqrt_n_samples; b++) {
      // generate unbiased distribution of spherical coords
      double x = (a + random(gen)) * oneoverN; // do not reuse results
      double y = (b + random(gen)) * oneoverN; // each sample must be random
      double theta = 2.0 * acos(sqrt(1.0 - x));
      double phi = 2.0 * M_PI * y;
      samples[i].sph = Vector3d{theta,phi,1.0};
      // convert spherical coords to unit vector
      Vector3d vec{sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)};
      samples[i].vec = vec;
      // precompute all SH coefficients for this sample
      for(int l=0; l<n_bands; ++l) {
        for(int m=-l; m<=l; ++m) {
          int index = l*(l+1)+m;
          samples[i].coeff[index] = SH(l,m,theta,phi);
        }
      }
      ++i;
    }
  }
}

double P(int l,int m,double x)
{
  // evaluate an Associated Legendre Polynomial P(l,m,x) at x
  double pmm = 1.0;
  if(m>0) {
    double somx2 = sqrt((1.0-x)*(1.0+x));
    double fact = 1.0;
    for(int i=1; i<=m; i++) {
      pmm *= (-fact) * somx2;
      fact += 2.0;
    }
  }
  if(l==m) return pmm;
  double pmmp1 = x * (2.0*m+1.0) * pmm;
  if(l==m+1) return pmmp1;
  double pll = 0.0;
  for(int ll=m+2; ll<=l; ++ll) {
    pll = ( (2.0*ll-1.0)*x*pmmp1-(ll+m-1.0)*pmm ) / (ll-m);
    pmm = pmmp1;
    pmmp1 = pll;
  }
  return pll;
}

double K(int l, int m)
{
  // renormalisation constant for SH function
  double temp = ((2.0*l+1.0)*factorial[l-m]) / (4.0*M_PI*factorial[l+m]);
  return sqrt(temp);
}

double SH(int l, int m, double theta, double phi)
{
  // return a point sample of a Spherical Harmonic basis function
  // l is the band, range [0..N]
  // m in the range [-l..l]
  // theta in the range [0..M_PI]
  // phi in the range [0..2*M_PI]
  const double sqrt2 = sqrt(2.0);
  if(m==0) return K(l,0)*P(l,m,cos(theta));
  else if(m>0) return sqrt2*K(l,m)*cos(m*phi)*P(l,m,cos(theta));
  else return sqrt2*K(l,-m)*sin(-m*phi)*P(l,-m,cos(theta));
}

void SH_project_polar_function(
  SH_polar_fn fn,
  const SHSample samples[],
  int n_samples,
  int n_bands,
  double result[])
{
  int n_coeff = n_bands*n_bands;
  const double weight = 4.0*M_PI;
  // for each sample
  for(int i=0; i<n_samples; ++i) {
    double theta = samples[i].sph.x;
    double phi = samples[i].sph.y;
    for(int n=0; n<n_coeff; ++n) {
      result[n] += fn(theta,phi) * samples[i].coeff[n];
    }
  }
  // divide the result by weight and number of samples
  double factor = weight / n_samples;
  for(int i=0; i<n_coeff; ++i) {
    result[i] = result[i] * factor;
  }
}

void SH_project_cart_function(
  SH_cart_fn fn,
  const SHSample samples[],
  int n_samples,
  int n_bands,
  double result[])
{
  int n_coeff = n_bands*n_bands;
  const double weight = 4.0*M_PI;
  // for each sample
  for(int i=0; i<n_samples; ++i) {
    double x = samples[i].vec.x;
    double y = samples[i].vec.y;
    double z = samples[i].vec.z;
    for(int n=0; n<n_coeff; ++n) {
      result[n] += fn(x,y,z) * samples[i].coeff[n];
    }
  }
  // divide the result by weight and number of samples
  double factor = weight / n_samples;
  for(int i=0; i<n_coeff; ++i) {
    result[i] = result[i] * factor;
  }
}

void SH_product_tensor(const SHSample samples[],
  int n_samples,
  int n_bands,
  double*** result)
{
  int n_coeff = n_bands*n_bands;
  const double weight = 4.0*M_PI;
  // for each sample
  for (int n = 0; n < n_samples; ++n)
  {
    for (int i = 0; i < n_coeff; ++i)
      for (int j = i; j < n_coeff; ++j)
        for (int k = j; k < n_coeff; ++k)
        {
          result[i][j][k] += samples[n].coeff[i] * samples[n].coeff[j]
            * samples[n].coeff[k];
        }
  }
  // divide the result by weight and number of samples
  double factor = weight / n_samples;
  for (int i = 0; i < n_coeff; ++i)
    for (int j = i; j < n_coeff; ++j)
      for (int k = j; k < n_coeff; ++k)
      {
        result[i][j][k]=result[i][k][j]=result[j][i][k]=result[j][k][i]
            =result[k][i][j]=result[k][j][i]=result[i][j][k] * factor;
      }
}
