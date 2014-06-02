#include "sh_lut.h"
#include "sh_functions.h"

using namespace std;

int main()
{
  int n_points = 100;
  double* sh_logs = new double[n_points*N_BANDS*N_BANDS];

  SH_make_lut(sh_logs, n_points);

  delete [] sh_logs;

  return 0;
}
