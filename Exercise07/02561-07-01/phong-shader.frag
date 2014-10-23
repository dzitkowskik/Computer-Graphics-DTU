#version 150 

uniform mat4 ModelView;
uniform vec4 MaterialColor;
uniform vec4 MaterialSpecularColor;
uniform float Shininess;

#define MAX_LIGHTS 5
uniform int NumLights;
uniform struct Light {
	vec3 position;
	vec3 color;
	float lightType;
	float attenuation;
	float ambientCoefficient;
} Lights[MAX_LIGHTS];

in vec3 fragPosition;
in vec3 fragNormal;

out vec4 fragColor;


vec3 ApplyLight(Light light, vec3 normal, vec3 position, vec3 lightPosition)
{
	// directional
	vec3 surfToLight = normalize(lightPosition);
	float attenuation = 1.0;

	if(light.lightType == 1.0) // point
	{
		surfToLight = normalize(lightPosition - position);
		float distance = length(lightPosition - position);
		attenuation = 1.0 / (1.0 + light.attenuation * pow(distance, 2));
	}

	vec3 surfToCamera = normalize(position);
	vec3 reflection = reflect(surfToLight, normal);

	// AMBIENT
	vec3 ambient = light.ambientCoefficient * MaterialColor.xyz * light.color;

	// DIFFUSE
	float diffuseCoefficient = clamp(max(dot(normal, surfToLight), 0.0), 0.0, 1.0);
	vec3 diffuse = diffuseCoefficient * MaterialColor.xyz * light.color;

	// SPECULAR
	float specularCoefficient = diffuseCoefficient > 0 ? clamp(pow(max(dot(reflection, surfToCamera), 0.0), Shininess), 0.0, 1.0) : 0.0;
	vec3 specular = specularCoefficient * MaterialSpecularColor.xyz * light.color;

	// LINEAR COLOR
	return ambient + attenuation*(diffuse + specular);
}


void main() 
{ 
	vec3 normal = normalize(transpose(inverse(mat3(ModelView))) * fragNormal);
	vec3 position = vec3(ModelView * vec4(fragPosition, 1.0));
	
	
	vec3 linearColor = vec3(0);
	for(int i = 0; i < NumLights; ++i)
	{
		vec3 lightPosition = vec3(ModelView * vec4(Lights[i].position, 1.0));
		linearColor += ApplyLight(Lights[i], normal, position, lightPosition);
	}

	fragColor = vec4(linearColor, 1.0);
} 

