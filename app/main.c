/*
 * main.c
 *
 *  Created on: Jan 30, 2020
 *      Author: isaac
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Utils.h"

static void onKeyPressed(unsigned char key, int x, int y) {
	if (key == 27) exit(0);
}

GLuint programId;

void init() {

	GLuint posVshId = compileShader("shaders/position.vsh", GL_VERTEX_SHADER);
	GLuint orangeFshId = compileShader("shaders/orange.fsh", GL_FRAGMENT_SHADER);
	programId = glCreateProgram();

	if (!shaderCompiled(posVshId)) return;
	if (!shaderCompiled(orangeFshId)) return;

	glAttachShader(programId, posVshId);
	glAttachShader(programId, orangeFshId);
	glLinkProgram(programId);

	float trianglePos[] = {
			 0.0,  0.9,
			-0.9, -0.9,
			 0.9, -0.9
	};
	glBindVertexArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(trianglePos), trianglePos, GL_STATIC_DRAW);

	GLuint vPosLoc = glGetAttribLocation(programId, "vertexPosition");
	glVertexAttribPointer(vPosLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosLoc);

// vertex, data type, gap between vertex, initial address offset
//	glVertexPointer(2, GL_FLOAT, 0, 0);
//	glEnableClientState(GL_VERTEX_ARRAY); // enable this buffer
}

static void onDisplayImage() {
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(1);
	glUseProgram(programId);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	int w = 600, h = 400;
	int x = (glutGet(GLUT_SCREEN_WIDTH) - w) / 2;
	int y = (glutGet(GLUT_SCREEN_HEIGHT) - h) / 2;

	glutInitWindowSize(w, h);
	glutInitWindowPosition(x, y);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("Working with shader");
	glutKeyboardFunc(onKeyPressed);
	glutDisplayFunc(onDisplayImage);

	glewInit();
	glClearColor(0.2, 0.2, 0.2, 1.0);

	init();

	glutMainLoop();

	return 0;
}



