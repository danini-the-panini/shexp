#ifndef NDC_QUAD_H
#define NDC_QUAD_H

#include "mesh.h"

class NDCQuad : public Mesh
{
typedef Mesh::float_list float_list;
typedef Mesh::uint_list uint_list;

public:
  NDCQuad();
  ~NDCQuad();

  void build_impl(float_list &vertices, uint_list &indices);

protected:

};

#endif
