#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec3 v_color;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    //v_color = color.xyz;
    v_color = vec3(0.0f, 0.0f, 1.0f);
    gl_Position = ubuf.mvp * position;
}
