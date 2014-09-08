#ifndef WAVEFRONT_MESH_H
#define WAVEFRONT_MESH_H

#include "gfx_include.h"
#include "mesh.h"
#include <vector>

class WavefrontMesh : public Mesh
{
public:
  typedef Mesh::float_list float_list;
  typedef Mesh::uint_list uint_list;

  WavefrontMesh(const char *);
  virtual ~WavefrontMesh();

  virtual void build_impl(float_list&, uint_list&);

protected:
  const char* _filename;
};

#endif
