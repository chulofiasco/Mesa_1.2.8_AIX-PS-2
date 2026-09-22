/*
 * Copyright (c) 1991, 1992, 1993 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF
 * ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#define _HPUX_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "gltk.h"

#include <sys/types.h>  /* required for clock_t and types used in struct tms */
#include <sys/times.h>
#include <sys/param.h>

#ifndef HZ
#define HZ 100    /* AIX 1.3 PS/2 default tick rate */
#endif

/* Number of ticks to benchmark each primitive type.
 * At HZ=100, BENCH_TICKS=20 means 200ms per test. */
#define BENCH_TICKS 20


#define GAP 10
#define ROWS 1
#define COLS 4


GLenum rgb, doubleBuffer, directRender, windType;
GLint windW, windH;

GLint boxW, boxH;

GLenum antialiasing = GL_FALSE;
GLenum depthTesting = GL_FALSE;
GLenum fogging = GL_FALSE, niceFogging = GL_FALSE;
GLenum lighting = GL_FALSE;
GLenum shading = GL_FALSE;
GLenum texturing = GL_FALSE;

GLint loopCount = 100;  /* primitives per inner batch */

GLubyte texture[4*3] = {
    0xFF, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0xFF, 0,
};


static void SetWindSize(int width, int height)
{

    windW = (GLint)width;
    windH = (GLint)height;
}

static GLenum Key(int key, GLenum mask)
{

    switch (key) {
      case TK_ESCAPE:
	tkQuit();
      case TK_a:
	antialiasing = !antialiasing;
	break;
      case TK_d:
	depthTesting = !depthTesting;
	break;
      case TK_f:
	fogging = !fogging;
	break;
      case TK_F:
	niceFogging = !niceFogging;
	break;
      case TK_s:
	shading = !shading;
	break;
      case TK_t:
	texturing = !texturing;
	break;
      default:
	return GL_FALSE;
    }
    return GL_TRUE;
}

static void Viewport(GLint row, GLint column)
{
    GLint x, y;

    boxW = (windW - (COLS + 1) * GAP) / COLS;
    boxH = (windH - (ROWS + 1) * GAP) / ROWS;

    x = GAP + column * (boxW + GAP);
    y = GAP + row * (boxH + GAP);

    glViewport(x, y, boxW, boxH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D((GLdouble)(-boxW/2), (GLdouble)(boxW/2),
               (GLdouble)(-boxH/2), (GLdouble)(boxH/2));
    glMatrixMode(GL_MODELVIEW);

   /* glEnable(GL_SCISSOR_TEST);*/
    glScissor(x, y, boxW, boxH);
}

/* Return current elapsed ticks from times() */
static clock_t cur_ticks(void)
{
    struct tms tm;
    return times(&tm);
}

/* Spin until a tick boundary, return the new tick count.
 * This ensures our benchmark starts right at a clean tick edge. */
static clock_t wait_for_tick(void)
{
    clock_t t0 = cur_ticks();
    clock_t t;
    while ((t = cur_ticks()) == t0)
	;
    return t;
}

static void Report(const char *msg, long count, clock_t ticks)
{
    double elapsed = (double)ticks / (double)HZ;
    if (ticks <= 0) {
	printf("%s: %ld drawn (ticks=0, HZ may be wrong)\n", msg, count);
    } else {
	printf("%s: %ld drawn in %ld ticks (%.2fs) = %.0f/sec\n",
	       msg, count, (long)ticks, elapsed,
	       (double)count / elapsed);
    }
}

static void Warmup(void)
{
    float v1[3], v2[3], v3[3];
    int j;

    v1[0] = 10; v1[1] = 10; v1[2] = 10;
    v2[0] = 20; v2[1] = 20; v2[2] = 10;
    v3[0] = 10; v3[1] = 20; v3[2] = 10;

    glBegin(GL_POINTS);
    for (j = 0; j < loopCount; j++) glVertex2fv(v1);
    glEnd();

    glBegin(GL_LINES);
    for (j = 0; j < loopCount; j++) { glVertex2fv(v1); glVertex2fv(v2); }
    glEnd();

    glBegin(GL_TRIANGLES);
    for (j = 0; j < loopCount; j++) {
        glVertex2fv(v1); glVertex2fv(v2); glVertex2fv(v3);
    }
    glEnd();

    glFinish();
}

static void Points(void)
{
    float v1[3];
    long count = 0;
    clock_t start, now;
    int j;

    v1[0] = 10; v1[1] = 10; v1[2] = 10;

    start = wait_for_tick();
    do {
	glBegin(GL_POINTS);
	for (j = 0; j < loopCount; j++) {
	    glVertex2fv(v1);
	}
	glEnd();
	count += loopCount;
	now = cur_ticks();
    } while (now - start < BENCH_TICKS);
    glFinish();
    Report("Points", count, now - start);
}

static void Lines(void)
{
    float v1[3], v2[3];
    long count = 0;
    clock_t start, now;
    int j;

    v1[0] = 10; v1[1] = 10; v1[2] = 10;
    v2[0] = 20; v2[1] = 20; v2[2] = 10;

    start = wait_for_tick();
    do {
	glBegin(GL_LINES);
	for (j = 0; j < loopCount; j++) {
	    glVertex2fv(v1);
	    glVertex2fv(v2);
	}
	glEnd();
	count += loopCount;
	now = cur_ticks();
    } while (now - start < BENCH_TICKS);
    glFinish();
    Report("Lines", count, now - start);
}

static void Triangles(void)
{
    float v1[3], v2[3], v3[3], t1[2], t2[2], t3[2];
    long count = 0;
    clock_t start, now;
    int j;

    v1[0] = 10; v1[1] = 10; v1[2] = 10;
    v2[0] = 20; v2[1] = 20; v2[2] = 10;
    v3[0] = 10; v3[1] = 20; v3[2] = 10;
    t1[0] = 0;  t1[1] = 0;
    t2[0] = 1;  t2[1] = 1;
    t3[0] = 0;  t3[1] = 1;

    start = wait_for_tick();
    do {
	glBegin(GL_TRIANGLES);
	for (j = 0; j < loopCount; j++) {
	    if (texturing) { glTexCoord2fv(t1); }
	    glVertex2fv(v1);
	    if (texturing) { glTexCoord2fv(t2); }
	    glVertex2fv(v2);
	    if (texturing) { glTexCoord2fv(t3); }
	    glVertex2fv(v3);
	}
	glEnd();
	count += loopCount;
	now = cur_ticks();
    } while (now - start < BENCH_TICKS);
    glFinish();
    Report("Triangles", count, now - start);
}

static void Rects(void)
{
    float v1[2], v2[2];
    long count = 0;
    clock_t start, now;
    int j;

    v1[0] = 10; v1[1] = 10;
    v2[0] = 20; v2[1] = 20;

    start = wait_for_tick();
    do {
	for (j = 0; j < loopCount; j++) {
	    glRectfv(v1, v2);
	}
	count += loopCount;
	now = cur_ticks();
    } while (now - start < BENCH_TICKS);
    glFinish();
    Report("Rects", count, now - start);
}

static void Draw(void)
{

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    TK_SETCOLOR(windType, TK_YELLOW);

    if (antialiasing) {
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	glEnable(GL_BLEND);

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	printf("antialiasing: on\n");
    }
    else {
       glDisable(GL_BLEND);
       glDisable(GL_POINT_SMOOTH);
       glDisable(GL_LINE_SMOOTH);
       glDisable(GL_POLYGON_SMOOTH);
       printf("antialiasing: off\n");
    }
    if (depthTesting) {
	glEnable(GL_DEPTH_TEST);
	printf("depthtest: on\n");
    }
    else {
       glDisable(GL_DEPTH_TEST);
       printf("depthtest: off\n");
    }
    if (fogging) {
	glEnable(GL_FOG);
	glHint(GL_FOG_HINT, (niceFogging) ? GL_NICEST : GL_FASTEST);
	printf("fog: on\n");
    }
    else {
       glDisable(GL_FOG);
       printf("fog: off\n");
    }
    if (lighting) {
	static GLfloat ambient[4] = {1, 0.5, 0.5, 0};

	glEnable(GL_NORMALIZE);
	glNormal3f(1.0, 1.0, 1.0);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	printf("lighting: on\n");
    }
    else {
       glDisable( GL_LIGHTING );
       printf("lighting: off\n");
     }
    if (shading) {
       glShadeModel(GL_SMOOTH);
       printf("shading: smooth\n");
    }
    else {
       glShadeModel(GL_FLAT);
       printf("shading: flat\n");
    }
    if (texturing) {
	static GLfloat modulate[1] = {GL_DECAL};
	static GLfloat clamp[1] = {GL_CLAMP};
	static GLfloat linear[1] = {GL_LINEAR};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE,
		     (GLvoid *)texture);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, modulate);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear);
	glEnable(GL_TEXTURE_2D);
	printf("texturing: on\n");
    }
    else {
       glDisable( GL_TEXTURE_2D );
       printf("texturing: off\n");
     }

    Warmup();
    Viewport(0, 0); Points();
    Viewport(0, 1); Lines();
    Viewport(0, 2); Triangles();
    Viewport(0, 3); Rects();

    glFlush();

    if (doubleBuffer) {
	tkSwapBuffers();
    }
}

static GLenum Args(int argc, char **argv)
{
    GLint i;

    rgb = GL_TRUE;
    doubleBuffer = GL_FALSE;
    directRender = GL_TRUE;

    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-ci") == 0) {
	    rgb = GL_FALSE;
	} else if (strcmp(argv[i], "-rgb") == 0) {
	    rgb = GL_TRUE;
	} else if (strcmp(argv[i], "-sb") == 0) {
	    doubleBuffer = GL_FALSE;
	} else if (strcmp(argv[i], "-db") == 0) {
	    doubleBuffer = GL_TRUE;
	} else if (strcmp(argv[i], "-dr") == 0) {
	    directRender = GL_TRUE;
	} else if (strcmp(argv[i], "-ir") == 0) {
	    directRender = GL_FALSE;
	} else if (strcmp(argv[i], "-h") == 0) {
	    printf("Usage: speed [options]\n");
	    printf("  -rgb   RGB color mode (default)\n");
	    printf("  -ci    Color index mode\n");
	    printf("  -sb    Single buffer (default)\n");
	    printf("  -db    Double buffer\n");
	    printf("  -dr    Direct render (default)\n");
	    printf("  -ir    Indirect render\n");
	    printf("  -h     Print this help\n");
	    printf("Keys: a=antialiasing  d=depth  f=fog  F=nicefog  s=shading  t=texturing\n");
	    return GL_FALSE;
	} else {
	    printf("%s (Bad option).\n", argv[i]);
	    return GL_FALSE;
	}
    }
    return GL_TRUE;
}

void main(int argc, char **argv)
{

    if (Args(argc, argv) == GL_FALSE) {
	tkQuit();
    }

    windW = 600;
    windH = 300;
    tkInitPosition(0, 0, windW, windH);

    windType = TK_DEPTH;
    windType |= (rgb) ? TK_RGB : TK_INDEX;
    windType |= (doubleBuffer) ? TK_DOUBLE : TK_SINGLE;
    windType |= (directRender) ? TK_DIRECT : TK_INDIRECT;
    tkInitDisplayMode(windType);

    if (tkInitWindow("Speed Test") == GL_FALSE) {
	tkQuit();
    }

    tkExposeFunc(SetWindSize);
    tkReshapeFunc(SetWindSize);
    tkKeyDownFunc(Key);
    tkDisplayFunc(Draw);
    tkExec();
}
