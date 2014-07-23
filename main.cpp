#include <cstdlib>
#include <iostream>

#include "gfx_boilerplate.h"
#include "sh_lut.h"
#include "sh_functions.h"

using namespace std;

int main()
{

  int n_points = 10;
  double* sh_logs = new double[n_points*N_BANDS*N_BANDS];

  SH_make_lut(sh_logs, n_points);

  GFXBoilerplate gfx;

  while (!glfwWindowShouldClose(gfx.window()))
  {
    // TODO: draw something

    glfwPollEvents();
  }

  gfx.cleanup();

  delete [] sh_logs;
  return 0;
}
