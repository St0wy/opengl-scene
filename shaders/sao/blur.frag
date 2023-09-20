#version 430

const float EDGE_SHARPNESS = 1.0;
const int SCALE = 2;
const int RADIUS = 4;

const float GAUSSIAN[RADIUS + 1] = float[](0.153170, 0.144893, 0.122649, 0.092902, 0.062970);

uniform sampler2D gSsao;

// (1, 0) or (0, 1)
uniform ivec2 axis;

float UnpackKey(vec2 p);



// Returns a number on (0, 1)
float UnpackKey(vec2 p)
{
    return p.x * (256.0 / 257.0) + p.y * (1.0 / 257.0);
}

void main()
{
}