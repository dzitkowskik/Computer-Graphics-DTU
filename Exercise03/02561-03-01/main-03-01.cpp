// 02561-03-01
#include <stdio.h>
#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Angel.h"
#include "vtk_reader.h"

using namespace std;
using namespace Angel;

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

GLuint shaderProgram;
GLuint projectionUniform,
	modelViewUniform,
	blendValueUniform,
	normalExtrusionUniform;
GLuint positionAttribute,
	colorAttribute,
	position2Attribute,
	color2Attribute,
	normalAttribute,
	normal2Attribute;
GLuint vertexArrayObject,
	vertexBuffer;

struct Vertex {
	vec3 position1;
	vec3 color1;
	vec3 position2;
	vec3 color2;
	vec3 normal;
	vec3 normal2;
};

vector<unsigned int> indices;

vec2 sphericalCoordinates; // two first components of spherical coordinates (azimuth and elevation)
float dist = 160; // last component of spherical coordinates
vec2 angleOffset;
vec2 mousePos;
float blendValue = 0.0f;
float normalExtrusion = 0.0f;

void loadShader();
void display();

vector<Vertex> interleaveData(vector<float> &position, vector<float> &color, vector<float> &position2, vector<float> &color2, vector<float> &normal, vector<float> &normal2){
	vector<Vertex> interleavedVertexData;
	for (int i=0;i<position.size();i=i+3){
		vec3 positionVec(position[i],position[i+1],position[i+2]);
		vec3 colorVec(color[i],color[i+1],color[i+2]);
		vec3 positionVec2(position2[i],position2[i+1],position2[i+2]);
		vec3 colorVec2(color2[i],color2[i+1],color2[i+2]);
		vec3 normalVec(normal[i],normal[i+1],normal[i+2]);
		vec3 normalVec2(normal2[i],normal2[i+1],normal2[i+2]);

		Vertex v = {
			positionVec,
			colorVec,
			positionVec2,
			colorVec2,
			normalVec,
			normalVec2
		};

		interleavedVertexData.push_back(v);
	}
	return interleavedVertexData;
}

void uploadData(vector<Vertex> &interleavedVertexData){
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, interleavedVertexData.size() * sizeof(Vertex), interleavedVertexData[0].position1, GL_STATIC_DRAW);

	GLuint stride = sizeof(Vertex);
	glEnableVertexAttribArray(positionAttribute);
	glEnableVertexAttribArray(colorAttribute);
	glEnableVertexAttribArray(position2Attribute);
	glEnableVertexAttribArray(color2Attribute);
	glEnableVertexAttribArray(normalAttribute);
	glEnableVertexAttribArray(normal2Attribute);

	glVertexAttribPointer(positionAttribute,	3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)*0));
	glVertexAttribPointer(colorAttribute,		3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)*1));
	glVertexAttribPointer(position2Attribute,	3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)*2));
	glVertexAttribPointer(color2Attribute,		3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)*3));
	glVertexAttribPointer(normalAttribute,		3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)*4));
	glVertexAttribPointer(normal2Attribute ,	3, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)(sizeof(vec3)*5));
}

void initData(char * mesh1, char * mesh2){
	vector<float> position[2];
	vector<float> color[2];
	vector<float> normal[2];
	char *meshes[] = {mesh1, mesh2};

	for (int i=0;i<2;i++){
		indices.clear(); // this avoid loading indices multiple times
		cout << "Loading file "<<meshes[i]<<endl;

		bool res = readVTKFile(meshes[i], position[i], color[i], normal[i], indices);
		if (!res){
			cout << "Cannot read " << mesh1 << flush<<endl;
			exit(1);
		}
	}
	cout << "Interleaving data" << endl;
	vector<Vertex> interleavedVertexData = interleaveData(position[0], color[0], position[1], color[1], normal[0], normal[1]);

	cout << "Uploading data" << endl;
	uploadData(interleavedVertexData);
	cout <<"Data uploaded"<<endl;
}

void loadUniform(GLuint shaderProgram, GLuint& uniform, const char* name)
{
	uniform = glGetUniformLocation(shaderProgram, name);
	if (uniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the " << name << "uniform." << endl;
	}
}

void loadAttribute(GLuint shaderProgram, GLuint& attribute, const char* name)
{
	attribute = glGetAttribLocation(shaderProgram, name); 
	if (attribute == GL_INVALID_INDEX){
		cerr << "Shader did not contain/use the " << name << "attribute." << endl;
	}
}

void loadShader(){
	shaderProgram = InitShader("blending.vert",  "blending.frag", "fragColor");

	// UNIFORMS
	loadUniform(shaderProgram, projectionUniform, "projection");
	loadUniform(shaderProgram, modelViewUniform, "modelView");
	loadUniform(shaderProgram, blendValueUniform, "blendValue");
	loadUniform(shaderProgram, normalExtrusionUniform, "normalExtrusion");

	// ATTRIBUTES
	loadAttribute(shaderProgram, colorAttribute, "color1");
	loadAttribute(shaderProgram, color2Attribute, "color2");
	loadAttribute(shaderProgram, positionAttribute, "position1");
	loadAttribute(shaderProgram, position2Attribute, "position2");
	loadAttribute(shaderProgram, normalAttribute, "normal1");
	loadAttribute(shaderProgram, normal2Attribute, "normal2");
}

void updateTitle(){
	static int count = 0;
	if (count == 10){
		char buffer[50];
		sprintf(buffer, "VertexBlend: %.2f / NormalExtrusion: %.2f", blendValue, normalExtrusion);
		glutSetWindowTitle(buffer);
		count = 0;
	}
	count++;
}

void display() {	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	const float timeScale = 0.008f;

	glUseProgram(shaderProgram);

	mat4 view = Translate(0,0,-dist) * RotateX(sphericalCoordinates.y) * RotateY(sphericalCoordinates.x);
	mat4 projection = Perspective(55, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.01, 1000);
	glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, view);
	glUniform1fv(blendValueUniform, 1, &blendValue);
	glUniform1fv(normalExtrusionUniform, 1, &normalExtrusion);

	// vertex shader uniforms
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);

	glutSwapBuffers();

	Angel::CheckError();

	updateTitle();
}

void reshape(int W, int H) {
	WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void animate() {	
	glutPostRedisplay();
}

void visible(int vis) {
	if (vis == GLUT_VISIBLE)
		glutIdleFunc(animate);
	else
		glutIdleFunc(0);
}

void mouse(int button, int state, int x, int y) {
	if(state == GLUT_DOWN) {
		mousePos = vec2(x,y);
		angleOffset = sphericalCoordinates;
	} 
}

void motion(int x, int y) {	
	const float motionSpeed = 32;
	vec2 deltaAng(M_PI * (mousePos.x - x) / WINDOW_WIDTH, 
		M_PI * (mousePos.y - y) / WINDOW_HEIGHT);
	sphericalCoordinates = angleOffset + deltaAng * motionSpeed;
}

void mouseWheel(int button, int dir, int x, int y) {
	if (dir > 0) {
		blendValue += 0.01;
		if (blendValue > 1.0){
			blendValue = 1.0;
		}
	} else {
		blendValue -= 0.01;
		if (blendValue < 0.0){
			blendValue = 0.0;
		}
	}
}

void keyboard(unsigned char key, int x, int y){
	if (key == '+') 
	{
		mouseWheel(0, 1, 0,0 );
	} 
	else if (key == '-') 
	{
		mouseWheel(0, -1, 0,0 );
	} 
	else if (key == 'n') 
	{
		normalExtrusion += 0.05;
		if (normalExtrusion > 10.0)
			normalExtrusion = 10.0;
	} 
	else if (key == 'm') 
	{
		normalExtrusion -= 0.05;
		if (normalExtrusion < -10.0)
			normalExtrusion = -10.0;
	}
}

void printHelp(){
	cout << "Use mouse drag to rotate around head."<< endl;
	cout << "Use mouse wheel or '+'/'-' to change blending (works when implemented)."<< endl;
}

int main(int argc, char* argv[]) {
	printHelp();
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_3_2_CORE_PROFILE);
	glutCreateWindow("02561-03-01");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(mouseWheel);
	glutVisibilityFunc(visible);
	glutIdleFunc(animate);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	Angel::InitOpenGL();

	glEnable(GL_DEPTH_TEST);

	loadShader();
	char * mesh1 = "Rasmus_DenseMesh_RGB_aligned.vtk";
	char * mesh2 = "Stine_DenseMesh_RGB_aligned.vtk";
	initData(mesh1, mesh2);

	Angel::CheckError();

	glutMainLoop();
}