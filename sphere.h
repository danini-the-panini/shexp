#ifndef SPHERE_H
#define SPHERE_H

#include "gfx_include.h"
#include <vector>

class Sphere
{
public:
  Sphere();
  virtual ~Sphere();
  void build();
  void destroy();
  void draw(GLenum type=GL_TRIANGLES);

private:
  GLuint  _vao, _vbo, _ibo;
  GLsizei _num_indices;

  GLuint gen_and_bind_buffer(GLenum type);
  void send_vertices_to_buffer(const vector<GLfloat> &vertices);
  void send_indices_to_buffer(const vector<GLuint> &indices);
  void enable_position_attr();
  void build_sphere(vector<GLfloat> &vertices, vector<GLuint> &indices);

};

#endif
