#ifndef MESH_H
#define MESH_H

#include "gfx_include.h"
#include <vector>

class Mesh
{
public:
  typedef vector<GLfloat> float_list;
  typedef vector<GLuint> uint_list;

  Mesh();
  virtual ~Mesh();
  virtual void build();
  virtual void destroy();
  virtual void draw(GLenum type=GL_TRIANGLES);

  virtual void build_impl(float_list&, uint_list&) = 0;

protected:
  GLuint  _vao, _vbo, _ibo;
  GLsizei _num_indices;

  GLuint gen_and_bind_buffer(GLenum type);
  void send_vertices_to_buffer(const float_list &vertices);
  void send_indices_to_buffer(const uint_list &indices);
  void enable_position_attr();
};

#endif
