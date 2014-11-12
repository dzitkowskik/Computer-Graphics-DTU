#version 150

uniform samplerCube cubemap;
uniform vec3 cameraPos;

in vec3 vPosition;
out vec4 fragColor;

void main(void)
{
	vec3 normal = normalize(vPosition);
	vec3 incidence = normalize(vPosition - cameraPos);
	vec3 reflection = normalize(reflect(incidence, normal));
	fragColor = texture(cubemap, reflection);
}
