#include "cube_map_array.h"

CubeMapArray::
CubeMapArray()
  : Texture(GL_TEXTURE_CUBE_MAP_ARRAY)
{
}

CubeMapArray::
~CubeMapArray()
{
}

void CubeMapArray::
load_cube_array(float *data, GLsizei size, GLsizei layers,
    GLint internalFormat, GLenum format, GLenum type)
{
  bind();

  glTexParameteri(
          GL_TEXTURE_CUBE_MAP_ARRAY,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP_ARRAY,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP_ARRAY,
          GL_TEXTURE_WRAP_R,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP_ARRAY,
          GL_TEXTURE_WRAP_S,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_CUBE_MAP_ARRAY,
          GL_TEXTURE_WRAP_T,
          GL_CLAMP_TO_EDGE);

  glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, internalFormat,
      size, size, layers*6, 0, format, type, data);
}
