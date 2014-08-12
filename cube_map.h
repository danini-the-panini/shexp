#ifndef CUBE_MAP_H
#define CUBE_MAP_H

#include "texture.h"

class CubeMap : public Texture
{
public:
  CubeMap();
  ~CubeMap();

  virtual void load_cube(const float **, GLsizei, GLsizei);
  virtual void build_impl();

protected:

};

#endif
