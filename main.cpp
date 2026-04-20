// ================= ARNOB's PART =================
#include <GL/glut.h>
#include <math.h>

// Defining PI for circle calculations
#define PI 3.14159265f
// For drawing planets
void drawCircle(float cx, float cy, float r, int seg)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < seg; i++)
    {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}
// For drawing orbits of the planets
void drawCircleOutline(float cx, float cy, float r, int seg)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < seg; i++)
    {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}
// For drawing planet names
void drawStr10(float x, float y, const char *s)
{
    glRasterPos2f(x, y);
    for (; *s; s++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *s);
}
// For drawing planet names
void drawStr12(float x, float y, const char *s)
{
    glRasterPos2f(x, y);
    for (; *s; s++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s);
}

// For drawing planet names
void drawStr18(float x, float y, const char *s)
{
    glRasterPos2f(x, y);
    for (; *s; s++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *s);
}

// drawing sun at the center of the solar system
void drawSun()
{
    glColor3f(1.0f, 0.85f, 0.0f);
    drawCircle(0, 0, 26, 80);
    glColor3f(1, 1, 1);
    drawStr10(-8, -34, "Sun");
}
// drawing orbits of the planets
void drawOrbits()
{
    glColor3f(0.22f, 0.22f, 0.22f);
    float r[] = {60, 95, 135, 178, 240, 305};
    for (int i = 0; i < 6; i++)
        drawCircleOutline(0, 0, r[i], 150);
}
// ================= ARNOB's PART END =================



// ================= PIASH'S PART START =================
#include <math.h>

#define PI 3.14159265f
#define D2R (PI/180.0f)

typedef struct {
    const char *name;
    float orbitR, orbitSpd, selfSpd;
    float radius;
    float r,g,b;
    float angle, selfAngle;
    const char *diameter;
    const char *distSun;
    const char *orbPeriod;
    const char *funFact;
} Planet;

Planet planets[6] = {
{"Mercury",60,4.1,2,5,0.7,0.7,0.7,0,0,"4,879 km","57.9M km","88 days","Closest"},
{"Venus",95,1.6,1.2,8,0.91,0.79,0.46,50,0,"12,104 km","108M km","225 days","Hottest"},
{"Earth",135,1.0,1.5,9,0.27,0.55,0.90,90,0,"12,742 km","150M km","365 days","Home"},
{"Mars",178,0.53,1.3,7,0.78,0.28,0.07,130,0,"6,779 km","228M km","687 days","Red"},
{"Jupiter",240,0.08,1.8,20,0.80,0.58,0.25,170,0,"139,820 km","778M km","12 years","Biggest"},
{"Saturn",305,0.03,1.0,16,0.90,0.83,0.55,210,0,"116,460 km","1.43B km","29 years","Rings"}
};

float moonAngle = 0;

void drawPlanet(int i) {
    Planet *p = &planets[i];
    float x = p->orbitR*cosf(p->angle*D2R);
    float y = p->orbitR*sinf(p->angle*D2R);

    glColor3f(p->r,p->g,p->b);
    drawCircle(x,y,p->radius,60);

    glColor3f(1,1,1);
    drawStr10(x+10,y+5,p->name);
}

void drawMoon() {
    float ex = planets[2].orbitR*cosf(planets[2].angle*D2R);
    float ey = planets[2].orbitR*sinf(planets[2].angle*D2R);

    float mx = ex + 20*cosf(moonAngle*D2R);
    float my = ey + 20*sinf(moonAngle*D2R);

    glColor3f(0.7,0.7,0.7);
    drawCircle(mx,my,3,20);
}
// ================= PIASH'S PART END =================



// ================= REDOWAN'S PART START =================









// ================= REDOWAN'S PART END =================



// ================= ALAWOL'S PART START =================

#include <GL/glut.h>

int winW=900, winH=650;
int paused=0;
int selectedPlanet=-1;

float zoom=1, camX=0, camY=0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawOrbits();
    drawSun();

    for(int i=0;i<6;i++) drawPlanet(i);
    drawMoon();

    drawHUD();
    drawInfoPanel();

    glutSwapBuffers();
}

void update(int v) {
    if(!paused) {
        for(int i=0;i<6;i++){
            planets[i].angle += planets[i].orbitSpd;
            planets[i].selfAngle += planets[i].selfSpd;
        }
        moonAngle += 10;
    }
    glutPostRedisplay();
    glutTimerFunc(16,update,0);
}

void keyboard(unsigned char key,int x,int y){
    if(key==27) exit(0);
    if(key==' ') paused=!paused;
}

void mouse(int b,int s,int x,int y){
    if(b==GLUT_LEFT_BUTTON && s==GLUT_DOWN){
        selectedPlanet = (selectedPlanet+1)%6;
    }
}

void reshape(int w,int h){
    winW=w; winH=h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-400,400,-300,300);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(winW,winH);
    glutCreateWindow("Solar System");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutReshapeFunc(reshape);
    glutTimerFunc(16,update,0);

    glutMainLoop();
}
// ================= ALAWOL'S PART END =================
