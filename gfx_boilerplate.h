#ifndef GFX_BOILERPLATE_H
#define GFX_BOILERPLATE_H

#include <cstdlib>
#include <iostream>

#include "gfx_include.h"

using namespace std;

void error_callback(int error, const char *description);
void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/);

class GFXBoilerplate
{
public:
  GFXBoilerplate(GLsizei, GLsizei);
  ~GFXBoilerplate();
  void init();
  void cleanup();

  GLFWwindow * window();

private:
  GLsizei _width, _height;
  GLFWwindow *_window;

  void initGlew();
  void initGlfw();

};


#endif
