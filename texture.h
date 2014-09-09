#ifndef TEXTURE_H
#define TEXTURE_H

#include "gfx_include.h"

class Texture
{
public:
  Texture(GLenum);
  virtual ~Texture();
  void build();
  void destroy();
  void bind();
  void use(GLenum);
  GLuint handle();

  virtual void build_impl() = 0;

protected:
  GLuint _texture;
  GLenum _target;

};

#endif
