/*
 * Sphere.h
 *
 *  Created on: Apr 19, 2020
 *      Author: isaac
 */

#ifndef SPHERE_H_
#define SPHERE_H_

#include <GL/freeglut.h>

typedef float vec3[3];

struct strSphere {
	float radius;
	int parallels;
	int meridians;
	vec3 sphereColor;
	float *model;
	float *colors;
	float *normals;
	GLushort *indexes;
	GLuint vertexArrayIds[1];
	GLuint bufferIds [4];
};

typedef struct strSphere* Sphere;
Sphere sphere_create(float radius, int parallels, int meridians, vec3 sphereColor, int verbose);
Sphere sphere_create_solid(float radius, int parallels, int meridians, vec3 sphereColor, int verbose);
void sphere_bind(Sphere sphere, GLuint vertexPosLoc, GLuint vertexColLoc, GLuint vertexNormalLoc, int verbose);
void sphere_draw(Sphere sphere);

#endif /* SPHERE_H_ */
