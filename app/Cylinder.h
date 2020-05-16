/*
 * Cylinder.h
 *
 *  Created on: Apr 19, 2020
 *      Author: isaac
 */

#ifndef CYLINDER_H_
#define CYLINDER_H_

#include <GL/freeglut.h>

typedef float vec3[3];

struct strCylinder {

	float length;
	int slices;
	int stacks;
	float bottomRadius;
	float topRadius;
	vec3 bottomColor;
	vec3 topColor;

	float *model;
	float *colors;
	float *normals;
	GLushort *indexes;

	float *bottomBaseModel;
	float *bottomBaseColors;
	float *bottomBaseNormals;

	float *topBaseModel;
	float *topBaseColors;
	float *topBaseNormals;

	GLuint vertexArrayIds[3];
	GLuint bufferIds [10];
};

typedef struct strCylinder* Cylinder;

Cylinder cylinder_create(float length, float bottomRadius, float topRadius, int slices, int stacks, vec3 bottomColor, vec3 topColor, int verbose);
Cylinder cylinder_create_solid(float length, float bottomRadius, float topRadius, int slices, int stacks, vec3 bottomColor, vec3 topColor, int verbose);
void cylinder_bind(Cylinder cylinder, GLuint vertexPosLoc, GLuint vertexColLoc, GLuint vertexNormalLoc, int verbose);
void cylinder_draw(Cylinder cylinder);
void cylinder_draw_lines(Cylinder cylinder);

#endif /* CYLINDER_H_ */
