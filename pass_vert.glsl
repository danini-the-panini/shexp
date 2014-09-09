#version 440

layout (location=1) in vec3 position;

out vec3 v_position;
out vec3 v_normal;

void main()
{
    v_position = position;
    gl_Position = vec4(position,1.0f);
}
