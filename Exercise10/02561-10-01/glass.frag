#version 150

uniform samplerCube cubemap;
uniform vec3 cameraPos;

in vec3 vPosition;
out vec4 fragColor;

void main(void)
{
	float air = 1.0;
	float glass = 1.62;
	float eta = air/glass;
	vec3 normal = normalize(vPosition);
	vec3 incidence = normalize(vPosition - cameraPos);
	vec4 reflection = texture(cubemap, reflect(incidence, normal));
	vec4 refraction = texture(cubemap, refract(incidence, normal, eta));

	// Schlick's approximation
	float cosFi = max(dot(normal, -incidence), 0.0);
	float R0 = pow((air - glass) / (air + glass), 2.0);
	float R = R0 + (1.0 - R0)*pow(1.0 - cosFi, 5.0);

	fragColor = R * reflection + (1 - R) * refraction;
}
