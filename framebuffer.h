#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "gfx_include.h"

class Framebuffer
{
public:
  Framebuffer(GLsizei, GLsizei, bool depth_use_texture = false);
  virtual ~Framebuffer();

  void build();

  void bind();
  static void unbind();
  void use();
  void bind_to_texture(GLenum, GLenum, GLint);

protected:
  GLsizei _width, _height;
  bool _depth_use_texture;
  GLuint _framebuffer;

};

#endif
