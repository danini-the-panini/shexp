#include <cstdlib>
#include <iostream>
#include <fstream>

#include "gfx_boilerplate.h"
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

int main(int argc, char** argv)
{
  int n = N_BANDS;
  ifstream in(argc > 1 ? argv[0] : "sh_lut.txt");
  if (!in)
  {
    cout << "Error loading file" << endl;
    return 1;
  }
  int lut_size;
  in >> lut_size;
  cout << lut_size << endl;
  float* sh_logs = new float[lut_size*n*n];

  double tmp;
  for (int i = 0; i < lut_size*n*n; i++)
  {
    in >> tmp;
    cout << tmp << " ";
    sh_logs[i] = static_cast<float>(tmp);
  }
  cout << endl;

  for (int i = 0; i < lut_size; i++)
    sh_logs[i] = (float)sh_logs[i];

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
  pass->updateFloatArray("sh_lut", sh_logs, lut_size*n*n);

  int num_sph = 3;

  vector<vec3> sphere_positions;
  sphere_positions.push_back(vec3(0,20,0));
  sphere_positions.push_back(vec3(30,20,0));
  sphere_positions.push_back(vec3(10,15,40));

  vector<float> sphere_radiuses;
  sphere_radiuses.push_back(10);
  sphere_radiuses.push_back(20);
  sphere_radiuses.push_back(15);

  vector<Transform> transforms;
  transforms.push_back(Transform(sphere_positions[0], quat(1,0,0,0), vec3(sphere_radiuses[0])));
  transforms.push_back(Transform(sphere_positions[1], quat(1,0,0,0), vec3(sphere_radiuses[1])));
  transforms.push_back(Transform(sphere_positions[2], quat(1,0,0,0), vec3(sphere_radiuses[2])));

  vector<vec3> colors;
  colors.push_back(vec3(1,0,1));
  colors.push_back(vec3(0,1,0));
  colors.push_back(vec3(0,1,1));

  Transform plane_transform(vec3(0,0,0),quat(1,0,0,0),vec3(200));

  float x = 0.0f;

  while (!glfwWindowShouldClose(gfx.window()))
  {
    x += 0.01f;

    handleInput(gfx.window());
    pass->updateMat4("view", camera.getView());

    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sphere_positions[0] = vec3(0,15+10*sin(x),0);
    transforms[0].set_translation(sphere_positions[0]);

    pass->updateFloatArray("radiuses", sphere_radiuses.data(), num_sph);
    pass->updateVec3Array("positions", sphere_positions.data(), num_sph);

    for (int i = 0; i < num_sph; i++)
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
