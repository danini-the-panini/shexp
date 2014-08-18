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
build_impl(float_list &vertices, uint_list &indices)
{
  const GLuint segments = 32;
  const GLfloat rad_per_lng = (2.f*(GLfloat)M_PI) / (GLfloat)segments;
  const GLfloat rad_per_lat = ((GLfloat)M_PI) / (GLfloat)segments;

  for (GLuint lat = 0; lat <= segments; lat++)
  {
    GLfloat lat_sin = (GLfloat)sin(lat * rad_per_lat - M_PI/2.0),
            lat_cos = (GLfloat)cos(lat * rad_per_lat - M_PI/2.0);

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

      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
    }
  }

  for (GLuint row = 0; row <= segments; row++)
  {
    for (GLuint col = 0; col <= segments; col++)
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
