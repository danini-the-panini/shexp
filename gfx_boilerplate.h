#ifndef GFX_BOILERPLATE_H
#define GFX_BOILERPLATE_H

#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

void error_callback(int error, const char *description);

void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/);

class GFXBoilerplate
{
public:
  GFXBoilerplate();
  ~GFXBoilerplate();
  void init();
  void cleanup();

  GLFWwindow * window();

private:
  GLFWwindow *_window;

  void initGlew();
  void initGlfw();

};


#endif
