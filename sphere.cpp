#include "sphere.h"

Sphere::
Sphere()
{
}

Sphere::
~Sphere()
{
}

void Sphere::
build()
{
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  vector<GLfloat> vertices;
  vector<GLuint> indices;
  build_sphere(vertices, indices);
  _num_indices = (GLsizei)indices.size();

  _vbo = gen_and_bind_buffer(GL_ARRAY_BUFFER);
  send_vertices_to_buffer(vertices);

  _ibo = gen_and_bind_buffer(GL_ELEMENT_ARRAY_BUFFER);
  send_indices_to_buffer(indices);

  enable_position_attr();
}

void Sphere::
destroy()
{
  glDeleteBuffers(1, &_vbo);
  glDeleteBuffers(1, &_ibo);
  glDeleteVertexArrays(1, &_vao);
}

void Sphere::
draw(GLenum type)
{
  glBindVertexArray(_vao);

  if (type == GL_PATCHES)
    glPatchParameteri(GL_PATCH_VERTICES, 3);

  glDrawElements(type, _num_indices, GL_UNSIGNED_INT, 0);
}

// private

  GLuint Sphere::
  gen_and_bind_buffer(GLenum type)
  {
    GLuint ptr;
    glGenBuffers(1, &ptr);
    glBindBuffer(type, ptr);

    return ptr;
  }

  void Sphere::
  send_vertices_to_buffer(const vector<GLfloat> &vertices)
  {
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
            vertices.data(), GL_STATIC_DRAW);
  }

  void Sphere::
  send_indices_to_buffer(const vector<GLuint> &indices)
  {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
            indices.data(), GL_STATIC_DRAW);
  }

  void Sphere::
  enable_position_attr()
  {
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  }

  void Sphere::
  build_sphere(vector<GLfloat> &vertices, vector<GLuint> &indices)
  {
    const GLuint segments = 32;
    const GLfloat rad_per_lng = (2*(GLfloat)M_PI) / (GLfloat)segments;
    const GLfloat rad_per_lat = ((GLfloat)M_PI) / (GLfloat)segments;

    for (GLuint lat = 0; lat <= segments; lat++)
    {
      GLfloat lat_sin = (GLfloat)sin(lat * rad_per_lat),
              lat_cos = (GLfloat)cos(lat * rad_per_lat);

      for (GLuint lng = 0; lng <= segments; lng++)
      {
        GLfloat lng_sin = (GLfloat)sin(lng * rad_per_lng),
                lng_cos = (GLfloat)cos(lng * rad_per_lng);

        GLfloat x = lat_cos * lng_cos,
                y = lat_sin,
                z = lat_cos * lng_sin;

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
      }
    }

    for (GLuint row = 0; row < segments; row++)
    {
      for (GLuint col = 0; col < segments; col++)
      {
        indices.push_back(row*segments + col);
        indices.push_back(row*segments + col + segments + 1);
        indices.push_back(row*segments + col + segments);

        indices.push_back(row*segments + col);
        indices.push_back(row*segments + col + 1);
        indices.push_back(row*segments + col + segments + 1);
      }
    }
  }
