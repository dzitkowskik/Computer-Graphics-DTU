// 02561-08-01 / 02561-09-01
#include <iostream>
#include <string>
#include <algorithm>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Angel.h"
#include "ObjLoader.h"
#include "TextureLoader.h"

using namespace std;
using namespace Angel;

#define RADIAN_TO_DEGREE 57.2957795f

int WINDOW_WIDTH = 500;
int WINDOW_HEIGHT = 500;
int SHADOW_MAP_SIZE = 1024;

struct Shader {
	GLuint shaderProgram;
	GLuint projectionUniform,
		modelUniform,
		viewUniform,
		normalMatUniform,
		textureUniform,
		pointLightPosUniform,
		colorUniform,
		clipPlaneUniform,
		shadowMapUniform,
		lightViewProjection;
	GLuint positionAttribute,
		normalAttribute,
		textureCoordinateAttribute;
};

struct MeshObject {
	Shader shader;
	GLuint vertexArrayObject;
	vector<int> indices;
	vec4 color;

	MeshObject():color(1,1,1,1){}

	MeshObject(Shader shader, GLuint vertexArrayObject, vector<int> indices)
		:shader(shader),vertexArrayObject(vertexArrayObject),indices(indices),color(1,1,1,1){
	}
};

vec3 teapotPosition;
vec3 lightPosition(0,300,0);

int shadow_type = 1;          // 0 Flattening, 1 projected
bool draw_mirror = 1;		 // draw mirror
bool freeze = false;
bool debugShadowMap = false;

struct Vertex {
    vec3 position;
	vec3 normal;
	vec2 textureCoordinate;
};

MeshObject planeObject;
MeshObject teapotObject;

float teapotBoundingRadius;

GLuint planeTextureId,
	shadowmapTextureId;

GLuint frameBufferObject;

// spherical coordinates of camera position (angles in radian)
float sphericalPolarAngle = 45;
float sphericalElevationAngle = 45;
float sphericalRadius = 600;

vec2 mousePos;
vec4 clipPlane(0,0,0,0);

// forward declaration
void loadShader();
void display();
vec3 sphericalToCartesian(float polarAngle, float elevationAngle, float radius);
GLuint createVertexBuffer(Vertex* vertices, int vertexCount);
string getFrameBufferStatusString(GLenum code);
void drawMeshObject(
	mat4 & projection, 
	mat4 & lightViewProjection, 
	mat4 & model, 
	mat4 & view, 
	MeshObject& meshObject);

mat4 getLightProjection() 
{
	float l = length(lightPosition-teapotPosition);
	float r = teapotBoundingRadius;
	float angle = 2 * asinf(r / l) * RADIAN_TO_DEGREE;
	return Perspective(angle, 1, 0.5, 500);
}

mat4 getLightView() {
	vec4 up(0,1,0,0);
	return Angel::LookAt(lightPosition, teapotPosition, up);
}

mat4 getLightViewProjection()
{
	return getLightProjection() * getLightView();
}

void updateProjShadowTexture() {
	// todo bind framebuffer, set viewport to shadowmap and clear to white
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	
	

	mat4 projection = getLightProjection();
	mat4 view = getLightView();
	mat4 model = Translate(teapotPosition);
	mat4 lightViewProjection = getLightViewProjection();

	// todo render black teapot
	clipPlane.w = 1.0;
	vec4 teapot_color = teapotObject.color;
	teapotObject.color = vec4(0, 0, 0, 1);
	drawMeshObject(projection, lightViewProjection, model, view, teapotObject);
	teapotObject.color = teapot_color;
	clipPlane.w = 0.0;

	// todo release framebuffer, setviewport to window
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

float computeRadius(vector<vec3> position) {
	float maxRadius = 0;
	for (int i=0;i<position.size();i++){
		maxRadius = max(maxRadius,length(position[i]));
	}
	return maxRadius;
}

GLuint buildPlaneVertexBuffer() {
	const int planeSize = 4;
	Vertex planeData[planeSize] = {
        { vec3(-200,  0.0, -200 ), vec3(0,1,0), vec2(0,0) },
        { vec3(-200,  0.0,  200 ), vec3(0,1,0), vec2(0,1) },
        { vec3( 200,  0.0,  200 ), vec3(0,1,0), vec2(1,1) },
        { vec3( 200,  0.0, -200 ), vec3(0,1,0), vec2(1,0) }
    };
	return createVertexBuffer(planeData, planeSize);
}

vector<int> buildPlaneIndices() {
	vector<int> res;
	res.push_back(0);
	res.push_back(1);
	res.push_back(2);
	res.push_back(0);
	res.push_back(2);
	res.push_back(3);
	return res;
}

GLuint buildTexture(int width, int height) {
	GLuint textureId;
	// generate texture
	glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GLuint internalFormat = GL_RED;
    GLuint format = GL_RED;
	GLuint storageType = GL_UNSIGNED_BYTE;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,
            0, format, storageType, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureId;
}

GLuint buildFrameBufferObject(int width, int height, GLuint textureId) {
	GLuint framebufferObjectId,
		renderBufferId;
	glGenFramebuffers(1, &framebufferObjectId);
	glGenRenderbuffers(1, &renderBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBufferId);

	GLenum frameBufferRes = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	cout << getFrameBufferStatusString(frameBufferRes)<<endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return framebufferObjectId;
}

GLuint createVertexArrayObject(GLuint vertexBuffer, const Shader & shader) {
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	GLuint vertexArrayObject;

    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

	glEnableVertexAttribArray(shader.positionAttribute);
    glVertexAttribPointer(shader.positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);

	if (shader.normalAttribute != GL_INVALID_INDEX) {
		glEnableVertexAttribArray(shader.normalAttribute);
		glVertexAttribPointer(shader.normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)sizeof(vec3));
	}
	if (shader.textureCoordinateAttribute != GL_INVALID_INDEX) {
		glEnableVertexAttribArray(shader.textureCoordinateAttribute);
		glVertexAttribPointer(shader.textureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)(sizeof(vec3)*2));
	}	
	return vertexArrayObject;
}

GLuint createVertexBuffer(Vertex* vertices, int vertexCount) {
	GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
    
	return vertexBuffer;
}

GLuint loadMesh(char *filename, vector<int> &indices, float scale = 1.0){
	vector<vec3> position;
	vector<vec3> normal;
	vector<vec2> uv; 
	loadObject(filename,position,indices,normal,uv, scale); 
	Vertex* vertexData = new Vertex[position.size()];

	teapotBoundingRadius = computeRadius(position);

	for (int i = 0; i < position.size(); i++) {
		vertexData[i].position = position[i];
		if (normal.size() > 0){
			vertexData[i].normal = normal[i];
		}
	}
	GLuint vertexBuffer = createVertexBuffer(vertexData, position.size());
	delete [] vertexData;
	return vertexBuffer;
}

Shader loadShader(const char* vertShader, const char* fragShader) {
	Shader shader;
	shader.shaderProgram = InitShader(vertShader, fragShader, "fragColor");
	
	// get uniform locations
	shader.projectionUniform = glGetUniformLocation(shader.shaderProgram, "projection");
	shader.viewUniform = glGetUniformLocation(shader.shaderProgram, "view");
	shader.modelUniform = glGetUniformLocation(shader.shaderProgram, "model");
	shader.textureUniform = glGetUniformLocation(shader.shaderProgram, "texture1");
	shader.shadowMapUniform = glGetUniformLocation(shader.shaderProgram, "shadowMap");
	shader.pointLightPosUniform = glGetUniformLocation(shader.shaderProgram, "pointLightPos");
	shader.colorUniform = glGetUniformLocation(shader.shaderProgram, "color");
	shader.normalMatUniform = glGetUniformLocation(shader.shaderProgram, "normalMat");
	shader.clipPlaneUniform = glGetUniformLocation(shader.shaderProgram, "clipPlane");
	shader.lightViewProjection = glGetUniformLocation(shader.shaderProgram, "lightViewProjection");
	
	// get attribute locations
	shader.positionAttribute = glGetAttribLocation(shader.shaderProgram, "position");
	shader.normalAttribute = glGetAttribLocation(shader.shaderProgram, "normal");
	shader.textureCoordinateAttribute = glGetAttribLocation(shader.shaderProgram, "textureCoordinate");
	return shader;
}

void drawMeshObject(
		mat4 & projection, 
		mat4 & lightViewProjection, 
		mat4 & model,
		mat4 & view, 
		MeshObject& meshObject) 
{
	glUseProgram(meshObject.shader.shaderProgram);

	if (meshObject.shader.projectionUniform != GL_INVALID_INDEX){
		glUniformMatrix4fv(meshObject.shader.projectionUniform, 1, GL_TRUE, projection);
	}
	if (meshObject.shader.modelUniform !=  GL_INVALID_INDEX){
		glUniformMatrix4fv(meshObject.shader.modelUniform, 1, GL_TRUE, model);
	}
	if (meshObject.shader.viewUniform !=  GL_INVALID_INDEX){
		glUniformMatrix4fv(meshObject.shader.viewUniform, 1, GL_TRUE, view);
	}
	if (meshObject.shader.lightViewProjection !=  GL_INVALID_INDEX){
		glUniformMatrix4fv(meshObject.shader.lightViewProjection, 1, GL_TRUE, lightViewProjection);
	}
	if (meshObject.shader.normalMatUniform != GL_INVALID_INDEX) {
		mat3 normalMat = Normal(model);
		glUniformMatrix3fv(meshObject.shader.normalMatUniform, 1, GL_TRUE, normalMat);
	}
	if (meshObject.shader.pointLightPosUniform != GL_INVALID_INDEX) {
		glUniform3fv(meshObject.shader.pointLightPosUniform, 1, lightPosition);
	}
	if (meshObject.shader.colorUniform != GL_INVALID_INDEX){
		glUniform4fv(meshObject.shader.colorUniform, 1, meshObject.color);
	}
	if (meshObject.shader.clipPlaneUniform != GL_INVALID_INDEX){
		glUniform4fv(meshObject.shader.clipPlaneUniform, 1, clipPlane);
	}
	if (meshObject.shader.textureUniform !=  GL_INVALID_INDEX){
		// bind texture to texture slot 0 and set the uniform value
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeTextureId);
		glUniform1i(meshObject.shader.textureUniform, 0);
	}
	if (meshObject.shader.shadowMapUniform !=  GL_INVALID_INDEX){
		// bind texture to texture slot 1 and set the uniform value
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowmapTextureId);
		glUniform1i(meshObject.shader.shadowMapUniform, 1);
	}
	
	glBindVertexArray(meshObject.vertexArrayObject);
	glDrawElements(GL_TRIANGLES, meshObject.indices.size(), GL_UNSIGNED_INT, &meshObject.indices[0]);
}

void drawMirror(mat4 &projection, mat4 &view) 
{
	mat4 model = Angel::Scale(1, -1, 1) * Translate(teapotPosition);
	lightPosition.y = -lightPosition.y;
	mat4 lightViewProjection = getLightViewProjection();
	clipPlane.w = -1.0;
	vec4 teapot_color = teapotObject.color;
	teapotObject.color = vec4(0.5, 0.5, 0.5, 1);
	drawMeshObject(projection, lightViewProjection, model, view, teapotObject);
	teapotObject.color = teapot_color;
	lightPosition.y = -lightPosition.y;
	clipPlane.w = 0.0;
}

void drawPlane(mat4 projection, mat4 view) {
	mat4 model;
	mat4 lightViewProjection = getLightViewProjection();
	glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF); 
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE); 
    glClear(GL_STENCIL_BUFFER_BIT); 

	drawMeshObject(projection, lightViewProjection, model, view, planeObject);	

	glStencilFunc(GL_EQUAL, 1, 0xFF); 
    glStencilMask(0x00);
    glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if (draw_mirror == 1) {
		drawMirror(projection, view);	
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);
	drawMeshObject(projection, lightViewProjection, model, view, planeObject);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
}

void updateTeapotPosition()
{
	static int counter = 42;
	teapotPosition.x = sin(counter / 180.0f * M_PI) * 120 + cos(counter / 70.0f * M_PI) * 50;
	teapotPosition.y = cos(counter / 180.0f * M_PI) *  40 + sin(counter / 70.0f * M_PI) * 40;
	teapotPosition.z = cos(counter / 180.0f * M_PI) * 120 + cos(counter / 70.0f * M_PI) * 50;
	if (!freeze){
		counter++;
	}
}

void display() {
	updateTeapotPosition();
	if (shadow_type == 1) {
		updateProjShadowTexture();
	}

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	
	mat4 projection = Perspective(60,WINDOW_WIDTH/(float)WINDOW_HEIGHT,1.5,1000);
	vec4 eye = sphericalToCartesian(sphericalPolarAngle, sphericalElevationAngle, sphericalRadius);
	vec4 at(0,0,0,1);
	vec4 up(0,1,0,0);
	mat4 view = LookAt(eye, at, up);
	drawPlane(projection, view);

	mat4 model = Translate(teapotPosition);
	mat4 lightViewProjection = getLightViewProjection();
	drawMeshObject(projection, lightViewProjection, model, view, teapotObject);

	glutSwapBuffers();
	Angel::CheckError();
}

void reshape(int W, int H) {
    WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void mouse(int button, int state, int x, int y) {
	if (state==GLUT_DOWN) {
		mousePos = vec2(x,y);
	}
}

void mouseMovement(int x, int y){
	float rotationSpeed = 0.01f;
	vec2 newMousePos = vec2(x,y);
	vec2 mousePosDelta = mousePos - newMousePos;
	sphericalPolarAngle += mousePosDelta.x*rotationSpeed;
	float rotate89Degrees = 89*DegreesToRadians;
	sphericalElevationAngle = min(rotate89Degrees,max(-rotate89Degrees,sphericalElevationAngle + mousePosDelta.y*rotationSpeed));
	mousePos = vec2(x,y);
}

void initPlaneTexture(){
	const char* imagepath = "textures/xamp23.bmp";

	unsigned int width, height;
	unsigned char * data = loadBMPRaw(imagepath, width, height);
	// Create one OpenGL texture
	glGenTextures(1, &planeTextureId);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, planeTextureId);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// Trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);
	delete[] data;
}

void initMeshAndShader(){
	vector<int> indices;
	GLuint teapotVertexBuffer = loadMesh("meshes/teapot.obj", indices);
	
	Shader shader = loadShader("diffuse.vert", "diffuse.frag");
	GLuint vertexArrayObject = createVertexArrayObject(teapotVertexBuffer, shader);
	teapotObject = MeshObject(
		shader,
		vertexArrayObject,
		indices
	);

	Shader planeShader = loadShader("plane.vert", "plane.frag");
	GLuint planeVertexBuffer = buildPlaneVertexBuffer();
	planeObject = MeshObject(
		planeShader,
		createVertexArrayObject(planeVertexBuffer, planeShader),
		buildPlaneIndices()
	);
}

void keyPress(unsigned char key, int x, int y) {
	switch (key){
		case '\033': 
			exit(0); 
			break;
		case 'f':
			freeze = !freeze;
			break;
		case 'd':
			debugShadowMap = !debugShadowMap;
			break;
		case 'm':
			draw_mirror = !draw_mirror;
			break;
	}
}

void animate(int x) {
	glutPostRedisplay();
	glutTimerFunc(10, animate, 0);
}

void printMenu(){
	// print key menu
	cout << "Key shortcuts"<<endl;
	cout << "f     - toggle freeze"<<endl;
	cout << "m     - toggle mirror"<<endl;
	cout << endl<<"Use mouse drag to rotate"<<endl;
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

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_STENCIL|GLUT_3_2_CORE_PROFILE);
	glutCreateWindow("02561-08_09");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMovement);
	glutTimerFunc(10, animate, 0);
	glutKeyboardFunc(keyPress);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	Angel::InitOpenGL();

	printMenu();

	shadowmapTextureId = buildTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	frameBufferObject = buildFrameBufferObject(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, shadowmapTextureId);

	initMeshAndShader();
	initPlaneTexture(); 
	glEnable(GL_DEPTH_TEST);

	Angel::CheckError();

	glutMainLoop();
}

// Convert from spherical coordinates to cartesian coordinates
vec3 sphericalToCartesian(float polarAngle, float elevationAngle, float radius){
	float a = radius * cos(elevationAngle);
	vec3 cart;
	cart.x = a * cos(polarAngle);
	cart.y = radius * sin(elevationAngle);
	cart.z = a * sin(polarAngle);
	return cart;
}

string getFrameBufferStatusString(GLenum code){
	switch (code){
        case GL_FRAMEBUFFER_COMPLETE:
            return "Framebuffer ok";
        case GL_FRAMEBUFFER_UNDEFINED:
            return "Framebuffer undefined";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "Framebuffer incomplete attachment";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "Framebuffer incomplete missing attachment";
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            return "Framebuffer incomplete read buffer";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "Framebuffer unsupported";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "Framebuffer incomplete multisample";
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            return "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            return "FRAMEBUFFER_INCOMPLETE_FORMATS";
        default:
            return "Unknown framebuffer status ";
    }
}