#ifndef CUBE_MAP_H
#define CUBE_MAP_H

#include "texture.h"

class CubeMap : public Texture
{
public:
  CubeMap(GLenum);
  ~CubeMap();

  virtual void load_cube(float**, GLsizei, GLsizei);
  virtual void build_impl();

protected:

};

#endif
