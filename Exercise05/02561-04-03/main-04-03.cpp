// 02561-01-01 Hello triangle world

#include <iostream>
#include <string>
#include <vector>
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
		// todo implement
		// update the modelView and render the component
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
vector<component_t> components;

// mouse x and mouse y in world coordinates
int mx = 0, my = 0;

// forward declaration
void loadShader();
void display();
void drawMenu();
GLuint loadBufferData(Vertex* vertices, int vertexCount);
void reshape(int W, int H);

void renderScene(bool select){
		// todo draw components[i] to screen or select buffer
		// if select mode, then use the variable 'i' as object-id. 
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
	// transistorGeometry = ;
	// todo insert code here
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


menu_item getSelectedMenuItem(int x, int y){

	return menu_none;
}

void motion(int x, int y)
{
    mouseOver = getSelectedMenuItem(x,y);
	// todo: transform from windows coordinates to world coordinates
	// if component is selected then perform a 
	// translate, rotate or scale of the selected component
	//save mouse position for later
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


void loadShader() {
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
void insertComponent(int x, int y){
	// Insert an instance of the selected component into the vector components.
	// The position should be at the current world position
	// Scale should be 40x40 and Rotation should be 0
}

int selectObject(int x, int y) {
	// Render the scene to the select buffer and use the select buffer to 
	// test for objects close to x,y (in radius of 5 pixels)
	// Return object id.
	// if no object is found then return -1

  return -1;
}

void mouse(int button, int state, int x, int y) {
	bool leftMouseDown = button == GLUT_LEFT_BUTTON && state == GLUT_DOWN; 
	bool leftMouseReleased = button == GLUT_LEFT_BUTTON && state == GLUT_UP;

	if (mouseOver != menu_none) {
		selectedItem = mouseOver;
	} else {
		if (selectedItem==move_item || selectedItem == rotate_item || selectedItem == scale_item){
			if (leftMouseDown){
				selectedComponent =  selectObject(x, y);
			} else if (leftMouseReleased) {
				selectedComponent = -1;
			}
		} else if (leftMouseReleased) {
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