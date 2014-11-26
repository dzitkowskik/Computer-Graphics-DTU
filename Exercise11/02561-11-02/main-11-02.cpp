#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Angel.h"
#include "NURBSCurve.h"
#include "NURBSRenderer.h"
#include <math.h>

using namespace std;
using namespace Angel;

mat4 projection;

NURBSRenderer *curveRenderer = NULL;
bool renderPoints = true;

void display() {	
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);	
	
	mat4 modelView = LookAt(vec4(0,0,1,1), vec4(0,0,0,1), vec4(0,1,0,0)); // identity modelView matrix
	
	curveRenderer->setColor(vec4(1,0,0,1));
	curveRenderer->render(projection, modelView);
	
	if (renderPoints){
		curveRenderer->setColor(vec4(0,0,1,1));
		curveRenderer->renderControlPoints(projection, modelView);
	}

	glutSwapBuffers();

	Angel::CheckError();
}

void reshape(int w, int h) {
	if (w <= h)
      projection = Ortho2D (-2.0, 2.0, -2.0*(GLfloat)h/(GLfloat)w, 
               2.0*(GLfloat)h/(GLfloat)w);
   else
      projection = Ortho2D(-2.0*(GLfloat)w/(GLfloat)h, 
               2.0*(GLfloat)w/(GLfloat)h, -2.0, 2.0);
    glViewport(0, 0, w, h);
}

void setupCurve(){
	float p = M_PI;
	float w = sqrtf(2.0f)/2.0f;
	const int numberOfControlPoints = 9;
	vec4 controlPoints [numberOfControlPoints] = {
		vec4(-1, 0, 0, 1),
		vec4(-1, 1, 0, w),
		vec4( 0, 1, 0, 1),
		vec4( 1, 1, 0, w),
		vec4( 1, 0, 0, 1),
		vec4( 1, -1, 0, w),
		vec4( 0, -1, 0, 1),
		vec4( -1, -1, 0, w),
		vec4(-1, 0, 0, 1)};

	NURBSCurve *curve = new NURBSCurve(numberOfControlPoints);

	for (int i=0;i<numberOfControlPoints;i++){
		curve->setControlPoint(i, controlPoints[i]);
	}
	const int numberOfKnots = 12;
	float knotVector[numberOfKnots] = 
		{0, 0, 0, p/2.0, p/2.0, p, p, 3.0*p/2.0, 3.0*p/2.0, 2.0*p, 2.0*p, 2.0*p};
	curve->setKnotVector(numberOfKnots, knotVector);
	curveRenderer = new NURBSRenderer(curve);	
}

void keyboard(unsigned char c, int x, int y){
	switch (c){
	case 'p':
	case 'P':
		renderPoints = !renderPoints;
		break;
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
	glutCreateWindow("02561-11-02");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutReshapeWindow(500, 500);

	Angel::InitOpenGL();

	setupCurve();
	
	Angel::CheckError();

	glutMainLoop();
}
