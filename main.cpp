#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "sh_lut.h"
#include "sh_functions.h"

using namespace std;

void error_callback(int error, const char* description)
{
  fprintf(stderr,"ERROR %d: %s",error, description);
}

void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

static void initGlew()
{
  if (glewInit() != GLEW_OK)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
}

static void initGlfw()
{
  if (!glfwInit())
      exit(EXIT_FAILURE);

  // glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
  // glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

int main()
{

  int n_points = 10;
  double* sh_logs = new double[n_points*N_BANDS*N_BANDS];

  SH_make_lut(sh_logs, n_points);

  initGlfw();
  glfwSetErrorCallback(error_callback);

  GLFWwindow* window = glfwCreateWindow(640, 480, "SHEXP", NULL, NULL);

  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window,key_callback);

  glewExperimental = GL_TRUE;
  glfwMakeContextCurrent(window);
  initGlew();

  while (!glfwWindowShouldClose(window))
  {
    // TODO: draw something

    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  delete [] sh_logs;
  return 0;
}
