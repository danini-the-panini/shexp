#include "texture2d.h"

Texture2d::
Texture2d()
  : Texture(GL_TEXTURE_2D)
{
}

Texture2d::
~Texture2d()
{
}

void Texture2d::
load_tex(const float* data, GLsizei width, GLsizei height, GLint internalFormat,
    GLenum format, GLenum type)
{
  bind();

  glTexParameteri(
          GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR);
  glTexParameteri(
          GL_TEXTURE_2D,
          GL_TEXTURE_WRAP_R,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_2D,
          GL_TEXTURE_WRAP_S,
          GL_CLAMP_TO_EDGE);
  glTexParameteri(
          GL_TEXTURE_2D,
          GL_TEXTURE_WRAP_T,
          GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
          width, height, 0, format,
          type,
          data);
}

void Texture2d::
build_impl()
{
}
