#version 310 es
precision highp float;

in vec2 texCoord;
out vec4 FragColor;

uniform float value;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
        FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), value);
}