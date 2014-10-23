#version 150 

uniform mat4 ModelView;
uniform mat4 Projection;

in  vec3 vPosition;
in  vec3 vNormal;

out vec3 fragPosition;
out vec3 fragNormal;

void main()
{
	fragPosition = vPosition;
	fragNormal = vNormal;

    gl_Position = Projection * ModelView * vec4(vPosition, 1.0);
}
