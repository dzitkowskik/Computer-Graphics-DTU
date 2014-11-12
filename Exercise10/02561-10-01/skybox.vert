#version 150

uniform mat4 projection;
uniform mat4 modelView;

in vec3 position;
out vec3 vObjPos;

void main(void)
{
	vObjPos = position;
	gl_Position = projection * modelView * vec4(position,1.0);
}