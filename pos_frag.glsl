#version 440

layout (location = 0) out vec4 out_colour;

uniform int screen_width;
uniform int screen_height;
uniform float zfar;

in vec3 v_position;

void main()
{
  float x = gl_FragCoord.x / float(screen_width);
  float y = gl_FragCoord.y / float(screen_height);
  float z = (gl_FragCoord.z / gl_FragCoord.w) / zfar;

  out_colour = vec4(x, y, z, 1);
}
