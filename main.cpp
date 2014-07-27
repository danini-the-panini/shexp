#include <cstdlib>
#include <iostream>

#include "gfx_boilerplate.h"
#include "sh_lut.h"
#include "sh_functions.h"
#include "shader.h"
#include "sphere.h"

using namespace std;

int main()
{
  int n_points = 10;
  double* sh_logs = new double[n_points*N_BANDS*N_BANDS];

  SH_make_lut(sh_logs, n_points);

  GFXBoilerplate gfx;
  gfx.init();

  Shader* pass = (new Shader())
    ->vertex("simple_vert.glsl")
    ->fragment("pass_frag.glsl")
    ->build();

  Sphere sph;
  sph.build();

  pass->use();
  pass->updateMat4("view",
      lookAt(vec3(3,3,3), vec3(0,0,0), vec3(0,1,0)));
  pass->updateMat4("projection",
      infinitePerspective(60.0f, 640.0f/480.0f, 0.1f));
  pass->updateMat4("world", mat4(1));
  pass->updateVec3("color", vec3(1,1,1));

  while (!glfwWindowShouldClose(gfx.window()))
  {
    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sph.draw();

    glfwSwapBuffers(gfx.window());
    glfwPollEvents();
  }

  sph.destroy();
  pass->destroy();
  delete pass;
  gfx.cleanup();

  delete [] sh_logs;
  return 0;
}
