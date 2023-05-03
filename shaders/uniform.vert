#version 310 es
precision highp float;

layout (location = 0) in vec2 aPos;

out vec3 color;

uniform float value;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0, 1.0);
    color = vec3(abs(aPos.x+0.5), abs(aPos.y+0.5), 1.0)*value;
}