#ifndef CUBE_MAP_ARRAY_H
#define CUBE_MAP_ARRAY_H

#include "texture.h"

class CubeMapArray : public Texture
{
public:
  CubeMapArray();
  virtual ~CubeMapArray();

  virtual void load_cube_array(float *, GLsizei, GLsizei,
      GLint, GLenum format, GLenum type);
  virtual void build_impl();

protected:

};

#endif
