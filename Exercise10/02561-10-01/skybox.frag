#version 150

uniform samplerCube cubemap;

out vec4 fragColor;
in vec3 vObjPos;

void main(void)
{
	fragColor = texture(cubemap, normalize(vObjPos));
}
