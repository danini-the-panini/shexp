#ifndef SHADER_H
#define SHADER_H

#include "gfx_include.h"
#include <map>

class Shader
{
public:
  static GLuint loadShader(const char* file, GLenum type);
  static void shaderLog(GLuint handle);
  static void programLog(GLuint handle);

  Shader();
  ~Shader();

  Shader* vertex(const char* file);
  Shader* geometry(const char* file);
  Shader* tess_control(const char* file);
  Shader* tess_eval(const char* file);
  Shader* fragment(const char* file);
  Shader* compute(const char* file);

  Shader* build();
  void destroy();

  GLint findAttribute(const char* name);
  GLint findUniform(const char* name);

  void updateMat4(const char* name, mat4 v);
  void updateFloat(const char* name, float);
  void updateFloatArray(const char* name, const float*, int);
  void updateInt(const char* name, int);
  void updateInts(const char* name, const int*, int);
  void updateVec3(const char* name, vec3 v);
  void updateVec4(const char* name, vec4 v);
  void updateVec3Array(const char* name, const vec3* v, int size);

  GLint getPositionLocation();
  GLint getNormalLocation();

  void use();

private:
  GLuint _program;
  GLuint _vert, _tess_control, _tess_eval, _geom, _frag, _comp;

  GLint _position;
  GLint _normal;

  map<const char*, GLint> _uniforms;
};

#endif
