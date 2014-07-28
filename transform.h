#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "gfx_include.h"

class Transform
{
public:
  Transform(vec3 t = vec3(0,0,0), quat r = quat(0,0,0,1), vec3 s = vec3(1,1,1));
  virtual ~Transform();

  mat4 world();

  vec3 get_translation();
  void set_translation(vec3);

  quat get_rotation();
  void set_rotation(quat);

  vec3 get_scale();
  void set_scale(vec3);

private:
  vec3 _t;
  quat _r;
  vec3 _s;
  mat4 _world;
  bool _dirty;

  void recalculate_world();
};

#endif
