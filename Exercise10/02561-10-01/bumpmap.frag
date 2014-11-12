#version 150

uniform samplerCube cubemap;
uniform sampler2D textureBump;
uniform vec3 cameraPos;

in vec3 vPosition;
out vec4 fragColor;

vec3 bumpMap(vec3 normal, vec3 position)
{
	vec3 n = normalize(normal);
	float radius = length(vec3(position));
	float PI = 3.14159265;
	vec2 tangent2 = vec2(acos(position.z/radius), atan(position.y, position.x));
	vec2 tangentUV = vec2(tangent2.x / (2*PI), tangent2.y / PI);
	vec3 T = vec3(cos(tangent2.x)*cos(tangent2.y), cos(tangent2.x)*sin(tangent2.y),-sin(tangent2.x));
	vec3 B = vec3(-sin(tangent2.x)*sin(tangent2.y), sin(tangent2.x)*cos(tangent2.y), 0.0);
	mat3 tbn = mat3(T,B,n);
	vec3 nn = texture(textureBump,tangentUV).xyz;
	vec3 V = (nn - vec3(0.5, 0.5, 0.5))*2.0;
	return normalize(tbn*V);
}

void main(void)
{
	// normal mapping - bump mapping
	vec3 normal = bumpMap(normalize(vPosition), vPosition);

	// glass effect
	float air = 1.0;
	float glass = 1.62;
	float eta = air/glass;
	float disp = 0.03;
	
	vec3 incidence = normalize(vPosition - cameraPos);
	vec4 reflection = texture(cubemap, reflect(incidence, normal));
	
	// refraction with chromatic dispersion
	vec4 refractionR = texture(cubemap, refract(incidence, normal, eta));
	vec4 refractionG = texture(cubemap, refract(incidence, normal, eta-disp));
	vec4 refractionB = texture(cubemap, refract(incidence, normal, eta-(2*disp)));
	vec4 refraction = vec4(refractionR.x, refractionG.y, refractionB.z, 1.0);

	// Schlick's approximation
	float cosFi = max(dot(normal, -incidence), 0.0);
	float R0 = pow((air - glass) / (air + glass), 2.0);
	float R = R0 + (1.0 - R0)*pow(1.0 - cosFi, 5.0);

	fragColor = R * reflection + (1 - R) * refraction;
}
