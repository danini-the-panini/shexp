#version 330

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

layout (location = 1) in vec3 position;
layout (location = 2) in vec3 normal;

out vec3 v_normal;
out vec3 v_position;
out vec3 v_eye;

void main()
{
  v_eye = (inverse(view) * vec4 (0, 0, 1, 1)).xyz;

  v_normal = (world * vec4(normal,0)).xyz;
  v_position = (world * vec4(position,1)).xyz;
  gl_Position = projection * view * world * vec4(position,1);
}

