#version 150

uniform sampler2D texture1;
uniform sampler2D shadowMap;
uniform mat4 lightViewProjection;

out vec4 fragColor;

in vec4 vWorldCoordinate;
in vec2 vTextureCoordinate;

void main(void) {
	if (!gl_FrontFacing){
		fragColor = vec4(1,1,1,1);
		return;
	}
	
	// todo find the shadow map UV coordinates
	mat4 biasMatrix;
	biasMatrix[0]=vec4(0.5, 0.0, 0.0, 0.0);
	biasMatrix[1]=vec4(0.0, 0.5, 0.0, 0.0);
	biasMatrix[2]=vec4(0.0, 0.0, 0.5, 0.0);
	biasMatrix[3]=vec4(0.5, 0.5, 0.5, 1.0);

	vec4 coordinates = biasMatrix * lightViewProjection * vWorldCoordinate;
	vec2 shadowUV = (coordinates / coordinates.w).xy;

	vec4 texColor = texture(texture1, vTextureCoordinate);
	vec4 shadowColor = texture(shadowMap, shadowUV);

	if(shadowColor.x == 1.0)
	{
		fragColor = texColor;
	}
	else
	{
		fragColor = mix(texColor, shadowColor, 0.5);
	}
}
