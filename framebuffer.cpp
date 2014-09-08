#include "framebuffer.h"

#include "iostream"

Framebuffer::
Framebuffer(GLsizei width, GLsizei height, bool depth_use_texture)
  : _width(width), _height(height), _depth_use_texture(depth_use_texture)
{
}

Framebuffer::
~Framebuffer()
{
}

void Framebuffer::
build()
{
  glGenFramebuffers(1, &_framebuffer);
  bind();

  if (_depth_use_texture)
  {
      GLuint depthTexture;
      glGenTextures(1, &depthTexture);

      glBindTexture(GL_TEXTURE_2D, depthTexture);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
          _width, _height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
          depthTexture, 0);
  }
  else
  {
      GLuint depthrenderBuffer;
      glGenRenderbuffers(1, &depthrenderBuffer);

      glBindRenderbuffer(GL_RENDERBUFFER, depthrenderBuffer);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
          _width, _height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
          GL_RENDERBUFFER, depthrenderBuffer);
  }

  int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE)
  {
    cerr << "Framebuffer incomplete: " << status << endl;
  }

  unbind();
}

void Framebuffer::
bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
}

void Framebuffer::
unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::
use()
{
  bind();

  GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, bufs); // "1" is the size of DrawBuffers

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, _width, _height);
}

void Framebuffer::
bind_to_texture(GLenum att, GLenum textgt, GLint tex)
{
  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, att, textgt, tex, 0);
  unbind();
}
