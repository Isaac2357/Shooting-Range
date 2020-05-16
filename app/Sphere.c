/*
 * Sphere.c
 *
 *  Created on: Apr 19, 2020
 *      Author: isaac
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Sphere.h"
#include <stdio.h>
#include <math.h>

static double degreesToRadians(double degrees) {
	return degrees * (M_PI/180);
}

Sphere sphere_create(float radius, int parallels, int meridians, vec3 color, int verbose) {
	if(verbose) puts("SPHERE CREATE");
	Sphere sphere = malloc(sizeof(struct strSphere));

	glGenVertexArrays(1, sphere->vertexArrayIds);
	glGenBuffers(4, sphere->bufferIds);

	sphere->radius = radius;
	sphere->parallels = parallels;
	sphere->meridians = meridians;
	sphere->sphereColor[0] = color[0];
	sphere->sphereColor[1] = color[1];
	sphere->sphereColor[2] = color[2];

	sphere->model = malloc(sizeof(float) * ((meridians + 1) * 2 * parallels * 3));
	sphere->colors = malloc(sizeof(float) * ((meridians + 1) * 2 * parallels * 3));
	sphere->normals = malloc(sizeof(float) * ((meridians + 1) * 2 * parallels * 3));
	sphere->indexes = malloc(sizeof(GLushort) * (((meridians + 1) * 2 * parallels + (parallels - 1))* 3));

	float phi = 0;
	float theta = 0;
	float deltaPhi = 180.0 / parallels;
	float deltaTheta = 360.0 / meridians;

	int iout2 = 0;
	int index = 0;
	int modelidx = 0;

	for (int i = 0; i < parallels; i++) {
		float r = (rand() % 255) / 255.0;
		float g = (rand() % 255) / 255.0;
		float b = (rand() % 255) / 255.0;
		for (int j = 0; j <= meridians; j++) {
			/* POSICIONES */
			sphere->model[index] = radius * sin(degreesToRadians(phi)) * cos(degreesToRadians(theta));
			sphere->model[index + 1] = radius * sin(degreesToRadians(phi)) * sin(degreesToRadians(theta));
			sphere->model[index + 2] = radius * cos(degreesToRadians(phi));

			sphere->model[index + 3] = radius * sin(degreesToRadians(phi + deltaPhi)) * cos(degreesToRadians(theta));
			sphere->model[index + 4] = radius * sin(degreesToRadians(phi + deltaPhi)) * sin(degreesToRadians(theta));
			sphere->model[index + 5] = radius * cos(degreesToRadians(phi + deltaPhi));

			/*COLORES*/
			sphere->colors[index]     = color[0]*0.8 + r*0.2;
			sphere->colors[index + 1] = color[1]*0.8 + g*0.2;
			sphere->colors[index + 2] = color[2]*0.8 + b*0.2;

			sphere->colors[index + 3] = color[0]*0.8 + r*0.2;
			sphere->colors[index + 4] = color[1]*0.8 + r*0.2;
			sphere->colors[index + 5] = color[2]*0.8 + r*0.2;

			/*NORMALES*/
			sphere->normals[index]     =  radius * sin(degreesToRadians(phi)) * cos(degreesToRadians(theta));
			sphere->normals[index + 1] =  radius * sin(degreesToRadians(phi)) * sin(degreesToRadians(theta));
			sphere->normals[index + 2] =  radius * cos(degreesToRadians(phi));

			sphere->normals[index + 3] = radius * sin(degreesToRadians(phi + deltaPhi)) * cos(degreesToRadians(theta));
			sphere->normals[index + 4] = radius * sin(degreesToRadians(phi + deltaPhi)) * sin(degreesToRadians(theta));
			sphere->normals[index + 5] = radius * cos(degreesToRadians(phi + deltaPhi));

			sphere->indexes[modelidx] = iout2++;
			sphere->indexes[modelidx + 1] = iout2++;

			if(verbose) {
				printf("Vertex: (%.2f, %.2f,%.2f) - ", sphere->model[index],       sphere->model[index + 1],       sphere->model[index + 2]);
				printf("Normal (%.2f, %.2f,%.2f)\n",   sphere->normals[index],     sphere->normals[index + 1],     sphere->normals[index + 2]);
				printf("Vertex: (%.2f, %.2f,%.2f) - ", sphere->model[index + 3],   sphere->model[index + 4],       sphere->model[index + 5]);
				printf("Normal: (%.2f, %.2f,%.2f)\n",  sphere->normals[index + 3], sphere->normals[index + 4],     sphere->normals[index + 5]);
			}

			theta += deltaTheta;
			index += 6;
			modelidx += 2;
		}
		sphere->indexes[modelidx] = 0xFFFF;
		modelidx++;
		phi += deltaPhi;
	}

	return sphere;
}

Sphere sphere_create_solid(float radius, int parallels, int meridians, vec3 color, int verbose) {
	if(verbose) puts("SPHERE CREATE SOLID");
	Sphere sphere = malloc(sizeof(struct strSphere));

	glGenVertexArrays(1, sphere->vertexArrayIds);
	glGenBuffers(4, sphere->bufferIds);

	sphere->radius = radius;
	sphere->parallels = parallels;
	sphere->meridians = meridians;
	sphere->sphereColor[0] = color[0];
	sphere->sphereColor[1] = color[1];
	sphere->sphereColor[2] = color[2];

	sphere->model = malloc(sizeof(float) * ((meridians + 1) * 2 * parallels * 3));
	sphere->colors = malloc(sizeof(float) * ((meridians + 1) * 2 * parallels * 3));
	sphere->normals = malloc(sizeof(float) * ((meridians + 1) * 2 * parallels * 3));
	sphere->indexes = malloc(sizeof(GLushort) * (((meridians + 1) * 2 * parallels + (parallels - 1))* 3));

	float phi = 0;
	float theta = 0;
	float deltaPhi = 180.0 / parallels;
	float deltaTheta = 360.0 / meridians;

	int iout2 = 0;
	int index = 0;
	int modelidx = 0;

	for (int i = 0; i < parallels; i++) {
		for (int j = 0; j <= meridians; j++) {
			/* POSICIONES */
			sphere->model[index] = radius * sin(degreesToRadians(phi)) * cos(degreesToRadians(theta));
			sphere->model[index + 1] = radius * sin(degreesToRadians(phi)) * sin(degreesToRadians(theta));
			sphere->model[index + 2] = radius * cos(degreesToRadians(phi));

			sphere->model[index + 3] = radius * sin(degreesToRadians(phi + deltaPhi)) * cos(degreesToRadians(theta));
			sphere->model[index + 4] = radius * sin(degreesToRadians(phi + deltaPhi)) * sin(degreesToRadians(theta));
			sphere->model[index + 5] = radius * cos(degreesToRadians(phi + deltaPhi));

			/*COLORES*/
			sphere->colors[index]     = color[0];
			sphere->colors[index + 1] = color[1];
			sphere->colors[index + 2] = color[2];

			sphere->colors[index + 3] = color[0];
			sphere->colors[index + 4] = color[1];
			sphere->colors[index + 5] = color[2];

			/*NORMALES*/
			sphere->normals[index]     =  radius * sin(degreesToRadians(phi)) * cos(degreesToRadians(theta));
			sphere->normals[index + 1] =  radius * sin(degreesToRadians(phi)) * sin(degreesToRadians(theta));
			sphere->normals[index + 2] =  radius * cos(degreesToRadians(phi));

			sphere->normals[index + 3] = radius * sin(degreesToRadians(phi + deltaPhi)) * cos(degreesToRadians(theta));
			sphere->normals[index + 4] = radius * sin(degreesToRadians(phi + deltaPhi)) * sin(degreesToRadians(theta));
			sphere->normals[index + 5] = radius * cos(degreesToRadians(phi + deltaPhi));

			sphere->indexes[modelidx] = iout2++;
			sphere->indexes[modelidx + 1] = iout2++;

			if(verbose) {
				printf("Vertex: (%.2f, %.2f,%.2f) - ", sphere->model[index],       sphere->model[index + 1],       sphere->model[index + 2]);
				printf("Normal (%.2f, %.2f,%.2f)\n",   sphere->normals[index],     sphere->normals[index + 1],     sphere->normals[index + 2]);
				printf("Vertex: (%.2f, %.2f,%.2f) - ", sphere->model[index + 3],   sphere->model[index + 4],       sphere->model[index + 5]);
				printf("Normal: (%.2f, %.2f,%.2f)\n",  sphere->normals[index + 3], sphere->normals[index + 4],     sphere->normals[index + 5]);
			}

			theta += deltaTheta;
			index += 6;
			modelidx += 2;
		}
		sphere->indexes[modelidx] = 0xFFFF;
		modelidx++;
		phi += deltaPhi;
	}

	return sphere;
}

void sphere_bind(Sphere sphere, GLuint vertexPosLoc, GLuint vertexColLoc, GLuint vertexNormalLoc, int verbose) {
	glBindVertexArray(sphere->vertexArrayIds[0]);
	glBindBuffer(GL_ARRAY_BUFFER, sphere->bufferIds[0]);

	if(verbose) {
		puts("SPHERE BIND");
		printf("Vertex Array Id: %d\n", sphere->vertexArrayIds[0]);
		for(int j = 0; j < 4; j++) printf("Buffer %d Id: %d\n", j, sphere->bufferIds[j]);

	}
	long size = (long) (sphere->meridians + 1) * 2 * sphere->parallels * 3 * sizeof(float);
	if(verbose) {
		printf("Model size: %ld\n", size);
		printf("Colors size: %ld\n", size);
		printf("Normals size: %ld\n", size);
	}
	glBufferData(GL_ARRAY_BUFFER, size, sphere->model, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosLoc);
	glVertexAttribPointer(vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->bufferIds[1]);
	glBufferData(GL_ARRAY_BUFFER, size, sphere->colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexColLoc);
	glVertexAttribPointer(vertexColLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->bufferIds[2]);
	glBufferData(GL_ARRAY_BUFFER, size, sphere->normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexNormalLoc);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);

	size = ((sphere->meridians + 1) * 2 * sphere->parallels + (sphere->parallels - 1))* 3 * sizeof(GLushort);

	if(verbose) printf("Indexes size: %ld\n", size);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->bufferIds[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, sphere->indexes, GL_STATIC_DRAW);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);
}

void sphere_draw(Sphere sphere) {
	GLsizei count = ((sphere->meridians + 1) * 2 * sphere->parallels + (sphere->parallels - 1));
	glBindVertexArray(sphere->vertexArrayIds[0]);
	glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_SHORT, 0);
}

