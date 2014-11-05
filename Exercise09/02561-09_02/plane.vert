#version 150

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

in vec3 position;
in vec2 textureCoordinate;

out vec4 vWorldCoordinate;
out vec2 vTextureCoordinate;

void main(void)
{
	gl_Position = projection * view * model * vec4(position, 1);
	vTextureCoordinate = textureCoordinate;
	vWorldCoordinate = model * vec4(position, 1);
}