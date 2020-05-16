/*
 * Cylinder.c
 *
 *  Created on: Apr 19, 2020
 *      Author: isaac
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>
#include "Cylinder.h"

static double degreesToRadians(double degrees) {
	return degrees * (M_PI/180);
}

/*
static void crossProduct(vec3 p1, vec3 p2, vec3 p3, vec3 res) {
//	printf("\tP1:(%.2f, %.2f, %.2f) \tP2:(%.2f, %.2f, %.2f) \tP3:(%.2f, %.2f, %.2f)\n", p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], p3[0], p3[1], p3[2]);
	vec3 u = { p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
	vec3 v = { p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2] };
	//printf("\tu:(%.2f, %.2f, %.2f) \tv:(%.2f, %.2f, %.2f)\n", u[0], u[1], u[2], v[0], v[1], v[2]);
	res[0] = u[1] * v[2] - u[2] * v[1];
	res[1] = u[2] * v[0] - u[0] * v[2];
	res[2] = u[0] * v[1] - u[1] * v[0];
//	printf("\tres:(%.2f, %.2f, %.2f) \n", res[0], res[1], res[2]);
}
*/

Cylinder cylinder_create_solid(float length, float bottomRadius, float topRadius, int slices, int stacks, vec3 bodyC, vec3 baseC, int verbose) {
	Cylinder c = malloc(sizeof(struct strCylinder));

	c->length = length;
	c->slices = slices;
	c->stacks = stacks;
	c->bottomRadius = bottomRadius;
	c->topRadius = topRadius;

	c->bottomColor[0] = baseC[0];
	c->bottomColor[1] = baseC[1];
	c->bottomColor[2] = baseC[2];

	c->topColor[0] = baseC[0];
	c->topColor[1] = baseC[1];
	c->topColor[2] = baseC[2];

	c->model = malloc(sizeof(float) * ((slices + 1) * 2 * stacks * 3));
	c->colors = malloc(sizeof(float) * ((slices + 1) * 2 * stacks * 3));
	c->normals = malloc(sizeof(float) * ((slices + 1) * 2 * stacks * 3));
	c->indexes = malloc(sizeof(GLushort) * (((slices + 1) * 2 * stacks + (stacks - 1))* 3));

	float y = -length/2;
	float deltaY = length / stacks;
	float radius = bottomRadius;
	float deltaRadius = (topRadius - bottomRadius) / stacks; // we need bottom and top radius to compute this
	float theta = 0;
	float deltaTheta = 360.0 / slices;

	int index = 0;
	int indexesIdx = 0, idxAux = 0;

	/* ------------- CYLINDER BODY -------------*/
	for (int i = 0; i < stacks; i++) {

		for (int j = 0; j <= slices; j++) {
			/* POSICIONES*/
			c->model[index] = radius * cos(degreesToRadians(theta));
			c->model[index + 1] = y;
			c->model[index + 2] = radius * sin(degreesToRadians(theta));

			c->model[index + 3] = (radius + deltaRadius) * cos(degreesToRadians(theta));
			c->model[index + 4] = y + deltaY;
			c->model[index + 5] = (radius + deltaRadius) * sin(degreesToRadians(theta));

			/* COLORES */

			c->colors[index]     = bodyC[0];
			c->colors[index + 1] = bodyC[1];
			c->colors[index + 2] = bodyC[2];

			c->colors[index + 3] = bodyC[0];
			c->colors[index + 4] = bodyC[1];
			c->colors[index + 5] = bodyC[2];

			/* INDICES*/
			c->indexes[indexesIdx] = idxAux++;
			c->indexes[indexesIdx + 1] = idxAux++;

			theta += deltaTheta;
			index += 6;
			indexesIdx += 2;
		}

		c->indexes[indexesIdx] = 0xFFFF;
		indexesIdx++;
		y += deltaY;
		radius += (deltaRadius);
	}

	/* ------------- CYLINDER TOP BASE -------------*/
	theta = 0.0;
	c->topBaseModel = malloc(sizeof(float) * ((slices + 2) * 3));
	c->topBaseColors = malloc(sizeof(float) * ((slices + 2) * 3));
	c->topBaseNormals = malloc(sizeof(float) * ((slices + 2) * 3));


	c->topBaseModel[0] = 0.0;
	c->topBaseModel[1] = y;
	c->topBaseModel[2] = 0.0;

	c->topBaseColors[0] = baseC[0];
	c->topBaseColors[1] = baseC[1];
	c->topBaseColors[2] = baseC[2];

	int topBaseIdx = 3;
	for (int k = 0; k <= slices; k++) {

		/* POSICIONES BASE SUPERIOR */
		c->topBaseModel[topBaseIdx] = topRadius * cos(degreesToRadians(theta));
		c->topBaseModel[topBaseIdx + 1] = y;
		c->topBaseModel[topBaseIdx + 2] = topRadius * sin(degreesToRadians(theta));

		/* COLORES BASE SUPERIOR */
		c->topBaseColors[topBaseIdx]     = baseC[0];
		c->topBaseColors[topBaseIdx + 1] = baseC[1];
		c->topBaseColors[topBaseIdx + 2] = baseC[2];

		topBaseIdx += 3;
		theta += deltaTheta;
	}

	/* ------------- CYLINDER BOTTOM BASE -------------*/

	theta = 0.0;
	c->bottomBaseModel = malloc(sizeof(float) * ((slices + 2) * 3));
	c->bottomBaseColors = malloc(sizeof(float) * ((slices + 2) * 3));
	c->bottomBaseNormals = malloc(sizeof(float) * ((slices + 2) * 3));

	c->bottomBaseModel[0] = 0.0;
	c->bottomBaseModel[1] = -y;
	c->bottomBaseModel[2] = 0.0;

	c->bottomBaseColors[0] = baseC[0];
	c->bottomBaseColors[1] = baseC[1];
	c->bottomBaseColors[2] = baseC[2];

	int bottomBaseIdx = 3;
	for (int k = 0; k <= slices; k++) {

		/* POSICIONES BASE INFERIOR */
		c->bottomBaseModel[bottomBaseIdx] = bottomRadius * cos(degreesToRadians(theta));
		c->bottomBaseModel[bottomBaseIdx + 1] = -y;
		c->bottomBaseModel[bottomBaseIdx + 2] = bottomRadius * sin(degreesToRadians(theta));

		/* COLORES BASE INFERIOR */
		c->bottomBaseColors[bottomBaseIdx]     = baseC[0];
		c->bottomBaseColors[bottomBaseIdx + 1] = baseC[1];
		c->bottomBaseColors[bottomBaseIdx + 2] = baseC[2];

		bottomBaseIdx += 3;
		theta += deltaTheta;
	}

	return c;
}

Cylinder cylinder_create(float length, float bottomRadius, float topRadius, int slices, int stacks, vec3 bottomColor, vec3 topColor, int verbose) {
	Cylinder c = malloc(sizeof(struct strCylinder));

	c->length = length;
	c->slices = slices;
	c->stacks = stacks;
	c->bottomRadius = bottomRadius;
	c->topRadius = topRadius;

	c->bottomColor[0] = bottomColor[0];
	c->bottomColor[1] = bottomColor[1];
	c->bottomColor[2] = bottomColor[2];

	c->topColor[0] = topColor[0];
	c->topColor[1] = topColor[1];
	c->topColor[2] = topColor[2];

	c->model = malloc(sizeof(float) * ((slices + 1) * 2 * stacks * 3));
	c->colors = malloc(sizeof(float) * ((slices + 1) * 2 * stacks * 3));
	c->normals = malloc(sizeof(float) * ((slices + 1) * 2 * stacks * 3));
	c->indexes = malloc(sizeof(GLushort) * (((slices + 1) * 2 * stacks + (stacks - 1))* 3));

	float y = -length/2;
	float deltaY = length / stacks;
	float radius = bottomRadius;
	float deltaRadius = (topRadius - bottomRadius) / stacks; // we need bottom and top radius to compute this
	float theta = 0;
	float deltaTheta = 360.0 / slices;

	vec3 color = {bottomColor[0], bottomColor[1], bottomColor[2]};
	float deltaR = (topColor[0] - bottomColor[0]) / stacks;
	float deltaG = (topColor[1] - bottomColor[1]) / stacks;
	float deltaB = (topColor[2] - bottomColor[2]) / stacks;

	int index = 0;
	int indexesIdx = 0, idxAux = 0;

	float r, g, b, rAux, gAux, bAux;

	/* ------------- CYLINDER BODY -------------*/
	for (int i = 0; i < stacks; i++) {

		r = (rand() % 255) / 255.0;
		g = (rand() % 255) / 255.0;
		b = (rand() % 255) / 255.0;

		if (i == 0) {
			rAux = r;
			gAux = g;
			bAux = b;
		}

		for (int j = 0; j <= slices; j++) {
			/* POSICIONES*/
			c->model[index] = radius * cos(degreesToRadians(theta));
			c->model[index + 1] = y;
			c->model[index + 2] = radius * sin(degreesToRadians(theta));

			c->model[index + 3] = (radius + deltaRadius) * cos(degreesToRadians(theta));
			c->model[index + 4] = y + deltaY;
			c->model[index + 5] = (radius + deltaRadius) * sin(degreesToRadians(theta));

			/* COLORES */

			c->colors[index]     = color[0]*0.8 + r*0.2;
			c->colors[index + 1] = color[1]*0.8 + g*0.2;
			c->colors[index + 2] = color[2]*0.8 + b*0.2;

			c->colors[index + 3] = color[0]*0.8 + r*0.2;
			c->colors[index + 4] = color[1]*0.8 + g*0.2;
			c->colors[index + 5] = color[2]*0.8 + b*0.2;

			/* INDICES*/
			c->indexes[indexesIdx] = idxAux++;
			c->indexes[indexesIdx + 1] = idxAux++;

			theta += deltaTheta;
			index += 6;
			indexesIdx += 2;
		}

		c->indexes[indexesIdx] = 0xFFFF;
		indexesIdx++;
		y += deltaY;
		radius += (deltaRadius);
		color[0] += deltaR;
		color[1] += deltaG;
		color[2] += deltaB;
	}

	/* ------------- CYLINDER TOP BASE -------------*/
	theta = 0.0;
	c->topBaseModel = malloc(sizeof(float) * ((slices + 2) * 3));
	c->topBaseColors = malloc(sizeof(float) * ((slices + 2) * 3));
	c->topBaseNormals = malloc(sizeof(float) * ((slices + 2) * 3));


	c->topBaseModel[0] = 0.0;
	c->topBaseModel[1] = y;
	c->topBaseModel[2] = 0.0;

	c->topBaseColors[0] = color[0]*0.8 + r*0.2;
	c->topBaseColors[1] = color[1]*0.8 + g*0.2;
	c->topBaseColors[2] = color[2]*0.8 + b*0.2;

	int topBaseIdx = 3;
	for (int k = 0; k <= slices; k++) {

		/* POSICIONES BASE SUPERIOR */
		c->topBaseModel[topBaseIdx] = topRadius * cos(degreesToRadians(theta));
		c->topBaseModel[topBaseIdx + 1] = y;
		c->topBaseModel[topBaseIdx + 2] = topRadius * sin(degreesToRadians(theta));

		/* COLORES BASE SUPERIOR */
		c->topBaseColors[topBaseIdx]     = color[0]*0.8 + r*0.2;
		c->topBaseColors[topBaseIdx + 1] = color[1]*0.8 + g*0.2;
		c->topBaseColors[topBaseIdx + 2] = color[2]*0.8 + b*0.2;

		topBaseIdx += 3;
		theta += deltaTheta;
	}

	/* ------------- CYLINDER BOTTOM BASE -------------*/

	theta = 0.0;
	c->bottomBaseModel = malloc(sizeof(float) * ((slices + 2) * 3));
	c->bottomBaseColors = malloc(sizeof(float) * ((slices + 2) * 3));
	c->bottomBaseNormals = malloc(sizeof(float) * ((slices + 2) * 3));

	c->bottomBaseModel[0] = 0.0;
	c->bottomBaseModel[1] = -y;
	c->bottomBaseModel[2] = 0.0;

	c->bottomBaseColors[0] = bottomColor[0]*0.8 + rAux*0.2;
	c->bottomBaseColors[1] = bottomColor[1]*0.8 + gAux*0.2;
	c->bottomBaseColors[2] = bottomColor[2]*0.8 + bAux*0.2;

	int bottomBaseIdx = 3;
	for (int k = 0; k <= slices; k++) {

		/* POSICIONES BASE INFERIOR */
		c->bottomBaseModel[bottomBaseIdx] = bottomRadius * cos(degreesToRadians(theta));
		c->bottomBaseModel[bottomBaseIdx + 1] = -y;
		c->bottomBaseModel[bottomBaseIdx + 2] = bottomRadius * sin(degreesToRadians(theta));

		/* COLORES BASE INFERIOR */
		c->bottomBaseColors[bottomBaseIdx]     = bottomColor[0]*0.8 + r*0.2;
		c->bottomBaseColors[bottomBaseIdx + 1] = bottomColor[1]*0.8 + g*0.2;
		c->bottomBaseColors[bottomBaseIdx + 2] = bottomColor[2]*0.8 + b*0.2;

		bottomBaseIdx += 3;
		theta += deltaTheta;
	}

	return c;
}


void cylinder_bind(Cylinder cylinder, GLuint vertexPosLoc, GLuint vertexColLoc, GLuint vertexNormalLoc, int verbose) {
	/* ------------- CYLINDER BODY -------------*/

	glGenVertexArrays(3, cylinder->vertexArrayIds);
	glGenBuffers(10, cylinder->bufferIds);

	glBindVertexArray(cylinder->vertexArrayIds[0]);

	long size = (long) sizeof(float) * ((cylinder->slices + 1) * 2 * cylinder->stacks * 3);
	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[0]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->model, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosLoc);
	glVertexAttribPointer(vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[1]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexColLoc);
	glVertexAttribPointer(vertexColLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[2]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexNormalLoc);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);

	size = sizeof(GLushort) * (((cylinder->slices + 1) * 2 * cylinder->stacks + (cylinder->stacks - 1))* 3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder->bufferIds[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, cylinder->indexes, GL_STATIC_DRAW);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

	/* ------------- CYLINDER TOP BASE -------------*/

	glBindVertexArray(cylinder->vertexArrayIds[1]);

	size = sizeof(float) * ((cylinder->slices + 2) * 3);
	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[4]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->topBaseModel, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosLoc);
	glVertexAttribPointer(vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[5]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->topBaseColors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexColLoc);
	glVertexAttribPointer(vertexColLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[6]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->topBaseNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexNormalLoc);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);

	/* ------------- CYLINDER BOTTOM BASE -------------*/
	glBindVertexArray(cylinder->vertexArrayIds[2]);

	size = sizeof(float) * ((cylinder->slices + 2) * 3);

	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[7]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->bottomBaseModel, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosLoc);
	glVertexAttribPointer(vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[8]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->bottomBaseColors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexColLoc);
	glVertexAttribPointer(vertexColLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cylinder->bufferIds[9]);
	glBufferData(GL_ARRAY_BUFFER, size, cylinder->bottomBaseNormals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexNormalLoc);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);

}
void cylinder_draw(Cylinder cylinder) {
	/* ------------- CYLINDER BODY -------------*/
	glBindVertexArray(cylinder->vertexArrayIds[0]);
	GLsizei count = ((cylinder->slices + 1) * 2 * cylinder->stacks + (cylinder->stacks - 1));
	glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_SHORT, 0);

	/* ------------- CYLINDER TOP BASE -------------*/
	glBindVertexArray(cylinder->vertexArrayIds[1]);
	count = (cylinder->slices + 2);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);

	/* ------------- CYLINDER BOTTOM BASE -------------*/
	glBindVertexArray(cylinder->vertexArrayIds[2]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);
}

void cylinder_draw_lines(Cylinder cylinder) {
	/* ------------- CYLINDER BODY -------------*/
	glBindVertexArray(cylinder->vertexArrayIds[0]);
	GLsizei count = ((cylinder->slices + 1) * 2 * cylinder->stacks + (cylinder->stacks - 1));
	glDrawElements(GL_LINE_STRIP, count, GL_UNSIGNED_SHORT, 0);

	/* ------------- CYLINDER TOP BASE -------------*/
	glBindVertexArray(cylinder->vertexArrayIds[1]);
	count = (cylinder->slices + 2);
	glDrawArrays(GL_LINE_STRIP, 1, count - 1);

	/* ------------- CYLINDER BOTTOM BASE -------------*/
	glBindVertexArray(cylinder->vertexArrayIds[2]);
	glDrawArrays(GL_LINE_STRIP, 1, count - 1);
}
