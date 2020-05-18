#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Transforms.h"
#include "Utils.h"
#include "Cylinder.h"
#include "Vec4.h"
#include "Vec3.h"
#include "Sphere.h"

#define toRadians(deg) deg * M_PI / 180.0
#define max(a, b) a > b ? a : b
#define N_PILLAR 6
#define N_TARGETS 15

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
static float observerZ = 18;
static float observerYaw = 0;
static float observerPitch = 0;
static float observerSpeed = 0.1;
static float observerR = 2.0;
static float angleSpeed = 0.1;
static int mouseX, mouseY;
static float timer2sec = 0;
static int currentTargets = 0;

typedef struct {
    Vec3 position;
    Vec3 color;
    bool shot;
} Target;

Target targets[N_TARGETS];

static Mat4   modelMatrix, projectionMatrix, viewMatrix;

static GLuint programId1, vertexPositionLoc, vertexColorLoc,
              vertexNormalLoc, vertexTexcoordLoc, modelMatrixLoc,
              projectionMatrixLoc, viewMatrixLoc, hLoc;

static GLuint programId2, vertexPositionLoc2, vertexColorLoc2, vertexTexcoordLoc2;

static GLuint programId3, vertexPositionLoc3, vertexColorLoc3,vertexNormalLoc3,
              modelMatrixLoc3, projectionMatrixLoc3, viewMatrixLoc3;

static GLuint programId4, vertexPositionLoc4;

static GLuint ambientLightLoc, materialALoc, materialDLoc;
static GLuint materialSLoc, cameraPositionLoc;

static GLuint roomVA, playerVA, crossVA, targetVA;

static vec3 ambientLight  = {0.5, 0.5, 0.5};

static vec3 materialA     = {0.8, 0.8, 0.8};
static vec3 materialD     = {0.6, 0.6, 0.6};
static vec3 materialS     = {0.6, 0.6, 0.6};

static float lights[]   = {
        1, 1, 0.93, 	// Color
        0.9238,    		// Sub-cutoff
        0, 3,  -10,  	// Position
        256,	  		// Exponent
        0, -1,  0,   	// Direction
        0.7071,			// Cutoff

        1, 1, 0.93,
        0.9238,
        0, 3,   0,
        256,
        0, -1,  0,
        0.7071,

        1, 1, 0.93,
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
static vec3 lightBodyColor = {34.0/255, 36.0/255, 38.0/255};
static vec3 lightBaseColor= {1, 1, 0.93};
static Cylinder lightObj;
static Sphere bullet;
static vec3 bulletC = {179.0/255, 36.0/255, 40.0/255};
static vec3 bulletPosition = {0,0,0};
static vec3 bulletDirection = {0,0,0};
static int shootActive = 0, bulletInRange = 1;

typedef struct {
    Vec3 pos;
    float r;
    float h;
    Bool shot;
} Pillar;

Pillar pillars [N_PILLAR] ;

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
    initTexture("textures/Gun.bmp",  textures[2]);
    initTexture("textures/Target.bmp",  textures[3]);
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


    vShader = compileShader("shaders/lightobjs.vsh", GL_VERTEX_SHADER);
    if(!shaderCompiled(vShader)) return err;
    fShader = compileShader("shaders/lightobjs.fsh", GL_FRAGMENT_SHADER);
    if(!shaderCompiled(vShader)) return err;

    programId3 = glCreateProgram();
    glAttachShader(programId3, vShader);
    glAttachShader(programId3, fShader);
    glLinkProgram(programId3);

    vertexPositionLoc3   = glGetAttribLocation(programId3, "vertexPosition");
    vertexColorLoc3      = glGetAttribLocation(programId3, "vertexColor");
    vertexNormalLoc3     = glGetAttribLocation(programId3, "vertexNormal");
    modelMatrixLoc3      = glGetUniformLocation(programId3, "modelMatrix");
    viewMatrixLoc3       = glGetUniformLocation(programId3, "viewMatrix");
    projectionMatrixLoc3 = glGetUniformLocation(programId3, "projMatrix");

    vShader = compileShader("shaders/cross.vsh", GL_VERTEX_SHADER);
    if(!shaderCompiled(vShader)) return err;
    fShader = compileShader("shaders/cross.fsh", GL_FRAGMENT_SHADER);
    if(!shaderCompiled(vShader)) return err;

    programId4 = glCreateProgram();
    glAttachShader(programId4, vShader);
    glAttachShader(programId4, fShader);
    glLinkProgram(programId4);

    vertexPositionLoc4   = glGetAttribLocation(programId4, "vertexPosition");

    glUseProgram(programId1);
    c = cylinder_create(ROOM_HEIGHT, 1, 1, 40, 40, col, col, 0);
    cylinder_bind(c, vertexPositionLoc, vertexColorLoc, vertexNormalLoc, 0);

    glUseProgram(programId3);
    lightObj = cylinder_create_solid(0.5, 0.5, 0.25, 40, 40, lightBodyColor, lightBaseColor, 0);
    cylinder_bind(lightObj, vertexPositionLoc3, vertexColorLoc3, vertexNormalLoc3, 0);
    bullet = sphere_create_solid(0.1, 30, 30, bulletC, 0);
    sphere_bind(bullet, vertexPositionLoc3, vertexColorLoc3, vertexNormalLoc3, 0);

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
    float positions[] =  { 0.0, -0.2,    0.0, -1,    0.5, -0.2,    0.5, -1 };
    float colors[] = { 1, 1, 1,  1, 1, 1, 1, 1, 1,  1, 1, 1};
    float textcoords[] = { 0, 1,  0, 0,  1, 1, 1, 0};

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

static void initCross() {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float aspect = (float)w / h;
//    printf("%d, %d %.4f\n", w, h, aspect);
    float scale = 0.05;
    float model [] = {
            0,  		1 * scale,
            0,		   -1 * scale,
            1 * scale / aspect,  0,
           -1 * scale / aspect,  0
    };

    glUseProgram(programId4);
    glGenVertexArrays(1, &crossVA);
    glBindVertexArray(crossVA);
    GLuint buffers[1];
    glGenBuffers(1, buffers);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(model), model, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexPositionLoc4, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(vertexPositionLoc4);
}

static void initPillars() {
    for(int i = 0; i < N_PILLAR; i++) {
        pillars[i].h = ROOM_HEIGHT;
        pillars[i].r = 1;
        pillars[i].shot = False;
    }
    pillars[0].pos.x = -ROOM_WIDTH/4;
    pillars[0].pos.y = 0;
    pillars[0].pos.z = 0;

    pillars[1].pos.x = ROOM_WIDTH/4;
    pillars[1].pos.y = 0;
    pillars[1].pos.z = 0;

    pillars[2].pos.x = -ROOM_WIDTH/4;
    pillars[2].pos.y = 0;
    pillars[2].pos.z = -ROOM_DEPTH/4;

    pillars[3].pos.x = -ROOM_WIDTH/4;
    pillars[3].pos.y = 0;
    pillars[3].pos.z = ROOM_DEPTH/4;

    pillars[4].pos.x = ROOM_WIDTH/4;
    pillars[4].pos.y = 0;
    pillars[4].pos.z = -ROOM_DEPTH/4;

    pillars[5].pos.x = ROOM_WIDTH/4;
    pillars[5].pos.y = 0;
    pillars[5].pos.z = ROOM_DEPTH/4;
}

static void initSquare() {
    float l1 = -0.5, l2 = 0.5;
    float positions[] = {l1, l1, l2, l2, l1, l2, l1, l2, l2, l2, l1, l2, l2, l2, l2, l1, l2, l2,  // Frente
                         l2, l1, l1, l1, l1, l1, l2, l2, l1, l1, l1, l1, l1, l2, l1, l2, l2, l1,  // Atrás
                         l1, l1, l1, l1, l1, l2, l1, l2, l1, l1, l1, l2, l1, l2, l2, l1, l2, l1,  // Izquierda
                         l2, l2, l1, l2, l2, l2, l2, l1, l1, l2, l2, l2, l2, l1, l2, l2, l1, l1,  // Derecha
                         l1, l1, l1, l2, l1, l1, l1, l1, l2, l2, l1, l1, l2, l1, l2, l1, l1, l2,  // Abajo
                         l2, l2, l1, l1, l2, l1, l2, l2, l2, l1, l2, l1, l1, l2, l2, l2, l2, l2   // Arriba
    };

    float normals[] = { 0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  // Frente
                        0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  // Atrás
                       -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0,  // Izquierda
                        1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  // Derecha
                        0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  // Abajo
                        0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  // Arriba
    };
    float texcoords[] = {       0, 2,       0, 0,  2, 0,   2 , 0,   2 , 2,       0, 2,
                           2 * 0.5, 2,  2 * 0.5, 0,       0, 0,        0, 0,        0, 2,  2 * 0.5, 2,

                                0, 2,       0, 0,  2 * 0.5, 0,   2 * 0.5, 0,   2 * 0.5, 2,       0, 2,
                           2 * 0.5, 2,  2 * 0.5, 0,       0, 0,        0, 0,        0, 2,  2 * 0.5, 2,

                                0, 2,       0, 0,  2 * 0.5, 0,   2 * 0.5, 0,   2 * 0.5, 2,       0, 2,
                           2 * 0.5, 2,  2 * 0.5, 0,       0, 0,        0, 0,        0, 2,  2 * 0.5, 2,
    };

    glUseProgram(programId1);
    glGenVertexArrays(1, &targetVA);
    glBindVertexArray(targetVA);
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

static void initTargets() {
    srand(time(NULL));
    int i;
    float xRange = ROOM_WIDTH/2 - 0.5;
    float yRange = ROOM_HEIGHT/2 - 0.5;
    float zRange = ROOM_DEPTH/2 - 0.5;
    for(i = 0; i < N_TARGETS; i ++) {
        targets[i].position.x = -xRange  + (xRange  + xRange)  * rand() / RAND_MAX;
        targets[i].position.y = -yRange + (yRange + yRange) * rand() / RAND_MAX;
        targets[i].position.z = -zRange  + (zRange  + zRange)  * rand() / RAND_MAX;
        targets[i].shot = false;
    }
}

static double distance2D(float x1, float x2, float y1, float y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1- y2) * (y1- y2));
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

static int collidesPillar(int x, int z) {
    for (int i = 0; i < N_PILLAR; i++) {
        double d  = distance2D(x, pillars[i].pos.x, z, pillars[i].pos.z);
        //printf("%.4lf <= %.4f\n", d, (observerR + pillars[i].r));
        if ( d <= (observerR + pillars[i].r)) {
            return i;
        }
    }
    return -1;
}

static void moveForward() {
    float yawRadians   = M_PI * observerYaw / 180;
    float incX = -observerSpeed * sin(yawRadians);
    float incZ = -observerSpeed * cos(yawRadians);

    int idx = collidesPillar(observerX + incX, observerZ + incZ);
    if (idx >= 0) {
//    	double d1 = distance2D(observerX, pillars[idx].pos.x, observerZ, pillars[idx].pos.z); // Current
//    	double d2 = distance2D(observerX + incX, pillars[idx].pos.x, observerZ + incZ, pillars[idx].pos.z); // New
//    	printf("F: %.4lf, %.4lf\n", d1, d2);
        return;
    }

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

    int idx = collidesPillar(observerX + incX, observerZ + incZ);
    if (idx >= 0) {
//    	double d1 = distance2D(observerX, pillars[idx].pos.x, observerZ, pillars[idx].pos.z); // Current
//    	double d2 = distance2D(observerX + incX, pillars[idx].pos.x, observerZ + incZ, pillars[idx].pos.z); // New
//    	printf("F: %.4lf, %.4lf\n", d1, d2);
        return;
    }

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

    int idx = collidesPillar(observerX - incX, observerZ + incZ);
    if (idx >= 0) {
//    	double d1 = distance2D(observerX, pillars[idx].pos.x, observerZ, pillars[idx].pos.z); // Current
//    	double d2 = distance2D(observerX - incX, pillars[idx].pos.x, observerZ + incZ, pillars[idx].pos.z); // New
//    	printf("L: %.4lf, %.4lf\n", d1, d2);
        return;
    }

    if(collides(observerX - incX, observerY, observerZ + incZ)) {
        observerX += incX;
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

    int idx = collidesPillar(observerX + incX, observerZ - incZ);
    if (idx >= 0) {
//    	double d1 = distance2D(observerX, pillars[idx].pos.x, observerZ, pillars[idx].pos.z); // Current
//    	double d2 = distance2D(observerX + incX, pillars[idx].pos.x, observerZ - incZ, pillars[idx].pos.z); // New
//    	printf("R: %.4lf, %.4lf\n", d1, d2);
        return;
    }

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

static void turnDown() {
    if(angleSpeed > 30) angleSpeed = 0.15;
    if(observerPitch >= -70) observerPitch -= angleSpeed;
}

static void turnUp() {
    if(angleSpeed > 30) angleSpeed = 0.15;
    if(observerPitch <= 90)observerPitch += angleSpeed;
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

static float floatingAbs(float val) {
    if (val >= 0.0) return val;
    return -val;
}

static void updateBulletPosition() {
    for (int i = 0; i < 3; i++) {
        bulletPosition[i] += bulletDirection[i];
    }
    if (floatingAbs(bulletPosition[0]) <= ROOM_WIDTH/2 &&
        floatingAbs(bulletPosition[1]) <= ROOM_HEIGHT/2 &&
        floatingAbs(bulletPosition[2]) <= ROOM_DEPTH/2 ) {
        bulletInRange = 1;
//        printf("%.2f, %.2f\n", floatingAbs(bulletPosition[0]), floatingAbs(bulletPosition[2]));
    } else {
        bulletInRange = 0;
        shootActive = 0;
    }
}

static void displayFunc() {
    mouseX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    mouseY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    glutWarpPointer(mouseX, mouseY);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(timer2sec == 0 && currentTargets <= N_TARGETS) currentTargets++;

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

    // Draw cylinders
//    glUseProgram(programId1);
//    mIdentity(&modelMatrix);
//    translate(&modelMatrix, -ROOM_WIDTH/4, 0, 0);
//    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
//    cylinder_draw(c);
//
//    mIdentity(&modelMatrix);
//    translate(&modelMatrix, ROOM_WIDTH/4, 0, 0);
//    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
//    cylinder_draw(c);
//
//    mIdentity(&modelMatrix);
//    translate(&modelMatrix, -ROOM_WIDTH/4, 0, -ROOM_DEPTH/4);
//    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
//    cylinder_draw(c);
//
//    mIdentity(&modelMatrix);
//    translate(&modelMatrix, ROOM_WIDTH/4, 0, -ROOM_DEPTH/4);
//    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
//    cylinder_draw(c);
//
//    mIdentity(&modelMatrix);
//    translate(&modelMatrix, -ROOM_WIDTH/4, 0, ROOM_DEPTH/4);
//    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
//    cylinder_draw(c);
//
//    mIdentity(&modelMatrix);
//    translate(&modelMatrix, ROOM_WIDTH/4, 0, ROOM_DEPTH/4);
//    glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
//    cylinder_draw(c);

    glUseProgram(programId1);
    for (int i = 0; i < N_PILLAR; i++) {
        mIdentity(&modelMatrix);
        translate(&modelMatrix, pillars[i].pos.x, pillars[i].pos.y, pillars[i].pos.z);
        glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
        cylinder_draw(c);
    }

    // Draw Targets
    mIdentity(&viewMatrix);
    glUseProgram(programId1);
    rotateX(&viewMatrix, -observerPitch);
    rotateY(&viewMatrix, -observerYaw);
    translate(&viewMatrix, -observerX, -observerY, -observerZ);
    glUniformMatrix4fv(viewMatrixLoc, 1, true, viewMatrix.values);

    for(int i = 0; i < currentTargets; i ++) {
        mIdentity(&modelMatrix);
        translate(&modelMatrix, targets[i].position.x, targets[i].position.y, targets[i].position.z);
        glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
        glBindVertexArray(targetVA);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    //Draw light objects
    glUseProgram(programId3);

    glUniformMatrix4fv(projectionMatrixLoc3, 1, true, projectionMatrix.values);

    mIdentity(&viewMatrix);
    rotateX(&viewMatrix, -observerPitch);
    rotateY(&viewMatrix, -observerYaw);
    translate(&viewMatrix, -observerX, -observerY, -observerZ);
    glUniformMatrix4fv(viewMatrixLoc3, 1, true, viewMatrix.values);

    mIdentity(&modelMatrix);
    translate(&modelMatrix, 0, ROOM_HEIGHT / 2, 0);
    glUniformMatrix4fv(modelMatrixLoc3, 1, true, modelMatrix.values);
    cylinder_draw(lightObj);

    mIdentity(&modelMatrix);
    translate(&modelMatrix, 0, ROOM_HEIGHT / 2, -ROOM_DEPTH/4);
    glUniformMatrix4fv(modelMatrixLoc3, 1, true, modelMatrix.values);
    cylinder_draw(lightObj);

    mIdentity(&modelMatrix);
    translate(&modelMatrix, 0, ROOM_HEIGHT / 2, ROOM_DEPTH/4);
    glUniformMatrix4fv(modelMatrixLoc3, 1, true, modelMatrix.values);
    cylinder_draw(lightObj);

    // Draw bullet
    if(shootActive && bulletInRange) {
        updateBulletPosition();
        mIdentity(&modelMatrix);
        translate(&modelMatrix, bulletPosition[0], bulletPosition[1], bulletPosition[2]);
        glUniformMatrix4fv(modelMatrixLoc3, 1, true, modelMatrix.values);
        sphere_draw(bullet);
    }

    // Draw cross
    glUseProgram(programId4);
    glBindVertexArray(crossVA);
    glLineWidth(2.0);
    glDrawArrays(GL_LINES, 0, 4);


    // Draw shotgun
    glUseProgram(programId2);
    glBindVertexArray(playerVA);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
    if(timer2sec <= 2.0) timer2sec += 0.01;
    else timer2sec = 0;
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

static void mouseClick(int button, int state, int x, int y) {
//    printf("%d, %d\n", x, y);
    if ( state != GLUT_UP) return;
    if(shootActive) return;

    if(button == GLUT_RIGHT_BUTTON) {
        // Norm coords
        float nx = 2.0 * x / glutGet(GLUT_WINDOW_WIDTH) - 1;
        float ny =  -1 * (2.0 * y / glutGet(GLUT_WINDOW_HEIGHT) - 1);
//        printf("Norm: %.2f, %.2f\n", nx, ny);

        Vec4 rayN = {nx, ny, -1, 1};
        // View coords
        Mat4 invProjectioMatrix;
        inverse(projectionMatrix, &invProjectioMatrix);
        Vec4 rayV;
        multiply(invProjectioMatrix, rayN, &rayV);
//        printf("View: %.2f, %.2f\n", rayV.x, rayV.y);

        // World coords
        rayV.z = -1;
        rayV.w = 0;
        Mat4 invViewMatrix;
        inverse(viewMatrix, &invViewMatrix);
        Vec4 rayM;
        multiply(invViewMatrix, rayV, &rayM);
        rayM.w = 0;
        vec4_normalize(&rayM);
//        printf("World: %.2f, %.2f, %.2f\n", rayM.x, rayM.y, rayM.z);
        // Set bullet initial position
        bulletPosition[0] = observerX;
        bulletPosition[1] = observerY;
        bulletPosition[2] = observerZ;
        // Set bullet direction
        bulletDirection[0] = rayM.x;
        bulletDirection[1] = rayM.y;
        bulletDirection[2] = rayM.z;

        shootActive = 1;
        bulletInRange = 1;
    }
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
    if(incY == -1.0) turnUp();
    else if(incY == 1.0) turnDown();
    glutPostRedisplay();
}

int main2(int argc, char **argv) {
    setbuf(stdout, NULL);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
//    glutInitWindowPosition(0 ,0);
//    glutInitWindowSize(900, 600);
    glutCreateWindow("Shooting Range App");
    glutFullScreen();
    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutTimerFunc(10, timerFunc, 1);
    glutKeyboardFunc(keyPressedFunc);
    glutKeyboardUpFunc(keyReleasedFunc);
    glutPassiveMotionFunc(mouseMotionFunc);
    glutMouseFunc(mouseClick);
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
    initCross();
    initPillars();
    initSquare();
    initTargets();

    glClearColor(0, 0, 0, 1.0);
    glutMainLoop();
    return 0;
}
