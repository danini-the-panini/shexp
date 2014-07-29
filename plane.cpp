#include "plane.h"

Plane::
Plane()
{
}

Plane::
~Plane()
{
}

void Plane::
build_impl(float_list &vertices, uint_list &indices)
{
  vertices.push_back(-0.5f);
  vertices.push_back(0);
  vertices.push_back(-0.5);

  vertices.push_back(0.5f);
  vertices.push_back(0);
  vertices.push_back(-0.5);

  vertices.push_back(0.5f);
  vertices.push_back(0);
  vertices.push_back(0.5);

  vertices.push_back(-0.5f);
  vertices.push_back(0);
  vertices.push_back(0.5);

  indices.push_back(0);
  indices.push_back(1);
  indices.push_back(2);

  indices.push_back(0);
  indices.push_back(2);
  indices.push_back(3);
}
