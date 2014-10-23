// 02561-07-01
#include <iostream>
#include <string>
#include <cmath>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <sstream>
#include "Angel.h"
#include "Axis.h"
#include "Uniform.h"
#include <vector>
#include "ObjLoader.h"

using namespace std;
using namespace Angel;

int WINDOW_WIDTH = 500;
int WINDOW_HEIGHT = 500;
const int FRAMERATE_MS = 16; /* 60 fps */

GLuint shaderProgram;
GLuint projectionUniform,
	modelViewUniform,
	colorUniform;
GLuint positionAttribute;
GLuint normalAttribute;
GLuint unitCubeVertexArrayObject;
GLuint wallVertexArrayObject;

vec3 eyePointSpericalCoordinate(40,2*M_PI, 0.5*M_PI); // radius, altitude, azimuth
vec4 eyePoint(0,12,0,0);

vec4 lightPos(14,5,2,1);
vec4 initialLights(14,5,2,1);
bool usePerspectiveShadow = true;
bool pauseLight = false;
Axis axis;

std::vector<int> meshIndices;
float lightType = 1.0;


struct Vertex {
    vec4 position;
};

// Light
struct Light 
{
	vec3 position;
	vec3 color;
	float lightType;
	float attenuation;
	float ambientCoefficient;
};

void loadShader();
void display();
GLuint loadBufferData(std::vector<vec3> positions, std::vector<vec3> normals);

void buildUnitCube() {
	std::vector<vec3> outPositions;
	meshIndices.clear();
	std::vector<vec3> outNormal;
	std::vector<vec2> outUv;
	loadObject("cube.obj", outPositions, meshIndices, outNormal, outUv, 2.5f);
	unitCubeVertexArrayObject = loadBufferData(outPositions, outNormal);
}

void drawSolidUnitCube() {
	glBindVertexArray(unitCubeVertexArrayObject);
	glDrawElements(GL_TRIANGLES, meshIndices.size(), GL_UNSIGNED_INT, &meshIndices[0]);
}

GLuint loadBufferData(std::vector<vec3> positions, std::vector<vec3> normals) {
	// Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vec3) * positions.size() * 2,
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vec3) * positions.size(), &positions[0] );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec3) * positions.size(),
		     sizeof(vec3) * positions.size(), &normals[0] );

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation( shaderProgram, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 3, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( shaderProgram, "vNormal" ); 
	if (vNormal != GL_INVALID_INDEX ){
		glEnableVertexAttribArray( vNormal );
		glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(sizeof(vec3) * positions.size()) );
	}

	return vao;
}

void loadShader(bool phong)
{
	if(phong)
	{
		shaderProgram = InitShader( "phong-shader.vert", "phong-shader.frag", "fragColor" );
		vec4 material_color( 0.2, 0.3, 0.5, 1.0 );
		vec4 material_specular_color( 1.0, 1.0, 1.0, 1.0 );
		float  material_shininess = 100.0;
		glUniform4fv( glGetUniformLocation(shaderProgram, "MaterialColor"), 1, material_color );
		glUniform4fv( glGetUniformLocation(shaderProgram, "MaterialSpecularColor"), 1, material_specular_color );
		glUniform1f( glGetUniformLocation(shaderProgram, "Shininess"), material_shininess );
	}
	else
	{
		shaderProgram = InitShader("const-shader.vert",  "const-shader.frag", "fragColor");
		colorUniform = glGetUniformLocation(shaderProgram, "Color");
		if (colorUniform == GL_INVALID_INDEX) {
			cerr << "Shader did not contain the 'Color' uniform."<<endl;
		}
	}

	glUseProgram( shaderProgram );

	projectionUniform = glGetUniformLocation(shaderProgram, "Projection");
	if (projectionUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain the 'projection' uniform."<<endl;
	}
	modelViewUniform = glGetUniformLocation(shaderProgram, "ModelView");
	if (modelViewUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain the 'modelView' uniform."<<endl;
	}
}

void drawScene(mat4 view){
	glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, view);
	drawSolidUnitCube();
}

void drawLightSource(mat4 view){
	mat4 model = Translate(lightPos);
	mat4 modelView = view * model;
	
	glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, modelView);
	drawSolidUnitCube();
}

GLfloat shadowDistance = -7.98;

mat4 createShadowProjection(vec4 light)
{
	mat4 N;
	float dist = shadowDistance - light[0];
	N[3][3] = 0;
	N[3][0] = 1.0/dist;
	return Translate(light) * N * Translate(-light);
}

mat4 createShadowProjectionPointLight()
{
	return createShadowProjection(lightPos);
}

mat4 createShadowProjectionDirectionalLight() 
{
	// move light source far away (with the same direction to (0,0,0))
	return createShadowProjection(9999 * lightPos);
}

template<typename T>
void setLightUniform(const char* uniformName, size_t index, const T& value)
{
	std::ostringstream ss;
	ss << "Lights[" << index << "]." << uniformName;
	std::string name = ss.str();
	_Uniform<T> uniformSetter;

	uniformSetter.Register(shaderProgram, name.c_str());
	uniformSetter.SetData(value);
}

void loadLights()
{
	std::vector<Light> lights;
	
	/*Light first;
	first.position = vec3(-4, 2, 4);
	first.color = vec3(1, 2, 2);
	first.ambientCoefficient = 0.1f; // no ambient
	first.attenuation = 0.1f;
	first.lightType = 0.0;
	lights.push_back(first);

	Light second;
	second.position = vec3(0, 0, 1);
	second.color = vec3(0, 2, 0);
	second.ambientCoefficient = 0.08f;
	second.attenuation = 0.2f;
	second.lightType = 0.0;
	lights.push_back(second);

	Light third;
	third.position = vec3(0, 4, 5);
	third.color = vec3(0, 0, 1);
	third.ambientCoefficient = 0.02f;
	third.attenuation = 0.03f;
	third.lightType = 0.0;
	lights.push_back(third);*/

	Light fourth;
	fourth.position = vec3(lightPos.x, lightPos.y, lightPos.z);
	fourth.color = vec3(2, 2, 2);
	fourth.ambientCoefficient = 0.3f;
	fourth.attenuation = 0.01f;
	fourth.lightType = lightType;
	lights.push_back(fourth);

	for(size_t i = 0; i < lights.size(); ++i)
	{
		setLightUniform("position",i,lights[i].position);
		setLightUniform("color",i,lights[i].color);
		setLightUniform("lightType",i,lights[i].lightType);
		setLightUniform("attenuation",i,lights[i].attenuation);
		setLightUniform("ambientCoefficient",i,lights[i].ambientCoefficient);
	}

	glUniform1i( glGetUniformLocation(shaderProgram, "NumLights"), lights.size() );
}

void display() 
{	
	loadShader(true);
	loadLights();
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	mat4 projection = Perspective(45,WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1, 100.);
	glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, projection);
	mat4 view = LookAt(eyePoint,vec4(0,0,0,0), vec4(0,1,0,0));
	
	glUniform4fv( glGetUniformLocation(shaderProgram, "MaterialColor"), 1, vec4(0.6, 0.0, 0.0, 1.0));
	drawScene(view);
	glUniform4fv( glGetUniformLocation(shaderProgram, "MaterialColor"), 1, vec4(0.6, .7, 0.0, 1.0));
	drawLightSource(view);

	loadShader(false);
	axis.renderWorldAxis(projection, eyePoint, vec4(0,0,0,0), vec4(0,1,0,0), WINDOW_WIDTH, WINDOW_HEIGHT);

	// draw shadow
	mat4 shadowProjection;
	if (usePerspectiveShadow){
		shadowProjection = createShadowProjectionPointLight();
	} else {
		shadowProjection = createShadowProjectionDirectionalLight();
	}
	glUniform4fv(colorUniform, 1, vec4(0.0, 0.0, 0.0, 1.0));
	drawScene(view * shadowProjection);

	glutSwapBuffers();
	Angel::CheckError();
}

void reshape(int W, int H) {
    WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void updateEyepoint(){
	eyePoint[0] = eyePointSpericalCoordinate[0]*sin(eyePointSpericalCoordinate[1])*cos(eyePointSpericalCoordinate[2]);
	eyePoint[2] = eyePointSpericalCoordinate[0]*sin(eyePointSpericalCoordinate[1])*sin(eyePointSpericalCoordinate[2]);
	eyePoint[1] = eyePointSpericalCoordinate[0]*cos(eyePointSpericalCoordinate[1]);
}

void update (float timedelta){
	static float time = 0;
	if (!pauseLight){
		time += timedelta;
	}

	float lightMotionRadius = 5;
	lightPos[1] = initialLights[1]+ sin(time*0.5) * lightMotionRadius;
	lightPos[2] = initialLights[2]+ cos(time*0.5) * lightMotionRadius;
	loadLights();
}

void mouseMotion(int x, int y){
	eyePointSpericalCoordinate[1] = y*0.008f;
	eyePointSpericalCoordinate[2] = x*0.008f;
	
	updateEyepoint();
}

// handle timer loop
// responsible for updating the model (update() function) and redisplaying the result)
void timerEvent(int v) {
	static int lastUpdate = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	int deltaTimeMs = time-lastUpdate;
	lastUpdate = time;

	float timeDelta = deltaTimeMs/1000.0f;
	update(timeDelta);
	glutPostRedisplay();

	glutTimerFunc(FRAMERATE_MS, &timerEvent, 0);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	case 'p':
		pauseLight = !pauseLight;
		if (pauseLight){
			cout << "Pause light motion" << endl;
		} else {
			cout << "Resume light motion" << endl;
		}
		break;
	case 't':
		usePerspectiveShadow = !usePerspectiveShadow;
		if (usePerspectiveShadow){
			cout << "Perspective shadow projection" << endl;
		} else {
			cout << "Orthogonale shadow projection" << endl;
		}
		glutPostRedisplay();
		break;
	case '1':
		lightType = 0.0f;
		break;
	case '2':
		lightType = 1.0f;
		break;
	}
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	glutInitContextVersion(3, 2);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    
	glutSetOption(
        GLUT_ACTION_ON_WINDOW_CLOSE,
        GLUT_ACTION_GLUTMAINLOOP_RETURNS
    );

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("02561-07-01");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(FRAMERATE_MS, timerEvent, 0);
	glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(mouseMotion);
	glutKeyboardFunc(keyboard);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	Angel::InitOpenGL();
	
	cout <<"P pause/resume light motion"<<endl;
	cout <<"T to toggle shadow projection type"<<endl;
	
	loadShader(true);
    loadLights();
	buildUnitCube();
	updateEyepoint();

	glEnable(GL_DEPTH_TEST);

	Angel::CheckError();

	glutMainLoop();
}