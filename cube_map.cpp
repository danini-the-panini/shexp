#include "cube_map.h"

CubeMap::
CubeMap()
  : Texture(GL_TEXTURE_CUBE_MAP)
{
}

CubeMap::
~CubeMap()
{
}

void CubeMap::
load_cube(const float **data, GLsizei width, GLsizei height, GLint internalFormat, GLenum format, GLenum type)
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

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

  for (int i = 0; i < 6; i++)
  {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, internalFormat,
              width, height, 0, format,
              type,
              data[i]);
  }
}

void CubeMap::
build_impl()
{
}
