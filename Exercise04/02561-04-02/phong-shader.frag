#version 150 

uniform mat4 ModelView;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 LightPosition;
uniform float Shininess;

in vec3 fragPosition;
in vec3 fragNormal;

out vec4 fragColor;

void main() 
{ 
	vec3 normal = normalize(transpose(inverse(mat3(ModelView))) * fragNormal);
	vec3 position = vec3(ModelView * vec4(fragPosition, 1.0));
	
	// LIGHT 
	vec3 light = vec3(LightPosition);
	if(LightPosition.w == 1.0)
		light = vec3(LightPosition) - position;

	// VECTORS
	vec3 surfToLight = normalize(light);
	vec3 surfToCamera = normalize(position);
	vec3 reflection = reflect(surfToLight, normal);

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
    fragColor = AmbientProduct + attenuation * (DiffuseProduct * diffuseCoefficient + SpecularProduct * specularCoefficient);
} 

