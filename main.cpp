#include <cstdlib>
#include <iostream>

#include "gfx_boilerplate.h"
#include "sh_lut.h"
#include "sh_functions.h"
#include "sphere.h"

using namespace std;

int main()
{
  int n_points = 10;
  double* sh_logs = new double[n_points*N_BANDS*N_BANDS];

  SH_make_lut(sh_logs, n_points);

  GFXBoilerplate gfx;
  gfx.init();

  Sphere sph;
  sph.build();

  while (!glfwWindowShouldClose(gfx.window()))
  {
    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sph.draw();

    glfwSwapBuffers(gfx.window());
    glfwPollEvents();
  }

  sph.destroy();
  gfx.cleanup();

  delete [] sh_logs;
  return 0;
}
