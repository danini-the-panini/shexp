#include "texture.h"

Texture::
Texture(GLenum target)
  : _target(target)
{
}

Texture::
~Texture()
{
}

void Texture::
build()
{
  glGenTextures(1, &_texture);

  bind();

  build_impl();
}

void Texture::
destroy()
{
  glDeleteTextures(1, &_texture);
}

void Texture::
bind()
{
  glBindTexture(_target, _texture);
}

void Texture::
use(GLenum slot)
{
  glActiveTexture(slot);
  bind();
}
