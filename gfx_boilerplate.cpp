#include "gfx_boilerplate.h"

void error_callback(int error, const char *description)
{
  fprintf(stderr,"ERROR %d: %s", error, description);
}

void key_callback(GLFWwindow *win, int key, int /*scancode*/, int action, int /*mods*/)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(win, GL_TRUE);
}

GFXBoilerplate::
GFXBoilerplate(GLsizei width, GLsizei height)
  : _width(width), _height(height)
{
}

GFXBoilerplate::
~GFXBoilerplate()
{
}

void GFXBoilerplate::
init()
{
  glfwSetErrorCallback(error_callback);
  initGlfw();

  _window = glfwCreateWindow(_width, _height, "SHEXP", NULL, NULL);

  if (!_window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glewExperimental = GL_TRUE;
  glfwMakeContextCurrent(_window);
  initGlew();

  glfwSetKeyCallback(_window,key_callback);
}

void GFXBoilerplate::
cleanup()
{
  glfwDestroyWindow(_window);
  glfwTerminate();
}

GLFWwindow * GFXBoilerplate::
window()
{
  return _window;
}

// private

  void GFXBoilerplate::
  initGlew()
  {
    if (glewInit() != GLEW_OK)
    {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
  }

  void GFXBoilerplate::
  initGlfw()
  {
    if (!glfwInit()) {
      exit(EXIT_FAILURE);
    }

    /*
     *glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
     *glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
     *glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
     *glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
     */
  }
