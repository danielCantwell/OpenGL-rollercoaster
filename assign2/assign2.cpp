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
GLuint skyTopTexture;
GLuint railTexture;
Pic *imageGround;
Pic *imageSky;
Pic *imageSkyTop;
Pic *imageRail;

spline rollercoasterSpline;
int pointCount = 0;
spline tangents;
spline normals;
spline binormals;

/* lighting */
GLfloat light_diffuse[] = { 0.0, 1.0, 1.0, 1.0 };
GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };


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

void initRailTexture() {
	imageGround = jpeg_read("woodTexture.jpg", NULL);

	// create placeholder for texture
	glGenTextures(1, &railTexture);

	// make texture active
	glBindTexture(GL_TEXTURE_2D, railTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// use linear filter both for magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// load image data stored at pointer "groundPointer" into currently active texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, imageGround->pix);
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


	//// SKY BOX - TOP

	imageSkyTop = jpeg_read("skytop.jpg", NULL);

	// create placeholder for texture
	glGenTextures(1, &skyTopTexture);

	// make texture active
	glBindTexture(GL_TEXTURE_2D, skyTopTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// use linear filter both for magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// load image data stored at pointer "groundPointer" into currently active texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, imageSkyTop->pix);
}

void calculateNormalsAndBinormals() {

	// -----------------------------------------------------------------------
	// -------------- calculate initial normal -------------------------------

	double xTan = tangents.points[0].x;
	double yTan = tangents.points[0].y;
	double zTan = tangents.points[0].z;

	double arbitraryX = 0, arbitraryY = 1, arbitraryZ = 0;
	double xNorm = (yTan * arbitraryZ) - (zTan * arbitraryY);
	double yNorm = (zTan * arbitraryX) - (xTan * arbitraryZ);
	double zNorm = (xTan * arbitraryY) - (yTan * arbitraryX);

	// calculate the magnitude of the normal vector for normalization
	double normalMagnitude = sqrt((xNorm*xNorm) + (yNorm*yNorm) + (zNorm*zNorm));
	// normalize the normal vector
	xNorm = xNorm / normalMagnitude;
	yNorm = yNorm / normalMagnitude;
	zNorm = zNorm / normalMagnitude;

	// store spline normals globally

	normals.points[0].x = xNorm;
	normals.points[0].y = yNorm;
	normals.points[0].z = zNorm;

	// ----------------------------------------------------------------------
	// ----------------- calculate initial binormal -------------------------

	double xBin = (yTan * zNorm) - (zTan * yNorm);
	double yBin = (zTan * xNorm) - (xTan * zNorm);
	double zBin = (xTan * yNorm) - (yTan * xNorm);

	// calculate the magnitude of the binormal vector for normalization
	double binormalMagnitude = sqrt((xBin*xBin) + (yBin*yBin) + (zBin*zBin));
	// normalize the binormal vector
	xBin = xBin / binormalMagnitude;
	yBin = yBin / binormalMagnitude;
	zBin = zBin / binormalMagnitude;

	// store the spline binormals globally
	binormals.points[0].x = xBin;
	binormals.points[0].y = yBin;
	binormals.points[0].y = zBin;


	// ----------------------------------------------------------------------
	// calculate normals  ----------  B[count - 1]  x  T[count] -------------

	for (int i = 1; i < rollercoasterSpline.numControlPoints; i++) {

		xBin = binormals.points[i - 1].x;
		yBin = binormals.points[i - 1].y;
		zBin = binormals.points[i - 1].z;

		xTan = tangents.points[i].x;
		yTan = tangents.points[i].y;
		zTan = tangents.points[i].z;

		xNorm = (yBin * zTan) - (zBin * yTan);
		yNorm = (zBin * xTan) - (xBin * zTan);
		zNorm = (xBin * yTan) - (yBin * xTan);

		// calculate the magnitude of the normal vector for normalization
		normalMagnitude = sqrt((xNorm*xNorm) + (yNorm*yNorm) + (zNorm*zNorm));
		// normalize the normal vector
		xNorm = xNorm / normalMagnitude;
		yNorm = yNorm / normalMagnitude;
		zNorm = zNorm / normalMagnitude;

		// store spline normals globally

		normals.points[i].x = xNorm;
		normals.points[i].y = yNorm;
		normals.points[i].z = zNorm;

	}

	// ---------------------------------------------------------------------
	// calculate binormals ----------  tangent vector  x  normal vector ----

	for (int i = 1; i < rollercoasterSpline.numControlPoints; i++) {

		xNorm = normals.points[i].x;
		yNorm = normals.points[i].y;
		zNorm = normals.points[i].z;

		xTan = tangents.points[i].x;
		yTan = tangents.points[i].y;
		zTan = tangents.points[i].z;

		xBin = (yTan * zNorm) - (zTan * yNorm);
		yBin = (zTan * xNorm) - (xTan * zNorm);
		zBin = (xTan * yNorm) - (yTan * xNorm);

		// calculate the magnitude of the binormal vector for normalization
		binormalMagnitude = sqrt((xBin*xBin) + (yBin*yBin) + (zBin*zBin));
		// normalize the binormal vector
		xBin = xBin / binormalMagnitude;
		yBin = yBin / binormalMagnitude;
		zBin = zBin / binormalMagnitude;

		// store the spline binormals globally
		binormals.points[i].x = xBin;
		binormals.points[i].y = yBin;
		binormals.points[i].z = zBin;

	}
}

void initSpline() {
	// Create spline and store it in display list

	rollercoasterSpline.numControlPoints = g_Splines[0].numControlPoints * 20;
	rollercoasterSpline.points = new point[rollercoasterSpline.numControlPoints];

	tangents.numControlPoints = rollercoasterSpline.numControlPoints;
	tangents.points = new point[tangents.numControlPoints];

	normals.numControlPoints = rollercoasterSpline.numControlPoints;
	normals.points = new point[normals.numControlPoints];

	binormals.numControlPoints = rollercoasterSpline.numControlPoints;
	binormals.points = new point[binormals.numControlPoints];


	double s = 0.5;

	double basisMatrix[4][4] = {
		{ -s, (2 - s), (s - 2), s },
		{ (2 * s), (s - 3), (3 - (2 * s)), -s },
		{ -s, 0, s, 0 },
		{ 0, 1, 0, 0 }
	};

	double resultMatrix[4][3] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// = basis matrix * control matrix

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
		// multiply by u' array to get x y z tangents
		double xTan = 0, yTan = 0, zTan = 0;
		double xNorm = 0, yNorm = 0, zNorm = 0;
		for (double i = 0.05; i <= 1; i += 0.05) {
			double u1 = i;
			double u2 = i * i;
			double u3 = i * i * i;

			double u1Prime = 1;
			double u2Prime = 2 * i;
			double u3Prime = 3 * i * i;

			// ----------------------------------------------------------------------
			// calculate vertices ---------------------------------------------------

			x = (u3 * resultMatrix[0][0]) + (u2 * resultMatrix[1][0]) + (u1 * resultMatrix[2][0]) + resultMatrix[3][0];
			y = (u3 * resultMatrix[0][1]) + (u2 * resultMatrix[1][1]) + (u1 * resultMatrix[2][1]) + resultMatrix[3][1];
			z = (u3 * resultMatrix[0][2]) + (u2 * resultMatrix[1][2]) + (u1 * resultMatrix[2][2]) + resultMatrix[3][2];

			// store the vertices in display list
			glVertex3f(x, y, z);

			// store spline globally in a spline struct
			pointCount++;
			rollercoasterSpline.points[pointCount].x = x;
			rollercoasterSpline.points[pointCount].y = y;
			rollercoasterSpline.points[pointCount].z = z;

			// -----------------------------------------------------------------------
			// calculate tangents ----------------------------------------------------

			xTan = (u3Prime * resultMatrix[0][0]) + (u2Prime * resultMatrix[1][0]) + (u1Prime * resultMatrix[2][0]);
			yTan = (u3Prime * resultMatrix[0][1]) + (u2Prime * resultMatrix[1][1]) + (u1Prime * resultMatrix[2][1]);
			zTan = (u3Prime * resultMatrix[0][2]) + (u2Prime * resultMatrix[1][2]) + (u1Prime * resultMatrix[2][2]);

			// calculate the magnitude of the tangent vector for normalization
			double tangentMagnitude = sqrt((xTan*xTan) + (yTan*yTan) + (zTan*zTan));

			xTan = xTan / tangentMagnitude;
			yTan = yTan / tangentMagnitude;
			zTan = zTan / tangentMagnitude;

			// store spline tangents globally

			tangents.points[pointCount].x = xTan;
			tangents.points[pointCount].y = yTan;
			tangents.points[pointCount].z = zTan;
		}
	}
	pointCount = 0;
}

/* openGL init */
void myInit() {

	/* Enable a single OpenGL light. */
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	initGroundTexture();
	initSkyTexture();
	initRailTexture();
	initSpline();
	calculateNormalsAndBinormals();

	/* create the display list */
	theSpline = glGenLists(1);
	glNewList(theSpline, GL_COMPILE);

	// modulate texture with lighting
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(GL_TEXTURE_2D, railTexture);
	// turn on texture mapping
	glEnable(GL_TEXTURE_2D);

	/* compute the list*/
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);

	double alpha = 0.05;
	for (int i = 0; i < rollercoasterSpline.numControlPoints; i++) {
		double pointX = rollercoasterSpline.points[i].x;
		double pointY = rollercoasterSpline.points[i].y;
		double pointZ = rollercoasterSpline.points[i].z;
		double normX = normals.points[i].x;
		double normY = normals.points[i].y;
		double normZ = normals.points[i].z;
		double binX = binormals.points[i].x;
		double binY = binormals.points[i].y;
		double binZ = binormals.points[i].z;

		double pointX2 = rollercoasterSpline.points[i + 1].x;
		double pointY2 = rollercoasterSpline.points[i + 1].y;
		double pointZ2 = rollercoasterSpline.points[i + 1].z;
		double normX2 = normals.points[i + 1].x;
		double normY2 = normals.points[i + 1].y;
		double normZ2 = normals.points[i + 1].z;
		double binX2 = binormals.points[i + 1].x;
		double binY2 = binormals.points[i + 1].y;
		double binZ2 = binormals.points[i + 1].z;

		double v0X = pointX - alpha*(normX + binX);
		double v0Y = pointY - alpha*(normY + binY);
		double v0Z = pointZ - alpha*(normZ + binZ);

		double v1X = pointX + alpha*(normX + binX);
		double v1Y = pointY + alpha*(normY + binY);
		double v1Z = pointZ + alpha*(normZ + binZ);

		double v2X = pointX + alpha*(normX - binX);
		double v2Y = pointY + alpha*(normY - binY);
		double v2Z = pointZ + alpha*(normZ - binZ);

		double v3X = pointX - alpha*(normX - binX);
		double v3Y = pointY - alpha*(normY - binY);
		double v3Z = pointZ - alpha*(normZ - binZ);

		double v4X = pointX2 - alpha*(normX2 + binX2);
		double v4Y = pointY2 - alpha*(normY2 + binY2);
		double v4Z = pointZ2 - alpha*(normZ2 + binZ2);

		double v5X = pointX2 + alpha*(normX2 + binX2);
		double v5Y = pointY2 + alpha*(normY2 + binY2);
		double v5Z = pointZ2 + alpha*(normZ2 + binZ2);

		double v6X = pointX2 + alpha*(normX2 - binX2);
		double v6Y = pointY2 + alpha*(normY2 - binY2);
		double v6Z = pointZ2 + alpha*(normZ2 - binZ2);

		double v7X = pointX2 - alpha*(normX2 - binX2);
		double v7Y = pointY2 - alpha*(normY2 - binY2);
		double v7Z = pointZ2 - alpha*(normZ2 - binZ2);

		// // // RAIL TWO // // //
		
		double separation = 0.8;

		double v0X2 = pointX - alpha*(normX + binX) + separation;
		double v0Y2 = pointY - alpha*(normY + binY);
		double v0Z2 = pointZ - alpha*(normZ + binZ);

		double v1X2 = pointX + alpha*(normX + binX) + separation;
		double v1Y2 = pointY + alpha*(normY + binY);
		double v1Z2 = pointZ + alpha*(normZ + binZ);

		double v2X2 = pointX + alpha*(normX - binX) + separation;
		double v2Y2 = pointY + alpha*(normY - binY);
		double v2Z2 = pointZ + alpha*(normZ - binZ);

		double v3X2 = pointX - alpha*(normX - binX) + separation;
		double v3Y2 = pointY - alpha*(normY - binY);
		double v3Z2 = pointZ - alpha*(normZ - binZ);

		double v4X2 = pointX2 - alpha*(normX2 + binX2) + separation;
		double v4Y2 = pointY2 - alpha*(normY2 + binY2);
		double v4Z2 = pointZ2 - alpha*(normZ2 + binZ2);

		double v5X2 = pointX2 + alpha*(normX2 + binX2) + separation;
		double v5Y2 = pointY2 + alpha*(normY2 + binY2);
		double v5Z2 = pointZ2 + alpha*(normZ2 + binZ2);

		double v6X2 = pointX2 + alpha*(normX2 - binX2) + separation;
		double v6Y2 = pointY2 + alpha*(normY2 - binY2);
		double v6Z2 = pointZ2 + alpha*(normZ2 - binZ2);

		double v7X2 = pointX2 - alpha*(normX2 - binX2) + separation;
		double v7Y2 = pointY2 - alpha*(normY2 - binY2);
		double v7Z2 = pointZ2 - alpha*(normZ2 - binZ2);
		
		// // // // // // // // //
		
		
		glNormal3f(normals.points[i].x, normals.points[i].y, normals.points[i].z);

		// right side face
		glTexCoord2f(1.0, 0.0);
		glVertex3f(v0X, v0Y, v0Z);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v1X, v1Y, v1Z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v5X, v5Y, v5Z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v4X, v4Y, v4Z);

		// top face
		glTexCoord2f(1.0, 0.0);
		glVertex3f(v1X, v1Y, v1Z);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v2X, v2Y, v2Z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v6X, v6Y, v6Z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v5X, v5Y, v5Z);
		
		// left side face
		glTexCoord2f(1.0, 0.0);
		glVertex3f(v2X, v2Y, v2Z);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v3X, v3Y, v3Z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v7X, v7Y, v7Z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v6X, v6Y, v6Z);

		// bottom face
		glTexCoord2f(1.0, 0.0);
		glVertex3f(v0X, v0Y, v0Z);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v3X, v3Y, v3Z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v7X, v7Y, v7Z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v4X, v4Y, v4Z);



		// // // RAIL TWO // // //
		// // // // // // // // //
		
		// right side face
		glTexCoord2f(1.0, 0.0);
		glVertex3f(v0X2, v0Y2, v0Z2);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v1X2, v1Y2, v1Z2);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v5X2, v5Y2, v5Z2);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v4X2, v4Y2, v4Z2);

		// top face
		glTexCoord2f(1.0, 0.0);
		glVertex3f(v1X2, v1Y2, v1Z2);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v2X2, v2Y2, v2Z2);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v6X2, v6Y2, v6Z2);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v5X2, v5Y2, v5Z2);

		// left side face
		glTexCoord2f(1.0, 0.0);
		glVertex3f(v2X2, v2Y2, v2Z2);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v3X2, v3Y2, v3Z2);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v7X2, v7Y2, v7Z2);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v6X2, v6Y2, v6Z2);


		// bottom face
		glTexCoord2f(1.0, 0.0);
		glVertex3f(v0X2, v0Y2, v0Z2);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v3X2, v3Y2, v3Z2);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v7X2, v7Y2, v7Z2);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v4X2, v4Y2, v4Z2);
		
	}
	glDisable(GL_TEXTURE_2D);
	glEnd();
	glEndList();


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

	glBindTexture(GL_TEXTURE_2D, skyTopTexture);
	// turn on texture mapping
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	// top of skybox
	glTexCoord2f(1.0, 1.0);
	glVertex3f(50.0, 50.0, 49.9);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(50.0, -20.0, 49.9);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-20.0, -20.0, 49.9);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-20.0, 50.0, 49.9);

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

	// make sure the point count does not go out of range
	if (pointCount == rollercoasterSpline.numControlPoints - 100) {
		pointCount = 0;
	}
	pointCount++;

	// set the eye coordinates of the camera to the position
	// where the rollercoaster cart would be on the track
	eyeX = rollercoasterSpline.points[pointCount].x + .4;
	eyeY = rollercoasterSpline.points[pointCount].y;
	eyeZ = rollercoasterSpline.points[pointCount].z + .5;

	double xTan = tangents.points[pointCount].x;
	double yTan = tangents.points[pointCount].y;
	double zTan = tangents.points[pointCount].z;

	// set the center coordinates of the camera to where
	// it should be looking from the cart, based on the unit tangent vector
	centerX = xTan + eyeX;
	centerY = yTan + eyeY;
	centerZ = zTan + eyeZ;

	
	double xNorm = normals.points[pointCount].x;
	double yNorm = normals.points[pointCount].y;
	double zNorm = normals.points[pointCount].z;

	std::cout << "Count : " << pointCount << " - ";

	std::cout << "Tangent ( " << xTan << ", " << yTan << ", " << zTan << " )  ";
	std::cout << "Normal ( " << xNorm << ", " << yNorm << ", " << zNorm << " )  ";
	std::cout << "Binormal ( " << binormals.points[pointCount].x << ", " << binormals.points[pointCount].y << ", " << binormals.points[pointCount].z << " )" << std::endl << std::endl;

	//upX = xNorm;
	//upY = yNorm;
	//upZ = zNorm;
}

/***** DISPLAY FUNCTION *****/
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
	gluPerspective(60.0, aspect, .01, 100);
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