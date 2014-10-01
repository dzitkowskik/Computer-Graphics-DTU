// 02561-01-01 Hello triangle world

#include <iostream>
#include <string>


#include "Angel.h"

using namespace std;
using namespace Angel;

int WINDOW_WIDTH = 600;
int WINDOW_HEIGHT = 600;

GLuint shaderProgram;
GLuint colorAttribute, positionAttribute;
GLuint vertexArrayObject;

struct Vertex{
	vec2 position;
	vec3 color;
};

const int NUMBER_OF_VERTICES = 6;
Vertex vertexData[NUMBER_OF_VERTICES] = {
	{ vec2(-0.5, -0.5 ), vec3( 1.0, 0.0, 0.0) },
	{ vec2( 0.5, -0.5 ), vec3( 0.0, 1.0, 0.0 ) },
	{ vec2( 0.5,  0.5 ), vec3( 0.0, 0.0, 1.0 ) },
	{ vec2(-0.5, -0.5 ), vec3( 1.0, 1.0, 1.0) },
	{ vec2( -0.5, 0.5 ), vec3( 1.0, 1.0, 1.0 ) },
	{ vec2( 0.5,  0.5 ), vec3( 1.0, 1.0, 1.0 ) }
};

// forward declaration
void loadShader();
void display();
void loadBufferData();
void reshape(int W, int H);

void display() {	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(shaderProgram);

	glBindVertexArray(vertexArrayObject);
    
	// glDrawArrays with flag GL_TRIANGLES draws a triangle for each 3 points in VertexArray
	glDrawArrays(GL_TRIANGLES, 0, NUMBER_OF_VERTICES);
	
	glutSwapBuffers();

	Angel::CheckError();
}

void reshape(int windowWidth, int windowHeight) {
	glViewport(0, 0, windowWidth, windowHeight);
}

void loadBufferData() {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	GLuint  vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, NUMBER_OF_VERTICES * sizeof(Vertex), vertexData, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(positionAttribute);
	glEnableVertexAttribArray(colorAttribute);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);
	glVertexAttribPointer(colorAttribute  , 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)sizeof(vec2));
}

void loadShader() {
	shaderProgram = InitShader("color-shader.vert",  "color-shader.frag", "fragColor");
	colorAttribute = glGetAttribLocation(shaderProgram, "color");
	positionAttribute = glGetAttribLocation(shaderProgram, "position");
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
	);

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_3_2_CORE_PROFILE);
	glutCreateWindow("02561-01-01");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	Angel::InitOpenGL();
		
	loadShader();
	loadBufferData();

	Angel::CheckError();
	glutMainLoop();
}

// glGetAttribLocation - Returns the location of an attribute variable
// glClearColor — specify clear values for the color buffers
// glClear — clear buffers to preset values
// glUseProgram — Installs a program object as part of current rendering state
// glBindVertexArray - binds the vertex array object with name
// glDrawArrays — render primitives from array data
// glutSwapBuffers - swaps the buffers of the current window if double buffered
// glViewport - sets the viewport
// glGenVertexArrays — generate vertex array object names
// glGenBuffers — generate buffer object names
// glBindBuffer — bind a named buffer object
// glBufferData — creates and initializes a buffer object's data store

/* glEnableVertexAttribArray - enables the generic vertex attribute array
 * (By default, all client-side capabilities are disabled, including all generic vertex attribute arrays. 
 * If enabled, the values in the generic vertex attribute array will be accessed and used for rendering
 * when calls are made to vertex array commands such as glDrawArrays)
 */

// glVertexAttribPointer - specify the location and data format of the array of generic vertex attributes at index index to use when rendering
// Angel::InitShader - initializes shader programs stored in files passed as attributes
// glutInit - initializes the GLUT library
// glutInitContextVersion - sets openGL version we use
// glutInitContextFlags - sets flags in GLUT library
// glutInitContextProfile - sets profile of GLUT library
// glutInitDisplayMode - sets the initial display mode
// glutCreateWindow - creates a top-level window with a name passed as argument.
// glutDisplayFunc - sets the display callback for the current window
// glutReshapeFunc - sets the reshape callback for the current window
// glutReshapeWindow - sets the window width and height
// Angel::InitOpenGL - initializes OpenGL
// glutMainLoop - enters the GLUT event processing loop
