#version 150
// 02561-03-01

in vec3 colorV;
out vec4 fragColor;

void main(void)
{
    fragColor = vec4(colorV, 1.0);
}