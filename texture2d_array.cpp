#include "texture2d_array.h"

Texture2dArray::
Texture2dArray()
  : Texture(GL_TEXTURE_2D_ARRAY)
{
}

Texture2dArray::
~Texture2dArray()
{
}

void Texture2dArray::
load_tex(const float* data, GLsizei width, GLsizei height, GLsizei layers,
    GLint internalFormat, GLenum format, GLenum type)
{
  bind();

  glTexParameteri(
          GL_TEXTURE_2D_ARRAY,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_2D_ARRAY,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_2D_ARRAY,
          GL_TEXTURE_WRAP_R,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_2D_ARRAY,
          GL_TEXTURE_WRAP_S,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_2D_ARRAY,
          GL_TEXTURE_WRAP_T,
          GL_CLAMP_TO_EDGE);

  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat,
          width, height, layers, 0, format, type, data);
}

void Texture2dArray::
build_impl()
{
}
