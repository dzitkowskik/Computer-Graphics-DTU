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

    color = vec4(0.0,1.0,0.0,1.0);
}
