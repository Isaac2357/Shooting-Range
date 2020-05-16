#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include "Transforms.h"
#include "Utils.h"
#include "Cylinder.h"

#define toRadians(deg) deg * M_PI / 180.0
#define max(a, b) a > b ? a : b

static const float ROOM_WIDTH  = 40;
static const float ROOM_HEIGHT = 6;
static const float ROOM_DEPTH  = 40;

typedef enum { None = 0, Left = 1, Right = 2, Bottom = 3, Top = 4, Front = 5, Back = 6 } Boundary;
typedef enum { False = 0, True = 1 } Bool;
typedef enum { IDLE, FORWARD, BACK, TURN_RIGHT, TURN_LEFT, RIGHT, LEFT } MOTION_TYPE;
typedef float vec3[3];

static MOTION_TYPE motionType = 0;

static float observerX = 0;
static float observerY = 0;
static float observerZ = 0;
static float observerYaw = 0;
static float observerPitch = 0;
static float observerSpeed = 0.1;
static float angleSpeed = 0.1;

static Mat4   modelMatrix, projectionMatrix, viewMatrix;

static GLuint programId1, vertexPositionLoc, vertexColorLoc,
			  vertexNormalLoc, vertexTexcoordLoc, modelMatrixLoc,
			  projectionMatrixLoc, viewMatrixLoc, hLoc;

static GLuint programId2, vertexPositionLoc2, vertexColorLoc2, vertexTexcoordLoc2;

static GLuint ambientLightLoc, materialALoc, materialDLoc;
static GLuint materialSLoc, cameraPositionLoc;

static GLuint roomVA, playerVA;

static vec3 ambientLight  = {0.5, 0.5, 0.5};

static vec3 materialA     = {0.8, 0.8, 0.8};
static vec3 materialD     = {0.6, 0.6, 0.6};
static vec3 materialS     = {0.6, 0.6, 0.6};

//                          Color    subcutoff,  Position  Exponent Direction  Cos(cutoff)
static float lights[]   = {
		1, 1, 1, 		// Color
		0.9238,    		// Sub-cutoff
		0, 3,  -10,  	// Position
		256,	  		// Exponent
		0, -1,  0,   	// Direction
		0.7071,			// Cutoff

        1, 1, 1,
		0.9238,
		0, 3,   0,
		256,
		0, -1,  0,
		0.7071,

		1, 1, 1,
		0.9238,
		0, 3,   10,
		256,
		0, -1,  0,
		0.7071
};

static GLuint lightsBufferId;

static GLuint textures[4];

static vec3 col = {1, 0.8, 0.0};
static Cylinder c;


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
    initTexture("textures/Shotgun.bmp",  textures[2]);
}

static int initShaders() {
	int err = 1;
    GLuint vShader = compileShader("shaders/phong.vsh", GL_VERTEX_SHADER);
    if(!shaderCompiled(vShader)) return err;
    GLuint fShader = compileShader("shaders/phong.fsh", GL_FRAGMENT_SHADER);
    if(!shaderCompiled(fShader)) return err;
    programId1 = glCreateProgram();
    glAttachShader(programId1, vShader);
    glAttachShader(programId1, fShader);
    glLinkProgram(programId1);

    vertexPositionLoc   = glGetAttribLocation(programId1, "vertexPosition");
    vertexColorLoc      = glGetAttribLocation(programId1, "vertexColor");
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
    hLoc   = glGetUniformLocation(programId1, "h");

    vShader = compileShader("shaders/pos_col.vsh", GL_VERTEX_SHADER);
    if(!shaderCompiled(vShader)) return err;
    fShader = compileShader("shaders/color.fsh", GL_FRAGMENT_SHADER);
    if(!shaderCompiled(vShader)) return err;
    programId2 = glCreateProgram();
    glAttachShader(programId2, vShader);
    glAttachShader(programId2, fShader);
    glLinkProgram(programId2);

    vertexPositionLoc2  = glGetAttribLocation(programId2, "vertexPosition");
    vertexColorLoc2     = glGetAttribLocation(programId2, "vertexColor");
    vertexTexcoordLoc2  = glGetAttribLocation(programId2, "vertexTexcoord");

    glUseProgram(programId1);
    c = cylinder_create(ROOM_HEIGHT, 1, 1, 40, 40, col, col, 0);
    cylinder_bind(c, vertexPositionLoc, vertexColorLoc, vertexNormalLoc, 0);

    return 0;

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
                         w2, h2, d2,  w2, h1, d2,  w1, h1, d2,   w1, h1, d2,  w1, h2, d2,  w2, h2, d2,  // Atr�s

                         w1, h2, d2,  w1, h1, d2,  w1, h1, d1,   w1, h1, d1,  w1, h2, d1,  w1, h2, d2,  // Izquierda

                         w2, h2, d1,  w2, h1, d1,  w2, h1, d2,   w2, h1, d2,  w2, h2, d2,  w2, h2, d1,  // Derecha
                         w1, h1, d1,  w1, h1, d2,  w2, h1, d2,   w2, h1, d2,  w2, h1, d1,  w1, h1, d1,  // Abajo
                         w1, h2, d2,  w1, h2, d1,  w2, h2, d1,   w2, h2, d1,  w2, h2, d2,  w1, h2, d2   // Arriba
    };

    float normals[] = { 0,  0,  1,   0,  0,  1,   0,  0,  1,    0,  0,  1,   0,  0,  1,   0,  0,  1,  // Frente
                        0,  0, -1,   0,  0, -1,   0,  0, -1,    0,  0, -1,   0,  0, -1,   0,  0, -1,  // Atr�s
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

static void initPlayer() {
    float positions[] =  { -0.2, -0.5,    -0.2, -1,    0.2, -0.5,    0.2, -1 };
    float colors[] = { 1, 1, 1,  1, 1, 1, 1, 1, 1,  1, 1, 1};
    // TODO Adjust texture or add new texture
    float textcoords[] = { 0, 2,   0, 0,  2, 2,  2, 0 };

    glUseProgram(programId2);
    glGenVertexArrays(1, &playerVA);
    glBindVertexArray(playerVA);
    GLuint buffers[3];
    glGenBuffers(3, buffers);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexPositionLoc2, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(vertexPositionLoc2);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexColorLoc2, 3, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(vertexColorLoc2);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textcoords), textcoords, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexTexcoordLoc2, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(vertexTexcoordLoc2);
}

static Bool collides(float x, float y, float z) {
    float startX = -ROOM_WIDTH / 2;
    float startY = -ROOM_HEIGHT / 2;
    float startZ = -ROOM_DEPTH / 2;
    float radius = 0.5;
    return   x - radius <= startX || x + radius >= startX + ROOM_WIDTH  ||
             y - radius <= startY || y + radius >= startY + ROOM_HEIGHT ||
             z - radius <= startZ || z + radius >= startZ + ROOM_DEPTH;
}

static void moveForward() {
    float yawRadians   = M_PI * observerYaw / 180;
    float incX = -observerSpeed * sin(yawRadians);
    float incZ = -observerSpeed * cos(yawRadians);
    if(collides(observerX + incX, observerY, observerZ + incZ)) {
        observerX -= incX;
        observerZ -= incZ;
        motionType = 0;
    } else {
        observerX += incX;
        observerZ += incZ;
    }
}

static void moveBack() {
    float yawRadians   = M_PI * observerYaw / 180;
    float incX = observerSpeed * sin(yawRadians);
    float incZ = observerSpeed * cos(yawRadians);
    if(collides(observerX + incX, observerY, observerZ + incZ)) {
        observerX -= incX;
        observerZ -= incZ;
        motionType = 0;
    } else {
        observerX += incX;
        observerZ += incZ;
    }
}

static void moveLeft() {
    float yawRadians   = M_PI * observerYaw / 180;
    float incX = observerSpeed * cos(yawRadians);
    float incZ = observerSpeed * sin(yawRadians);
    if(collides(observerX - incX, observerY, observerZ + incZ)) {
        observerX -= incX;
        observerZ -= incZ;
        motionType = 0;
    } else {
        observerX -= incX;
        observerZ += incZ;
    }
}

static void moveRight() {
    float yawRadians   = M_PI * observerYaw / 180;
    float incX = observerSpeed * cos(yawRadians);
    float incZ = observerSpeed * sin(yawRadians);
    if(collides(observerX + incX, observerY, observerZ - incZ)) {
        observerX -= incX;
        observerZ -= incZ;
        motionType = 0;
    } else {
        observerX += incX;
        observerZ -= incZ;
    }
}

static void turnLeft() {
    if(angleSpeed > 30) angleSpeed = 0.15;
    observerYaw += angleSpeed;
}

static void turnRight() {
    if(angleSpeed > 30) angleSpeed = 0.15;
    observerYaw -= angleSpeed;
}

static float dot(vec3 u, vec3 v) {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

static float magnitude(vec3 v) {
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static void toUnit(vec3 v) {
    float m = magnitude(v);
    v[0] /= m;
    v[1] /= m;
    v[2] /= m;
}

static int mouseX, mouseY;

static void displayFunc() {
    mouseX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    mouseY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    glutWarpPointer(mouseX, mouseY);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (motionType) {
        case FORWARD:       moveForward(); break;
        case BACK:          moveBack(); break;
        case TURN_RIGHT:    turnRight(); break;
        case TURN_LEFT:     turnLeft(); break;
        case RIGHT:         moveRight(); break;
        case LEFT:          moveLeft(); break;
        case IDLE:			break;
    }

    glUseProgram(programId1);

    //Draw room
    glUniformMatrix4fv(projectionMatrixLoc, 1, true, projectionMatrix.values);
    mIdentity(&viewMatrix);
    rotateX(&viewMatrix, -observerPitch);
    rotateY(&viewMatrix, -observerYaw);
    translate(&viewMatrix, -observerX, -observerY, -observerZ);
    glUniform3f(glGetUniformLocation(programId1, "camera"), observerX, observerY, observerZ);
    glUniformMatrix4fv(viewMatrixLoc, 1, true, viewMatrix.values);
    glUniform1f(hLoc, (float)glutGet(GLUT_WINDOW_HEIGHT));
    mIdentity(&modelMatrix);
    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
    glBindVertexArray(roomVA);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glDrawArrays(GL_TRIANGLES,  0, 24);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glDrawArrays(GL_TRIANGLES, 24,  6);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glDrawArrays(GL_TRIANGLES, 30,  6);

    glUseProgram(programId2);
    glBindVertexArray(playerVA);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(programId1);
    mIdentity(&modelMatrix);
    translate(&modelMatrix, -ROOM_WIDTH/4, 0, 0);
    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
    cylinder_draw(c);

    mIdentity(&modelMatrix);
    translate(&modelMatrix, ROOM_WIDTH/4, 0, 0);
    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
    cylinder_draw(c);

    mIdentity(&modelMatrix);
    translate(&modelMatrix, -ROOM_WIDTH/4, 0, -ROOM_DEPTH/4);
    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
    cylinder_draw(c);

    mIdentity(&modelMatrix);
    translate(&modelMatrix, ROOM_WIDTH/4, 0, -ROOM_DEPTH/4);
    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
    cylinder_draw(c);

    mIdentity(&modelMatrix);
    translate(&modelMatrix, -ROOM_WIDTH/4, 0, ROOM_DEPTH/4);
    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
    cylinder_draw(c);

    mIdentity(&modelMatrix);
    translate(&modelMatrix, ROOM_WIDTH/4, 0, ROOM_DEPTH/4);
    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
    cylinder_draw(c);


    glutSwapBuffers();
}

static void reshapeFunc(int w, int h) {
    if(h == 0) h = 1;
    glViewport(0, 0, w, h);
    float aspect = (float) w / h;
    setPerspective(&projectionMatrix, 45, aspect, -0.1, -500);
    glUniformMatrix4fv(projectionMatrixLoc, 1, true, projectionMatrix.values);
}

static void timerFunc(int id) {
    glutTimerFunc(10, timerFunc, id);
    glutPostRedisplay();
}

static void keyReleasedFunc(unsigned char key, int x, int y) {
    motionType = IDLE;
}

static void keyPressedFunc(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0);
    }
    switch(key) {
        case 'w':
        case 'W': motionType = FORWARD; break;
        case 's':
        case 'S': motionType = BACK; break;
        case 'd':
        case 'D': motionType = RIGHT; break;
        case 'a':
        case 'A': motionType = LEFT; break;
    }
    glutPostRedisplay();
}

static void mouseMotionFunc(int x, int y) {
    int incX = x - mouseX;
    int incY = y - mouseY;
    float m = sqrt(incX * incX + incY * incY);
    if(m == 0) return;
    angleSpeed = m / 10;
    incX /= m;
    incY /= m;
    if(incX == 1.0) turnRight();
    else if(incX == -1.0) turnLeft();
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    puts("Isaac");
    glutInitWindowPosition(0 ,0);
    glutInitWindowSize(600,600);
    glutCreateWindow("Shooting Range Isaac");
//    glutFullScreen();
    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutTimerFunc(10, timerFunc, 1);
    glutKeyboardFunc(keyPressedFunc);
    glutKeyboardUpFunc(keyReleasedFunc);
    glutPassiveMotionFunc(mouseMotionFunc);
    glutSetCursor(GLUT_CURSOR_NONE);
    glewInit();
    glEnable(GL_DEPTH_TEST);

    if (initShaders() != 0) {
    	puts("At least one shader did not compile.");
    	exit(1);
    }

    initTextures();
    initLights();
    initRoom();
    initPlayer();

    glClearColor(0, 0, 0, 1.0);
    glutMainLoop();
    return 0;
}
