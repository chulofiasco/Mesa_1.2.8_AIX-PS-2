/*
 * mesatest.c  --  Mesa 1.2.8 Comprehensive Library Test
 *
 * Exercises all four Mesa libraries:
 *
 *   libMesaGL.a   matrices, lighting, fog, blending, depth, stencil,
 *                 textures, display lists, evaluators, glReadPixels
 *   libMesaGLU.a  gluPerspective, gluLookAt, quadrics (sphere/cylinder/disk),
 *                 tessellator (concave polygon)
 *   libMesatk.a   tkInitWindow, tkExec, tkGetSystem (window + event loop)
 *   libMesaaux.a  auxSolidTeapot, auxSolidSphere, auxSolidTorus,
 *                 auxSolidCone, auxSolidCylinder
 *
 * X11:  XServerVendor / screen-size query at startup;
 *       XStoreName per scene via tkGetSystem(TK_X_DISPLAY).
 *
 * Build from Mesa-1.2.8 root:
 *   cc -I./include mesatest.c -L./lib \
 *      -lMesaaux -lMesatk -lMesaGLU -lMesaGL -lm -lXext -lX11 -o mesatest
 *
 * Usage:
 *   mesatest        -- console mode: off-screen GLX pixmap, PASS/FAIL per scene
 *   mesatest -x11   -- interactive X11 window, cycle scenes with keys
 *
 * Controls (X11 mode):  SPACE / N = next scene    P = previous    ESC / Q = quit
 *
 * C89 (ANSI C) -- no C99 features; all declarations at top of block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "GL/osmesa.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "glaux.h"
#include "gltk.h"

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */

#define WIN_X      100
#define WIN_Y      100
#define WIN_W      640
#define WIN_H      480
#define N_SCENES   8
#define TEX_SIZE   64

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Forward declarations (C89: required before use)                   */
/* ------------------------------------------------------------------ */

static void draw(void);
static void update_title(void);

/* ------------------------------------------------------------------ */
/*  Globals                                                            */
/* ------------------------------------------------------------------ */

static int                  g_scene  = 0;
static float                g_angle  = 0.0f;
static GLuint               g_listid = 0;
static int                  g_width  = WIN_W;
static int                  g_height = WIN_H;
static Display             *g_dpy    = NULL;
static Window               g_xwin   = 0;
static GLUtriangulatorObj  *g_tobj   = NULL;

/* Title strings indexed by scene number */
static const char *g_titles[N_SCENES] = {
    "Mesa Test 1/8: GL Matrices & glReadPixels",
    "Mesa Test 2/8: Lighting (GLU Quadric Sphere)",
    "Mesa Test 3/8: Fog (aux Teapot)",
    "Mesa Test 4/8: Blending",
    "Mesa Test 5/8: Texture Mapping",
    "Mesa Test 6/8: Stencil Buffer (GLU Disk mask)",
    "Mesa Test 7/8: Display List (aux Torus)",
    "Mesa Test 8/8: GLU Tessellator (concave L-shape)"
};

/*
 * L-shaped concave polygon for GLU tessellator test.
 * Six vertices forming an L: bottom-left origin, goes up, right,
 * in, right, down.
 */
static GLdouble g_lshape[6][3] = {
    {0.0, 0.0, 0.0},
    {0.0, 2.0, 0.0},
    {1.0, 2.0, 0.0},
    {1.0, 1.0, 0.0},
    {2.0, 1.0, 0.0},
    {2.0, 0.0, 0.0}
};

/* ------------------------------------------------------------------ */
/*  X11 utility: print display info before GL context creation        */
/* ------------------------------------------------------------------ */

static void print_x11_info(void)
{
    Display *dpy;
    int      screen;

    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        printf("[X11] Cannot open display -- skipping X11 info\n");
        return;
    }
    screen = DefaultScreen(dpy);
    printf("[X11] Server vendor    : %s\n",    XServerVendor(dpy));
    printf("[X11] Vendor release   : %d\n",    XVendorRelease(dpy));
    printf("[X11] Protocol version : %d.%d\n", XProtocolVersion(dpy),
                                                XProtocolRevision(dpy));
    printf("[X11] Screens          : %d\n",    XScreenCount(dpy));
    printf("[X11] Screen %d size   : %d x %d pixels\n",
           screen,
           XDisplayWidth(dpy, screen),
           XDisplayHeight(dpy, screen));
    XCloseDisplay(dpy);
}

/* ------------------------------------------------------------------ */
/*  X11 utility: update window title bar for current scene            */
/* ------------------------------------------------------------------ */

static void update_title(void)
{
    if (g_dpy != NULL && g_xwin != 0) {
        XStoreName(g_dpy, g_xwin, g_titles[g_scene]);
        XSync(g_dpy, 0);
    }
}

/* ------------------------------------------------------------------ */
/*  Scene 1: GL core -- matrices, triangle fan, glReadPixels          */
/* ------------------------------------------------------------------ */

static void draw_scene0(void)
{
    GLubyte pixel[3];
    int     i;
    float   angle;
    float   colors[5][3];

    colors[0][0] = 1.0f; colors[0][1] = 0.0f; colors[0][2] = 0.0f;
    colors[1][0] = 1.0f; colors[1][1] = 1.0f; colors[1][2] = 0.0f;
    colors[2][0] = 0.0f; colors[2][1] = 1.0f; colors[2][2] = 0.0f;
    colors[3][0] = 0.0f; colors[3][1] = 0.0f; colors[3][2] = 1.0f;
    colors[4][0] = 1.0f; colors[4][1] = 0.0f; colors[4][2] = 1.0f;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.5, 1.5, -1.5, 1.5, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(g_angle, 0.0f, 0.0f, 1.0f);

    /* Triangle fan: tests glPushMatrix/glPopMatrix and color ops */
    glPushMatrix();
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(0.0f, 0.0f);
    for (i = 0; i <= 5; i++) {
        angle = (float)(i * 72) * (float)(M_PI / 180.0);
        glColor3fv(colors[i % 5]);
        glVertex2f((float)cos((double)angle), (float)sin((double)angle));
    }
    glEnd();
    glPopMatrix();

    /* glReadPixels: verify center pixel on first frame of this scene.
     * g_angle reaches 0.5 on the first frame (incremented before draw). */
    if (g_angle < 1.0f) {
        glReadPixels(g_width / 2, g_height / 2, 1, 1,
                     GL_RGB, GL_UNSIGNED_BYTE, pixel);
        printf("[Scene 1] glReadPixels center = (%u, %u, %u)  "
               "(non-zero = rendering OK)\n",
               (unsigned)pixel[0],
               (unsigned)pixel[1],
               (unsigned)pixel[2]);
    }
}

/* ------------------------------------------------------------------ */
/*  Scene 2: Lighting + GLU quadric sphere                            */
/* ------------------------------------------------------------------ */

static void draw_scene1(void)
{
    static GLfloat light_pos[4] = {1.0f, 2.0f, 3.0f, 0.0f};
    static GLfloat ambient[4]   = {0.1f, 0.1f, 0.1f, 1.0f};
    static GLfloat diffuse[4]   = {0.9f, 0.7f, 0.3f, 1.0f};
    static GLfloat specular[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat shininess[1] = {80.0f};
    GLUquadricObj *q;

    glClearColor(0.0f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_STENCIL_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)g_width / (double)g_height, 0.5, 50.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);
    glRotatef(g_angle, 0.3f, 1.0f, 0.2f);

    /* Material and light -- tests GL_LIGHTING path */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,            specular);
    glMaterialfv(GL_FRONT, GL_SHININESS,           shininess);

    /* GLU smooth sphere -- exercises libMesaGLU quadric renderer */
    q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    gluQuadricNormals(q, GLU_SMOOTH);
    gluSphere(q, 1.5, 32, 24);
    gluDeleteQuadric(q);

    glDisable(GL_LIGHTING);
}

/* ------------------------------------------------------------------ */
/*  Scene 3: Fog + aux teapot                                         */
/* ------------------------------------------------------------------ */

static void draw_scene2(void)
{
    static GLfloat fog_color[4]  = {0.55f, 0.55f, 0.55f, 1.0f};
    static GLfloat mat_diff[4]   = {0.8f, 0.4f, 0.2f, 1.0f};
    static GLfloat mat_spec[4]   = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat shininess[1]  = {60.0f};
    static GLfloat light_pos[4]  = {2.0f, 3.0f, 4.0f, 0.0f};

    glClearColor(0.55f, 0.55f, 0.55f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_STENCIL_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)g_width / (double)g_height, 0.1, 20.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 1.5, 7.0,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);
    glRotatef(g_angle, 0.0f, 1.0f, 0.0f);

    /* Fog: GL_LINEAR from 4 to 14 */
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START,  4.0f);
    glFogf(GL_FOG_END,   14.0f);
    glFogfv(GL_FOG_COLOR, fog_color);

    /* Lighting */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR,            mat_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS,           shininess);

    /* aux teapot -- exercises libMesaaux */
    auxSolidTeapot(1.5);

    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
}

/* ------------------------------------------------------------------ */
/*  Scene 4: Blending                                                  */
/* ------------------------------------------------------------------ */

static void draw_scene3(void)
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Opaque blue background */
    glDisable(GL_BLEND);
    glColor4f(0.1f, 0.2f, 0.8f, 1.0f);
    glRectf(0.05f, 0.05f, 0.95f, 0.95f);

    /* Three overlapping translucent quads -- tests GL_BLEND */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.0f, 0.0f, 0.55f);
    glRectf(0.05f, 0.30f, 0.65f, 0.85f);

    glColor4f(0.0f, 1.0f, 0.0f, 0.55f);
    glRectf(0.35f, 0.10f, 0.92f, 0.72f);

    glColor4f(1.0f, 1.0f, 1.0f, 0.30f);
    glRectf(0.38f, 0.38f, 0.62f, 0.62f);

    glDisable(GL_BLEND);
}

/* ------------------------------------------------------------------ */
/*  Scene 5: Texture mapping (procedural checkerboard, OGL 1.0 style) */
/* ------------------------------------------------------------------ */

static void init_texture(void)
{
    /*
     * Static storage for the checkerboard texture image.
     * Declared static so it survives past the function call --
     * glTexImage2D reads from this buffer.
     */
    static GLubyte texdata[TEX_SIZE][TEX_SIZE][3];
    int i, j, c;

    for (i = 0; i < TEX_SIZE; i++) {
        for (j = 0; j < TEX_SIZE; j++) {
            c = (((i / 8) + (j / 8)) & 1) ? 255 : 30;
            texdata[i][j][0] = (GLubyte)c;
            texdata[i][j][1] = (GLubyte)(c > 100 ? 180 : 0);
            texdata[i][j][2] = (GLubyte)0;
        }
    }

    /* OpenGL 1.0 style: no texture objects, just bind the image */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
                 TEX_SIZE, TEX_SIZE, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &texdata[0][0][0]);
}

static void draw_scene4(void)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)g_width / (double)g_height, 0.1, 50.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 2.5, 5.0,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);
    glRotatef(g_angle * 0.5f, 0.0f, 1.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor3f(1.0f, 1.0f, 1.0f);

    /* Textured quad -- tests GL_TEXTURE_2D pipeline */
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -2.0f, 0.0f);
        glTexCoord2f(4.0f, 0.0f); glVertex3f( 2.0f, -2.0f, 0.0f);
        glTexCoord2f(4.0f, 4.0f); glVertex3f( 2.0f,  2.0f, 0.0f);
        glTexCoord2f(0.0f, 4.0f); glVertex3f(-2.0f,  2.0f, 0.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

/* ------------------------------------------------------------------ */
/*  Scene 6: Stencil buffer (GLU disk writes mask, stripes fill it)   */
/* ------------------------------------------------------------------ */

static void draw_scene5(void)
{
    GLUquadricObj *q;
    int            s;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -2.0, 2.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(g_angle * 0.2f, 0.0f, 0.0f, 1.0f);

    /* Pass 1: write stencil mask AND a dim base colour into the disk.
     * Keeping colour writes active means the disk area is non-black even
     * when the platform has no stencil buffer (e.g. OSMesa 1.2.8).
     * When stencil IS available, Pass 2 paints bright stripes on top. */
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glColor3f(0.15f, 0.15f, 0.15f);    /* dim base; non-black fallback */

    q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    gluDisk(q, 0.0, 1.5, 64, 1);
    gluDeleteQuadric(q);

    /* Pass 2: draw diagonal stripes, visible only inside stencil */
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glBegin(GL_QUADS);
    for (s = -10; s < 10; s++) {
        float x  = (float)s * 0.4f;
        float hue = (float)(s + 10) / 20.0f;
        glColor3f(hue * 0.6f + 0.2f, 0.5f, 1.0f - hue * 0.5f);
        glVertex2f(x,         -2.5f);
        glVertex2f(x + 0.28f, -2.5f);
        glVertex2f(x + 2.78f,  2.5f);
        glVertex2f(x + 2.50f,  2.5f);
    }
    glEnd();

    glDisable(GL_STENCIL_TEST);
}

/* ------------------------------------------------------------------ */
/*  Scene 7: Display list containing aux torus + aux sphere           */
/* ------------------------------------------------------------------ */

static void init_display_list(void)
{
    g_listid = glGenLists(1);
    glNewList(g_listid, GL_COMPILE);
        /* aux torus inside a display list -- tests libMesaaux + GL lists */
        auxSolidTorus(0.35, 1.0);
    glEndList();
}

static void draw_scene6(void)
{
    static GLfloat light_pos[4]  = {4.0f, 4.0f, 4.0f, 0.0f};
    static GLfloat torus_col[4]  = {0.2f, 0.7f, 0.9f, 1.0f};
    static GLfloat sphere_col[4] = {0.9f, 0.5f, 0.1f, 1.0f};
    static GLfloat spec[4]       = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat shininess[1]  = {90.0f};

    glClearColor(0.05f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_STENCIL_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, (double)g_width / (double)g_height, 0.5, 30.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 1.5, 7.0,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    /* Torus from display list, spinning on two axes */
    glPushMatrix();
    glRotatef(g_angle,          1.0f, 0.0f, 0.0f);
    glRotatef(g_angle * 0.7f,   0.0f, 1.0f, 0.0f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, torus_col);
    glCallList(g_listid);   /* <<< display list call */
    glPopMatrix();

    /* Small aux sphere orbiting the torus */
    glPushMatrix();
    glRotatef(g_angle * 1.5f, 0.0f, 1.0f, 0.0f);
    glTranslatef(2.0f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sphere_col);
    auxSolidSphere(0.35);
    glPopMatrix();

    glDisable(GL_LIGHTING);
}

/* ------------------------------------------------------------------ */
/*  Scene 8: GLU tessellator -- concave L-shaped polygon              */
/* ------------------------------------------------------------------ */

static void init_tessellator(void)
{
    g_tobj = gluNewTess();
    /*
     * GLenum constants GLU_BEGIN/VERTEX/END are 100100..100102 (>65535).
     * These work correctly because GLenum is typedef'd as unsigned int
     * (32-bit) in this build, not as a C enum type.
     */
    gluTessCallback(g_tobj, GLU_BEGIN,  (void(*)())glBegin);
    gluTessCallback(g_tobj, GLU_VERTEX, (void(*)())glVertex2dv);
    gluTessCallback(g_tobj, GLU_END,    (void(*)())glEnd);
}

static void draw_scene7(void)
{
    static GLfloat palette[6][3] = {
        {1.0f, 0.3f, 0.3f},
        {0.3f, 1.0f, 0.3f},
        {0.3f, 0.3f, 1.0f},
        {1.0f, 1.0f, 0.3f},
        {0.3f, 1.0f, 1.0f},
        {1.0f, 0.3f, 1.0f}
    };
    int   ci;
    int   i;

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* View the L-shape (spans 0..2 x 0..2) centred */
    glOrtho(-0.5, 2.5, -0.5, 2.5, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.0f, 1.0f, 0.0f);
    glRotatef(g_angle * 0.4f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-1.0f, -1.0f, 0.0f);

    ci = ((int)(g_angle / 45.0f)) % 6;
    glColor3fv(palette[ci]);

    /* Tessellate the concave L-shape -- verifies libMesaGLU tessellator */
    gluBeginPolygon(g_tobj);
    for (i = 0; i < 6; i++) {
        gluTessVertex(g_tobj, g_lshape[i], g_lshape[i]);
    }
    gluEndPolygon(g_tobj);
}

/* ------------------------------------------------------------------ */
/*  Render dispatch                                                    */
/* ------------------------------------------------------------------ */

static void draw(void)
{
    /* Advance animation angle */
    g_angle += 0.5f;
    if (g_angle >= 360.0f) {
        g_angle -= 360.0f;
    }

    switch (g_scene) {
        case 0: draw_scene0(); break;
        case 1: draw_scene1(); break;
        case 2: draw_scene2(); break;
        case 3: draw_scene3(); break;
        case 4: draw_scene4(); break;
        case 5: draw_scene5(); break;
        case 6: draw_scene6(); break;
        case 7: draw_scene7(); break;
        default: break;
    }

    tkSwapBuffers();
}

/* ------------------------------------------------------------------ */
/*  Keyboard handler                                                   */
/* ------------------------------------------------------------------ */

static GLenum key(int k, GLenum mask)
{
    switch (k) {
        case TK_ESCAPE:
        case TK_q:
            tkQuit();
            return GL_FALSE;

        case TK_SPACE:
        case TK_n:
            g_scene = (g_scene + 1) % N_SCENES;
            g_angle = 0.0f;
            update_title();
            printf("[Scene %d] %s\n", g_scene + 1, g_titles[g_scene]);
            return GL_TRUE;

        case TK_p:
            g_scene = (g_scene + N_SCENES - 1) % N_SCENES;
            g_angle = 0.0f;
            update_title();
            printf("[Scene %d] %s\n", g_scene + 1, g_titles[g_scene]);
            return GL_TRUE;
    }

    return GL_FALSE;
}

/* ------------------------------------------------------------------ */
/*  Reshape / expose callback                                          */
/* ------------------------------------------------------------------ */

static void reshape(int w, int h)
{
    g_width  = w;
    g_height = (h > 0) ? h : 1;
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

/* ------------------------------------------------------------------ */
/*  One-time GL / library initialisation                               */
/* ------------------------------------------------------------------ */

static void init_gl(void)
{
    const GLubyte *renderer;
    const GLubyte *vendor;
    const GLubyte *version;
    const GLubyte *glu_ver;

    vendor   = glGetString(GL_VENDOR);
    renderer = glGetString(GL_RENDERER);
    version  = glGetString(GL_VERSION);
    glu_ver  = gluGetString(GLU_VERSION);

    printf("[GL]   Vendor    : %s\n", vendor   ? (const char *)vendor   : "?");
    printf("[GL]   Renderer  : %s\n", renderer ? (const char *)renderer : "?");
    printf("[GL]   Version   : %s\n", version  ? (const char *)version  : "?");
    printf("[GLU]  Version   : %s\n", glu_ver  ? (const char *)glu_ver  : "?");
    printf("\n");

    glDepthFunc(GL_LESS);

    init_texture();      /* load checkerboard into texture unit 0 */
    init_display_list(); /* compile torus into display list       */
    init_tessellator();  /* set up GLU tessellator callbacks      */
}

/* ------------------------------------------------------------------ */
/*  Console mode: off-screen OSMesa -- no X11 connection needed       */
/* ------------------------------------------------------------------ */

static int run_console_tests(void)
{
    OSMesaContext  osmesa_ctx;
    GLubyte       *osmesa_buf;
    GLubyte        pixel[3];
    int            s;
    int            fails;
    int            any;

    /* 4 bytes per pixel: OSMesa 1.2.8 only supports GL_RGBA format */
    osmesa_buf = (GLubyte *)malloc((size_t)(WIN_W * WIN_H * 4));
    if (osmesa_buf == NULL) {
        fprintf(stderr, "[console] malloc failed for render buffer\n");
        return 1;
    }

    osmesa_ctx = OSMesaCreateContext(GL_RGBA, NULL);
    if (osmesa_ctx == NULL) {
        fprintf(stderr, "[console] OSMesaCreateContext failed\n");
        free(osmesa_buf);
        return 1;
    }

    if (OSMesaMakeCurrent(osmesa_ctx, osmesa_buf,
                          GL_UNSIGNED_BYTE, WIN_W, WIN_H) == GL_FALSE) {
        fprintf(stderr, "[console] OSMesaMakeCurrent failed\n");
        OSMesaDestroyContext(osmesa_ctx);
        free(osmesa_buf);
        return 1;
    }
    /* OSMesaMakeCurrent sets the viewport; no separate glViewport needed */

    init_gl();  /* queries GL/GLU strings, loads texture, display list, tess */

    printf("%-52s  %s\n", "Scene", "Result");
    printf("%-52s  %s\n",
           "-----------------------------------------------------",
           "------");

    fails = 0;
    for (s = 0; s < N_SCENES; s++) {
        g_scene = s;
        g_angle = 45.0f;  /* enough rotation to have pixels on screen */

        switch (s) {
            case 0: draw_scene0(); break;
            case 1: draw_scene1(); break;
            case 2: draw_scene2(); break;
            case 3: draw_scene3(); break;
            case 4: draw_scene4(); break;
            case 5: draw_scene5(); break;
            case 6: draw_scene6(); break;
            case 7: draw_scene7(); break;
            default: break;
        }

        glFlush();
        pixel[0] = pixel[1] = pixel[2] = 0;
        glReadPixels(WIN_W / 2, WIN_H / 2, 1, 1,
                     GL_RGB, GL_UNSIGNED_BYTE, pixel);
        any = ((int)pixel[0] | (int)pixel[1] | (int)pixel[2]) != 0;
        if (!any) { fails++; }

        printf("  [%d/8] %-44s  %s  (px %u,%u,%u)\n",
               s + 1, g_titles[s],
               any ? "PASS" : "FAIL",
               (unsigned)pixel[0],
               (unsigned)pixel[1],
               (unsigned)pixel[2]);
    }

    printf("\n");
    if (fails == 0) {
        printf("All %d scenes PASSED.\n", N_SCENES);
    } else {
        printf("%d of %d scenes FAILED.\n", fails, N_SCENES);
    }

    OSMesaDestroyContext(osmesa_ctx);
    free(osmesa_buf);

    return (fails == 0) ? 0 : 1;
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    int    i;
    int    use_x11;
    GLenum type;

    use_x11 = 0;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-x11") == 0) {
            use_x11 = 1;
        }
    }

    printf("Mesa 1.2.8 Comprehensive Library Test ~ @chulofiasco, 2026\n");
    printf("===========================================================\n");
    printf("Libraries: libMesaGL  libMesaGLU  libMesatk  libMesaaux\n");
    if (use_x11) {
        printf("Mode     : X11 interactive window\n");
        printf("Controls : SPACE/N = next scene   P = prev   ESC/Q = quit\n");
    } else {
        printf("Mode     : console (off-screen OSMesa, no X11 needed)\n");
        printf("Tip      : run with -x11 for interactive window\n");
    }
    printf("\n");

    if (!use_x11) {
        return run_console_tests();
    }

    /* --- X11 info: open a temporary connection before GL context --- */
    print_x11_info();
    printf("\n");

    /* --- tk window: tests libMesatk.a ----------------------------- */
    tkInitPosition(WIN_X, WIN_Y, WIN_W, WIN_H);
    type = TK_RGB | TK_DOUBLE | TK_DEPTH | TK_STENCIL;
    tkInitDisplayMode(type);

    if (tkInitWindow("Mesa 1.2.8 Library Test") == GL_FALSE) {
        fprintf(stderr, "Error: tkInitWindow failed\n");
        return 1;
    }

    tkGetSystem(TK_X_DISPLAY, (void *)&g_dpy);
    tkGetSystem(TK_X_WINDOW,  (void *)&g_xwin);

    if (g_dpy != NULL) {
        printf("[X11]  GL display : %s\n\n", XDisplayString(g_dpy));
    }

    init_gl();

    printf("[Scene 1] %s\n", g_titles[0]);
    update_title();

    tkExposeFunc(reshape);
    tkReshapeFunc(reshape);
    tkDisplayFunc(draw);
    tkKeyDownFunc(key);
    tkIdleFunc(draw);
    tkExec();  /* does not return */

    return 0;
}
