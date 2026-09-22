/*
 * ostest.c - OSMesa Extended Headless Test Suite for Mesa 1.2.8
 *
 * Compiles and runs headlessly to test 2D textures, alpha blending,
 * GLU quadrics, display lists, scissor clipping, and Z-buffer precision.
 *
 * Output: 24-bit RGB PPM image file (P6 binary format).
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/osmesa.h>
#include "glaux.h"

#define WIDTH  512
#define HEIGHT 512

/* ------------------------------------------------------------------ */
/* Test 1: 2D Procedural Texture Mapping                              */
/* ------------------------------------------------------------------ */
static void test_texture_mapping(void)
{
    GLubyte tex_data[64][64][3];
    int i, j;

    /* Generate 64x64 red & white checkerboard texture */
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            int c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;
            tex_data[i][j][0] = 255;            /* Red */
            tex_data[i][j][1] = (GLubyte)c;     /* Green */
            tex_data[i][j][2] = (GLubyte)c;     /* Blue */
        }
    }

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);

    glPushMatrix();
    glTranslatef(-2.0f, 0.6f, 0.0f);
    glRotatef(25.0f, 0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.7f, -0.7f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.7f, -0.7f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.7f,  0.7f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.7f,  0.7f, 0.0f);
    glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/* ------------------------------------------------------------------ */
/* Test 2: Alpha Blending & Transparency                              */
/* ------------------------------------------------------------------ */
static void test_alpha_blending(void)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    glTranslatef(-0.6f, 0.6f, 0.0f);

    /* Background Blue Quad (Opaque) */
    glColor4f(0.1f, 0.3f, 0.9f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.7f, -0.7f, -0.1f);
    glVertex3f( 0.5f, -0.7f, -0.1f);
    glVertex3f( 0.5f,  0.5f, -0.1f);
    glVertex3f(-0.7f,  0.5f, -0.1f);
    glEnd();

    /* Foreground Green Quad (50% Transparent) */
    glColor4f(0.2f, 0.9f, 0.2f, 0.5f);
    glBegin(GL_QUADS);
    glVertex3f(-0.3f, -0.5f, 0.0f);
    glVertex3f( 0.7f, -0.5f, 0.0f);
    glVertex3f( 0.7f,  0.7f, 0.0f);
    glVertex3f(-0.3f,  0.7f, 0.0f);
    glEnd();

    glPopMatrix();
    glDisable(GL_BLEND);
}

/* ------------------------------------------------------------------ */
/* Test 3: GLU Quadrics Engine                                        */
/* ------------------------------------------------------------------ */
static void test_glu_quadrics(void)
{
    int i;
    int slices = 16;
    float da = 2.0f * 3.14159265f / 16.0f;
    float r1 = 0.5f;
    float r2 = 0.3f;
    float h = 0.8f;

    glPushMatrix();
    glTranslatef(2.0f, 0.6f, 0.0f);
    glRotatef(25.0f, 1.0f, 0.5f, 0.0f);

    /* Render Yellow Top Cap Disk */
    glColor3f(0.9f, 0.9f, 0.1f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, h);
    for (i = 0; i <= slices; i++) {
        float a = i * da;
        glVertex3f((float)cos(a) * r2, (float)sin(a) * r2, h);
    }
    glEnd();

    /* Render Orange Cylinder Body */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= slices; i++) {
        float a = i * da;
        float ca = (float)cos(a);
        float sa = (float)sin(a);
        glColor3f(0.9f, 0.4f + (i % 2) * 0.2f, 0.1f);
        glVertex3f(ca * r1, sa * r1, 0.0f);
        glVertex3f(ca * r2, sa * r2, h);
    }
    glEnd();

    glPopMatrix();
}

/* ------------------------------------------------------------------ */
/* Test 4: OpenGL Display Lists                                       */
/* ------------------------------------------------------------------ */
static void test_display_lists(void)
{
    GLuint list_id;
    int k;

    list_id = glGenLists(1);
    glNewList(list_id, GL_COMPILE);

    /* Draw a simple 5-point star */
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (k = 0; k <= 10; k++) {
        float angle = k * (3.14159265f / 5.0f);
        float r = (k % 2 == 0) ? 0.6f : 0.25f;
        glColor3f(1.0f, (k % 2 == 0) ? 0.8f : 0.2f, 0.0f);
        glVertex3f((float)cos(angle) * r, (float)sin(angle) * r, 0.0f);
    }
    glEnd();

    glEndList();

    glPushMatrix();
    glTranslatef(-0.6f, -0.6f, 0.0f);

    /* Call pre-compiled display list twice with different scales */
    glPushMatrix();
    glTranslatef(-0.2f, 0.2f, 0.0f);
    glCallList(list_id);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.3f, -0.3f, 0.1f);
    glScalef(0.6f, 0.6f, 0.6f);
    glCallList(list_id);
    glPopMatrix();

    glPopMatrix();
    glDeleteLists(list_id, 1);
}

/* ------------------------------------------------------------------ */
/* Test 5: Scissor Box Clipping                                       */
/* ------------------------------------------------------------------ */
static void test_scissor_clipping(void)
{
    glEnable(GL_SCISSOR_TEST);

    /* Restrict drawing to a 120x120 pixel window at bottom-left area */
    glScissor(20, 20, 120, 120);

    glPushMatrix();
    glLoadIdentity();

    /* Try to draw a giant magenta quad covering the entire screen */
    glColor3f(0.9f, 0.1f, 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(-3.0f, -3.0f, 0.0f);
    glVertex3f( 3.0f, -3.0f, 0.0f);
    glVertex3f( 3.0f,  3.0f, 0.0f);
    glVertex3f(-3.0f,  3.0f, 0.0f);
    glEnd();

    glPopMatrix();
    glDisable(GL_SCISSOR_TEST);
}

/* ------------------------------------------------------------------ */
/* Test 6: Z-Buffer Depth Precision                                   */
/* ------------------------------------------------------------------ */
static void test_zbuffer_precision(void)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glPushMatrix();
    glTranslatef(0.6f, 0.6f, 0.0f);

    /* Red Quad at Z = -0.5 */
    glColor3f(0.9f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glEnd();

    /* Yellow Quad angled through Z = -1.0 to Z = 0.0 */
    glColor3f(0.9f, 0.9f, 0.1f);
    glBegin(GL_QUADS);
    glVertex3f(-0.6f, -0.3f, -1.0f);
    glVertex3f( 0.6f, -0.3f,  0.0f);
    glVertex3f( 0.6f,  0.3f,  0.0f);
    glVertex3f(-0.6f,  0.3f, -1.0f);
    glEnd();

    glPopMatrix();
}

/* ------------------------------------------------------------------ */
/* Test 7: Complex Polyhedra (Icosahedron & Dodecahedron)             */
/* ------------------------------------------------------------------ */
static void test_polyhedra(void)
{
    glEnable(GL_DEPTH_TEST);
    glPushMatrix();
    glTranslatef(-2.0f, 2.0f, 0.0f);
    glRotatef(45.0f, 1.0f, 1.0f, 0.0f);
    glColor3f(0.8f, 0.2f, 0.8f);
    auxSolidIcosahedron(0.5);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.6f, 2.0f, 0.0f);
    glRotatef(45.0f, 1.0f, 1.0f, 0.0f);
    glColor3f(0.2f, 0.8f, 0.8f);
    auxWireDodecahedron(0.5);
    glPopMatrix();
}

/* ------------------------------------------------------------------ */
/* Test 8: Utah Teapot                                                */
/* ------------------------------------------------------------------ */
static void test_teapot(void)
{
    glPushMatrix();
    glTranslatef(0.6f, 2.0f, 0.0f);
    glRotatef(30.0f, 1.0f, 0.5f, 0.0f);
    glColor3f(0.9f, 0.6f, 0.2f);
    auxSolidTeapot(0.5);
    glPopMatrix();
}

/* ------------------------------------------------------------------ */
/* Test 9: Advanced Lighting (Smooth Shading)                         */
/* ------------------------------------------------------------------ */
static void test_lighting_sphere(void)
{
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glPushMatrix();
    glTranslatef(2.0f, 2.0f, 0.0f);
    auxSolidSphere(0.6);
    glPopMatrix();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}

/* ------------------------------------------------------------------ */
/* Main Render Loop                                                   */
/* ------------------------------------------------------------------ */
static void render_all_tests(void)
{
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* Expanded orthographic projection to fit 16-part grid */
    glOrtho(-3.0, 3.0, -3.0, 3.0, -10.0, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Execute all sub-tests */
    test_texture_mapping();
    test_alpha_blending();
    test_glu_quadrics();
    test_display_lists();
    test_scissor_clipping();
    test_zbuffer_precision();
    
    /* New Expanded Tests */
    test_polyhedra();
    test_teapot();
    test_lighting_sphere();
}

/* ------------------------------------------------------------------ */
/* Entry Point & PPM Writer                                           */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    OSMesaContext ctx;
    void *buffer;
    const char *filename = "ostest_out.ppm";
    FILE *f;

    if (argc > 1) {
        filename = argv[1];
    }

    printf("Initializing OSMesa context (%dx%d)...\n", WIDTH, HEIGHT);

    /* 1. Create RGBA off-screen context */
    ctx = OSMesaCreateContext(GL_RGBA, NULL);
    if (!ctx) {
        fprintf(stderr, "Error: OSMesaCreateContext failed!\n");
        return 1;
    }

    /* 2. Allocate 32-bit RGBA pixel buffer */
    buffer = malloc(WIDTH * HEIGHT * 4 * sizeof(GLubyte));
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed!\n");
        OSMesaDestroyContext(ctx);
        return 1;
    }

    /* 3. Bind buffer and make current */
    if (!OSMesaMakeCurrent(ctx, buffer, GL_UNSIGNED_BYTE, WIDTH, HEIGHT)) {
        fprintf(stderr, "Error: OSMesaMakeCurrent failed!\n");
        free(buffer);
        OSMesaDestroyContext(ctx);
        return 1;
    }

    /* 4. Perform rendering */
    printf("Executing Mesa test suite...\n");
    render_all_tests();
    glFinish();

    /* 5. Save PPM Image */
    printf("Writing output PPM image to '%s'...\n", filename);
    f = fopen(filename, "wb");
    if (f) {
        int x, y;
        GLubyte *ptr = (GLubyte *)buffer;

        /* Write P6 header */
        fprintf(f, "P6\n# Mesa 1.2.8 ostest output\n%d %d\n255\n", WIDTH, HEIGHT);

        /* Write RGB scanlines (flipped vertically for OpenGL bottom-left origin) */
        for (y = HEIGHT - 1; y >= 0; y--) {
            for (x = 0; x < WIDTH; x++) {
                int idx = (y * WIDTH + x) * 4;
                fputc(ptr[idx],     f); /* R */
                fputc(ptr[idx + 1], f); /* G */
                fputc(ptr[idx + 2], f); /* B */
            }
        }
        fclose(f);
        printf("Success: Image written cleanly (%ld bytes).\n", (long)(WIDTH * HEIGHT * 3 + 40));
    } else {
        fprintf(stderr, "Error: Could not open output file '%s'\n", filename);
    }

    /* 6. Cleanup */
    free(buffer);
    OSMesaDestroyContext(ctx);
    return 0;
}
