#version 440

layout (location = 0) out vec4 out_color;

uniform sampler2D shexps;
uniform int screen_width;
uniform int screen_height;
uniform vec3 color;

void main()
{
  vec2 screen = vec2( gl_FragCoord.x / float(screen_width),
      gl_FragCoord.y / float(screen_height));

  out_color = vec4(color * texture(shexps, screen).r, 1);
}
