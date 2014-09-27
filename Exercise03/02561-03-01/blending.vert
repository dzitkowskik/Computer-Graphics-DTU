#version 150
// 02561-03-01

uniform mat4 projection;
uniform mat4 modelView;
uniform float blendValue;

in vec3 position1;
in vec3 color1;
in vec3 position2;
in vec3 color2;

out vec3 colorV;

void main (void) {
    colorV = color1;
    gl_Position = projection * modelView * vec4(position1, 1.0);
}