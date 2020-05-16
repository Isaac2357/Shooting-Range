/*
 * Cylinder.c
 *
 *  Created on: 11/04/2020
 *      Author: hpiza
 */

#include "Cylinder.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct strCylinder {
	GLfloat *positions, *colors, *normals;
	GLuint *indexes;
	int vertexCount, indexCount, slices;
	GLuint vertexArrayId, bufferId;
};

typedef float vec3[3];

void crossProduct(vec3 p0, vec3 p1, vec3 p2, vec3 res) {
	res[0] = (p1[1] - p0[1]) * (p2[2] - p0[2]) - (p1[2] - p0[2]) * (p2[1] - p0[1]);
	res[1] = (p1[2] - p0[2]) * (p2[0] - p0[0]) - (p1[0] - p0[0]) * (p2[2] - p0[2]);
	res[2] = (p1[0] - p0[0]) * (p2[1] - p0[1]) - (p1[1] - p0[1]) * (p2[0] - p0[0]);
}

Cylinder cylinder_create(float length, float bottomRadius, float topRadius, int slices, int stacks, float bottomColor[3], float topColor[3]) {
	Cylinder c = (Cylinder) malloc(sizeof(struct strCylinder));
	c->vertexCount = stacks * (slices + 1) * 2 + 2 * (slices + 2);
	c->positions = (GLfloat*) malloc(3 * c->vertexCount * sizeof(float));
	c->colors    = (GLfloat*) malloc(3 * c->vertexCount * sizeof(float));
	c->normals   = (GLfloat*) malloc(3 * c->vertexCount * sizeof(float));
	c->indexCount = stacks * (slices + 1) * 2 + stacks - 1;
	c->indexes   = (GLuint*) malloc(c->indexCount * sizeof(GLuint));
	c->slices    = slices;
	int s, v, st;
	float radius = bottomRadius;
	float incRadius = (topRadius - bottomRadius) / stacks;
	float y = -length / 2;
	float incY = length / stacks;
	float r = bottomColor[0], g = bottomColor[1], b = bottomColor[2];
	float incR = (topColor[0] - bottomColor[0]) / stacks;
	float incG = (topColor[1] - bottomColor[1]) / stacks;
	float incB = (topColor[2] - bottomColor[2]) / stacks;
	int ii = 0, vi = 0;
	for(st = 1, v = 0; st <= stacks; st ++) {
		float rr = r * 0.8 + (float) rand() / RAND_MAX * 0.2;
		float gg = g * 0.8 + (float) rand() / RAND_MAX * 0.2;
		float bb = b * 0.8 + (float) rand() / RAND_MAX * 0.2;
		float angle = 0, incAngle = 2 * M_PI / slices;
		for(s = 1; s <= slices + 1; s ++, v += 6) {
			c->positions[v]     =  radius    * cos(angle);
			c->positions[v + 1] =  y;
			c->positions[v + 2] =  radius    * sin(angle);
			c->positions[v + 3] =  (radius + incRadius) * cos(angle);
			c->positions[v + 4] =  y + incY;
			c->positions[v + 5] =  (radius + incRadius) * sin(angle);
			c->colors[v]     = rr;
			c->colors[v + 1] = gg;
			c->colors[v + 2] = bb;
			c->colors[v + 3] = rr;
			c->colors[v + 4] = gg;
			c->colors[v + 5] = bb;

			vec3 v0 = { topRadius    * cos(angle),             length / 2, topRadius    * sin(angle) };
			vec3 v1 = { topRadius    * cos(angle + incAngle),  length / 2, topRadius    * sin(angle + incAngle) };
			vec3 v2 = { bottomRadius * cos(angle),            -length / 2, bottomRadius * sin(angle) };
			vec3 normal1;
			crossProduct(v0, v1, v2, normal1);
			c->normals[v]     = normal1[0];
			c->normals[v + 1] = normal1[1];
			c->normals[v + 2] = normal1[2];
			c->normals[v + 3] = normal1[0];
			c->normals[v + 4] = normal1[1];
			c->normals[v + 5] = normal1[2];

			angle += incAngle;

			c->indexes[ii ++] = vi ++;
			c->indexes[ii ++] = vi ++;
		}
		r += incR;
		g += incG;
		b += incB;
		y += incY;
		radius += incRadius;
		c->indexes[ii ++] = 0xFFFF;
	}

	c->positions[v]     = 0;
	c->positions[v + 1] = -length / 2;
	c->positions[v + 2] = 0;
	c->colors[v]        = bottomColor[0];
	c->colors[v + 1]    = bottomColor[1];
	c->colors[v + 2]    = bottomColor[2];
	c->normals[v]       =  0;
	c->normals[v + 1]   = -1;
	c->normals[v + 2]   =  0;
	v += 3;
	float rad = 0, incRad = 2 * M_PI / slices;
	int i;
	for (i = 0; i <= slices; i++, v += 3) {
		c->positions[v]     = bottomRadius * cos(rad);
		c->positions[v + 1] = -length / 2;
		c->positions[v + 2] = bottomRadius * sin(rad);
		c->colors[v]        = bottomColor[0];
		c->colors[v + 1]    = bottomColor[1];
		c->colors[v + 2]    = bottomColor[2];
		c->normals[v]       =  0;
		c->normals[v + 1]   = -1;
		c->normals[v + 2]   =  0;
		rad += incRad;
	}
	c->positions[v] = 0;
	c->positions[v + 1] = length / 2;
	c->positions[v + 2] = 0;
	c->colors[v] = topColor[0];
	c->colors[v + 1] = topColor[1];
	c->colors[v + 2] = topColor[2];
	c->normals[v]      = 0;
	c->normals[v + 1]  = 1;
	c->normals[v + 2]  = 0;
	v += 3;
	for (i = 0; i <= slices; i++, v += 3) {
		c->positions[v] = topRadius * cos(rad);
		c->positions[v + 1] = length / 2;
		c->positions[v + 2] = topRadius * sin(rad);
		c->colors[v]     = topColor[0];
		c->colors[v + 1] = topColor[1];
		c->colors[v + 2] = topColor[2];
		c->normals[v]     = 0;
		c->normals[v + 1] = 1;
		c->normals[v + 2] = 0;
		rad -= incRad;
	}
	return c;
}

void cylinder_bind(Cylinder c, GLuint vLoc, GLuint cLoc, GLuint nLoc) {
	glGenVertexArrays(1, &(c->vertexArrayId));
	glBindVertexArray(c->vertexArrayId);

	GLuint bufferIds[4];
	glGenBuffers(4, bufferIds);

	glBindBuffer(GL_ARRAY_BUFFER, bufferIds[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * c->vertexCount * sizeof(float), c->positions, GL_STATIC_DRAW);
	glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vLoc);

	glBindBuffer(GL_ARRAY_BUFFER, bufferIds[1]);
	glBufferData(GL_ARRAY_BUFFER, 3 * c->vertexCount * sizeof(float), c->colors, GL_STATIC_DRAW);
	glVertexAttribPointer(cLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(cLoc);

	glBindBuffer(GL_ARRAY_BUFFER, bufferIds[2]);
	glBufferData(GL_ARRAY_BUFFER, 3 * c->vertexCount * sizeof(float), c->normals, GL_STATIC_DRAW);
	glVertexAttribPointer(nLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(nLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, c->indexCount * sizeof(GLuint), c->indexes, GL_STATIC_DRAW);
	c->bufferId = bufferIds[3];
}

void cylinder_destroy(Cylinder c) {
	glDeleteVertexArrays(1, &(c->vertexArrayId));
	free(c->positions);
	free(c->colors);
	free(c->normals);
	free(c->indexes);
	free(c);
}

void cylinder_draw(Cylinder c) {
	glEnable(GL_CULL_FACE);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);
	glBindVertexArray(c->vertexArrayId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->bufferId);
	glDrawElements(GL_TRIANGLE_STRIP, c->indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(c->vertexArrayId);
	glDrawArrays(GL_TRIANGLE_FAN, c->vertexCount - 2 * (c->slices + 2), c->slices + 2);
	glBindVertexArray(c->vertexArrayId);
	glDrawArrays(GL_TRIANGLE_FAN, c->vertexCount - (c->slices + 2), c->slices + 2);
}

void cylinder_drawlines(Cylinder c) {
	glEnable(GL_CULL_FACE);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);
	glBindVertexArray(c->vertexArrayId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->bufferId);
	glDrawElements(GL_LINE_STRIP, c->indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(c->vertexArrayId);
	int start = c->vertexCount - 2 * (c->slices + 2) + 1;
	int count = c->slices + 1;
	glDrawArrays(GL_LINE_STRIP, start, count);
	glBindVertexArray(c->vertexArrayId);
	glDrawArrays(GL_LINE_STRIP, start + count + 1, count);
}
