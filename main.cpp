#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI      3.14159265f
#define D2R     (PI / 180.0f)

/* ================================================================
   SOLAR SYSTEM — OpenGL/GLUT Project
   Division of Work:
     Arnob : Global Setup, Planet Data & Drawing Helpers
     Piash : Sun, Orbit Paths & Planet Rendering
     Redwan : Moon, Info Panel & HUD Overlay
     Nelu : Display Loop, Update Timer, Input & Main
   ================================================================ */


/* ================================================================
  Arnob START
   ================================================================ */

/* ─── Window ──────────────────────────────────────── */
int winW = 900, winH = 650;

/* ─── Camera ──────────────────────────────────────── */
float zoom   = 1.0f;
float camX   = 0.0f;
float camY   = 0.0f;

/* ─── State ───────────────────────────────────────── */
int paused        = 0;
int selectedPlanet = -1;   /* -1 = none */
int followTarget  = -1;    /* -1 = free, 0-5 = planet */

/* ─── Planet data ───────────────────────────────────
   Each planet stores orbital mechanics, visual properties,
   and display strings for the info panel.
*/
typedef struct {
    const char *name;
    float orbitR;      /* orbit radius */
    float orbitSpd;    /* orbit speed (deg/frame) */
    float selfSpd;     /* self-rotation speed */
    float radius;      /* planet draw radius */
    float r, g, b;     /* color */
    float angle;       /* current orbit angle */
    float selfAngle;   /* self-rotation angle */
    /* info panel strings */
    const char *diameter;
    const char *distSun;
    const char *orbPeriod;
    const char *funFact;
} Planet;

Planet planets[6] = {
  /*  name       orbitR  oSpd   sSpd  rad   R     G     B    angle selfA  diam         dist        period          fact */
    {"Mercury",   60,   4.10f, 2.0f,  5,  0.70f,0.70f,0.70f,  0,  0, "4,879 km",  "57.9M km", "88 days",   "Closest to Sun"},
    {"Venus",     95,   1.60f, 1.2f,  8,  0.91f,0.79f,0.46f, 50,  0, "12,104 km", "108M km",  "225 days",  "Hottest planet"},
    {"Earth",    135,   1.00f, 1.5f,  9,  0.27f,0.55f,0.90f, 90,  0, "12,742 km", "150M km",  "365 days",  "Our home planet"},
    {"Mars",     178,   0.53f, 1.3f,  7,  0.78f,0.28f,0.07f,130,  0, "6,779 km",  "228M km",  "687 days",  "The Red Planet"},
    {"Jupiter",  240,   0.08f, 1.8f, 20,  0.80f,0.58f,0.25f,170,  0, "139,820 km","778M km",  "12 years",  "Largest planet"},
    {"Saturn",   305,   0.03f, 1.0f, 16,  0.90f,0.83f,0.55f,210,  0, "116,460 km","1.43B km", "29 years",  "Famous for rings"},
};

float moonAngle = 0.0f;  /* Moon orbits Earth */

/* ─── Drawing Helpers ───────────────────────────────
   Reusable low-level OpenGL primitives used throughout
   the project by all team members.
*/

/* Draws a filled circle at (cx, cy) with radius r using 'seg' segments */
void drawCircle(float cx, float cy, float r, int seg) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < seg; i++) {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

/* Draws only the outline of a circle (used for orbit paths) */
void drawCircleOutline(float cx, float cy, float r, int seg) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < seg; i++) {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

/* Bitmap text rendering at different font sizes */
void drawStr10(float x, float y, const char *s) {
    glRasterPos2f(x, y);
    for (; *s; s++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *s);
}

void drawStr12(float x, float y, const char *s) {
    glRasterPos2f(x, y);
    for (; *s; s++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s);
}

void drawStr18(float x, float y, const char *s) {
    glRasterPos2f(x, y);
    for (; *s; s++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *s);
}

/* ================================================================
   Arnob END
   ================================================================ */


/* ================================================================
   Piash START
   ================================================================ */

/* Draws the Sun at the center with a label */
void drawSun() {
    glColor3f(1.0f, 0.85f, 0.0f);
    drawCircle(0, 0, 26, 80);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawStr10(-8, -34, "Sun");
}

/* Draws the circular orbit path for each of the 6 planets */
void drawOrbits() {
    glColor3f(0.22f, 0.22f, 0.22f);
    float radii[] = {60, 95, 135, 178, 240, 305};
    for (int i = 0; i < 6; i++)
        drawCircleOutline(0, 0, radii[i], 150);
}

/* Draws a single planet by index:
   - Computes world position from orbit angle
   - Fills planet body with its color
   - Draws a self-rotation stripe (dark line rotating over surface)
   - Draws Saturn's rings (idx == 5) as an elliptical QUAD_STRIP
   - Draws Jupiter's atmospheric bands (idx == 4)
   - Highlights selected planet with a yellow outline ring
   - Draws planet name label next to the body
*/
void drawPlanet(int idx) {
    Planet *p = &planets[idx];
    float px = p->orbitR * cosf(p->angle * D2R);
    float py = p->orbitR * sinf(p->angle * D2R);

    /* body */
    glColor3f(p->r, p->g, p->b);
    drawCircle(px, py, p->radius, 60);

    /* self-rotation: a dark stripe rotating across the planet */
    float sa  = p->selfAngle * D2R;
    float ax  = cosf(sa) * p->radius * 0.85f;
    float ay  = sinf(sa) * p->radius * 0.85f;
    glColor4f(0.0f, 0.0f, 0.0f, 0.30f);
    glLineWidth(2.2f);
    glBegin(GL_LINES);
        glVertex2f(px - ax, py - ay);
        glVertex2f(px + ax, py + ay);
    glEnd();
    glLineWidth(1.0f);

    /* Saturn ring — drawn in front of planet here, behind is handled in display() */
    if (idx == 5) {
        glColor4f(0.85f, 0.75f, 0.45f, 0.60f);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 180; i++) {
            float a = 2.0f * PI * i / 180;
            float ri = p->radius + 5, ro = p->radius + 15;
            glVertex2f(px + ri*cosf(a), py + ri*sinf(a)*0.35f);
            glVertex2f(px + ro*cosf(a), py + ro*sinf(a)*0.35f);
        }
        glEnd();
    }

    /* Jupiter atmospheric band */
    if (idx == 4) {
        glColor4f(0.55f, 0.35f, 0.10f, 0.45f);
        glBegin(GL_QUADS);
            glVertex2f(px - p->radius, py - 4);
            glVertex2f(px + p->radius, py - 4);
            glVertex2f(px + p->radius, py - 1);
            glVertex2f(px - p->radius, py - 1);
            glVertex2f(px - p->radius, py + 2);
            glVertex2f(px + p->radius, py + 2);
            glVertex2f(px + p->radius, py + 5);
            glVertex2f(px - p->radius, py + 5);
        glEnd();
    }

    /* selection highlight ring */
    if (selectedPlanet == idx) {
        glColor3f(1.0f, 1.0f, 0.2f);
        glLineWidth(1.8f);
        drawCircleOutline(px, py, p->radius + 5, 60);
        glLineWidth(1.0f);
    }

    /* name label */
    glColor3f(0.88f, 0.88f, 0.88f);
    drawStr10(px + p->radius + 2, py + 2, p->name);
}

/* ================================================================
   piash END
   ================================================================ */


/* ================================================================
   Redwan START
  
   ================================================================ */

/* Draws Earth's Moon orbiting at a fixed radius around Earth's position */
void drawMoon() {
    float ex = planets[2].orbitR * cosf(planets[2].angle * D2R);
    float ey = planets[2].orbitR * sinf(planets[2].angle * D2R);
    float mx = ex + 20.0f * cosf(moonAngle * D2R);
    float my = ey + 20.0f * sinf(moonAngle * D2R);
    glColor3f(0.75f, 0.75f, 0.75f);
    drawCircle(mx, my, 3, 20);
    glColor3f(0.6f, 0.6f, 0.6f);
    drawStr10(mx + 4, my + 1, "Moon");
}

/* Draws a semi-transparent info panel in the top-right corner
   showing selected planet's diameter, distance, orbit period, and fun fact.
   Uses screen-space (pixel) coordinates via a temporary projection matrix.
*/
void drawInfoPanel() {
    if (selectedPlanet < 0) return;
    Planet *p = &planets[selectedPlanet];

    /* switch to screen coords */
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();

    float pw = 220, ph = 152;
    float bx = winW - pw - 14, by = 14;

    /* dark background panel */
    glColor4f(0.04f, 0.04f, 0.14f, 0.90f);
    glBegin(GL_QUADS);
        glVertex2f(bx,      by);
        glVertex2f(bx + pw, by);
        glVertex2f(bx + pw, by + ph);
        glVertex2f(bx,      by + ph);
    glEnd();

    /* blue border outline */
    glColor3f(0.35f, 0.65f, 1.0f);
    glLineWidth(1.2f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(bx,      by);
        glVertex2f(bx + pw, by);
        glVertex2f(bx + pw, by + ph);
        glVertex2f(bx,      by + ph);
    glEnd();
    glLineWidth(1.0f);

    /* planet name (tinted with its own color) */
    glColor3f(p->r + 0.15f, p->g + 0.15f, p->b + 0.15f);
    drawStr18(bx + 10, by + ph - 28, p->name);

    /* horizontal divider line */
    glColor3f(0.25f, 0.45f, 0.75f);
    glBegin(GL_LINES);
        glVertex2f(bx + 8,      by + ph - 35);
        glVertex2f(bx + pw - 8, by + ph - 35);
    glEnd();

    /* data rows: label (blue) + value (white) */
    float ly = by + ph - 55, ls = 22;

    glColor3f(0.55f, 0.80f, 1.0f); drawStr12(bx + 10, ly,      "Diameter :");
    glColor3f(1.00f, 1.00f, 1.0f); drawStr12(bx + 108, ly,     p->diameter);

    ly -= ls;
    glColor3f(0.55f, 0.80f, 1.0f); drawStr12(bx + 10, ly,      "Distance :");
    glColor3f(1.00f, 1.00f, 1.0f); drawStr12(bx + 108, ly,     p->distSun);

    ly -= ls;
    glColor3f(0.55f, 0.80f, 1.0f); drawStr12(bx + 10, ly,      "Orbit    :");
    glColor3f(1.00f, 1.00f, 1.0f); drawStr12(bx + 108, ly,     p->orbPeriod);

    ly -= ls;
    glColor3f(1.00f, 0.88f, 0.35f); drawStr10(bx + 10, ly,     p->funFact);

    /* restore previous projection */
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

/* Draws the HUD: top info bar showing controls, zoom level, and follow target.
   Also switches to screen-space coordinates temporarily.
*/
void drawHUD() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();

    /* semi-transparent top bar background */
    glColor4f(0.0f, 0.0f, 0.10f, 0.70f);
    glBegin(GL_QUADS);
        glVertex2f(0,    winH - 26);
        glVertex2f(winW, winH - 26);
        glVertex2f(winW, winH);
        glVertex2f(0,    winH);
    glEnd();

    /* control hints — show PAUSED in orange when simulation is paused */
    glColor3f(0.55f, 0.85f, 1.0f);
    if (paused) {
        glColor3f(1.0f, 0.45f, 0.25f);
        drawStr12(8, winH - 17, "PAUSED");
        glColor3f(0.55f, 0.85f, 1.0f);
        drawStr12(72, winH - 17,
            "SPACE=resume  F=follow planet  R=reset  Click planet=info  Scroll=zoom  ESC=quit");
    } else {
        drawStr12(8, winH - 17,
            "SPACE=pause  F=follow planet  R=reset  Click planet=info  Scroll=zoom  ESC=quit");
    }

    /* zoom level badge (bottom-right) */
    char buf[32];
    snprintf(buf, sizeof(buf), "Zoom: %.1fx", zoom);
    glColor3f(0.70f, 0.70f, 0.90f);
    drawStr10(winW - 80, 8, buf);

    /* follow indicator (bottom-left) */
    if (followTarget >= 0) {
        glColor3f(1.0f, 0.92f, 0.35f);
        char fbuf[64];
        snprintf(fbuf, sizeof(fbuf), "Following: %s", planets[followTarget].name);
        drawStr12(8, 8, fbuf);
    }

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

/* ================================================================
   Redwan END
   ================================================================ */


/* ================================================================
   Alawol START
   ================================================================ */
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glScalef(zoom, zoom, 1.0f);
    glTranslatef(-camX, -camY, 0.0f);

    drawOrbits();
    drawSun();

    /* Saturn ring drawn BEHIND the planet body first */
    {
        Planet *p = &planets[5];
        float px = p->orbitR * cosf(p->angle * D2R);
        float py = p->orbitR * sinf(p->angle * D2R);
        glColor4f(0.85f, 0.75f, 0.45f, 0.60f);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 180; i++) {
            float a = 2.0f * PI * i / 180;
            float ri = p->radius + 5, ro = p->radius + 15;
            glVertex2f(px + ri*cosf(a), py + ri*sinf(a)*0.35f);
            glVertex2f(px + ro*cosf(a), py + ro*sinf(a)*0.35f);
        }
        glEnd();
    }

    for (int i = 0; i < 6; i++) drawPlanet(i);
    drawMoon();

    /* screen-space overlays on top of everything */
    drawHUD();
    drawInfoPanel();

    glutSwapBuffers();
}

/* Timer callback (~60 fps at 16ms interval).
   Advances each planet's orbit angle and self-rotation angle.
   Also advances moonAngle and updates camera if following a planet.
*/
void update(int v) {
    if (!paused) {
        for (int i = 0; i < 6; i++) {
            planets[i].angle     += planets[i].orbitSpd;
            planets[i].selfAngle += planets[i].selfSpd * 3.5f;
            if (planets[i].angle     >= 360.0f) planets[i].angle     -= 360.0f;
            if (planets[i].selfAngle >= 360.0f) planets[i].selfAngle -= 360.0f;
        }
        moonAngle += 13.5f;
        if (moonAngle >= 360.0f) moonAngle -= 360.0f;

        /* camera follows selected planet if follow mode active */
        if (followTarget >= 0) {
            Planet *fp = &planets[followTarget];
            camX = fp->orbitR * cosf(fp->angle * D2R);
            camY = fp->orbitR * sinf(fp->angle * D2R);
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

/* Mouse callback — handles scroll wheel zoom and left-click planet selection.
   Converts screen pixel coordinates to world coordinates for hit testing.
*/
void mouseFunc(int button, int state, int mx, int my) {
    /* scroll up = zoom in */
    if (button == 3 && state == GLUT_DOWN) {
        zoom *= 1.12f;
        if (zoom > 6.0f) zoom = 6.0f;
        return;
    }
    /* scroll down = zoom out */
    if (button == 4 && state == GLUT_DOWN) {
        zoom /= 1.12f;
        if (zoom < 0.25f) zoom = 0.25f;
        return;
    }

    /* left click: convert to world coords and test against each planet */
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float asp = (float)winW / (float)winH;
        float wx  = ((float)mx / winW - 0.5f) * 800.0f * asp / zoom + camX;
        float wy  = (0.5f - (float)my / winH) * 600.0f       / zoom + camY;
        selectedPlanet = -1;
        for (int i = 0; i < 6; i++) {
            float px = planets[i].orbitR * cosf(planets[i].angle * D2R);
            float py = planets[i].orbitR * sinf(planets[i].angle * D2R);
            float dx = wx - px, dy = wy - py;
            float hr = planets[i].radius + 12.0f;  /* hit radius slightly larger than visual */
            if (dx*dx + dy*dy <= hr*hr) { selectedPlanet = i; return; }
        }
    }
}

/* Keyboard callback:
   ESC  : exit follow mode, or quit if not following
   SPACE: toggle pause
   F/f  : cycle through planets in follow mode at zoom 3.5x
   R/r  : reset zoom, camera, and follow state
*/
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:  /* ESC */
        if (followTarget >= 0) { followTarget = -1; camX = 0; camY = 0; }
        else exit(0);
        break;
    case ' ':
        paused = !paused;
        break;
    case 'f': case 'F':
        followTarget = (followTarget + 1) % 6;
        zoom = 3.5f;
        break;
    case 'r': case 'R':
        zoom = 1.0f; camX = 0; camY = 0; followTarget = -1;
        break;
    }
}

/* Reshape callback — called whenever the window is resized.
   Updates viewport and recalculates the orthographic projection
   to maintain aspect ratio.
*/
void reshape(int w, int h) {
    winW = w; winH = (h > 0 ? h : 1);
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    float asp = (float)winW / winH;
    gluOrtho2D(-400*asp, 400*asp, -300, 300);
    glMatrixMode(GL_MODELVIEW);
}

/* Entry point — initializes GLUT window, OpenGL blend/smooth settings,
   registers all callbacks, prints controls to console, and starts loop.
*/
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(100, 60);
    glutCreateWindow("2D Solar System — OpenGL/GLUT");

    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseFunc);
    glutTimerFunc(16, update, 0);

    printf("Controls: SPACE=pause  F=follow  R=reset  Click=info  Scroll=zoom  ESC=quit\n");
    glutMainLoop();
    return 0;
}

/* ================================================================
   Alawol END
   ================================================================ */
