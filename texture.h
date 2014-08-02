#ifndef TEXTURE_H
#define TEXTURE_H

#include "gfx_include.h"

class Texture
{
public:
  Texture(GLenum);
  virtual ~Texture();
  virtual void build();
  virtual void destroy();
  virtual void use(GLenum);

  virtual void build_impl() = 0;

protected:
  GLuint _texture;
  GLenum _target;

};

#endif
