#version 440
#define PI (3.1415926535897932384626433832795)

layout (location = 0) out vec4 sh0_3;
layout (location = 1) out vec4 sh4_7;
layout (location = 2) out vec4 sh8_11;
layout (location = 3) out vec4 sh12_15;

const int N_BANDS = 4;
const int N_COEFFS = N_BANDS*N_BANDS;

uniform sampler2D position;
uniform sampler2D normal;

uniform int screen_width;
uniform int screen_height;

uniform float proxy_radius;
uniform vec3 proxy_position;
uniform float oracle_factor;

uniform sampler2D sh_lut;

float[N_COEFFS] window(float[N_COEFFS] c)
{
  float h = 2.0*N_BANDS;
  float[N_COEFFS] result;
  for (int l = 0; l < N_BANDS; l++)
  {
    float a = cos(PI/2.0*(float(l)/h));
    for (int m = -l; m <= l; m++)
    {
      int i = l*(l+1)+m;
      result[i] = c[i] * a;
    }
  }
  return result;
}

// assume v is normalized!
float[N_COEFFS] y(vec3 vo)
{
  vec3 v = vo.xzy;
  return float[N_COEFFS](
    0.5*sqrt(1.0/PI),

    sqrt(3.0/(4.0*PI)) * v.y,
    sqrt(3.0/(4.0*PI)) * v.z,
    sqrt(3.0/(4.0*PI)) * v.x,

    0.5*sqrt(15.0/PI)*v.x*v.y,
    0.5*sqrt(15.0/PI)*v.y*v.z,
    0.25*sqrt(5.0/PI)*(-(v.x*v.x)-(v.y*v.y)+2.0*(v.z*v.z)),
    0.5*sqrt(15.0/PI)*v.z*v.x,
    0.25*sqrt(15.0/PI)*((v.x*v.x)-(v.y*v.y)),

    0.25*sqrt(35.0/(2.0*PI))*(3.0*v.x*v.x - v.y*v.y)*v.y,
    0.5*sqrt(105.0/PI)*v.x*v.y*v.z,
    0.25*sqrt(21.0/(2.0*PI))*v.y*(4.0*v.z*v.z-v.x*v.x-v.y*v.y),
    0.25*sqrt(7.0/PI)*v.z*(2.0*v.z*v.z-3.0*v.x*v.x-3.0*v.y*v.y),
    0.25*sqrt(21.0/(2.0*PI))*v.x*(4.0*v.z-v.z-v.x*v.x-v.y*v.y),
    0.25*sqrt(105.0/PI)*(v.x*v.x-v.y*v.y)*v.z,
    0.25*sqrt(35.0/(2.0*PI))*(v.x*v.x-3.0*v.y*v.y)*v.x

    // ...
  );
}

float[N_COEFFS] rotate_to(float[N_COEFFS] sh, vec3 v)
{
  float[N_COEFFS] yv = y(v);
  float sh0 = sh[0]/sqrt(4.0*PI);
  float sh1 = sh[2]/sqrt((4.0*PI)/3.0);
  float sh2 = sh[6]/sqrt((4.0*PI)/5.0);
  float sh3 = sh[12]/sqrt((4.0*PI)/7.0);
  // ...

  return float[N_COEFFS](
    sh0*yv[0],
    sh1*yv[1],sh1*yv[2],sh1*yv[3],
    sh2*yv[4],sh2*yv[5],sh2*yv[6],sh2*yv[7],sh2*yv[8],
    sh3*yv[9],sh3*yv[10],sh3*yv[11],sh3*yv[12],sh3*yv[13],sh3*yv[14],sh3*yv[15]
    // ...
  );
}

float angular_radius(float d, float r)
{
  return asin(r/d);
}

float[N_COEFFS] get_coeff(vec3 v, float radius)
{
  float d = length(v);
  float ar = angular_radius(d, radius);
  float fi = clamp(ar/(PI*0.5),0,1);
  float[N_COEFFS] log_coeff;
  for (int j = 0; j < N_COEFFS; j++)
  {
    log_coeff[j] = texture(sh_lut, vec2((float(j)+0.5)/float(N_COEFFS),fi)).r;
  }
  return rotate_to(log_coeff, normalize(v));
}

void main()
{
  vec2 screen = vec2( gl_FragCoord.x / float(screen_width),
      gl_FragCoord.y / float(screen_height));

  vec3 p_position = texture(position, screen).xyz;
  vec3 p_normal = texture(normal, screen).xyz;

  vec3 v = proxy_position - p_position;

  float v_length = length(v);

  if (v_length > proxy_radius*oracle_factor)
    discard;

  float v_dot_normal = dot(v, p_normal);

  // behind tangent plane
  if (v_dot_normal < -proxy_radius)
    discard;

  // straddling and containing
  if (v_dot_normal < 0 && v_length < proxy_radius)
    discard;

  vec3 p = proxy_position;
  float r = proxy_radius;

  // straddling and not containing
  if (v_dot_normal < 0)
  {
    p += p_normal * (r - v_dot_normal)/2;
    r = (r + v_dot_normal)/2;
  } else if (v_dot_normal < r)
  {
    p += p_normal * (r - v_dot_normal)/2;
    r -= (r - v_dot_normal)/2;
  }

  v = p - p_position;

  float[N_COEFFS] f = get_coeff(v, r);

  sh0_3 = vec4(f[0], f[1], f[2], f[3]);
  sh4_7 = vec4(f[4], f[5], f[6], f[7]);
  sh8_11 = vec4(f[8], f[9], f[10], f[11]);
  sh12_15 = vec4(f[12], f[13], f[14], f[15]);
}

