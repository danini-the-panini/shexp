#version 440

#define EPSILON 0.01

layout (location = 0) out vec4 out_color;

uniform sampler2D shexps;
uniform sampler2D normal;
uniform int screen_width;
uniform int screen_height;
uniform int buf_width;
uniform int buf_height;
uniform vec3 color;

float bilateral_filter(vec2 uv)
{
  vec2 buf_uv = uv*vec2(buf_width, buf_height);
  ivec2 p_course = ivec2(buf_uv.x, buf_uv.y);
  vec2 ratio = buf_uv - p_course;

  float v_course[4] =
  {
    texelFetch(shexps, p_course, 0).r,
    texelFetch(shexps, p_course+ivec2(0,1), 0).r,
    texelFetch(shexps, p_course+ivec2(1,0), 0).r,
    texelFetch(shexps, p_course+ivec2(1,1), 0).r
  };

  float a1 = ratio.x;
  float a2 = (1.0 - ratio.x);
  float b1 = ratio.y;
  float b2 = (1.0 - ratio.y);

  float bilinear_weights[4] =
  {
    a2 * b2, a2 * b1, a1 * b2, a1 * b1
  };

  float c = 0.0;
  for (int i = 0; i < 4; i++)
  {
    c += v_course[i] * bilinear_weights[i];
  }

  vec4 norm_depth = texture(normal, uv);
  vec4 nd_course[4] =
  {
    texture(normal, p2uv(p_course), 0),
    texture(normal, p2uv(p_course+ivec2(0,1)), 0),
    texture(normal, p2uv(p_course+ivec2(1,0)), 0),
    texture(normal, p2uv(p_course+ivec2(1,1)), 0)
  };

  float final_value = 0.0;
  float total_weight = 0.0;
  for(int i=0;i<4;i++)
  {
    float normal_weight = dot(nd_course[i].xyz, norm_depth.xyz);
    normal_weight = pow(normal_weight, 32);

    float depth_diff = norm_depth.a - nd_course[i].a;
    float depth_weight = 1.0/(EPSILON + abs(depth_diff));

    float weight = normal_weight * depth_weight * bilinear_weights[i];
    total_weight += weight;
    final_value += v_course[i]*weight;
  }

  return final_value / total_weight;
}

void main()
{
  vec2 screen = vec2( gl_FragCoord.x / float(screen_width),
      gl_FragCoord.y / float(screen_height));

  /*out_color = vec4(color * bilateral_filter(screen), 1);*/
  out_color = vec4(color * bilateral_filter(screen), 1);
}
