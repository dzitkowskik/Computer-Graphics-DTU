#version 150
// 02561-03-01

uniform mat4 projection;
uniform mat4 modelView;
uniform float blendValue;
uniform float normalExtrusion;

in vec3 position1;
in vec3 color1;
in vec3 position2;
in vec3 color2;
in vec3 normal1;
in vec3 normal2;

out vec3 colorV;
out vec3 position;

void main (void) {
	vec3 blendVector = vec3(blendValue);
    colorV = (1 - blendVector) * color1 + (blendVector * color2);
	vec3 normal = (1 - blendVector) * normal1 + (blendVector * normal2);
	position = (1 - blendVector) * position1 + (blendVector * position2) + (normal * normalExtrusion);
	gl_Position = projection * modelView * vec4(position, 1.0);
}