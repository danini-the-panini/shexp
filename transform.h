#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "gfx_include.h"

class Transform
{
public:
  Transform(vec3 t = vec3(0,0,0), quat r = quat(0,0,0,1), vec3 s = vec3(1,1,1));
  virtual ~Transform();

  mat4 world();
private:
  vec3 _t;
  quat _r;
  vec3 _s;
};

#endif
