#ifndef SPHERE_H
#define SPHERE_H

#include "gfx_include.h"
#include "mesh.h"
#include <vector>

class Sphere : public Mesh
{
public:
  typedef Mesh::float_list float_list;
  typedef Mesh::uint_list uint_list;

  Sphere();
  virtual ~Sphere();
  virtual void build_impl(float_list&, uint_list&);
};

#endif
