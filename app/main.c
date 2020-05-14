
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Transforms.h"
#include "Utils.h"
#include <stdio.h>
#include <math.h>

#define toRadians(deg) deg * M_PI / 180.0

typedef enum { IDLE, LEFT, RIGHT, UP, DOWN, FRONT, BACK } MOTION_TYPE;

typedef float vec3[3];

static Mat4   modelMatrix, projectionMatrix, viewMatrix;
static GLuint programId1, vertexPositionLoc,  vertexNormalLoc, vertexTexcoordLoc, modelMatrixLoc,  projectionMatrixLoc,  viewMatrixLoc;
static GLuint ambientLightLoc, materialALoc, materialDLoc;
static GLuint materialSLoc, cameraPositionLoc;

GLuint cubeVA, roomVA, rhombusVA, rhombusBuffer[4];
GLuint roomBuffers[3];

static MOTION_TYPE motionType      = 0;

static float cameraSpeed     = 0.2;
static float cameraX         = 0;
static float cameraZ         = 5;
static float cameraAngle     = 0;

static const int ROOM_WIDTH  = 40;
static const int ROOM_HEIGHT =  6;
static const int ROOM_DEPTH  = 40;

static vec3 ambientLight  = {0.5, 0.5, 0.5};

static vec3 materialA     = {0.8, 0.8, 0.8};
static vec3 materialD     = {0.6, 0.6, 0.6};
static vec3 materialS     = {0.6, 0.6, 0.6};

//                          Color    subcutoff,  Position  Exponent Direction  Cos(cutoff)
static float lights[]   = { 1, 1, 1,  0.9238,    0, 3,  -10,  256,	  0, -1,  0,   0.7071,		// Luz Roja
		                    1, 1, 1,  0.9238,    0, 3,   0,  256,     0, -1,  0,   0.7071, 	// Luz Verde
		                    1, 1, 1,  0.9238,    0, 3,   10,  256,     0, -1,  0,   0.7071    // Luz Azul
};

static GLuint lightsBufferId;

static GLuint textures[4];

static void initTexture(const char* filename, GLuint textureId) {
	unsigned char* data;
	unsigned int width, height;
	glBindTexture(GL_TEXTURE_2D, textureId);
	loadBMP(filename, &data, &width, &height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void initTextures() {
	glGenTextures(4, textures);
	initTexture("textures/Brick.bmp",    textures[0]);
	initTexture("textures/Ceiling.bmp",  textures[1]);
}


static void initShaders() {
	GLuint vShader = compileShader("shaders/phong.vsh", GL_VERTEX_SHADER);
	if(!shaderCompiled(vShader)) return;
	GLuint fShader = compileShader("shaders/phong.fsh", GL_FRAGMENT_SHADER);
	if(!shaderCompiled(fShader)) return;
	programId1 = glCreateProgram();
	glAttachShader(programId1, vShader);
	glAttachShader(programId1, fShader);
	glLinkProgram(programId1);

	vertexPositionLoc   = glGetAttribLocation(programId1, "vertexPosition");
	vertexNormalLoc     = glGetAttribLocation(programId1, "vertexNormal");
	vertexTexcoordLoc   = glGetAttribLocation(programId1, "vertexTexcoord");
	modelMatrixLoc      = glGetUniformLocation(programId1, "modelMatrix");
	viewMatrixLoc       = glGetUniformLocation(programId1, "viewMatrix");
	projectionMatrixLoc = glGetUniformLocation(programId1, "projMatrix");
	ambientLightLoc     = glGetUniformLocation(programId1, "ambientLight");
	materialALoc        = glGetUniformLocation(programId1, "materialA");
	materialDLoc        = glGetUniformLocation(programId1, "materialD");
	materialSLoc        = glGetUniformLocation(programId1, "materialS");
	cameraPositionLoc   = glGetUniformLocation(programId1, "cameraPosition");

}

static void initLights() {
	glUseProgram(programId1);
	glUniform3fv(ambientLightLoc,  1, ambientLight);

	glUniform3fv(materialALoc,     1, materialA);
	glUniform3fv(materialDLoc,     1, materialD);
	glUniform3fv(materialSLoc,     1, materialS);

	glGenBuffers(1, &lightsBufferId);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsBufferId);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), lights, GL_DYNAMIC_DRAW);

	GLuint uniformBlockIndex = glGetUniformBlockIndex(programId1, "LightBlock");
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsBufferId);
	glUniformBlockBinding(programId1, uniformBlockIndex, 0);
}

static void initRoom() {
	float w1 = -ROOM_WIDTH  / 2, w2 = ROOM_WIDTH  / 2;
	float h1 = -ROOM_HEIGHT / 2, h2 = ROOM_HEIGHT / 2;
	float d1 = -ROOM_DEPTH  / 2, d2 = ROOM_DEPTH  / 2;

	float positions[] = {w1, h2, d1,  w1, h1, d1,  w2, h1, d1,   w2, h1, d1,  w2, h2, d1,  w1, h2, d1,  // Frente
			             w2, h2, d2,  w2, h1, d2,  w1, h1, d2,   w1, h1, d2,  w1, h2, d2,  w2, h2, d2,  // Atrás

						 w1, h2, d2,  w1, h1, d2,  w1, h1, d1,   w1, h1, d1,  w1, h2, d1,  w1, h2, d2,  // Izquierda

			             w2, h2, d1,  w2, h1, d1,  w2, h1, d2,   w2, h1, d2,  w2, h2, d2,  w2, h2, d1,  // Derecha
			             w1, h1, d1,  w1, h1, d2,  w2, h1, d2,   w2, h1, d2,  w2, h1, d1,  w1, h1, d1,  // Abajo
						 w1, h2, d2,  w1, h2, d1,  w2, h2, d1,   w2, h2, d1,  w2, h2, d2,  w1, h2, d2   // Arriba
	};

	float normals[] = { 0,  0,  1,   0,  0,  1,   0,  0,  1,    0,  0,  1,   0,  0,  1,   0,  0,  1,  // Frente
						0,  0, -1,   0,  0, -1,   0,  0, -1,    0,  0, -1,   0,  0, -1,   0,  0, -1,  // Atrás
					    1,  0,  0,   1,  0,  0,   1,  0,  0,    1,  0,  0,   1,  0,  0,   1,  0,  0,  // Izquierda
					   -1,  0,  0,  -1,  0,  0,  -1,  0,  0,   -1,  0,  0,  -1,  0,  0,  -1,  0,  0,  // Derecha
					    0,  1,  0,   0,  1,  0,   0,  1,  0,    0,  1,  0,   0,  1,  0,   0,  1,  0,  // Abajo
					    0, -1,  0,   0, -1,  0,   0, -1,  0,    0, -1,  0,   0, -1,  0,   0, -1,  0   // Arriba
	};

	float wh = (float) ROOM_WIDTH / ROOM_HEIGHT;
	float dh = (float) ROOM_DEPTH / ROOM_HEIGHT;
	float wd = (float) ROOM_WIDTH / ROOM_DEPTH;

	float texcoords[] = {       0, 2,       0, 0,  2 * wh, 0,   2 * wh, 0,   2 * wh, 2,       0, 2,
				           2 * wh, 2,  2 * wh, 0,       0, 0,        0, 0,        0, 2,  2 * wh, 2,

						        0, 2,       0, 0,  2 * dh, 0,   2 * dh, 0,   2 * dh, 2,       0, 2,
						   2 * dh, 2,  2 * dh, 0,       0, 0,        0, 0,        0, 2,  2 * dh, 2,

					        	0, 2,       0, 0,  2 * wd, 0,   2 * wd, 0,   2 * wd, 2,       0, 2,
						   2 * wd, 2,  2 * wd, 0,       0, 0,        0, 0,        0, 2,  2 * wd, 2,
	};

	glUseProgram(programId1);
	glGenVertexArrays(1, &roomVA);
	glBindVertexArray(roomVA);
	GLuint buffers[3];
	glGenBuffers(3, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexPositionLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexPositionLoc);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexNormalLoc);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexTexcoordLoc, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexTexcoordLoc);
}

static void displayFunc() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	Actualizar posición de la cámara
	float dist = 0.2;
	float w1 = -ROOM_WIDTH  / 2 + dist, w2 = ROOM_WIDTH  / 2 - dist;
	//float h1 = -ROOM_HEIGHT / 2 + dist, h2 = ROOM_HEIGHT / 2 - dist;
	float d1 = -ROOM_DEPTH  / 2 + dist, d2 = ROOM_DEPTH  / 2 - dist;

	switch(motionType) {
  		case  LEFT  :  if(cameraX - cameraSpeed >  w1) cameraX -= cameraSpeed; break;
  		case  RIGHT :  if(cameraX + cameraSpeed <  w2) cameraX += cameraSpeed; break;
		case  FRONT :  if(cameraZ - cameraSpeed >  d1) cameraZ -= cameraSpeed; break;
		case  BACK  :  if(cameraZ + cameraSpeed <  d2) cameraZ += cameraSpeed; break;
		case  UP    :
		case  DOWN  :
		case  IDLE  :  ;
	}

//	Envío de proyección, vista y posición de la cámara al programa 1 (cuarto, rombo)
	glUseProgram(programId1);
	glUniformMatrix4fv(projectionMatrixLoc, 1, true, projectionMatrix.values);
	mIdentity(&viewMatrix);
	rotateY(&viewMatrix, -cameraAngle);
	translate(&viewMatrix, -cameraX, 0, -cameraZ);
	glUniformMatrix4fv(viewMatrixLoc, 1, true, viewMatrix.values);
	glUniform3f(cameraPositionLoc, cameraX, 0, cameraZ);

//	Dibujar el cuarto
	mIdentity(&modelMatrix);
	glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
	glBindVertexArray(roomVA);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glDrawArrays(GL_TRIANGLES,  0, 24);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 24,  6);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 30,  6);
	glutSwapBuffers();
}

static void reshapeFunc(int w, int h) {
    if(h == 0) h = 1;
    glViewport(0, 0, w, h);
    float aspect = (float) w / h;
    setPerspective(&projectionMatrix, 45, aspect, -0.1, -500);
}

static void timerFunc(int id) {
	glutTimerFunc(10, timerFunc, id);
	glutPostRedisplay();
}

static void keyReleasedFunc(unsigned char key,int x, int y) {
	motionType = IDLE;
}

static void keyPressedFunc(unsigned char key, int x, int y) {
	switch(key) {
		case 'a':
		case 'A': motionType = LEFT; break;
		case 'd':
		case 'D': motionType = RIGHT; break;
		case 'w':
		case 'W': motionType = FRONT; break;
		case 's':
		case 'S': motionType = BACK; break;
		case 27 : exit(0);
	}
 }

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Shooting Range");
    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutTimerFunc(10, timerFunc, 1);
    glutKeyboardFunc(keyPressedFunc);
    glutKeyboardUpFunc(keyReleasedFunc);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    initTextures();
    initShaders();
    initLights();
    initRoom();
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glutMainLoop();
	return 0;
}
