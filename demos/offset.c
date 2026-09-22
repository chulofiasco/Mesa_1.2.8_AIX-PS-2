/****************************************************************************
Copyright 1995 by Silicon Graphics Incorporated, Mountain View, California.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Silicon Graphics not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

****************************************************************************/

/*
 * Derived from code written by Kurt Akeley, November 1992
 *
 *	Uses PolygonOffset to draw hidden-line images.  PolygonOffset
 *	    shifts the z values of polygons an amount that is
 *	    proportional to their slope in screen z.  This keeps
 *	    the lines, which are drawn without displacement, from
 *	    interacting with their respective polygons, and
 *	    thus eliminates line dropouts.
 *
 *	The left image shows an ordinary antialiased wireframe image.
 *	The center image shows an antialiased hidden-line image without
 *	    PolygonOffset.
 *	The right image shows an antialiased hidden-line image using
 *	    PolygonOffset to reduce artifacts.
 *
 *	Drag with a mouse button pressed to rotate the models.
 *	Press the escape key to exit.
 */

#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef EXIT_FAILURE
#  define EXIT_FAILURE    1
#endif
#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS    0
#endif

#define MAXQUAD 6

typedef float Vertex[3];

typedef Vertex Quad[4];

/* data to define the six faces of a unit cube */
Quad quads[MAXQUAD] = {
    0,0,0, 1,0,0, 1,1,0, 0,1,0,
    0,0,1, 1,0,1, 1,1,1, 0,1,1,
    0,0,0, 1,0,0, 1,0,1, 0,0,1,
    0,1,0, 1,1,0, 1,1,1, 0,1,1,
    0,0,0, 0,0,1, 0,1,1, 0,1,0,
    1,0,0, 1,0,1, 1,1,1, 1,1,0
};

#define WIREFRAME	0
#define HIDDEN_LINE	1

static void error(const char* prog, const char* msg);
static void cubes(int mx, int my, int mode);
static void fill(Quad quad);
static void outline(Quad quad);
static void draw_hidden(Quad quad, int mode, int face_index);
static void process_input(Display *dpy, Window win);
static int query_extension(char* extName);

static int attributeListRGB[] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 1, None };

static int attributeListCI[] = { GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 1, None };

static int dimension = 3;
static int use_rgb = 0;
static unsigned long c_black = 0, c_white = 1;
static unsigned long c_red, c_green, c_blue, c_yellow, c_cyan, c_magenta;

static unsigned long alloc_color(Display *dpy, Colormap cmap, int r, int g, int b) {
    XColor xc;
    xc.red = r * 65535;
    xc.green = g * 65535;
    xc.blue = b * 65535;
    xc.flags = DoRed | DoGreen | DoBlue;
    XAllocColor(dpy, cmap, &xc);
    return xc.pixel;
}

main(int argc, char** argv) {
    Display *dpy;
    XVisualInfo *vi;
    XSetWindowAttributes swa;
    Window win;
    GLXContext cx;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-rgb") == 0) {
            use_rgb = 1;
        }
    }

    dpy = XOpenDisplay(0);
    if (!dpy) error(argv[0], "can't open display");

    vi = glXChooseVisual(dpy, DefaultScreen(dpy), use_rgb ? attributeListRGB : attributeListCI);
    if (!vi) error(argv[0], "no suitable visual");

    cx = glXCreateContext(dpy, vi, 0, GL_TRUE);

    if (vi->visualid == XVisualIDFromVisual(DefaultVisual(dpy, DefaultScreen(dpy)))) {
        swa.colormap = DefaultColormap(dpy, DefaultScreen(dpy));
    } else {
        swa.colormap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
    }

    /* Allocate our colors for Color Index mode */
    c_black   = alloc_color(dpy, swa.colormap, 0, 0, 0);
    c_white   = alloc_color(dpy, swa.colormap, 1, 1, 1);
    c_red     = alloc_color(dpy, swa.colormap, 1, 0, 0);
    c_green   = alloc_color(dpy, swa.colormap, 0, 1, 0);
    c_blue    = alloc_color(dpy, swa.colormap, 0, 0, 1);
    c_yellow  = alloc_color(dpy, swa.colormap, 1, 1, 0);
    c_cyan    = alloc_color(dpy, swa.colormap, 0, 1, 1);
    c_magenta = alloc_color(dpy, swa.colormap, 1, 0, 1);

    swa.border_pixel = 0;
    swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask |
	ButtonPressMask | ButtonMotionMask;
    win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, 900, 300,
			0, vi->depth, InputOutput, vi->visual,
			CWBorderPixel|CWColormap|CWEventMask, &swa);
    XStoreName(dpy, win, "hiddenline");
    XMapWindow(dpy, win);

    glXMakeCurrent(dpy, win, cx);

    if (!query_extension("GL_EXT_polygon_offset"))
        error(argv[0], "polygon_offset extension is not available");

    glMatrixMode(GL_PROJECTION);
    gluPerspective(20.0, 1.0, 0.1, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0.0, 0.0, -15.0);

    glEnable(GL_DEPTH_TEST);
    
    /* 
     * Kept the aggressive 5.0 factor and 0.001 bias here because the resulting 
     * "x-ray" effect (pushing the polygons so far back that interior wireframes bleed through)
     * looks really cool!
     */
    glPolygonOffsetEXT(5.0, 0.001);

    if (!use_rgb) {
        glClearIndex((GLfloat)c_black);
    } else {
        glClearColor(0.0, 0.0, 0.0, 0.0);
    }

    while (1) process_input(dpy, win);
}

static void draw_scene(int mx, int my) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glTranslatef(-1.7, 0.0, 0.0);
    cubes(mx, my, WIREFRAME);
    glPopMatrix();

    glPushMatrix();
    cubes(mx, my, HIDDEN_LINE);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.7, 0.0, 0.0);
    glEnable(GL_POLYGON_OFFSET_EXT);
    cubes(mx, my, HIDDEN_LINE);
    glDisable(GL_POLYGON_OFFSET_EXT);
    glPopMatrix();
}

static void cubes(int mx, int my, int mode) {
    int x, y, z, i;
    glRotatef(mx / 2.0, 0.0, 1.0, 0.0);
    glRotatef(my / 2.0, 1.0, 0.0, 0.0);
    glTranslatef(-0.5, -0.5, -0.5);
    glScalef(1.0/dimension, 1.0/dimension, 1.0/dimension);
    for (z = 0; z < dimension; z++) {
	for (y = 0; y < dimension; y++) {
	    for (x = 0; x < dimension; x++) {
		glPushMatrix();
		glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
		glScalef(0.8, 0.8, 0.8);
		for (i = 0; i < MAXQUAD; i++)
		    draw_hidden(quads[i], mode, i);
		glPopMatrix();
	    }
	}
    }
}

static void fill(Quad quad) {
    glBegin(GL_QUADS);
    glVertex3fv(quad[0]); glVertex3fv(quad[1]);
    glVertex3fv(quad[2]); glVertex3fv(quad[3]);
    glEnd();
}

static void outline(Quad quad) {
    glBegin(GL_LINE_LOOP);
    glVertex3fv(quad[0]); glVertex3fv(quad[1]);
    glVertex3fv(quad[2]); glVertex3fv(quad[3]);
    glEnd();
}

static void draw_hidden(Quad quad, int mode, int face_index) {
    if (use_rgb) glColor3f(1.0, 1.0, 1.0);
    else glIndexi(c_white);
    
    outline(quad);

    if (mode == HIDDEN_LINE) {
        if (use_rgb) {
            switch(face_index) {
                case 0: glColor3f(1.0, 0.0, 0.0); break; /* Red */
                case 1: glColor3f(0.0, 1.0, 0.0); break; /* Green */
                case 2: glColor3f(0.0, 0.0, 1.0); break; /* Blue */
                case 3: glColor3f(1.0, 1.0, 0.0); break; /* Yellow */
                case 4: glColor3f(0.0, 1.0, 1.0); break; /* Cyan */
                case 5: glColor3f(1.0, 0.0, 1.0); break; /* Magenta */
            }
        } else {
            switch(face_index) {
                case 0: glIndexi(c_red); break;
                case 1: glIndexi(c_green); break;
                case 2: glIndexi(c_blue); break;
                case 3: glIndexi(c_yellow); break;
                case 4: glIndexi(c_cyan); break;
                case 5: glIndexi(c_magenta); break;
            }
        }
	fill(quad);
    }
}

static void
process_input(Display *dpy, Window win) {
    XEvent event;
    static int prevx, prevy;
    static int deltax = 90, deltay = 40;

    do {
	char buf[31];
	KeySym keysym;

	XNextEvent(dpy, &event);
	switch(event.type) {
	case Expose:
	    break;
	case ConfigureNotify: {
	    /* this approach preserves a 1:1 viewport aspect ratio */
	    int vX, vY, vW, vH;
	    int eW = event.xconfigure.width, eH = event.xconfigure.height;
	    if (eW >= eH) {
		vX = 0;
		vY = (eH - eW) >> 1;
		vW = vH = eW;
	    } else {
		vX = (eW - eH) >> 1;
		vY = 0;
		vW = vH = eH;
	    }
	    glViewport(vX, vY, vW, vH);
	    }
	    break;
	case KeyPress:
	    (void) XLookupString(&event.xkey, buf, sizeof(buf), &keysym, NULL);
	    switch (keysym) {
	    case XK_Escape:
		exit(EXIT_SUCCESS);
	    default:
		break;
	    }
	case ButtonPress:
	    prevx = event.xbutton.x;
	    prevy = event.xbutton.y;
	    break;
	case MotionNotify:
	    deltax += (event.xbutton.x - prevx); prevx = event.xbutton.x;
	    deltay += (event.xbutton.y - prevy); prevy = event.xbutton.y;
	    break;
	default:
	    break;
	}
    } while (XPending(dpy));

    draw_scene(deltax, deltay);
    glXSwapBuffers(dpy, win);
}

static void
error(const char *prog, const char *msg) {
    fprintf(stderr, "%s: %s\n", prog, msg);
    exit(EXIT_FAILURE);
}

static int
query_extension(char* extName) {
    char *p = (char *) glGetString(GL_EXTENSIONS);
    char *end = p + strlen(p);
    while (p < end) {
        int n = strcspn(p, " ");
        if ((strlen(extName) == n) && (strncmp(extName, p, n) == 0))
            return GL_TRUE;
        p += (n + 1);
    }
    return GL_FALSE;
}

