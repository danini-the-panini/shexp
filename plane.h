#ifndef PLANE_H
#define PLANE_H

#include "mesh.h"

class Plane : public Mesh
{
public:
  typedef Mesh::float_list float_list;
  typedef Mesh::uint_list uint_list;

  Plane();
  virtual ~Plane();

  virtual void build_impl(float_list&, uint_list&);
};

#endif
