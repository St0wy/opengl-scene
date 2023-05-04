#version 310 es
precision highp float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 inColor;

out vec3 color;

uniform float value;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    color = inColor * value;
}