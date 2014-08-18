#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include "texture.h"

class Texture2d : public Texture
{
public:
  Texture2d();
  ~Texture2d();

  virtual void load_tex(const float *, GLsizei, GLsizei, GLint internalFormat, GLenum format, GLenum type);
  virtual void build_impl();

protected:

};

#endif
