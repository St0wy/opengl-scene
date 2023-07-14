#version 430

const int BLUR_SIZE = 4;
const int END = (BLUR_SIZE / 2);
const int START = -END;

out float FragColor;

in vec2 TexCoords;

uniform sampler2D gSsao;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(gSsao, 0));
	float result = 0.0;
	for (int x = START; x < END; x++)
	{
		for (int y = START; y < END; y++)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(gSsao, TexCoords + offset).r;
		}
	}

	FragColor = result / (float(BLUR_SIZE) * float(BLUR_SIZE));
}
