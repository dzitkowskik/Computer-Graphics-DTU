#version 150

uniform vec4 lightPosition;
uniform vec4 color;

in vec3 vNormal;
in vec3 vPos;

out vec4 fragColor;

void main(void) {
	fragColor = color;
}
