/*
 * Cylinder.h
 *
 *  Created on: 11/04/2020
 *      Author: hpiza
 */

#ifndef CYLINDER_H_
#define CYLINDER_H_

#include <GL/glew.h>

typedef struct strCylinder* Cylinder;

Cylinder cylinder_create(float length, float bottomRadius, float topRadius, int slices, int stack, float bottomColor[3], float topColor[3]);

void cylinder_bind(Cylinder c, GLuint, GLuint, GLuint);

void cylinder_destroy(Cylinder);

void cylinder_draw(Cylinder);

void cylinder_drawlines(Cylinder);


#endif /* CYLINDER_H_ */
