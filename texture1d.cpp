#include "texture1d.h"

Texture1d::
Texture1d()
  : Texture(GL_TEXTURE_1D)
{
}

Texture1d::
~Texture1d()
{
}

void Texture1d::
load_tex(const float* data, GLsizei size, GLint internalFormat,
    GLenum format, GLenum type)
{
  bind();

  glTexParameteri(
          GL_TEXTURE_1D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_1D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_1D,
          GL_TEXTURE_WRAP_R,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_1D,
          GL_TEXTURE_WRAP_S,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_1D,
          GL_TEXTURE_WRAP_T,
          GL_CLAMP_TO_EDGE);

  glTexImage1D(GL_TEXTURE_1D, 0, internalFormat,
          size, 0, format, type, data);
}

void Texture1d::
build_impl()
{
}
