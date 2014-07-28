#include "transform.h"

Transform::
Transform(vec3 t, quat r, vec3 s)
  : _t(t), _r(r), _s(s)
{
  recalculate_world();
}

Transform::
~Transform()
{
}

mat4 Transform::
world()
{
  if (_dirty) recalculate_world();
  return _world;
}

vec3 Transform::
get_translation()
{
  return _t;
}

void Transform::
set_translation(vec3 t)
{
  _t = t;
  _dirty = true;
}

quat Transform::
get_rotation()
{
  return _r;
}

void Transform::
set_rotation(quat r)
{
  _r = r;
  _dirty = true;
}

vec3 Transform::
get_scale()
{
  return _s;
}

void Transform::
set_scale(vec3 s)
{
  _s = s;
  _dirty = true;
}

// private

  void Transform::
  recalculate_world()
  {
    _world = mat4(1);

    _world = translate(_world, _t);
    _world = mat4_cast(_r) * _world;
    _world = scale(_world, _s);

    _dirty = false;
  }
