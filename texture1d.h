#ifndef TEXTURE_1D_H
#define TEXTURE_1D_H

#include "texture.h"

class Texture1d : public Texture
{
public:
  Texture1d();
  virtual ~Texture1d();

  virtual void load_tex(const float *, GLsizei, GLint internalFormat,
      GLenum format, GLenum type);
  virtual void build_impl();

protected:

};

#endif
