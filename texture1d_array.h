#ifndef TEXTURE_1D_ARRAY_H
#define TEXTURE_1D_ARRAY_H

#include "texture.h"

class Texture1dArray : public Texture
{
public:
  Texture1dArray();
  virtual ~Texture1dArray();

  virtual void load_tex(const float *, GLsizei, GLsizei, GLint internalFormat,
      GLenum format, GLenum type);
  virtual void build_impl();

protected:

};

#endif
