#version 300 es
precision highp float;

layout (location = 0) in vec3 aPos;

//out vec3 color;

//uniform float value;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    //    color = vec3(abs(aPos.x+0.5), abs(aPos.y+0.5), 1.0)*value;
    //    color = vec3(1.0, 1.0, 1.0);
}