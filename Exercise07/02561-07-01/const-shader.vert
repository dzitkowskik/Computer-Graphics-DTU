#version 150

uniform mat4 Projection;
uniform mat4 ModelView;

in vec4 position;

void main (void)
{
    gl_Position = Projection * ModelView * position;
}