#ifndef _NURBSRenderer_H
#define _NURBSRenderer_H

#include "NURBS.h"

/// Renders a NURBS object
class NURBSRenderer
{
public:
	NURBSRenderer(NURBS * nurbs);
	~NURBSRenderer(void);

	/// reload the nurbs data. Must be called when the NURBS object has been modified.
	void reloadData();

	/// Render the curve using the projection and modelView transforms
	/// If curve, then the light position is ignored
	void render(mat4 &projection, mat4 &modelView, vec4 lightPosition = vec4(0));

	/// Render the control points
	void renderControlPoints(mat4 &projection, mat4 &modelView, float pointSize = 5.0f);

	/// Render the normals
	void renderNormals(mat4 &projection, mat4 &modelView);

	// set the color of the geometry
	void setColor(vec4 color);

	// get the color of the geometry
	vec4 getColor();

	int getVertexCount() { return vertexCount; }
private:
	void setupShader();
	NURBS * nurbs;

	GLuint vao;
	GLuint vertexBuffer;
	int vertexCount;

	GLuint vaoControlPoints;
	GLuint controlPointVertexBuffer;
	int controlPointVertexCount;

	GLuint vaoNormals;
	GLuint normalsVertexBuffer;
	int normalCount;
	
	vec4 color;

	static GLuint shaderProgram;
	static GLuint projectionUniform,
		modelViewUniform,
		lightPositionUniform,
		colorUniform;

	static GLuint positionAttribute, 
		normalAttribute, 
		uvAttribute;

	std::vector<GLuint> meshDataIndices;
	GLenum primitiveType; // lines or triangle strips
};

#endif // _NURBSRenderer_H
