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
  void use(GLenum[], GLsizei);
  void use(const GLsizei n=1);
  void bind_to_texture(GLenum, GLuint);
  void bind_to_texture_layer(GLenum, GLuint, GLint);
  void destroy();

protected:
  GLsizei _width, _height;
  bool _depth_use_texture;
  GLuint _framebuffer;

};

#endif
