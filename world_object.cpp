#include "world_object.h"

WorldObject::
WorldObject(Mesh *mesh, Transform *transform)
  : _mesh(mesh), _transform(transform)
{
  color = vec3(1,1,1);
}

WorldObject::
~WorldObject()
{
}

void WorldObject::
draw(Shader *shader)
{
  shader->updateMat4("world", _transform->world());
  shader->updateVec3("color", color);
  _mesh->draw();
}
