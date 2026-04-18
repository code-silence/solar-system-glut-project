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
