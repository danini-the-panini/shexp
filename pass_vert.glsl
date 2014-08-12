#version 440

in vec3 position;
in vec3 normal;

out vec3 g_normal;

void main()
{
    g_normal = normal;
    gl_Position = vec4(position,1);
}
