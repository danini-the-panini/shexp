#ifndef ORTHONORMALISE_H
#define ORTHONORMALISE_H

#include "gfx_include.h"

template<typename mat4_type>
mat4_type orthonormalise(mat4_type m)
{
  auto x = row(m,0).xyz();
  auto y = row(m,1).xyz();
  auto z = row(m,2).xyz();

  z = normalize(-cross(x,y));
  y = normalize(cross(x,z));

  return mat4_type(
      x.x, y.x, z.x, 0,
      x.y, y.y, z.y, 0,
      x.z, y.z, z.z, 0,
      0, 0, 0, 1
  );
}

template <typename T, precision P>
tvec3<T,P> project(tvec3<T,P> u, tvec3<T,P> v)
{
  T a = dot(u, v);
  T b = dot(u, u);

  const float EPSILON = 0.000005f;
  if (abs(b) < EPSILON)
  {
    return tvec3<T,P>(0, 0, 0);
  }
  else
  {
    T s = a / b;
    return u * s;
  }
}

template<typename mat4_type>
mat4_type orthonormalise_gram_schmidt(mat4_type m)
{
  auto v_1 = column(m,0).xyz();
  auto v_2 = column(m,1).xyz();
  auto v_3 = column(m,2).xyz();

  auto u_1 = v_1;
  auto u_2 = v_2 - project(u_1, v_2);
  auto u_3 = v_3 - project(u_1, v_3) - project(u_2, v_3);

  auto e_1 = normalize(u_1);
  auto e_2 = normalize(u_2);
  auto e_3 = normalize(u_3);

  return mat4_type(
    e_1.x, e_1.y, e_1.z, 0,
    e_2.x, e_2.y, e_2.z, 0,
    e_3.x, e_3.y, e_3.z, 0,
    0, 0, 0, 1
  );
}

#endif
