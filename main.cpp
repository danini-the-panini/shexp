#include <cstdlib>
#include <iostream>

#include "gfx_boilerplate.h"
#include "sh_lut.h"
#include "sh_functions.h"
#include "shader.h"
#include "sphere.h"
#include "plane.h"
#include "camera.h"
#include "transform.h"

using namespace std;

typedef OrthoRotMatCamera<float, highp> ORMCamF;
ORMCamF camera(ORMCamF::vec3_type(0,30,-50));

const int KEYS[] = {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D,
  GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_E, GLFW_KEY_Q};
void handleInput(GLFWwindow *window)
{
  for (unsigned i = 0; i < sizeof(KEYS)/sizeof(int); i++)
  {
    if (glfwGetKey(window, KEYS[i]) == GLFW_PRESS)
    {
      camera.doKey(KEYS[i]);
    }
  }
}

bool first_mouse = true;
double mx, my;
void mouse_callback(GLFWwindow *window, double x, double y)
{
  if (first_mouse)
  {
    first_mouse = false;
  } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
  {
    double dx = x - mx;
    double dy = y - my;

    camera.mouseLook((float)dx, (float)dy);
  }

  mx = x;
  my = y;
}

int main()
{
  int n_points = 10;
  int lut_size = n_points*N_BANDS*N_BANDS;
  double* sh_logs = new double[lut_size];
  float* sh_logs_f = new float[lut_size];

  SH_make_lut(sh_logs, n_points);

  for (int i = 0; i < lut_size; i++)
    sh_logs_f[i] = (float)sh_logs[i];

  GFXBoilerplate gfx;
  gfx.init();

  glEnable(GL_DEPTH_TEST);

  glfwSetCursorPosCallback(gfx.window(), mouse_callback);

  Shader* pass = (new Shader())
    ->vertex("simple_vert.glsl")
    ->fragment("sh_frag.glsl")
    ->build();

  Sphere sph;
  sph.build();

  Plane pln;
  pln.build();

  pass->use();
  pass->updateMat4("projection",
      infinitePerspective(45.0f, 640.0f/480.0f, 0.1f));
  pass->updateFloatArray("sh_lut", sh_logs_f, lut_size);

  vec3 sphere_positions[] = {
    vec3(0,20,0), vec3(30,20,0)
  };

  float sphere_radiuses[] = { 10, 20 };

  Transform transforms[] = {
    Transform(sphere_positions[0], quat(1,0,0,0), vec3(sphere_radiuses[0])),
    Transform(sphere_positions[1], quat(1,0,0,0), vec3(sphere_radiuses[1]))
  };

  vec3 colors[] = {
    vec3(1,0,1), vec3(0,1,0)
  };

  pass->updateFloatArray("radiuses", sphere_radiuses, 2);
  pass->updateVec3Array("positions", sphere_positions, 2);

  Transform plane_transform(vec3(0,0,0),quat(1,0,0,0),vec3(200));

  while (!glfwWindowShouldClose(gfx.window()))
  {
    handleInput(gfx.window());
    pass->updateMat4("view", camera.getView());

    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < 2; i++)
    {
      pass->updateMat4("world", transforms[i].world());
      pass->updateVec3("color", colors[i]);
      sph.draw();
    }

    pass->updateMat4("world", plane_transform.world());
    pass->updateVec3("color", vec3(1,1,1));
    pln.draw();

    glfwSwapBuffers(gfx.window());
    glfwPollEvents();
  }

  sph.destroy();
  pln.destroy();
  pass->destroy();
  delete pass;
  gfx.cleanup();

  delete [] sh_logs;
  return 0;
}
