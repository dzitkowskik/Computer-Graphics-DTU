#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Angel.h"
#include "select.h"

using namespace std;
using namespace Angel;

int WINDOW_WIDTH = 600;
int WINDOW_HEIGHT = 600;

const int menuSize = 40;

enum component_type {
	capacitor, 
	resistor, 
	transistor, 
	line
};

enum menu_item {
	insert_capacitor,
	insert_resistor,
	insert_transistor,
	insert_line,
	move_item,
	rotate_item,
	scale_item,
	delete_item,
	menu_none
};

struct Geometry {
	Geometry(){}
	Geometry(GLuint vao, int size, int mode):vao(vao),size(size), mode(mode){}
	GLuint vao;
	int size;
	int mode; // such as GL_LINES, etc
	void draw() {
		if (size==0) return;
		glBindVertexArray(vao);
		glDrawArrays(mode,0,size);
	}
};

struct SelectedGeomData
{
	vec2 selectionPoint;
	vec2 selectedGeomPosition;
	int selectedGeomRotation;
	float scale;
};

struct component_t
{
	component_t(component_type t, vec2 trans, float rot, vec2 scal, Geometry geo) 
		: type(t), translate(trans), rotationDegrees(rot), scale(scal), geometry(geo) {}

	component_type type;
	vec2 translate;
	float rotationDegrees;
	vec2 scale;
	Geometry geometry;

	void render(GLuint modelViewUniform){
		// update the modelView and render the component
		mat4 modelView;
		modelView = Translate(vec3(translate,0)) * Scale(scale.x, scale.y, 1) * RotateZ(rotationDegrees);
		glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, modelView);
		geometry.draw();
	}
};

typedef struct {
	vec4 position;
} Vertex;


GLuint shaderProgram;
GLuint projectionUniform,
	modelViewUniform,
	colorUniform;
GLuint positionAttribute;

Geometry capacitorGeometry,
	resistorGeometry,
	transistorGeometry,
	lineGeometry,
	menuRectGeometry,
	moveGeometry,
	rotateGeometry,
	scaleGeometry;

SelectBuffer *selectBuffer = NULL;

menu_item selectedItem = move_item;
menu_item mouseOver = menu_none;
int selectedComponent = -1;
SelectedGeomData selectedComponentData;
vector<component_t> components;

// mouse x and mouse y in world coordinates
int mx = 0, my = 0;

// forward declaration
void loadShader();
void display();
void drawMenu();
GLuint loadBufferData(Vertex* vertices, int vertexCount);
void reshape(int W, int H);

void renderScene(bool select)
{
	// if select mode, then use the variable 'i' as object-id. 
	int index = 0;
	for (int i = 0; i < components.size(); i++) {
		if (select)
		{
			selectBuffer->setId(i);
		}
		components[i].render(modelViewUniform);
	}
}

void buildCapacitor() {
	// build capacitor in local space between (-1,-1, 0) and (1, 1, 0)
	Vertex capData[8] = {
		{ vec4(-1.0,    0.0, 0.0, 1.0 ) },
		{ vec4(-0.15f,  0.0, 0.0, 1.0 ) },
		{ vec4(-0.15f,  1, 0.0, 1.0 ) },
		{ vec4(-0.15f, -1, 0.0, 1.0 ) },
		{ vec4(0.15f,   1, 0.0, 1.0 ) },
		{ vec4(0.15f,  -1, 0.0, 1.0 ) },
		{ vec4(0.15f,   0, 0.0, 1.0 ) },
		{ vec4( 1.0,0, 0.0, 1.0 ) },
	};
	int capVAO = loadBufferData(capData, 8);
	capacitorGeometry = Geometry(capVAO, 8, GL_LINES);
}

void buildResistor() {
	// build resistor in local space between (-1,-1, 0) and (1, 1, 0)
	Vertex resData[10] = {
		{ vec4(-1.0,    0.0, 0.0, 1.0 ) },
		{ vec4(-0.6666f,0.0, 0.0, 1.0 ) },
		{ vec4(-0.555556f , 0.666667f, 0.0, 1.0 ) },
		{ vec4(-0.333333f , -0.666667f, 0.0, 1.0 ) },
		{ vec4(-0.111111f , 0.666667f, 0.0, 1.0 ) },
		{ vec4(0.111111f , -0.666667f, 0.0, 1.0 ) },
		{ vec4(0.333333f , 0.666667f, 0.0, 1.0 ) },
		{ vec4(0.555556f , -0.666667f, 0.0, 1.0 ) },
		{ vec4(0.666667f , 0, 0.0, 1.0 ) },
		{ vec4(1.f , 0, 0.0, 1.0 ) }
	};
	int resVAO = loadBufferData(resData, 10);
	resistorGeometry = Geometry(resVAO, 10, GL_LINE_STRIP);
}

void buildTransistor() {
	// build transistor in local space between (-1,-1, 0) and (1, 1, 0)
	// todo insert code here
	const int length = 100;
	const float diff = 1.0f/(length - 12);
	Vertex transistorVertices[length] = {
		{ vec4(-1.0f, 0.0f, 0.0, 1.0) },	// 1
		{ vec4(-0.5f, 0.0f, 0.0, 1.0) },	// 2
		{ vec4(-0.5f, -0.5, 0.0, 1.0) },	// 3
		{ vec4(-0.5f, 0.5f, 0.0, 1.0) },	// 4
		{ vec4(-0.5f, .25f, 0.0, 1.0) },	// 5
		{ vec4(    0, 0.5f, 0.0, 1.0) },	// 6
		{ vec4(-0.5f, -.25, 0.0, 1.0) },	// 7
		{ vec4(    0, -0.5, 0.0, 1.0) },	// 8
		{ vec4(    0, -.5f, 0.0, 1.0) },	// 9
		{ vec4(    0, -1.0, 0.0, 1.0) },	// 10
		{ vec4(    0, 0.5f, 0.0, 1.0) },	// 11
		{ vec4(    0, 1.0f, 0.0, 1.0) }};	// 12

	// circle in an origin of (0,0,0) and radius of 1.0f
	float R = 1.0f;
	for (int i = 12; i < length; i++)
	{
		float fraction = (i-12)*diff;
		transistorVertices[i].position = vec4(R*sin(fraction*2*M_PI),R*cos(fraction*2*M_PI),0,1);
	}

	int transistorVAO = loadBufferData(transistorVertices, length);
	transistorGeometry = Geometry(transistorVAO, length, GL_LINES);
}

void buildLine() {
	// build line in local space between (-1,-1, 0) and (1, 1, 0)
	Vertex lineData[2] = {
		{ vec4(-1.0, 0.0, 0.0, 1.0 ) },
		{ vec4(1.0,  0.0, 0.0, 1.0 ) }
	};
	int lineVAO = loadBufferData(lineData, 2);
	lineGeometry = Geometry(lineVAO,2,GL_LINES);
}

void buildMenuRect() {
	Vertex rectangleData[4] = {
		{ vec4(-1.0, -1.0, 0.0, 1.0 ) },
		{ vec4(-1.0,  1.0, 0.0, 1.0 ) },
		{ vec4( 1.0,  1.0, 0.0, 1.0 ) },
		{ vec4( 1.0, -1.0, 0.0, 1.0 ) }
	};
	int menuRectVAO = loadBufferData(rectangleData, 4);
	menuRectGeometry = Geometry(menuRectVAO,4,GL_TRIANGLE_FAN);
}

void buildMoveGeometry() {
	// build move-icon in local space between (-1,-1, 0) and (1, 1, 0)
	Vertex moveData[20] = {
		// horizontal lines & vertical lines
		{ vec4(-1.5*.6, 0.0, 0.0, 1.0 ) },
		{ vec4( 1.5*.6, 0.0, 0.0, 1.0 ) },
		{ vec4( 0.0, 1.5*.6, 0.0, 1.0 ) },
		{ vec4( 0.0,-1.5*.6, 0.0, 1.0 ) },
		// arrrows
		{ vec4( -1.2*.6,0.3*.6, 0.0, 1.0 ) },
		{ vec4( -1.5*.6,0, 0.0, 1.0 ) },
		{ vec4( -1.5*.6,0, 0.0, 1.0 ) },
		{ vec4( -1.2*.6,-0.3*.6, 0.0, 1.0 ) },
		// arrrows
		{ vec4( 1.2*.6,0.3*.6, 0.0, 1.0 ) },
		{ vec4( 1.5*.6,0*.6, 0.0, 1.0 ) },
		{ vec4( 1.5*.6,0*.6, 0.0, 1.0 ) },
		{ vec4( 1.2*.6,-0.3*.6, 0.0, 1.0 ) },
		// arrrows
		{ vec4( 0.3*.6,1.2*.6, 0.0, 1.0 ) },
		{ vec4( 0*.6,1.5*.6, 0.0, 1.0 ) },
		{ vec4( 0*.6,1.5*.6, 0.0, 1.0 ) },
		{ vec4( -0.3*.6,1.2*.6, 0.0, 1.0 ) },
		// arrrows
		{ vec4( 0.3*.6,-1.2*.6, 0.0, 1.0 ) },
		{ vec4( 0*.6,-1.5*.6, 0.0, 1.0 ) },
		{ vec4( 0*.6,-1.5*.6, 0.0, 1.0 ) },
		{ vec4( -0.3*.6,-1.2*.6, 0.0, 1.0 ) },
	};
	int moveVAO = loadBufferData(moveData, 20);
	moveGeometry = Geometry(moveVAO,20,GL_LINES);
}

void buildRotateGeometry() {
	// build rotate-icon in local space between (-1,-1, 0) and (1, 1, 0)
	Vertex transData[84] = {
		{ vec4(0,.6, 0.0, 1.0 ) },
		{ vec4(0.2,.4, 0.0, 1.0 ) },
		{ vec4(0,.6, 0.0, 1.0 ) },
		{ vec4(0.2,.8, 0.0, 1.0 ) },
	};
	float r = 0.6f;
	for (int i=0;i<30;i++){
		float fraction = i/40.0f;
		transData[4+i*2].position = vec4(r*sin(fraction*2*M_PI),r*cos(fraction*2*M_PI),0,1);
		fraction += 1/40.0f;
		transData[4+i*2+1].position = vec4(r*sin(fraction*2*M_PI),r*cos(fraction*2*M_PI),0,1);
	}
	glEnd();
	int transVAO = loadBufferData(transData, 84);
	rotateGeometry = Geometry(transVAO, 84, GL_LINES);
}

void buildScaleGeometry() {
	// build scale-icon in local space between (-1,-1, 0) and (1, 1, 0)
	Vertex scaleData[22] = {
		// draw small quad
		{ vec4(-1,1,  0.0, 1.0 ) },
		{ vec4(-1,.5, 0.0, 1.0 ) },
		{ vec4(-1,.5, 0.0, 1.0 ) },
		{ vec4(-0.5,.5, 0.0, 1.0 ) },
		{ vec4(-0.5,.5, 0.0, 1.0 ) },
		{ vec4(-0.5,1, 0.0, 1.0 ) },
		{ vec4(-0.5,1, 0.0, 1.0 ) },
		{ vec4(-1,1,  0.0, 1.0 ) },
		// draw large quad
		{ vec4(0,0,  0.0, 1.0 ) },
		{ vec4(1,0,  0.0, 1.0 ) },
		{ vec4(1,0,  0.0, 1.0 ) },
		{ vec4(1,-1,  0.0, 1.0 ) },
		{ vec4(1,-1,  0.0, 1.0 ) },
		{ vec4(0,-1,  0.0, 1.0 ) },
		{ vec4(0,-1,  0.0, 1.0 ) },
		{ vec4(0,0,  0.0, 1.0 ) },
		// draw arrow
		{ vec4(0,0,  0.0, 1.0 ) },
		{ vec4(-0.5,0.5, 0,1.0 ) },
		{ vec4(0,0,  0.0, 1.0 ) },
		{ vec4(0,  0.2, 0,1.0 ) },
		{ vec4(0,0,  0.0, 1.0 ) },
		{ vec4(-0.2,0,  0.0, 1.0 ) },
	};
	int scaleVAO = loadBufferData(scaleData, 22);
	scaleGeometry = Geometry(scaleVAO,22,GL_LINES);
}

void loadGeometry(){
	buildCapacitor();
	buildResistor();
	buildTransistor();
	buildLine();
	buildMenuRect();
	buildMoveGeometry();
	buildRotateGeometry();
	buildScaleGeometry();
}

GLuint loadBufferData(Vertex* vertices, int vertexCount) {
	GLuint vertexArrayObject;

	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionAttribute);
	glVertexAttribPointer(positionAttribute, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);

	return vertexArrayObject;
}

void display() {	
	glClearColor(0.9, 0.9, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);

	mat4 projection = Ortho(-WINDOW_WIDTH/4, WINDOW_WIDTH/4, -WINDOW_HEIGHT/4, WINDOW_HEIGHT/4, -1.0, 1.0);
	glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, projection);

	renderScene(false);

	drawMenu();

	glutSwapBuffers();

	Angel::CheckError();
}

bool isBetween(int a, int size, int count)
{
	if(a >= count*size && a < (count+1)*size)
		return true;
	return false;
}

menu_item getSelectedMenuItem(int x, int y)
{
	int menuItemSize = 2*menuSize;
	if(isBetween(y, menuItemSize, 0))
	{
		if(isBetween(x, menuItemSize, 0))
			return insert_capacitor;
		else if(isBetween(x, menuItemSize, 1))
			return insert_resistor;
		else if(isBetween(x, menuItemSize, 2))
			return insert_transistor;
		else if(isBetween(x, menuItemSize, 3))
			return insert_line;
	}
	else if(isBetween(y, menuItemSize, 1)) 
	{
		if(isBetween(x, menuItemSize, 0))
			return move_item;
		else if(isBetween(x, menuItemSize, 1))
			return rotate_item;
		else if(isBetween(x, menuItemSize, 2))
			return scale_item;
		else if(isBetween(x, menuItemSize, 3))
			return delete_item;
	}
	return menu_none;
}

vec2 toWorldCoordinates(int x, int y)
{
	int width = WINDOW_WIDTH/2;
	int height = WINDOW_HEIGHT/2;
	return vec2((x-width)/2, (height-y)/2);
}

int gGetAngleABC(vec2 a, vec2 b, vec2 c)
{
    vec2 ab = vec2(b.x - a.x, b.y - a.y);
    vec2 cb = vec2(b.x - c.x, b.y - c.y);

    float dot = (ab.x * cb.x + ab.y * cb.y); // dot product
    float cross = (ab.x * cb.y - ab.y * cb.x); // cross product

    float alpha = atan2(cross, dot);

    return (int) floor(alpha * 180. / M_PI + 0.5);
}

void motion(int x, int y)
{
	mouseOver = getSelectedMenuItem(x,y);
	// todo: transform from windows coordinates to world coordinates
	// if component is selected then perform a 
	// translate, rotate or scale of the selected component
	// save mouse position for later
	
	int angle = 0;
	vec2 worldCoordinates = toWorldCoordinates(x,y);
	vec2 pos_offset = selectedComponentData.selectedGeomPosition - selectedComponentData.selectionPoint;

	if(selectedComponent != -1 && selectedItem != -1)
	{
		switch (selectedItem)
		{
		case move_item:
			components[selectedComponent].translate = worldCoordinates + pos_offset;
			glutPostRedisplay();
			break;
		case rotate_item:
			angle = gGetAngleABC(selectedComponentData.selectionPoint, selectedComponentData.selectedGeomPosition, worldCoordinates);
			cout << angle << endl;
			components[selectedComponent].rotationDegrees = selectedComponentData.selectedGeomRotation + angle;
			glutPostRedisplay();
			break;
		case scale_item:
			{
				vec2 offset = selectedComponentData.selectedGeomPosition - worldCoordinates;
				float scaleFactor = length(offset)/length(pos_offset);
				components[selectedComponent].scale = selectedComponentData.scale * scaleFactor;
				glutPostRedisplay();
			}
			break;
		}
	}
}

void drawMenu(){
	for (int i = 0; i < 4; i++) {
		float dx = (i + .5) * menuSize;
		for (int y = 0, s = 0; y < 2 * menuSize;y += menuSize, s++) {
			int index = i + s * 4;
			mat4 modelView = Translate(-WINDOW_WIDTH/4.0+(i+.5)*menuSize, +WINDOW_HEIGHT/4.0-(0.5*menuSize)-y,0)*Scale(menuSize*.5,menuSize*.5,menuSize*.5);
			glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, modelView);
			for (int j = 0; j < 2; j++) {
				vec4 color;
				if (j == 0) {
					if (index == mouseOver) {
						color = vec4(0.8, 0.8, 0.8, 1.0);
					} else if (index == selectedItem) {
						color = vec4(0.9, 0.9, 0.9, 1.0);
					} else {
						color = vec4(1, 1, 1, 1);	
					}
				} else {
					color = vec4(0, 0, 0, 1);
				}
				glUniform4fv(colorUniform, 1, color);
				bool drawSolid = j==0;
				if (drawSolid) {
					menuRectGeometry.draw();
				} else {
					glDrawArrays(GL_LINE_LOOP,0,menuRectGeometry.size);
				}
			}
			float dy = y+0.5*menuSize;

			modelView = modelView * Scale(.7,.7,.7);
			glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, modelView);

			switch(index){
			case insert_capacitor:
				capacitorGeometry.draw();
				break;
			case insert_resistor:
				resistorGeometry.draw();
				break;
			case insert_transistor:
				transistorGeometry.draw();
				break;
			case insert_line:
				lineGeometry.draw();
				break;
			case move_item:
				moveGeometry.draw();
				break;
			case rotate_item:
				rotateGeometry.draw();
				break;
			case scale_item:
				scaleGeometry.draw();
				break;
			case delete_item:
				modelView = modelView * RotateZ(45);
				glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, modelView);
				lineGeometry.draw();
				modelView = modelView * RotateZ(90);
				glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, modelView);
				lineGeometry.draw();
				break;
			}
		}
	}
}

void reshape(int W, int H) {
	WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	if (selectBuffer != NULL){
		delete selectBuffer;
	}
	selectBuffer = new SelectBuffer(W,H);
}

void loadShader() 
{
	shaderProgram = InitShader("const-shader.vert",  "const-shader.frag", "fragColour");
	projectionUniform = glGetUniformLocation(shaderProgram, "projection");
	if (projectionUniform < 0) {
		cerr << "Shader did not contain the 'projection' uniform."<<endl;
	}
	modelViewUniform = glGetUniformLocation(shaderProgram, "modelView");
	if (modelViewUniform < 0) {
		cerr << "Shader did not contain the 'modelView' uniform."<<endl;
	}
	colorUniform = glGetUniformLocation(shaderProgram, "color");
	if (modelViewUniform < 0) {
		cerr << "Shader did not contain the 'color' uniform."<<endl;
	}
	positionAttribute = glGetAttribLocation(shaderProgram, "position");
	if (positionAttribute < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
}

// Called when selected item is insert_capacitor, Insert_resistor, insert_transistor, or insert_line
// and when left mouse button is clicked outside menu
void insertComponent(int x, int y)
{
	// Insert an instance of the selected component into the vector components.
	// The position should be at the current world position
	// Scale should be 40x40 and Rotation should be 0

	vec2 coordinates = toWorldCoordinates(x, y);
	vec2 scale(40, 40);

	switch (selectedItem)
	{
	case insert_capacitor:
		{
			component_t newComponent = component_t(capacitor, coordinates, 0, scale, capacitorGeometry);
			components.push_back(newComponent); 
		}
		break;
	case insert_resistor:
		{
			component_t newComponent = component_t(resistor, coordinates, 0, scale, resistorGeometry);
			components.push_back(newComponent); 
		}
		break;
	case insert_transistor:
		{
			component_t newComponent = component_t(transistor, coordinates, 0, scale, transistorGeometry);
			components.push_back(newComponent); 
		}
		break;
	case insert_line:
		{
			component_t newComponent = component_t(line, coordinates, 0, scale, lineGeometry);
			components.push_back(newComponent); 
		}
		break;
	default:
		break;
	}
}

int selectObject(int x, int y) {
	// Render the scene to the select buffer and use the select buffer to 
	// test for objects close to x,y (in radius of 5 pixels)
	// Return object id.
	// if no object is found then return -1

	selectBuffer->setColorUniform(colorUniform);
	selectBuffer->bind();
	renderScene(true);
	int id = -1;
	int size = 5;
	for (int i = 0; i <= 2*size && id < 0; i++)
		for (int j = 0; j <= 2*size && id < 0; j++)
			id = selectBuffer->getId(x+i-size,WINDOW_HEIGHT-(y+i-size));
	selectBuffer->release();
	return id;
}

void mouse(int button, int state, int x, int y) {
	bool leftMouseDown = button == GLUT_LEFT_BUTTON && state == GLUT_DOWN; 
	bool leftMouseReleased = button == GLUT_LEFT_BUTTON && state == GLUT_UP;

	if (mouseOver != menu_none) {
		selectedItem = mouseOver;
	} 
	else 
	{
		if (selectedItem==move_item || selectedItem == rotate_item || selectedItem == scale_item)
		{
			if (leftMouseDown)
			{
				selectedComponent =  selectObject(x, y);
				if(selectedComponent != -1)
				{
					selectedComponentData.selectionPoint = toWorldCoordinates(x, y);
					selectedComponentData.selectedGeomPosition = components[selectedComponent].translate;
					selectedComponentData.selectedGeomRotation = components[selectedComponent].rotationDegrees;
					selectedComponentData.scale = components[selectedComponent].scale.x;
				}
			} 
			else if (leftMouseReleased) 
			{
				selectedComponent = -1;
			}
		}
		else if( selectedItem==delete_item)
		{
			if (leftMouseDown)
			{
				selectedComponent =  selectObject(x, y);
				if(selectedComponent != -1)
					components.erase(components.begin() + selectedComponent);
				selectedComponent = -1;
			} 	
		}
		else if (leftMouseReleased) 
		{
			insertComponent(x,y);
		}
	}
	glutPostRedisplay();
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
	glutCreateWindow("02561-04-03");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	Angel::InitOpenGL();

	loadShader();
	loadGeometry();

	Angel::CheckError();

	glutMainLoop();
}