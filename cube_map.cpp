#include "cube_map.h"

CubeMap::
CubeMap(GLenum target)
  : Texture(target)
{
}

CubeMap::
~CubeMap()
{
}

void CubeMap::
load_cube(float ** data, GLsizei width, GLsizei height)
{
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP,
          GL_TEXTURE_WRAP_R,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP,
          GL_TEXTURE_WRAP_S,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP,
          GL_TEXTURE_WRAP_T,
          GL_CLAMP_TO_EDGE);

  for (int i = 0; i < 6; i++)
  {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB,
              width, height, 0, GL_RGB,
              GL_UNSIGNED_BYTE,
              data[i]);
  }
}

void CubeMap::
build_impl()
{
}
