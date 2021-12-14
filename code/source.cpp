#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <iostream> 
using namespace std;

struct particle{
    double x, y, z, vx, vy, vz, ax, ay, az, size;
};

//default values of the materrial parameters;
GLfloat DEFAULT_AMIBENT[4] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat DEFAULT_DIFFUSE[4] = { 0.8, 0.8, 0.8f, 1.0 };
GLfloat DEFAULT_SPECULAR[4] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat DEFAULT_SHINESS[1] = { 0.0 };
GLfloat DEFAULT_EMISSION[4] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat DEFAULT_COLOR_INDEXES[3] = { 0,1,1 };

// parameters to for the whole program.
static GLint w;
static GLint h;
static GLint pixellength;
static unsigned char* pImage;
const float PI = 3.14159;
int snowNum = 3000;
particle snow[3000];
GLUquadricObj* qobj;
GLUquadricObj* skySphere;
double timer = 0;
float sunRadius = 20;
int time_interval = 16;
int lamp_state = 0, streetLight_state = 0, snow_state = 0;
GLfloat oldx, oldy;
float cx = -5, cy = 2, cz = 0, lx = 5, ly = -2, lz= 0;
float angle1 = 0, angle2 = 0;


// read the bmp files by its filenames and extract textures
void readTexture(const char* filename) {
    BITMAPFILEHEADER fileheader;
    BITMAPINFOHEADER infoheader;
    RGBTRIPLE pixel;
    FILE* pFile = NULL;
    fopen_s(&pFile, filename, "rb");
    if (pFile == NULL) exit(0);
    fread(&fileheader, sizeof(fileheader), 1, pFile);
    fread(&infoheader, sizeof(infoheader), 1, pFile);
    int nSize;
    w = (int)(infoheader.biWidth);
    h = (int)(infoheader.biHeight);
    nSize = w * h * 4;
    pImage = (unsigned char*)new char[nSize];
    int j = 0;
    for (int i = 0; i < w * h; i++)
    {
        fread(&pixel, sizeof(pixel), 1, pFile);
        pImage[j + 0] = pixel.rgbtRed;
        pImage[j + 1] = pixel.rgbtGreen;
        pImage[j + 2] = pixel.rgbtBlue;
        pImage[j + 3] = 255;
        j += 4;
    }
    fclose(pFile);
}


//set material parameter for objects
void setMaterial(GLenum face, GLfloat ambient[4], GLfloat diffuse[4], GLfloat specular[4],
    GLfloat shine[1], GLfloat emission[4], GLfloat indexes[3]) {
    glMaterialfv(face, GL_AMBIENT, ambient);
    glMaterialfv(face, GL_DIFFUSE, diffuse);
    glMaterialfv(face, GL_SPECULAR, specular);
    glMaterialfv(face, GL_SHININESS, shine);
    glMaterialfv(face, GL_EMISSION, emission);
    glMaterialfv(face, GL_COLOR_INDEXES, indexes);
}


//input int value to set the color
void setColor(float r, float g, float b) {
    r = r / 255.f;
    g = g / 255.f;
    b = b / 255.f;
    glColor3f(r, g, b);

}


//Initialize and control the light system
void lightInit() {
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    //sun light
    float ambientLight1[] = { 0.3f,0.3f,0.3f,1.0f };
    float diffuseLight1[] = { 255.0 / 255.0, 210.0 / 255.0, 166.0 / 255.0,1.0f };
    float positionLight1[] = { sunRadius * cos(timer), sunRadius * sin(timer),3,0 };
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight1);
    glLightfv(GL_LIGHT1, GL_POSITION, positionLight1);
    
    if (timer<PI)
    {
        glEnable(GL_LIGHT1);
    }
    else {
        glDisable(GL_LIGHT1);
    }

    //moon light
    float ambientLight2[] = { 0.1f,0.1f,0.1f,1.0f };
    float diffuseLight2[] = { 0.4f,0.4f,0.4f,1.0f };
    float positionLight2[] = { sunRadius * cos(timer-PI), sunRadius * sin(timer-PI),3,0 };
    glLightfv(GL_LIGHT2, GL_AMBIENT, ambientLight2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuseLight2);
    glLightfv(GL_LIGHT2, GL_POSITION, positionLight2);
    if (timer > PI)
    {
        glEnable(GL_LIGHT2);
    }
    else {
        glDisable(GL_LIGHT2);
    }

    //lamp light
    float ambientLight3[] = { 0.5f,0.4804f,0.313f,0.2f };
    float diffuseLight3[] = { 1.0f,0.9608f,0.6157f,1.0f };
    float positionLight3[] = { -1.6f,-0.5f,-1.0f,1.0f };
    glLightfv(GL_LIGHT3, GL_AMBIENT, ambientLight3);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuseLight3);
    glLightfv(GL_LIGHT3, GL_POSITION, positionLight3);
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 0.0);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 3);
    glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 1);
    
    if (lamp_state == 1) {
        glEnable(GL_LIGHT3);
    }
    else {
        glDisable(GL_LIGHT3);
    }

    //street lamps light
    float ambientLights[] = { 0.3f,0.3f,0.3f,0.1f };
    float diffuseLights[] = { 1.0f,1.0f,1.0f,1.0f };
    float position4[] = { 5.4, 2.1, 4.65, 1};
    float position5[] = { 9.6, 2.1, 4.65 ,1};
    float position6[] = { 5.4, 2.1, -5.05, 1};
    float position7[] = { 9.6, 2.1, -5.05, 1};

    glLightfv(GL_LIGHT4, GL_AMBIENT, ambientLights);
    glLightfv(GL_LIGHT4, GL_DIFFUSE, diffuseLights);
    glLightfv(GL_LIGHT4, GL_POSITION, position4);
    glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 20.0f);

    glLightfv(GL_LIGHT5, GL_AMBIENT, ambientLights);
    glLightfv(GL_LIGHT5, GL_DIFFUSE, diffuseLights);
    glLightfv(GL_LIGHT5, GL_POSITION, position5);
    glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 20.0f);

    glLightfv(GL_LIGHT6, GL_AMBIENT, ambientLights);
    glLightfv(GL_LIGHT6, GL_DIFFUSE, diffuseLights);
    glLightfv(GL_LIGHT6, GL_POSITION, position6);
    glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, 20.0f);

    glLightfv(GL_LIGHT7, GL_AMBIENT, ambientLights);
    glLightfv(GL_LIGHT7, GL_DIFFUSE, diffuseLights);
    glLightfv(GL_LIGHT7, GL_POSITION, position7);
    glLightf(GL_LIGHT7, GL_SPOT_CUTOFF, 20.0f);

    if (streetLight_state == 1) {
        glEnable(GL_LIGHT4);
        glEnable(GL_LIGHT5);
        glEnable(GL_LIGHT6);
        glEnable(GL_LIGHT7);
    }
    else {
        glDisable(GL_LIGHT4);
        glDisable(GL_LIGHT5);
        glDisable(GL_LIGHT6);
        glDisable(GL_LIGHT7);
    }
}


// draw cuboid with left front bottom point (x,y,z) and length x1, height y1, width z1
void drawCuboid(double x, double y, double z, double x1, double y1, double z1) {
    double cuboid[8][3];
    cuboid[0][0] = x;
    cuboid[0][1] = y;
    cuboid[0][2] = z;

    cuboid[1][0] = x;
    cuboid[1][1] = y;
    cuboid[1][2] = z + z1;

    cuboid[2][0] = x + x1;
    cuboid[2][1] = y;
    cuboid[2][2] = z + z1;

    cuboid[3][0] = x + x1;
    cuboid[3][1] = y;
    cuboid[3][2] = z;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (j == 1)
                cuboid[i + 4][j] = cuboid[i][j] + y1;
            else
                cuboid[i + 4][j] = cuboid[i][j];
        }
    }

    glBegin(GL_POLYGON);        //draw bottom
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(cuboid[0][0], cuboid[0][1], cuboid[0][2]);
    glVertex3f(cuboid[1][0], cuboid[1][1], cuboid[1][2]);
    glVertex3f(cuboid[2][0], cuboid[2][1], cuboid[2][2]);
    glVertex3f(cuboid[3][0], cuboid[3][1], cuboid[3][2]);
    glEnd();

    glBegin(GL_POLYGON);        //draw top
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(cuboid[4][0], cuboid[4][1], cuboid[4][2]);
    glVertex3f(cuboid[7][0], cuboid[7][1], cuboid[7][2]);
    glVertex3f(cuboid[6][0], cuboid[6][1], cuboid[6][2]);
    glVertex3f(cuboid[5][0], cuboid[5][1], cuboid[5][2]);
    glEnd();

    glBegin(GL_POLYGON);       //draw front
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(cuboid[5][0], cuboid[5][1], cuboid[5][2]);
    glVertex3f(cuboid[6][0], cuboid[6][1], cuboid[6][2]);
    glVertex3f(cuboid[2][0], cuboid[2][1], cuboid[2][2]);
    glVertex3f(cuboid[1][0], cuboid[1][1], cuboid[1][2]);
    glEnd();

    glBegin(GL_POLYGON);        //draw behind
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(cuboid[0][0], cuboid[0][1], cuboid[0][2]);
    glVertex3f(cuboid[3][0], cuboid[3][1], cuboid[3][2]);
    glVertex3f(cuboid[7][0], cuboid[7][1], cuboid[7][2]);
    glVertex3f(cuboid[4][0], cuboid[4][1], cuboid[4][2]);
    glEnd();

    glBegin(GL_POLYGON);        //draw left
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(cuboid[1][0], cuboid[1][1], cuboid[1][2]);
    glVertex3f(cuboid[0][0], cuboid[0][1], cuboid[0][2]);
    glVertex3f(cuboid[4][0], cuboid[4][1], cuboid[4][2]);
    glVertex3f(cuboid[5][0], cuboid[5][1], cuboid[5][2]);
    glEnd();

    glBegin(GL_POLYGON);        //draw right
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(cuboid[7][0], cuboid[7][1], cuboid[7][2]);
    glVertex3f(cuboid[6][0], cuboid[6][1], cuboid[6][2]);
    glVertex3f(cuboid[2][0], cuboid[2][1], cuboid[2][2]);
    glVertex3f(cuboid[3][0], cuboid[3][1], cuboid[3][2]);
    glEnd();
}


// draw a four trustum of a pyramid with left front bottom point (x1,y1,z1), right behind bottom point (x2,y2,z2),
// left front top point (x3,y3,z3), right behind top point (x4,y4,z4)
void drawFourPrism(double x1, double y1, double z1, double x2, double y2, double z2,
    double x3, double y3, double z3, double x4, double y4, double z4) {
    double fPrism[8][3];
    fPrism[0][0] = x1;
    fPrism[0][1] = y1;
    fPrism[0][2] = z1;

    fPrism[2][0] = x2;
    fPrism[2][1] = y2;
    fPrism[2][2] = z2;

    fPrism[4][0] = x3;
    fPrism[4][1] = y3;
    fPrism[4][2] = z3;

    fPrism[6][0] = x4;
    fPrism[6][1] = y4;
    fPrism[6][2] = z4;

    fPrism[1][0] = x1;
    fPrism[1][1] = y1;
    fPrism[1][2] = z2;

    fPrism[3][0] = x2;
    fPrism[3][1] = y1;
    fPrism[3][2] = z1;

    fPrism[5][0] = x3;
    fPrism[5][1] = y3;
    fPrism[5][2] = z4;

    fPrism[7][0] = x4;
    fPrism[7][1] = y3;
    fPrism[7][2] = z3;

    glBegin(GL_POLYGON);            //top
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(fPrism[4][0], fPrism[4][1], fPrism[4][2]);
    glVertex3f(fPrism[7][0], fPrism[7][1], fPrism[7][2]);
    glVertex3f(fPrism[6][0], fPrism[6][1], fPrism[6][2]);
    glVertex3f(fPrism[5][0], fPrism[5][1], fPrism[5][2]);
    glEnd();

    glBegin(GL_POLYGON);            //bottom
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(fPrism[0][0], fPrism[0][1], fPrism[0][2]);
    glVertex3f(fPrism[1][0], fPrism[1][1], fPrism[1][2]);
    glVertex3f(fPrism[2][0], fPrism[2][1], fPrism[2][2]);
    glVertex3f(fPrism[3][0], fPrism[3][1], fPrism[3][2]);
    glEnd();

    glBegin(GL_POLYGON);            //behind
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(fPrism[5][0], fPrism[5][1], fPrism[5][2]);
    glVertex3f(fPrism[6][0], fPrism[6][1], fPrism[6][2]);
    glVertex3f(fPrism[2][0], fPrism[2][1], fPrism[2][2]);
    glVertex3f(fPrism[1][0], fPrism[1][1], fPrism[1][2]);
    glEnd();

    glBegin(GL_POLYGON);            //front
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(fPrism[0][0], fPrism[0][1], fPrism[0][2]);
    glVertex3f(fPrism[3][0], fPrism[3][1], fPrism[3][2]);
    glVertex3f(fPrism[7][0], fPrism[7][1], fPrism[7][2]);
    glVertex3f(fPrism[4][0], fPrism[4][1], fPrism[4][2]);
    glEnd();

    glBegin(GL_POLYGON);            //left
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(fPrism[1][0], fPrism[1][1], fPrism[1][2]);
    glVertex3f(fPrism[0][0], fPrism[0][1], fPrism[0][2]);
    glVertex3f(fPrism[4][0], fPrism[4][1], fPrism[4][2]);
    glVertex3f(fPrism[5][0], fPrism[5][1], fPrism[5][2]);
    glEnd();

    glBegin(GL_POLYGON);            //right
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(fPrism[7][0], fPrism[7][1], fPrism[7][2]);
    glVertex3f(fPrism[6][0], fPrism[6][1], fPrism[6][2]);
    glVertex3f(fPrism[2][0], fPrism[2][1], fPrism[2][2]);
    glVertex3f(fPrism[3][0], fPrism[3][1], fPrism[3][2]);
    glEnd();
}


// draw a circle with bottom central point (x,y,z) and radius r
void drawCircle(float x, float y, float z, float r) {
    GLfloat vertex[3];
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(vertex);
    for (int i = 0; i <= 60; i++)
    {
        vertex[0] = cos(2.0 * PI / 60 * i) * r + x;
        vertex[1] = y;
        vertex[2] = sin(2.0 * PI / 60 * i) * r + z;
        glVertex3fv(vertex);
    }
    glEnd();
}


// draw a circular truncated cone with bottom central point (x,y,z), bottom radius r1, top radius r2 and height h
void drawCylinder(float x, float y, float z, float r1, float r2, float h){
    GLfloat vertex[3];
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;
    glBegin(GL_QUAD_STRIP);
    glVertex3fv(vertex);
    for (int i = 0; i <= 60; i++)
    {
        vertex[0] = cos(2.0 * PI / 60 * i) * r1 + x;
        vertex[1] = y;
        vertex[2] = sin(2.0 * PI / 60 * i) * r1 + z;
        glVertex3fv(vertex);
        vertex[0] = cos(2.0 * PI / 60 * i) * r2 + x;
        vertex[1] = y + h;
        vertex[2] = sin(2.0 * PI / 60 * i) * r2 + z;
        glVertex3fv(vertex);
    }
    glEnd();
    drawCircle(x, y, z, r1);
    drawCircle(x, y+h, z, r2);
}


// draw snow with info snow[i]
void drawSnow(int i) {
    setColor(255, 255, 255);
    glPushMatrix();
    glTranslatef(snow[i].x, snow[i].y, snow[i].z);
    glutSolidSphere(snow[i].size, 10, 10);
    glPopMatrix();

}


// random float number generator, from a to b
double randGen(int a, int b) {        
    return b - rand() / (double)RAND_MAX * (b - a);
}


// snow initializer
void snowInit(int i) {       
    snow[i].x = randGen(-15,15);
    snow[i].y = 15;
    snow[i].z = randGen(-15, 15);

    snow[i].vx = randGen(-2, 2) / 30.f;
    snow[i].vy = randGen(1, 10) / 1000;
    snow[i].vz = randGen(-2, 2) / 30.f;

    snow[i].ax = randGen(-1, 1) / 500.f;
    snow[i].ay = randGen(0, 5) / 1000;
    snow[i].az = randGen(-1, 1) / 500.f;

    snow[i].size = randGen(6, 8)/200;
}


// create snows
void creatsnow() {      
    for (int i = 0; i < snowNum; i++) {
        snowInit(i);
    }
}


// update the state of each snow, kill the dead snow and initialze the new one
void snowUpdate() {     
    for (int i = 0; i < snowNum; i++) {
        snow[i].x += snow[i].vx;
        snow[i].y -= snow[i].vy;
        snow[i].z -= snow[i].vz;

        snow[i].vx += snow[i].ax;
        snow[i].vy += snow[i].ay;
        snow[i].vz += snow[i].az;

        if (snow[i].y < -20) {
            snowInit(i);
        }
    }
}


// let the snow fall
void snowFall() {     
    if (snow_state == 1) {
        Sleep(33);
        snowUpdate();
        for (int i = 0; i < snowNum; i++) {
            drawSnow(i);
        }
    }

}


// init some parameters and open depth test
void myinit(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}


// draw the grass
void drawGrass() {
    char str[] = "grass.bmp";
    readTexture(str);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    free(pImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(100.0f, 100.0f); glVertex3f(-100.0f, -1.0f, -100.0f);
    glTexCoord2f(0.0f, 100.0f); glVertex3f(100.0f, -1.0f, -100.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(100.0f, -1.0f, 100.0f);
    glTexCoord2f(100.0f, 0.0f); glVertex3f(-100.0f, -1.0f, 100.0f);
    glEnd();
}


//draw the brick of the house
void drawHouse() {
    char str[] = "wall.bmp";
    readTexture(str);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    free(pImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 2.f);
    glTexCoord2f(2.0f, 0.0f); glVertex3f(1.0f, -1.0f, 2.f);
    glTexCoord2f(2.0f, 2.0f); glVertex3f(1.0f, 1.0f, 2.f);
    glTexCoord2f(0.0f, 2.0f); glVertex3f(-1.0f, 1.0f, 2.f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, -1.0);
    glTexCoord2f(2.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -2.f);
    glTexCoord2f(2.0f, 2.0f); glVertex3f(-1.0f, 1.0f, -2.f);
    glTexCoord2f(0.0f, 2.0f); glVertex3f(1.0f, 1.0f, -2.f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -2.f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0f, 4.0f); glVertex3f(-1.0f, 1.0f, -2.f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 2.f);
    glTexCoord2f(2.0f, 0.0f); glVertex3f(1.0f, 1.0f, 2.f);
    glTexCoord2f(2.0f, 4.0f); glVertex3f(1.0f, 1.0f, -2.f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0, -1.0, 0.0);
    glTexCoord2f(4.0f, 2.0f); glVertex3f(-1.0f, -1.0f, -2.f);
    glTexCoord2f(0.0f, 2.0f); glVertex3f(1.0f, -1.0f, -2.f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 2.f);
    glTexCoord2f(4.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 2.f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(1.0, 0.0, 0.0);
    glTexCoord2f(4.0f, 0.0f); glVertex3f(1.0f, -1.0f, -2.f);
    glTexCoord2f(4.0f, 2.0f); glVertex3f(1.0f, 1.0f, -2.f);
    glTexCoord2f(0.0f, 2.0f); glVertex3f(1.0f, 1.0f, 2.f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 2.f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -2.f);
    glTexCoord2f(4.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 2.f);
    glTexCoord2f(4.0f, 2.0f); glVertex3f(-1.0f, 1.0f, 2.f);
    glTexCoord2f(0.0f, 2.0f); glVertex3f(-1.0f, 1.0f, -2.f);
    glEnd();
}


//draw the roof of the house
void drawRoof() {
    glBegin(GL_QUADS);
    glVertex3f(-1.2f, 1.0f, -2.2f);
    glVertex3f(-1.2f, 1.0f, 2.2f);
    glVertex3f(1.2f, 1.0f, 2.2f);
    glVertex3f(1.2f, 1.0f, -2.2f);
    glEnd();
    char str[] = "roof.bmp";
    readTexture(str);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    free(pImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_POLYGON);
    glTexCoord2f(1.f, 1.f); glVertex3f(0.f, 1.8f, 1.5f);
    glTexCoord2f(0.f, 1.f); glVertex3f(0.f, 1.8f, -1.5f);
    glTexCoord2f(0, 0.0f); glVertex3f(1.2f, 1.f, -2.2f);
    glTexCoord2f(1, 0.0f); glVertex3f(1.2f, 1.f, 2.2f);
    glEnd();

    glBegin(GL_POLYGON);
    glTexCoord2f(1.f, 1.f); glVertex3f(0.f, 1.8f, 1.5f);
    glTexCoord2f(0.f, 1.f); glVertex3f(0.f, 1.8f, -1.5f);
    glTexCoord2f(0, 0.0f); glVertex3f(-1.2f, 1.f, -2.2f);
    glTexCoord2f(1, 0.0f); glVertex3f(-1.2f, 1.f, 2.2f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0, 0.0f); glVertex3f(1.2f, 1.f, 2.2f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(-1.2f, 1.f, 2.2f);
    glTexCoord2f(0.25f, 1.0f); glVertex3f(0.f, 1.8f, 1.5f);

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0, 0.0f); glVertex3f(1.2f, 1.f, -2.2f);
    glTexCoord2f(2.4, 0.0f); glVertex3f(-1.2f, 1.f, -2.2f);
    glTexCoord2f(1.2f, 0.8f); glVertex3f(0.f, 1.8f, -1.5f);
    glEnd();
}


//draw two windows of the house
void drawWindow() {
    char str[] = "window.bmp";
    readTexture(str);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    free(pImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glNormal3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.001f, -0.5f, -1.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.001f, 0.5f, -1.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.001f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.001f, -0.5f, -0.5f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.001f, -0.5f, -1.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.001f, 0.5f, -1.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.001f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.001f, -0.5f, -0.5f);
    glEnd();
}


//draw a door
void drawDoor() {
    char str[] = "door.bmp";
    readTexture(str);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    free(pImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.001f, -1.0f, 0.72f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.001f, 0.5f, 0.72f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.001f, 0.5f, 1.36f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.001f, -1.0f, 1.36f);
    glEnd();
}


// draw the road with two ways
void drawRoad() {
    char str[] = "road.bmp";
    readTexture(str);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    free(pImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(6.f, -0.99f, 20.f);
    glTexCoord2f(2.0f, 0.0f); glVertex3f(10.f, -0.99f, 20.f);
    glTexCoord2f(2.0f, 20.0f); glVertex3f(10.f, -0.99f, -20.f);
    glTexCoord2f(0.0f, 20.0f); glVertex3f(6.f, -0.99f, -20.f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    
    // draw the spilt line
    for (int i = -50; i < 45; i += 5) {
        glBegin(GL_QUADS);
        glVertex3f(7.8f, -0.98f, (float)i);
        glVertex3f(8.2f, -0.98f, (float)i);
        glVertex3f(8.2f, -0.98f, (float)i + 3.f);
        glVertex3f(7.8f, -0.98f, (float)i + 3.f);
        glEnd();
    }
    
}


// draw a curving path from the door 
void drawPath() {
    char str[] = "path.bmp";
    readTexture(str);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    free(pImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    glNormal3f(1.0, 0.0, 0.0);
    for (int i = 0; i > -50; i--) {
        glBegin(GL_QUADS);
        float x1 = 6 * PI / 50 * i;
        float z1 = 0.5*sin(x1);
        float x2 = 6 * PI / 50 * (i + 1);
        float z2 = 0.5*sin(x2);
        glTexCoord2f(x1, 0.0f); glVertex3f(x1-1, -0.99f, z1+0.2);
        glTexCoord2f(x1, 1.0f); glVertex3f(x1-1, -0.99f, z1+1.8);
        glTexCoord2f(x2, 1.0f); glVertex3f(x2-1, -0.99f, z2+1.8);
        glTexCoord2f(x2, 0.0f); glVertex3f(x2-1, -0.99f, z2+0.2);
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
}


//draw a lamp before the window
void drawLamp() {
    setColor(69, 90, 100);
    drawFourPrism(-1.8, -0.99, -1.2, -1.4, -0.99, -0.8, -1.7, -0.9, -1.1, -1.5, -0.9, -0.9);
    setColor(120, 144, 156);
    drawCylinder(-1.6, -0.9, -1.0, 0.06, 0.06, 0.3);
    setColor(194, 205, 211);
    drawCylinder(-1.6, -0.6, -1.0, 0.2, 0.1, 0.2);
}


//draw four street lights
void drawStreetLight() {
    setColor(224, 224, 224);
    drawCuboid(5,-0.99,4.7,0.3,0.1,0.3);
    drawCylinder(5.15,-0.9,4.85,0.1,0.1,3);
    drawCuboid(5, 2.1, 4.65, 1.0, 0.05, 0.4);

    drawCuboid(10.7, -0.99, 4.7, 0.3, 0.1, 0.3);
    drawCylinder(10.85, -0.9, 4.85, 0.1, 0.1, 3);
    drawCuboid(10, 2.1, 4.65, 1.0, 0.05, 0.4);

    drawCuboid(5, -0.99, -5, 0.3, 0.1, 0.3);
    drawCylinder(5.15, -0.9, -4.85, 0.1, 0.1, 3);
    drawCuboid(5, 2.1, -5.05, 1.0, 0.05, 0.4);

    drawCuboid(10.7, -0.99, -5, 0.3, 0.1, 0.3);
    drawCylinder(10.85, -0.9, -4.85, 0.1, 0.1, 3);
    drawCuboid(10, 2.1, -5.05, 1.0, 0.05, 0.4);
}


//draw sun or moon 
void drawSun() {
    glPushMatrix();
    qobj = gluNewQuadric();

    if (timer < PI) {
        char str[] = "sun.bmp";
        glTranslatef(sunRadius * cos(timer), sunRadius * sin(timer), 0);
        readTexture(str);
    }
    else {
        char str[] = "moon.bmp";
        glTranslatef(sunRadius * cos(timer - PI), sunRadius * sin(timer - PI), 0);
        readTexture(str);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    free(pImage);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glEnable(GL_TEXTURE_2D);

    gluQuadricTexture(qobj, GL_TRUE);
    gluSphere(qobj, 1.0, 20, 20);

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


// draw the sky ball
void drawSky(){
    glPushMatrix();
    skySphere = gluNewQuadric();
    if (timer < PI) {
        char str[] = "sky_day.bmp";
        readTexture(str);
    }
    else {
        char str[] = "sky_night.bmp";
        readTexture(str);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glEnable(GL_TEXTURE_2D);

    gluQuadricTexture(skySphere, GL_TRUE);
    gluSphere(skySphere, 20, 80, 80);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


//set the timer, when timer exceed one day, reset it to zero
void OnTimer(int value) {
    if (timer < 2 * PI) {
        timer += 0.01;
    }
    else {
        timer = 0;
    }
    glutTimerFunc(time_interval, OnTimer, 1);
}


void when_in_mainloop() {
    glutPostRedisplay();
}


// reshape the scene when window changed
void myReshape(GLsizei w, GLsizei h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0 * (GLfloat)w / (GLfloat)h, 1, 30.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -3.6);
}


// detect keyboard inputs for camera changing and state control
void keyboard_input(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'W':
    case 'w':
        if (cx < 7 && cz > -7 && cx > -7 && cz < 7) {
            cx += lx * 0.3;
            cz += lz * 0.3;
        }
        break;
    case 'A':
    case 'a':
        if (cx < 7 && cz > -7 && cx > -7 && cz < 7) {
            cx += lz * 0.3;
            cz -= lx * 0.3;
        }
        break;
    case 'S':
    case 's':
        if (cx < 7 && cz > -7 && cx > -7 && cz < 7) {
            cx -= lx * 0.3;
            cz -= lz * 0.3;
        }
        break;
    case 'D':
    case 'd':
        if (cx < 7 && cz > -7 && cx > -7 && cz < 7) {
            cx -= lz * 0.3;
            cz += lx * 0.3;
        }
        break;
    case 'Z':
    case 'z':
        if (cy > 1) {
            cy -= 0.3;
        }
        break;
    case 'X':
    case 'x':
        if (cy < 5) {
            cy += 0.3;
        }
        break;
    case 'R':
    case 'r':
        cx = -5, cy = 2, cz = 0, lx = 5, ly = -2, lz = 0;
        break;
    case '1':
        if (snow_state == 0) {
            snow_state = 1;
        }
        else {
            snow_state = 0;
        }
        break;
    case '2':
        if (lamp_state == 0) {
            lamp_state = 1;
            printf("1");
        }
        else {
            lamp_state = 0;
            printf("0");
        }
        break;
    case '3':
        if (streetLight_state == 0) {
            streetLight_state = 1;
        }
        else {
            streetLight_state = 0;
        }
        break;
    default:
        break;
    }
}


// detect mouse position when it is left clicked
void mouse_input(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        oldx = x;
        oldy = y;
    }
}


// get the motion and change the viewpoint
void motion_input(int x, int y)
{

    GLint deltax = -(oldx - x);
    GLint deltay = (oldy - y);

    oldx = x;
    oldy = y;

    angle1 += deltax * 0.01;
    angle2 += deltay * 0.01;

    lx = sin(angle1);
    lz = -cos(angle1);
    ly = sin(angle2);
}


//display everyting and set the projection
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(cx, cy, cz, cx + lx, cy + ly, cz + lz, 0.0, 0.1, 0.0);
    setMaterial(GL_FRONT, DEFAULT_AMIBENT, DEFAULT_DIFFUSE, DEFAULT_SPECULAR, DEFAULT_SHINESS, DEFAULT_EMISSION, DEFAULT_COLOR_INDEXES);
    snowFall();
    drawSky();
    drawSun();
    lightInit();
    drawGrass();
    drawHouse();
    drawRoof();
    drawWindow();
    drawDoor();
    drawRoad();
    drawPath();
    drawLamp();
    drawStreetLight();
    glutSwapBuffers();
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH); 
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1920, 1080);
    if (!glutCreateWindow("CW3 - 3D Modelling Project"))
        exit(0);
    myinit();
    creatsnow();
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(when_in_mainloop);
    glutTimerFunc(time_interval, OnTimer, 1);
    glutKeyboardFunc(keyboard_input);
    glutMouseFunc(mouse_input);
    glutMotionFunc(motion_input);
    glutMainLoop();
}
