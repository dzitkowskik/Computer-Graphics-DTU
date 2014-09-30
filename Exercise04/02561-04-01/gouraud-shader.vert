#version 150 

in  vec3 vPosition;
in  vec3 vNormal;
out vec4 color;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform float Shininess;

void main()
{
    gl_Position = Projection * ModelView * vec4(vPosition, 1.0);

	vec3 normal = normalize(transpose(inverse(mat3(ModelView))) * vNormal);
	//vec3 normal = normalize(ModelView * vec4(vNormal, 0.0)).xyz;
	
	vec3 position = vec3(ModelView * vec4(vPosition, 1.0));
	
	vec3 surfToLight = normalize(vec3(LightPosition) - position);
	vec3 surfToCamera = normalize(-position);
	vec3 reflection = reflect(-surfToLight, normal);

	// AMBIENT
	float ambientCoefficient = 1.0;
	
	// DIFFUSE
	float diffuseCoefficient = clamp(max(dot(normal, surfToLight), 0.0), 0.0, 1.0);

	// SPECULAR
	float specularCoefficient = diffuseCoefficient > 0 ? clamp(pow(max(dot(reflection, surfToCamera), 0.0), Shininess), 0.0, 1.0) : 0.0;

	// ATTENUATION
	float lightAttenuation = 0.05;
	float distanceToLight = length(surfToLight);
	float attenuation = 1.0 / (1.0 + lightAttenuation * pow(distanceToLight, 2));

	// LINEAR COLOR
    color = AmbientProduct + attenuation * (DiffuseProduct * diffuseCoefficient + SpecularProduct * specularCoefficient);
}
