#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

GLuint Shader::
loadShader(const char* file, GLenum type)
{
  cout << "Loading shader " << file << endl;
  GLuint _program = glCreateShader(type);

  ifstream in(file);
  if (!in)
  {
    cout << "Failed to load shader " << file << endl;
  }
  stringstream buffer;
  buffer << in.rdbuf();
  string content(buffer.str());
  in.close();

  const char * source = content.c_str();
  glShaderSource(_program, 1, &source, NULL);
  glCompileShader(_program);

  shaderLog(_program);

  return _program;
}
void Shader::
shaderLog(GLuint _program)
{
  int status;
  glGetShaderiv(_program, GL_COMPILE_STATUS, &status);

  if (status == GL_FALSE)
  {
     /* Get the length of the info log. */
     int len;
     glGetShaderiv(_program, GL_INFO_LOG_LENGTH, &len);

     /* Get the info log. */
     char* log = new char[len];
     glGetShaderInfoLog(_program, len, &len, log);

     /* Print error */
     cerr << "Compilation error: " << log << endl;

     /* Finally, free the memory allocated. */
     delete log;

     /* Exit the program. */
     exit(-1);
  }
}

void Shader::
programLog(GLuint _program)
{
  int status;
  glGetProgramiv(_program, GL_LINK_STATUS, &status);

  if (status == GL_FALSE)
  {
     /* Get the length of the info log. */
     int len;
     glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &len);

     /* Get the info log. */
     char* log = new char[len];
     glGetProgramInfoLog(_program, len, &len, log);

     /* Print error */
     cerr << "Link error: " << log << endl;

     /* Finally, free the memory allocated. */
     delete log;

     /* Exit the program. */
     exit(-1);
  }
}

Shader::
Shader()
  : _vert(0), _tess_control(0), _tess_eval(0), _geom(0), _frag(0), _comp(0)
{
  _program = glCreateProgram();
}
Shader::
~Shader()
{

}

Shader* Shader::
vertex(const char* file)
{
  _vert = loadShader(file, GL_VERTEX_SHADER);
  return this;
}
Shader* Shader::
tess_control(const char* file)
{
  _tess_control = loadShader(file, GL_TESS_CONTROL_SHADER);
  return this;
}
Shader* Shader::
tess_eval(const char* file)
{
  _tess_eval = loadShader(file, GL_TESS_EVALUATION_SHADER);
  return this;
}
Shader* Shader::
geometry(const char* file)
{
  _geom = loadShader(file, GL_GEOMETRY_SHADER);
  return this;
}
Shader* Shader::
fragment(const char* file)
{
  _frag = loadShader(file, GL_FRAGMENT_SHADER);
  return this;
}
Shader* Shader::
compute(const char* file)
{
  _comp = loadShader(file, GL_COMPUTE_SHADER);
  return this;
}

Shader* Shader::
build()
{
  if (_vert) glAttachShader(_program, _vert);
  if (_tess_control) glAttachShader(_program, _tess_control);
  if (_tess_eval) glAttachShader(_program, _tess_eval);
  if (_geom) glAttachShader(_program, _geom);
  if (_frag) glAttachShader(_program, _frag);
  if (_comp) glAttachShader(_program, _comp);

  glLinkProgram(_program);

  programLog(_program);

  use();

  _position = findAttribute("position");
  _normal = findAttribute("normal");

  return this;
}

void Shader::
destroy()
{
  glDeleteProgram(_program);
}

GLint Shader::
findAttribute(const char* name)
{
  return glGetAttribLocation(_program, name);
}
GLint Shader::
findUniform(const char* name)
{
  auto it = _uniforms.find(name);
  if (it == _uniforms.end())
  {
    GLint x = glGetUniformLocation(_program, name);
    _uniforms.emplace(name,x);
    return x;
  }
  return it->second;
}

void Shader::
updateMat4(const char* name, mat4 v)
{
  GLint x = findUniform(name);
  if (x != -1) glUniformMatrix4fv(x, 1, GL_FALSE, value_ptr(v));
}
void Shader::
updateFloat(const char* name, float v)
{
  GLint x = findUniform(name);
  if (x != -1) glUniform1f(x, v);
}
void Shader::
updateFloatArray(const char* name, const float *v, int size)
{
  GLint x = findUniform(name);
  if (x != -1) glUniform1fv(x, size, v);
}
void Shader::
updateInt(const char* name, int v)
{
  GLint x = findUniform(name);
  if (x != -1) glUniform1i(x,v);
}
void Shader::
updateInts(const char* name, const int *v, int size)
{
  GLint x = findUniform(name);
  if (x != -1) glUniform1iv(x,size,v);
}
void Shader::
updateVec3(const char* name, vec3 v)
{
  GLint x = findUniform(name);
  if (x != -1) glUniform3fv(x, 1, value_ptr(v));
}
void Shader::
updateVec4(const char* name, vec4 v)
{
  GLint x = findUniform(name);
  if (x != -1) glUniform4fv(x, 1, value_ptr(v));
}
void Shader::
updateVec3Array(const char* name, const vec3* arr, int size)
{
  GLint x = findUniform(name);
  if (x != -1)
  {
    float* floats = new float[3*size];
    for (int i = 0; i < size; i++)
    {
      floats[i*3] = arr[i].x; floats[i*3+1] = arr[i].y; floats[i*3+2] = arr[i].z;
    }
    glUniform3fv(x, size, floats);
    delete [] floats;
  }
}

GLint Shader::
getPositionLocation()
{
  return _position;
}
GLint Shader::
getNormalLocation()
{
  return _normal;
}

void Shader::
use()
{
  glUseProgram(_program);
}
