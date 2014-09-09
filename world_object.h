#ifndef WORLD_OBJECT_H
#define WORLD_OBJECT_H

#include "gfx_include.h"
#include "mesh.h"
#include "shader.h"
#include "transform.h"

class WorldObject
{
public:
  WorldObject(Mesh*, Transform*);
  virtual ~WorldObject();

  void draw(Shader*);

  vec3 color;

protected:
  Mesh *_mesh;
  Transform *_transform;
};

#endif
