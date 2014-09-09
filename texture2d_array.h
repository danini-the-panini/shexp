#ifndef TEXTURE_2D_ARRAY_H
#define TEXTURE_2D_ARRAY_H

#include "texture.h"

class Texture2dArray : public Texture
{
public:
  Texture2dArray();
  virtual ~Texture2dArray();

  virtual void load_tex(const float *, GLsizei, GLsizei,  GLsizei,
      GLint internalFormat, GLenum format, GLenum type);
  virtual void build_impl();

protected:

};

#endif
