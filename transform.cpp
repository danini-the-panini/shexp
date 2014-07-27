#include "transform.h"

Transform::
Transform(vec3 t, quat r, vec3 s)
  : _t(t), _r(r), _s(s)
{
}

Transform::
~Transform()
{
}

mat4 Transform::
world()
{
  mat4 world(1);

  world = translate(world, _t);
  world = mat4_cast(_r) * world;
  world = scale(world, _s);

  return world;
}
