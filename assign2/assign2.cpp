// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 480 Computer Graphics
	Assignment 2: Simulating a Roller Coaster
	C++ starter code
	*/

#include "stdafx.h"
#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>

/* represents one control point along the spline */
struct point {
	double x;
	double y;
	double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
	int numControlPoints;
	struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

/* values for gluLookAt */
double eyeX = 0.0, eyeY = 0.0, eyeZ = 5.0;
double centerX = 0.0, centerY = 0.0, centerZ = 0.0;
double upX = 0.0, upY = 0.0, upZ = 1.0;

/* display list */
GLuint theSpline;
/* textures */
GLuint groundTexture;
GLuint skyTexture;
Pic *imageGround;
Pic *imageSky;

spline rollercoasterSpline;
int pointCount = 0;
spline rollercoasterSplineTangents;
int tangentCount = 0;


int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf("can't open file\n");
		exit(1);
	}

	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);

	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf",
			&g_Splines[j].points[i].x,
			&g_Splines[j].points[i].y,
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

void initGroundTexture() {
	imageGround = jpeg_read("dirtTexture.jpg", NULL);

	// create placeholder for texture
	glGenTextures(1, &groundTexture);

	// make texture active
	glBindTexture(GL_TEXTURE_2D, groundTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// use linear filter both for magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// load image data stored at pointer "groundPointer" into currently active texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, imageGround->pix);
}

void initSkyTexture() {
	imageSky = jpeg_read("skyPic.jpeg", NULL);

	// create placeholder for texture
	glGenTextures(1, &skyTexture);

	// make texture active
	glBindTexture(GL_TEXTURE_2D, skyTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// use linear filter both for magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// load image data stored at pointer "groundPointer" into currently active texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, imageSky->pix);
}

/* openGL init */
void myInit() {
	// Create spline and store it in display list
	rollercoasterSpline.numControlPoints = g_Splines[0].numControlPoints * 20;
	rollercoasterSpline.points = new point[rollercoasterSpline.numControlPoints];

	rollercoasterSplineTangents.numControlPoints = rollercoasterSpline.numControlPoints;
	rollercoasterSplineTangents.points = new point[rollercoasterSplineTangents.numControlPoints];

	double s = 0.5;

	double basisMatrix[4][4] = {
		{ -s, (2 - s), (s - 2), s },
		{ (2 * s), (s - 3), (3 - (2 * s)), -s },
		{ -s, 0, s, 0 },
		{ 0, 1, 0, 0 }
	};

	double resultMatrix[4][3] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// = basis matrix * control matrix

	/* create the display list */
	theSpline = glGenLists(1);
	glNewList(theSpline, GL_COMPILE);

	/* compute the list*/
	glBegin(GL_LINE_STRIP);
	glColor3f(1.0, 1.0, 1.0);

	int numPoints = g_Splines->numControlPoints;

	for (int l = 0; l < numPoints - 3; l++) {
		double x1 = g_Splines[0].points[l].x;
		double x2 = g_Splines[0].points[l + 1].x;
		double x3 = g_Splines[0].points[l + 2].x;
		double x4 = g_Splines[0].points[l + 3].x;
		double y1 = g_Splines[0].points[l].y;
		double y2 = g_Splines[0].points[l + 1].y;
		double y3 = g_Splines[0].points[l + 2].y;
		double y4 = g_Splines[0].points[l + 3].y;
		double z1 = g_Splines[0].points[l].z;
		double z2 = g_Splines[0].points[l + 1].z;
		double z3 = g_Splines[0].points[l + 2].z;
		double z4 = g_Splines[0].points[l + 3].z;
		// create control matrix
		double controlMatrix[4][3] = {
			{ x1, y1, z1 },
			{ x2, y2, z2 },
			{ x3, y3, z3 },
			{ x4, y4, z4 }
		};
		// clear result matrix
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 3; j++) {
				resultMatrix[i][j] = 0;
			}
		}
		// multiply basis matrix by control matrix and store in result matrix
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < 4; k++) {
					resultMatrix[i][j] += basisMatrix[i][k] * controlMatrix[k][j];
				}
			}
		}
		// multiply by u array to get x y z coords
		double x = 0, y = 0, z = 0;
		for (double i = 0.05; i <= 1; i += 0.05) {
			double u1 = i;
			double u2 = i * i;
			double u3 = i * i * i;

			x = (u3 * resultMatrix[0][0]) + (u2 * resultMatrix[1][0]) + (u1 * resultMatrix[2][0]) + resultMatrix[3][0];
			y = (u3 * resultMatrix[0][1]) + (u2 * resultMatrix[1][1]) + (u1 * resultMatrix[2][1]) + resultMatrix[3][1];
			z = (u3 * resultMatrix[0][2]) + (u2 * resultMatrix[1][2]) + (u1 * resultMatrix[2][2]) + resultMatrix[3][2];

			glVertex3f(x, y, z);

			// store spline globally in a spline struct

			pointCount++;
			rollercoasterSpline.points[pointCount].x = x;
			rollercoasterSpline.points[pointCount].y = y;
			rollercoasterSpline.points[pointCount].z = z;
		}
		// multiply by u' array to get x y z tangents
		double xTan = 0, yTan = 0, zTan = 0;
		for (double i = 0.05; i <= 1; i += 0.05) {
			double u0 = 0;
			double u1 = 1;
			double u2 = 2 * i;
			double u3 = 3 * i * i;

			xTan = (u3 * resultMatrix[0][0]) + (u2 * resultMatrix[1][0]) + (u1 * resultMatrix[2][0]) + (u0 * resultMatrix[3][0]);
			yTan = (u3 * resultMatrix[0][1]) + (u2 * resultMatrix[1][1]) + (u1 * resultMatrix[2][1]) + (u0 * resultMatrix[3][1]);
			zTan = (u3 * resultMatrix[0][2]) + (u2 * resultMatrix[1][2]) + (u1 * resultMatrix[2][2]) + (u0 * resultMatrix[3][2]);

			// store spline tangents globally

			tangentCount++;
			rollercoasterSplineTangents.points[tangentCount].x = xTan;
			rollercoasterSplineTangents.points[tangentCount].y = yTan;
			rollercoasterSplineTangents.points[tangentCount].z = zTan;
		}
	}
	pointCount = 0;
	tangentCount = 0;

	glEnd();

	glEndList();

	initGroundTexture();
	initSkyTexture();

	/* setup gl view here */
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

/* Draw Ground */
/* large plane - texture mapped */
void renderGround() {

	// no modulation of texture color with lighting
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, groundTexture);
	// turn on texture mapping
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-20.0, -20.0, -1.0);
	glTexCoord2f(0.0, 30.0);
	glVertex3f(-20.0, 50.0, -1.0);
	glTexCoord2f(30.0, 0.0);
	glVertex3f(50.0, 50.0, -1.0);
	glTexCoord2f(30.0, 30.0);
	glVertex3f(50.0, -20.0, -1.0);

	glEnd();

	glDisable(GL_TEXTURE_2D);
}

/* Draw Skybox */
/* Can use a cube - dome is better */
void renderSky() {
	// http://www.flipcode.com/archives/Sky_Domes.shtml

	// no modulation of texture color with lighting
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, skyTexture);
	// turn on texture mapping
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	// quad side one
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-20.0, -20.0, -1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(50.0, -20.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(50.0, -20.0, 50.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-20.0, -20.0, 50.0);

	// quad side two
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-20.0, -20.0, -1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-20.0, 50.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-20.0, 50.0, 50.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-20.0, -20.0, 50.0);

	// quad side three
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-20.0, 50.0, -1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(50.0, 50.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(50.0, 50.0, 50.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-20.0, 50.0, 50.0);

	// quad side four
	glTexCoord2f(1.0, 1.0);
	glVertex3f(50.0, 50.0, -1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(50.0, -20.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(50.0, -20.0, 50.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(50.0, 50.0, 50.0);

	glEnd();

	glDisable(GL_TEXTURE_2D);
}

/* draw spline with material propterties */
// recursive subdivision is extra credit....
// subdivide(u0, d1, maxLineLength)
// umid = (u0 + u1) / 2
// x0 = f(u0)  x1 = f(u1)
// if  | x1 - x0 | > maxLineLength
//		subdivide (u0, umid, maxLineLength)
//		subdivide (umid, u1, maxLineLength)
// else drawLine(x0, x1)
//
// can subdivide based on curvature instead (2nd derivative of f)
void renderSpline() {

	glColor3f(1.0, 1.0, 1.0);
	glCallList(theSpline);
	glFlush();
}

// setting camera values for animation
void animateRide() {
	
	if (pointCount == rollercoasterSpline.numControlPoints) {
		pointCount = 0;
	}
	pointCount++;
	eyeX = rollercoasterSpline.points[pointCount].x;
	eyeY = rollercoasterSpline.points[pointCount].y;
	eyeZ = rollercoasterSpline.points[pointCount].z;

	double x = rollercoasterSplineTangents.points[pointCount].x;
	double y = rollercoasterSplineTangents.points[pointCount].y;
	double z = rollercoasterSplineTangents.points[pointCount].z;

	double magnitude = sqrt((x - eyeX)*(x - eyeX) + (y - eyeY)*(y - eyeY) + (z - eyeZ)*(z - eyeZ));

	centerX = (x / magnitude) + eyeX;
	centerY = y / magnitude + eyeY;
	centerZ = z / magnitude + eyeZ;

	std::cout << "X :  " << centerX << "   Y :  " << centerY << "   Z :  " << centerZ << std::endl;
	
	/*
	upX = (eyeY * centerZ) - (eyeZ * centerY);
	upY = (eyeZ * centerX) - (eyeX * centerZ);
	upZ = (eyeX * centerY) - (eyeY * centerX);
	*/
}

/***** MAIN DISPLAY FUNCTION *****/
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	glShadeModel(GL_SMOOTH);

	glLoadIdentity();

	//animateRide();

	/* camera view */
	gluLookAt(eyeX, eyeY, eyeZ,
		centerX, centerY, centerZ,
		upX, upY, upZ);
	/* Move Camera - camera points along the tangent vector of the curve
		t(u) = unit(f'(u)) = unit([3u^2 2u 1 0] M C) */
	/* Establish a local coord system for each point on the curve*/
	/* T tangent, N, B   -- make camera up vector = N or B

	Camera Speed - extra credit - realistic in terms of gravity
	u_new = u_current + (delta(t)) * sq(2g(h_max - h))/||dp/du||
	h_max = max height anywhere on the rollercoaster + a tiny bit
	h = current height
	p = position. | dp/du | = length of position

	kinetic energy + potential energy = constant
	*/

	renderSky();
	renderGround();
	renderSpline();

	/* needed for double buffering*/
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/* Perspective camera view */
	gluPerspective(60.0, aspect, .1, 1000);
	glMatrixMode(GL_MODELVIEW);
}

/* called when no other events are being called */
void idle()
{
	/* make the screen update */
	glutPostRedisplay();
}

/* keyboard special button presses */
void keySpecial(int key, int x, int y)
{
	switch (glutGetModifiers()) {
		/* change eyeZ position. essentially, zoom in and out */
	case GLUT_ACTIVE_ALT:
		switch (key) {
		case GLUT_KEY_UP:
			eyeZ -= 1.0;
			break;
		case GLUT_KEY_DOWN:
			eyeZ += 1.0;
			break;
		case GLUT_KEY_LEFT:
			centerZ -= 1.0;
			break;
		case GLUT_KEY_RIGHT:
			centerZ += 1.0;
			break;
		}
		break;
		/* change the center position - where the camera is looking */
	case GLUT_ACTIVE_CTRL:
		switch (key) {
		case GLUT_KEY_UP:
			centerY += 1.0;
			break;
		case GLUT_KEY_DOWN:
			centerY -= 1.0;
			break;
		case GLUT_KEY_LEFT:
			centerX -= 1.0;
			break;
		case GLUT_KEY_RIGHT:
			centerX += 1.0;
			break;
		}
		break;
		/* change the eye position - where the camera is positioned */
	case GLUT_ACTIVE_SHIFT:
		switch (key) {
		case GLUT_KEY_UP:
			eyeY += 1.0;
			break;
		case GLUT_KEY_DOWN:
			eyeY -= 1.0;
			break;
		case GLUT_KEY_LEFT:
			eyeX -= 1.0;
			break;
		case GLUT_KEY_RIGHT:
			eyeX += 1.0;
			break;
		}
		break;
		/* change the eye and center positions */
	default:
		switch (key) {
		case GLUT_KEY_UP:
			eyeY += 1.0;
			centerY += 1.0;
			break;
		case GLUT_KEY_DOWN:
			eyeY -= 1.0;
			centerY -= 1.0;
			break;
		case GLUT_KEY_LEFT:
			eyeX -= 1.0;
			centerX -= 1.0;
			break;
		case GLUT_KEY_RIGHT:
			eyeX += 1.0;
			centerX += 1.0;
			break;
		}
		break;
	}

	switch (key) {
	case GLUT_KEY_PAGE_UP:
		if (upX == 1.0) {
			upY = 1.0;
			upX = 0.0;
		}
		else if (upY == 1.0) {
			upZ = 1.0;
			upY = 0.0;
		}
		else if (upZ == 1.0) {
			upX = 1.0;
			upZ = 0.0;
		}
		break;
	case GLUT_KEY_PAGE_DOWN:
		animateRide();
		break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	// I've set the argv[1] to track.txt.
	// To change it, on the "Solution Explorer",
	// right click "assign2", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your track file name for the "Command Arguments"
	if (argc < 2)
	{
		printf("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	loadSplines(argv[1]);

	/* initialization */
	glutInit(&argc, (char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	/* create window */
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("CSCI 420 Assignment 2 - Rollercoaster");

	/* used for double buffering */
	glEnable(GL_DEPTH_TEST);

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	/* replace with any animate code */
	glutIdleFunc(idle);

	/* user keyboard input */
	glutSpecialFunc(keySpecial);

	/* enable materials */

	myInit();

	glutMainLoop();

	return 0;
}