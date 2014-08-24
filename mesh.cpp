#include "mesh.h"

Mesh::
Mesh(bool normals)
  : _has_normals(normals)
{
}

Mesh::
~Mesh()
{
}

void Mesh::
build()
{
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  vector<GLfloat> vertices;
  vector<GLuint> indices;
  build_impl(vertices, indices);
  _num_indices = (GLsizei)indices.size();

  _vbo = gen_and_bind_buffer(GL_ARRAY_BUFFER);
  send_vertices_to_buffer(vertices);

  _ibo = gen_and_bind_buffer(GL_ELEMENT_ARRAY_BUFFER);
  send_indices_to_buffer(indices);

  enable_position_attr();
  if (_has_normals) enable_normal_attr();
}

void Mesh::
destroy()
{
  glDeleteBuffers(1, &_vbo);
  glDeleteBuffers(1, &_ibo);
  glDeleteVertexArrays(1, &_vao);
}

void Mesh::
draw(GLenum type)
{
  glBindVertexArray(_vao);

  if (type == GL_PATCHES)
    glPatchParameteri(GL_PATCH_VERTICES, 3);

  glDrawElements(type, _num_indices, GL_UNSIGNED_INT, 0);
}


// protected

  GLuint Mesh::
  gen_and_bind_buffer(GLenum type)
  {
    GLuint ptr;
    glGenBuffers(1, &ptr);
    glBindBuffer(type, ptr);

    return ptr;
  }

  void Mesh::
  send_vertices_to_buffer(const vector<GLfloat> &vertices)
  {
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
            vertices.data(), GL_STATIC_DRAW);
  }

  void Mesh::
  send_indices_to_buffer(const vector<GLuint> &indices)
  {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
            indices.data(), GL_STATIC_DRAW);
  }

  void Mesh::
  enable_position_attr()
  {
    GLsizei stride = _has_normals ? 6 : 3;
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat), 0);
  }

  void Mesh::
  enable_normal_attr()
  {
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
        (GLvoid*) (3 * sizeof(GLfloat)));
  }
