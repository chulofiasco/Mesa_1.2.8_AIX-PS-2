/* Unified Visual Test Gallery */
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "glaux.h"

int current_scene = 0;
int num_scenes = 5;
int auto_rotate = 1;
int wave_active = 0;
float wave_time = 0.0f;
int invert_colors = 0;

/* Handlers moved down */

/* Globals from test_lighting.c */
int is_lit = 1;
int is_textured = 1;
int is_dithered = 1;
int wireframe = 0;
int current_shape = 0;
float xRotation = 30.0f, yRotation = 30.0f;

/* Globals from test_antialias.c */
typedef struct { float x, y; } jitter_point;
jitter_point j8[8] = {
    {-0.334818,  0.435331},
    { 0.286438, -0.393495},
    { 0.459462,  0.141540},
    {-0.414498, -0.192829},
    {-0.183790,  0.082102},
    {-0.079263, -0.317383},
    { 0.102254,  0.299133},
    { 0.164216, -0.054399}
};
int use_accum = 0;
int use_smooth = 0;



GLuint checker_tex;
float rot_angle = 0.0f;

/* Toggle handlers */
void toggle_invert(void) {
    if (current_scene != 0 && current_scene != 1) return;
    invert_colors = !invert_colors;
    printf("Inverted Colors %s\n", invert_colors ? "ON" : "OFF");
}

void toggle_wave(void) {
    if (current_scene != 4) return;
    wave_active = !wave_active;
    printf("Wave %s\n", wave_active ? "ON" : "OFF");
}

void toggle_auto_rotate(void) {
    auto_rotate = !auto_rotate;
    printf("Auto-rotation %s\n", auto_rotate ? "ON" : "OFF");
}

void rotate_geom(void) {
    rot_angle += 15.0f;
    xRotation += 15.0f;
    yRotation += 15.0f;
    wave_time += 1.0f;
    printf("Geometry rotation bumped. Current rot_angle: %.1f\n", rot_angle);
}

void toggle_shape(void) {
    if (current_scene != 2) return;
    current_shape = (current_shape + 1) % 2;
    printf("Shape switched to %d\n", current_shape);
}

void toggle_wireframe(void) {
    if (current_scene != 2) return;
    wireframe = !wireframe;
    printf("Wireframe %s\n", wireframe ? "ON" : "OFF");
}

void toggle_texture(void) {
    if (current_scene != 2) return;
    is_textured = !is_textured;
    printf("Textures %s\n", is_textured ? "ON" : "OFF");
}

void toggle_lighting(void) {
    if (current_scene != 2) return;
    is_lit = !is_lit;
    printf("Lighting %s\n", is_lit ? "ON" : "OFF");
}

void toggle_accum(void) {
    if (current_scene != 3) return;
    use_accum = !use_accum;
    printf("Accumulation Buffer %s\n", use_accum ? "ON" : "OFF");
}

void toggle_smooth(void) {
    if (current_scene != 3) return;
    use_smooth = !use_smooth;
    printf("GL_LINE_SMOOTH %s\n", use_smooth ? "ON" : "OFF");
}

void rotate_x_up(void) { if (current_scene == 2) xRotation += 5.0f; }
void rotate_x_down(void) { if (current_scene == 2) xRotation -= 5.0f; }
void rotate_y_up(void) { if (current_scene == 2) yRotation += 5.0f; }
void rotate_y_down(void) { if (current_scene == 2) yRotation -= 5.0f; }
void make_checker_texture(void) {
    int i, j, c;
    unsigned char image[64][64][3];
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            c = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;
            image[i][j][0] = (unsigned char) c;
            image[i][j][1] = (unsigned char) c;
            image[i][j][2] = (unsigned char) c;
        }
    }
    
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (GLfloat)GL_MODULATE);
}
void DrawStaticArraySphere(void) {
    int stacks = 32, slices = 32;
    double drho, dtheta;
    double x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3;
    int i, j;
    double radius = 1.0;
    
    double *s_th, *c_th;
    double *s_rho, *c_rho;

    s_th = (double*) malloc(64 * sizeof(double));
    c_th = (double*) malloc(64 * sizeof(double));
    s_rho = (double*) malloc(64 * sizeof(double));
    c_rho = (double*) malloc(64 * sizeof(double));

    drho = 3.14159265358979323846 / (double) stacks;
    dtheta = 2.0 * 3.14159265358979323846 / (double) slices;

    for (j = 0; j <= slices; j++) {
        s_th[j] = sin(j * dtheta);
        c_th[j] = cos(j * dtheta);
    }
    for (i = 0; i <= stacks; i++) {
        s_rho[i] = sin(i * drho);
        c_rho[i] = cos(i * drho);
    }

    for (i = 0; i < stacks; i++) {
        glBegin(GL_QUADS);
        for (j = 0; j < slices; j++) {
            x0 = -s_th[j] * s_rho[i];
            y0 = c_th[j] * s_rho[i];
            z0 = c_rho[i];
            
            x1 = -s_th[j] * s_rho[i+1];
            y1 = c_th[j] * s_rho[i+1];
            z1 = c_rho[i+1];
            
            x2 = -s_th[j+1] * s_rho[i+1];
            y2 = c_th[j+1] * s_rho[i+1];
            z2 = c_rho[i+1];
            
            x3 = -s_th[j+1] * s_rho[i];
            y3 = c_th[j+1] * s_rho[i];
            z3 = c_rho[i];
            
            glNormal3f(x0, y0, z0);
            glTexCoord2f((float)j / slices, (float)i / stacks);
            glVertex3f(x0 * radius, y0 * radius, z0 * radius);
            
            glNormal3f(x1, y1, z1);
            glTexCoord2f((float)j / slices, (float)(i+1) / stacks);
            glVertex3f(x1 * radius, y1 * radius, z1 * radius);
            
            glNormal3f(x2, y2, z2);
            glTexCoord2f((float)(j+1) / slices, (float)(i+1) / stacks);
            glVertex3f(x2 * radius, y2 * radius, z2 * radius);
            
            glNormal3f(x3, y3, z3);
            glTexCoord2f((float)(j+1) / slices, (float)i / stacks);
            glVertex3f(x3 * radius, y3 * radius, z3 * radius);
        }
        glEnd();
    }
    
    free(s_th);
    free(c_th);
    free(s_rho);
    free(c_rho);
}
void DrawTorus(void) {
    double innerRadius = 0.4, outerRadius = 0.8;
    int nsides = 20, rings = 30;
    int i, j;
    double theta, phi, theta1;
    double cosTheta, sinTheta;
    double cosTheta1, sinTheta1;
    double ringDelta, sideDelta;

    ringDelta = 2.0 * 3.14159265358979323846 / rings;
    sideDelta = 2.0 * 3.14159265358979323846 / nsides;

    theta = 0.0;
    cosTheta = 1.0;
    sinTheta = 0.0;
    for (i = rings - 1; i >= 0; i--) {
        theta1 = theta + ringDelta;
        cosTheta1 = cos(theta1);
        sinTheta1 = sin(theta1);
        glBegin(GL_QUAD_STRIP);
        phi = 0.0;
        for (j = nsides; j >= 0; j--) {
            double cosPhi, sinPhi, dist;

            phi += sideDelta;
            cosPhi = cos(phi);
            sinPhi = sin(phi);
            dist = outerRadius + innerRadius * cosPhi;

            glNormal3f(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi);
            glTexCoord2f((float)i / rings, (float)j / nsides);
            glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, innerRadius * sinPhi);

            glNormal3f(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi);
            glTexCoord2f((float)(i+1) / rings, (float)j / nsides);
            glVertex3f(cosTheta * dist, -sinTheta * dist, innerRadius * sinPhi);
        }
        glEnd();
        theta = theta1;
        cosTheta = cosTheta1;
        sinTheta = sinTheta1;
    }
}
void draw_scene(void) {
    /* Draw high-contrast diagonal lines */
    glColor3f(1.0, 1.0, 0.0);
    glLineWidth(2.0);
    glPushMatrix();
    glRotatef(-rot_angle * 2.0, 1.0, 1.0, 0.0); /* Tumble in 3D */
    glBegin(GL_LINES);
    glVertex2f(-1.5, -1.5);
    glVertex2f( 1.5,  1.5);
    glVertex2f(-1.5,  1.5);
    glVertex2f( 1.5, -1.5);
    glEnd();
    glPopMatrix();

    /* Draw a wireframe sphere to show curves */
    glColor3f(0.0, 1.0, 1.0);
    glPushMatrix();
    glRotatef(rot_angle, 1.0, 0.5, 0.2);
    auxWireSphere(1.0);
    glPopMatrix();
}

void scene_0_display(void) {
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };

    if (invert_colors) {
        glClearColor(1.0, 1.0, 1.0, 1.0);
        mat_diffuse[0] = 0.1; mat_diffuse[1] = 0.1; mat_diffuse[2] = 0.1;
    } else {
        glClearColor(0.2, 0.2, 0.2, 1.0);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glColor3f (1.0, 1.0, 1.0);

    /* Draw Wire Sphere */
    glPushMatrix();
    glTranslatef (-3.0, 3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxWireSphere(0.8);
    glPopMatrix();

    /* Draw Solid Sphere */
    glPushMatrix();
    glTranslatef (0.0, 3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxSolidSphere(0.8);
    glPopMatrix();

    /* Draw Wire Cone */
    glPushMatrix();
    glTranslatef (3.0, 3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxWireCone(0.8, 1.5);
    glPopMatrix();

    /* Draw Solid Cone */
    glPushMatrix();
    glTranslatef (-3.0, 0.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxSolidCone(0.8, 1.5);
    glPopMatrix();

    /* Draw Wire Cylinder */
    glPushMatrix();
    glTranslatef (0.0, 0.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxWireCylinder(0.8, 1.5);
    glPopMatrix();

    /* Draw Solid Cylinder */
    glPushMatrix();
    glTranslatef (3.0, 0.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxSolidCylinder(0.8, 1.5);
    glPopMatrix();

    /* Draw Wire Cube */
    glPushMatrix();
    glTranslatef (-3.0, -3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxWireCube(1.2);
    glPopMatrix();

    /* Draw Solid Cube */
    glPushMatrix();
    glTranslatef (0.0, -3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxSolidCube(1.2);
    glPopMatrix();

    /* Draw Wire Torus (just to test non-GLU aux primitive) */
    glPushMatrix();
    glTranslatef (3.0, -3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    auxWireTorus(0.3, 0.8);
    glPopMatrix();

    
}
void scene_0_init(void) {
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
    glClearColor (0.2, 0.2, 0.2, 1.0);
}
void scene_1_display(void) {
    GLUquadricObj *qobj;
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };

    if (invert_colors) {
        glClearColor(1.0, 1.0, 1.0, 1.0);
        mat_diffuse[0] = 0.1; mat_diffuse[1] = 0.1; mat_diffuse[2] = 0.1;
    } else {
        glClearColor (0.2, 0.2, 0.2, 1.0);
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glColor3f (1.0, 1.0, 1.0);

    qobj = gluNewQuadric();
    gluQuadricDrawStyle(qobj, GLU_FILL);
    gluQuadricNormals(qobj, GLU_SMOOTH);

    /* Draw gluDisk (FILL) */
    glPushMatrix();
    glTranslatef (-3.0, 3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluDisk(qobj, 0.2, 0.8, 15, 5);
    glPopMatrix();

    /* Draw gluPartialDisk (FILL) */
    glPushMatrix();
    glTranslatef (0.0, 3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluPartialDisk(qobj, 0.2, 0.8, 15, 5, 0.0, 270.0);
    glPopMatrix();

    /* Draw gluSphere (LINE) */
    gluQuadricDrawStyle(qobj, GLU_LINE);
    glPushMatrix();
    glTranslatef (3.0, 3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluSphere(qobj, 0.8, 15, 15);
    glPopMatrix();

    /* Draw gluSphere (POINT) */
    gluQuadricDrawStyle(qobj, GLU_POINT);
    glPushMatrix();
    glTranslatef (-3.0, 0.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluSphere(qobj, 0.8, 15, 15);
    glPopMatrix();

    /* Draw gluCylinder (FILL) */
    gluQuadricDrawStyle(qobj, GLU_FILL);
    glPushMatrix();
    glTranslatef (0.0, 0.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluCylinder(qobj, 0.8, 0.8, 1.5, 15, 5);
    glPopMatrix();

    /* Draw gluCylinder (LINE) */
    gluQuadricDrawStyle(qobj, GLU_LINE);
    glPushMatrix();
    glTranslatef (3.0, 0.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluCylinder(qobj, 0.8, 0.8, 1.5, 15, 5);
    glPopMatrix();

    /* Draw gluCylinder (CONE FILL) */
    gluQuadricDrawStyle(qobj, GLU_FILL);
    glPushMatrix();
    glTranslatef (-3.0, -3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluCylinder(qobj, 0.8, 0.0, 1.5, 15, 5);
    glPopMatrix();

    /* Draw gluCylinder (CONE LINE) */
    gluQuadricDrawStyle(qobj, GLU_LINE);
    glPushMatrix();
    glTranslatef (0.0, -3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluCylinder(qobj, 0.8, 0.0, 1.5, 15, 5);
    glPopMatrix();

    /* Draw gluCylinder (CONE POINT) */
    gluQuadricDrawStyle(qobj, GLU_POINT);
    glPushMatrix();
    glTranslatef (3.0, -3.0, -10.0);
    glRotatef(rot_angle, 1.0, 1.0, 0.0);
    gluCylinder(qobj, 0.8, 0.0, 1.5, 15, 5);
    glPopMatrix();

    gluDeleteQuadric(qobj);
    
}
void scene_1_init(void) {
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel (GL_SMOOTH);
    glClearColor (0.2, 0.2, 0.2, 1.0);
}
void scene_2_display(void) {
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };

    glClearColor (0.2, 0.2, 0.2, 1.0); /* Original dark grey background */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glDisable(GL_CULL_FACE);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);

    if (is_dithered) glEnable(GL_DITHER);
    else glDisable(GL_DITHER);

    if (is_lit) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);

    if (is_textured) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glPushMatrix();
    glTranslatef(0.0, 0.0, -4.0);
    glRotatef(xRotation, 1.0, 0.0, 0.0);
    glRotatef(yRotation, 0.0, 1.0, 0.0);

    glColor3f(1.0, 1.0, 1.0); /* Base color if unlit */

    if (current_shape == 0) {
        DrawStaticArraySphere();
    } else if (current_shape == 1) {
        DrawTorus();
    }

    glPopMatrix();
    
}
void scene_2_init(void) {
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    
    make_checker_texture();
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}
void scene_3_display(void) {
    if (use_smooth) {
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_BLEND);
    }

    if (use_accum) {
        GLint viewport[4];
        volatile int jitter;
        glGetIntegerv(GL_VIEWPORT, viewport);
        glClear(GL_ACCUM_BUFFER_BIT);
        
        for (jitter = 0; jitter < 8; jitter++) {
            int j = jitter;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glPushMatrix();
            
            /* The * 4.5 maps fractional pixels to world coordinates */
            /* We multiply by 2.0 extra to exaggerate the jitter for testing */
            glTranslatef(j8[j].x * 9.0 / viewport[2], j8[j].y * 9.0 / viewport[3], -5.0);
            
            draw_scene();
            glPopMatrix();
            glAccum(GL_ACCUM, 1.0 / 8.0);
        }
        glAccum(GL_RETURN, 1.0);
    } else {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        glTranslatef(0.0, 0.0, -5.0);
        draw_scene();
        glPopMatrix();
    }
    
    
}

void get_scene4_color(float x, float y) {
    float u, v, r, g, b;
    float c00[3] = {1,1,0};
    float c10[3], c01[3], c11[3];
    
    if (x <= 0.0f && y >= 0.0f) {
        u = -x/1.5f; v = y/1.5f;
        c10[0]=1; c10[1]=0; c10[2]=0;
        c01[0]=0; c01[1]=0; c01[2]=0.5f;
        c11[0]=1; c11[1]=0; c11[2]=0;
    } else if (x >= 0.0f && y >= 0.0f) {
        u = x/1.5f; v = y/1.5f;
        c10[0]=0; c10[1]=0.5f; c10[2]=0.5f;
        c01[0]=0; c01[1]=0; c01[2]=0.5f;
        c11[0]=0; c11[1]=0; c11[2]=1;
    } else if (x >= 0.0f && y <= 0.0f) {
        u = x/1.5f; v = -y/1.5f;
        c10[0]=0; c10[1]=0.5f; c10[2]=0.5f;
        c01[0]=0; c01[1]=1; c01[2]=0;
        c11[0]=0; c11[1]=1; c11[2]=0;
    } else {
        u = -x/1.5f; v = -y/1.5f;
        c10[0]=1; c10[1]=0; c10[2]=0;
        c01[0]=0; c01[1]=1; c01[2]=0;
        c11[0]=0; c11[1]=0; c11[2]=1;
    }
    
    r = (1-u)*(1-v)*c00[0] + u*(1-v)*c10[0] + (1-u)*v*c01[0] + u*v*c11[0];
    g = (1-u)*(1-v)*c00[1] + u*(1-v)*c10[1] + (1-u)*v*c01[1] + u*v*c11[1];
    b = (1-u)*(1-v)*c00[2] + u*(1-v)*c10[2] + (1-u)*v*c01[2] + u*v*c11[2];
    glColor3f(r, g, b);
}

void scene_4_display(void) {
    int i, j;
    int segments = 24;
    float step = 3.0f / segments;
    
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (wave_active) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    
    glShadeModel(GL_SMOOTH);

    glPushMatrix();
    glTranslatef(0.0, 0.0, -3.2);
    
    if (wave_active) {
        glRotatef(-45.0f, 1.0, 0.0, 0.0);
        glRotatef(30.0f, 0.0, 1.0, 0.0);
        glScalef(0.6f, 0.6f, 0.6f);
    }
    
    /* Draw the colored grid */
    for (i = 0; i < segments; i++) {
        glBegin(GL_QUADS);
        for (j = 0; j < segments; j++) {
            float x0 = -1.5f + j * step;
            float y0 = -1.5f + i * step;
            float x1 = x0 + step;
            float y1 = y0 + step;
            
            float z00 = wave_active ? sin(x0*4.0f + wave_time)*cos(y0*4.0f + wave_time)*0.3f : 0.0f;
            float z10 = wave_active ? sin(x1*4.0f + wave_time)*cos(y0*4.0f + wave_time)*0.3f : 0.0f;
            float z11 = wave_active ? sin(x1*4.0f + wave_time)*cos(y1*4.0f + wave_time)*0.3f : 0.0f;
            float z01 = wave_active ? sin(x0*4.0f + wave_time)*cos(y1*4.0f + wave_time)*0.3f : 0.0f;

            get_scene4_color(x0, y0); glVertex3f(x0, y0, z00);
            get_scene4_color(x1, y0); glVertex3f(x1, y0, z10);
            get_scene4_color(x1, y1); glVertex3f(x1, y1, z11);
            get_scene4_color(x0, y1); glVertex3f(x0, y1, z01);
        }
        glEnd();
    }

    /* Draw stippled lines overlay floating slightly above */
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xAAAA);
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(1.0);
    glDisable(GL_DEPTH_TEST); /* Draw over everything */
    
    glBegin(GL_LINES);
    /* Cross */
    glVertex3f(-1.5, 0.0, wave_active ? sin(-1.5f*4.0f+wave_time)*cos(0.0f)*0.3f+0.05f : 0.0f);
    glVertex3f( 1.5, 0.0, wave_active ? sin( 1.5f*4.0f+wave_time)*cos(0.0f)*0.3f+0.05f : 0.0f);
    glVertex3f(0.0, -1.5, wave_active ? sin(0.0f)*cos(-1.5f*4.0f+wave_time)*0.3f+0.05f : 0.0f);
    glVertex3f(0.0,  1.5, wave_active ? sin(0.0f)*cos( 1.5f*4.0f+wave_time)*0.3f+0.05f : 0.0f);
    
    /* Diamond */
    glVertex3f(0.0, 1.5, wave_active ? sin(0.0f)*cos(1.5f*4.0f+wave_time)*0.3f+0.05f : 0.0f);
    glVertex3f(1.5, 0.0, wave_active ? sin(1.5f*4.0f+wave_time)*cos(0.0f)*0.3f+0.05f : 0.0f);
    
    glVertex3f(1.5, 0.0, wave_active ? sin(1.5f*4.0f+wave_time)*cos(0.0f)*0.3f+0.05f : 0.0f);
    glVertex3f(0.0, -1.5, wave_active ? sin(0.0f)*cos(-1.5f*4.0f+wave_time)*0.3f+0.05f : 0.0f);
    
    glVertex3f(0.0, -1.5, wave_active ? sin(0.0f)*cos(-1.5f*4.0f+wave_time)*0.3f+0.05f : 0.0f);
    glVertex3f(-1.5, 0.0, wave_active ? sin(-1.5f*4.0f+wave_time)*cos(0.0f)*0.3f+0.05f : 0.0f);
    
    glVertex3f(-1.5, 0.0, wave_active ? sin(-1.5f*4.0f+wave_time)*cos(0.0f)*0.3f+0.05f : 0.0f);
    glVertex3f(0.0, 1.5, wave_active ? sin(0.0f)*cos(1.5f*4.0f+wave_time)*0.3f+0.05f : 0.0f);
    glEnd();

    glDisable(GL_LINE_STIPPLE);
    if (wave_active) glEnable(GL_DEPTH_TEST);

    glPopMatrix();
}

void master_init(void) {
    /* Set default states */
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glShadeModel(GL_FLAT);

    if (current_scene == 0) scene_0_init();
    if (current_scene == 1) scene_1_init();
    if (current_scene == 2) scene_2_init();
}

void master_display(void) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* Auto-animate globals */
    if (auto_rotate) {
        rot_angle += 1.0f;
        if (rot_angle >= 360.0f) rot_angle -= 360.0f;
        xRotation += 0.5f;
        yRotation += 0.7f;
        if (wave_active) wave_time += 0.05f;
    }
    
    /* Reset state leakage from previous scenes */
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    
    if (current_scene == 0) scene_0_display();
    if (current_scene == 1) scene_1_display();
    if (current_scene == 2) scene_2_display();
    if (current_scene == 3) scene_3_display();
    if (current_scene == 4) scene_4_display();

    auxSwapBuffers();
}

const char *scene_names[] = {
    "0: Basic Geometry (aux primitives)",
    "1: GLU Quadrics (Spheres, Cylinders, Disks, Cones)",
    "2: Texturing, Lighting, and Material State",
    "3: Accumulation Buffer Antialiasing and Line Smooth",
    "4: Color Gamut and Vertex Deformation (Wave)"
};

void next_scene(void) {
    current_scene = (current_scene + 1) % num_scenes;
    printf("\n--- Switching to Scene %s ---\n", scene_names[current_scene]);
    master_init();
}

void myReshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    auxInitDisplayMode(AUX_DOUBLE | AUX_RGB | AUX_DEPTH | AUX_ACCUM);
    auxInitPosition(0, 0, 500, 500);
    auxInitWindow("Gallery (SPACE: cycle, P: pause)");
    master_init();
    auxReshapeFunc(myReshape);
    
    auxKeyFunc(AUX_i, toggle_invert);
    auxKeyFunc(AUX_I, toggle_invert);
    auxKeyFunc(AUX_SPACE, next_scene);
    auxKeyFunc(AUX_v, toggle_wave);
    auxKeyFunc(AUX_V, toggle_wave);
    auxKeyFunc(AUX_p, toggle_auto_rotate);
    auxKeyFunc(AUX_P, toggle_auto_rotate);
    auxKeyFunc(AUX_r, rotate_geom);
    auxKeyFunc(AUX_R, rotate_geom);
    auxKeyFunc(AUX_c, toggle_shape);
    auxKeyFunc(AUX_C, toggle_shape);
    auxKeyFunc(AUX_w, toggle_wireframe);
    auxKeyFunc(AUX_W, toggle_wireframe);
    auxKeyFunc(AUX_t, toggle_texture);
    auxKeyFunc(AUX_T, toggle_texture);
    auxKeyFunc(AUX_l, toggle_lighting);
    auxKeyFunc(AUX_L, toggle_lighting);
    auxKeyFunc(AUX_a, toggle_accum);
    auxKeyFunc(AUX_A, toggle_accum);
    auxKeyFunc(AUX_s, toggle_smooth);
    auxKeyFunc(AUX_S, toggle_smooth);
    auxKeyFunc(AUX_x, rotate_x_down);
    auxKeyFunc(AUX_X, rotate_x_up);
    auxKeyFunc(AUX_y, rotate_y_down);
    auxKeyFunc(AUX_Y, rotate_y_up);
    auxIdleFunc(master_display);
    auxMainLoop(master_display);
    return 0;
}
