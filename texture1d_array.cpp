#include "texture1d_array.h"

Texture1dArray::
Texture1dArray()
  : Texture(GL_TEXTURE_1D_ARRAY)
{
}

Texture1dArray::
~Texture1dArray()
{
}

void Texture1dArray::
load_tex(const float* data, GLsizei size, GLsizei layers, GLint internalFormat,
    GLenum format, GLenum type)
{
  bind();

  glTexParameteri(
          GL_TEXTURE_1D_ARRAY,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_1D_ARRAY,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_1D_ARRAY,
          GL_TEXTURE_WRAP_R,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_1D_ARRAY,
          GL_TEXTURE_WRAP_S,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_1D_ARRAY,
          GL_TEXTURE_WRAP_T,
          GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, internalFormat,
          size, layers, 0, format, type, data);
}

void Texture1dArray::
build_impl()
{
}
