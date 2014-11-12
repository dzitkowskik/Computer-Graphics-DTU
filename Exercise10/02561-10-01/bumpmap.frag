#version 150

uniform samplerCube cubemap;
uniform sampler2D textureBump;
uniform vec3 cameraPos;

in vec3 vNormal;

out vec4 fragColor;

void main(void)
{
	// todo replace this
	vec3 normalizedNormal = (vNormal + vec3(1.0,1.0,1.0)) / 2.0;
	fragColor = vec4(normalizedNormal, 1.0);
}