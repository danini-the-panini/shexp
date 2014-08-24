#include "ndc_quad.h"

NDCQuad::
NDCQuad()
  : Mesh(false)
{
}

NDCQuad::
~NDCQuad()
{
}

void NDCQuad::
build_impl(float_list &vertices, uint_list &indices)
{
  vertices.push_back(-1.0f);
  vertices.push_back(1.0f);
  vertices.push_back(0.99f);

  vertices.push_back(1.0f);
  vertices.push_back(1.0f);
  vertices.push_back(0.99f);

  vertices.push_back(1.0f);
  vertices.push_back(-1.0f);
  vertices.push_back(0.99f);

  vertices.push_back(-1.0f);
  vertices.push_back(-1.0f);
  vertices.push_back(0.99f);

  indices.push_back(0);
  indices.push_back(2);
  indices.push_back(1);

  indices.push_back(0);
  indices.push_back(3);
  indices.push_back(2);
}
