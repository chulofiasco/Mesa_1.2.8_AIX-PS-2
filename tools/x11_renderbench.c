#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Scale.h>
#include <Xm/PushB.h>
#include <Xm/FileSB.h>
#include <Xm/ScrolledW.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/SelectioB.h>
#include <Xm/Frame.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/xmesa.h>
#include <glaux.h>
#include "gltk.h"

/* The Mesa context */
XMesaContext context = NULL;
XVisualInfo *visinfo = NULL;
Widget gl_widget = NULL;
Widget topLevel = NULL;
Widget fileDialog = NULL;

/* Rotations */
GLfloat xrot = 0.0, yrot = 0.0, zrot = 0.0;
GLfloat xrot_speed = 0.0, yrot_speed = 0.0, zrot_speed = 0.0;
XtWorkProcId work_proc_id = 0;
Widget scaleX_w = NULL, scaleY_w = NULL, scaleZ_w = NULL, zoomScale = NULL;

/* Global UI Widgets */
Widget toggleLight, toggleFog, toggleTex, toggleSmooth, toggleBlend;
Widget scaleR, scaleG, scaleB;
Widget scaleLA, scaleLD, scalePX, scalePY, scalePZ;
Widget toggleNative, toggleLinear, toggleSphere;
Widget radioShapeBox, radioMatBox, radioWrapBox;
Widget polyFill, polyLine, polyPoint;

/* Globals for state */
int current_shape = 0;
int lighting_enabled = 1;
int fog_enabled = 0;
int texturing_enabled = 0;
int smooth_shading = 1;
int blend_materials = 0;
int auto_spin = 0;
int poly_mode = 2; /* 0=Point, 1=Line, 2=Fill */
GLfloat zoom_level = 1.0;
int mat_type = 0; /* 0=Brass, 1=Chrome, 2=Emerald, 3=Rubber, 4=Normal */
GLfloat current_color[3] = {1.0, 1.0, 1.0};
GLfloat bg_color_value = 0.6;
int texture_wrap_mode = 0; /* 0=Native, 1=Linear, 2=Sphere */
int texture_needs_upload = 1;
int rainbow_mode = 0;
int frame_counter = 0;
int suppress_redraws = 0;
XmFontList global_font = NULL;

/* The Mesa contexts */
XMesaContext colorContext = NULL;
Widget colorPreviewArea = NULL;

XMesaContext lightContext = NULL;
Widget lightPreviewArea = NULL;

int window_mapped = 0;
GLfloat light_ambient = 0.2;
GLfloat light_diffuse = 1.0;
GLfloat light_pos_x = 10.0;
GLfloat light_pos_y = 10.0;
GLfloat light_pos_z = 10.0;

/* Texture state */
TK_RGBImageRec *textureImage = NULL;

/* Quadric object */
GLUquadricObj *quadObj = NULL;

/* Forward declarations */
static void RedrawGL(void);
static Boolean IdleFunc(XtPointer client_data);
static void LightExposeCB(Widget w, XtPointer client_data, XtPointer call_data);

/* Checkerboard Texture */
static GLubyte checkerImage[32][32][3];
static void makeCheckerImage(void) {
    int i, j, c;
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            c = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) ? 255 : 64;
            checkerImage[i][j][0] = (GLubyte)c;
            checkerImage[i][j][1] = (GLubyte)c;
            checkerImage[i][j][2] = (GLubyte)c;
        }
    }
}

static void MapHandler(Widget w, XtPointer client_data, XEvent *event, Boolean *continue_to_dispatch)
{
    if (event->type == MapNotify) {
        window_mapped = 1;
        RedrawGL();
    }
}

/* Callbacks */
static void QuitCB(Widget w, XtPointer client_data, XtPointer call_data) {
    exit(0);
}

static void TextureWrapCB(Widget w, XtPointer client_data, XtPointer call_data) {
    XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct *)call_data;
    if (cbs->set) {
        texture_wrap_mode = (int)client_data;
        if (context && window_mapped && !suppress_redraws) { XMesaMakeCurrent(context); RedrawGL(); }
    }
}

static void ResetDefaultsCB(Widget w, XtPointer client_data, XtPointer call_data) {
    suppress_redraws = 1;
    /* Reset variables */
    current_shape = 0;
    lighting_enabled = 1;
    fog_enabled = 0;
    texturing_enabled = 0;
    smooth_shading = 1;
    blend_materials = 0;
    auto_spin = 0;
    poly_mode = 2;
    mat_type = 0;
    current_color[0] = 1.0; current_color[1] = 1.0; current_color[2] = 1.0;
    bg_color_value = 0.6;
    texture_wrap_mode = 0;
    
    xrot = 0.0; yrot = 0.0; zrot = 0.0;
    xrot_speed = 0.0; yrot_speed = 0.0; zrot_speed = 0.0;
    zoom_level = 1.0;
    
    light_ambient = 0.2; light_diffuse = 1.0;
    light_pos_x = 10.0; light_pos_y = 10.0; light_pos_z = 10.0;
    
    /* Reset Motif Widgets visually */
    if (toggleLight) XmToggleButtonSetState(toggleLight, True, False);
    if (toggleFog) XmToggleButtonSetState(toggleFog, False, False);
    if (toggleTex) XmToggleButtonSetState(toggleTex, False, False);
    if (toggleSmooth) XmToggleButtonSetState(toggleSmooth, True, False);
    if (toggleBlend) XmToggleButtonSetState(toggleBlend, False, False);
    
    if (toggleNative) XmToggleButtonSetState(toggleNative, True, False);
    if (toggleLinear) XmToggleButtonSetState(toggleLinear, False, False);
    if (toggleSphere) XmToggleButtonSetState(toggleSphere, False, False);
    
    if (polyFill) XmToggleButtonSetState(polyFill, True, False);
    if (polyLine) XmToggleButtonSetState(polyLine, False, False);
    if (polyPoint) XmToggleButtonSetState(polyPoint, False, False);
    
    if (textureImage) {
        free(textureImage->data);
        free(textureImage);
        textureImage = NULL;
    }
    
    if (scaleR) XmScaleSetValue(scaleR, 100);
    if (scaleG) XmScaleSetValue(scaleG, 100);
    if (scaleB) XmScaleSetValue(scaleB, 100);
    
    if (scaleX_w) XmScaleSetValue(scaleX_w, 0);
    if (scaleY_w) XmScaleSetValue(scaleY_w, 0);
    if (scaleZ_w) XmScaleSetValue(scaleZ_w, 0);
    if (zoomScale) XmScaleSetValue(zoomScale, 100);
    
    if (scaleLA) XmScaleSetValue(scaleLA, 20);
    if (scaleLD) XmScaleSetValue(scaleLD, 100);
    if (scalePX) XmScaleSetValue(scalePX, 50);
    if (scalePY) XmScaleSetValue(scalePY, 50);
    if (scalePZ) XmScaleSetValue(scalePZ, 50);
    
    suppress_redraws = 0;
    
    if (colorContext && window_mapped) {
        XMesaMakeCurrent(colorContext);
        glClearColor(current_color[0], current_color[1], current_color[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        XMesaSwapBuffers();
    }
    
    if (lightContext && window_mapped) {
        LightExposeCB(NULL, NULL, NULL);
    }
    
    if (context && window_mapped) { 
        XMesaMakeCurrent(context); 
        glClearColor(bg_color_value, bg_color_value, bg_color_value, 1.0);
        RedrawGL(); 
    }
}

static float simple_hash(int x, int y) {
    unsigned int h;
    x = x % 32;
    y = y % 32;
    h = ((unsigned int)x * 374761393) + ((unsigned int)y * 668265263);
    h = (h ^ (h >> 13)) * 1274126177;
    return (float)(h & 0x7FFFFFFF) / 2147483647.0f;
}

static float smooth_noise_fast(float x, float y) {
    float gx = x * 7.0f;
    float gy = y * 7.0f;
    int ix, iy;
    float fx, fy, v00, v10, v01, v11, nx0, nx1;
    
    if (gx < 0.0f) gx += 10000.0f;
    if (gy < 0.0f) gy += 10000.0f;
    
    ix = (int)gx;
    iy = (int)gy;
    fx = gx - (float)ix;
    fy = gy - (float)iy;
    
    v00 = simple_hash(ix, iy);
    v10 = simple_hash(ix + 1, iy);
    v01 = simple_hash(ix, iy + 1);
    v11 = simple_hash(ix + 1, iy + 1);
    
    /* Smooth the interpolation coordinates using a Hermite curve to remove blockiness */
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);
    
    nx0 = v00 * (1.0f - fx) + v10 * fx;
    nx1 = v01 * (1.0f - fx) + v11 * fx;
    return nx0 * (1.0f - fy) + nx1 * fy;
}

static float fast_perlin(float x, float y) {
    float n1 = smooth_noise_fast(x, y);
    float n2 = smooth_noise_fast(x * 2.0f, y * 2.0f);
    float n3 = smooth_noise_fast(x * 4.0f, y * 4.0f);
    return (n1 * 0.5f) + (n2 * 0.25f) + (n3 * 0.125f);
}

static void GenerateProceduralTexture(int type, GLubyte *data, int width, int height) {
    int x, y, i;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* Use safe literal multiplication to avoid FPU division bugs */
            float fx = (float)x * 0.0078125f;
            float fy = (float)y * 0.0078125f;
            GLubyte color = 0;
            int idx = (y * width + x) * 3;
            
            if (type == 0) { /* Clouds */
                /* Hardcode literal to avoid High C FPU variable multiplication bug */
                float n = fast_perlin(fx * 4.571428f, fy * 4.571428f);
                n = n * 1.14285f; /* Multiply instead of divide for safety */
                
                n = (n - 0.35f) * 1.5f;
                if (n > 1.0f) n = 1.0f;
                if (n < 0.0f) n = 0.0f;
                
                /* Simplify lerp using safe literal evaluation */
                data[idx] = (GLubyte)(60.0f + 195.0f * n);
                data[idx+1] = (GLubyte)(130.0f + 125.0f * n);
                data[idx+2] = (GLubyte)(220.0f + 35.0f * n);
                continue;
            } else if (type == 1) { /* Wood */
                float dx = fx - 0.5f;
                float dy = fy - 0.5f;
                float dist_sq = dx*dx + dy*dy;
                float dist = sqrt(dist_sq);
                float n = fast_perlin(fx * 2.0f, fy * 2.0f);
                float ring_arg = (dist + (n * 0.15f)) * 60.0f;
                float ring = sin(ring_arg);
                float t = (ring + 1.0f) * 0.5f; /* 0.0 to 1.0 */
                /* Light brown to Dark brown */
                data[idx]   = (GLubyte)(80.0f + 100.0f * t);
                data[idx+1] = (GLubyte)(40.0f + 70.0f * t);
                data[idx+2] = (GLubyte)(20.0f + 30.0f * t);
                continue;
            } else if (type == 2) { /* Marble Swirl with Sharp Veins */
                float n1 = smooth_noise_fast(fx * 3.0f, fy * 3.0f);
                float tx2 = (fx * 0.8f - fy * 0.6f) * 6.0f;
                float ty2 = (fx * 0.6f + fy * 0.8f) * 6.0f;
                float n2 = smooth_noise_fast(tx2, ty2);
                
                float turb1 = fabs(n1 - 0.5f);
                float turb2 = fabs(n2 - 0.5f) * 0.5f;
                float turb = turb1 + turb2;
                
                float wave_arg = (fx + fy + (turb * 2.0f)) * 15.0f;
                float wave = sin(wave_arg) * 0.5f + 0.5f;
                float vein = wave * wave; /* avoid pow() for safety */
                vein = vein * vein;
                
                /* vein is 0.0 at the center of the vein, 1.0 in the base marble */
                /* Base color: dark teal (30, 80, 80). Veins: White (255, 255, 255) */
                data[idx]   = (GLubyte)(255.0f * (1.0f - vein) + 30.0f * vein);
                data[idx+1] = (GLubyte)(255.0f * (1.0f - vein) + 80.0f * vein);
                data[idx+2] = (GLubyte)(255.0f * (1.0f - vein) + 80.0f * vein);
                continue;
            } else if (type == 3) { /* Voronoi Cells */
                float min_dist = 1.0f;
                for (i = 0; i < 15; i++) {
                    /* Pseudo-random points for the cells */
                    float px = (float)( (i * 17) % 100 ) / 100.0f;
                    float py = (float)( (i * 23) % 100 ) / 100.0f;
                    float dx = fx - px;
                    float dy = fy - py;
                    float dist = sqrt(dx*dx + dy*dy);
                    if (dist < min_dist) min_dist = dist;
                }
                /* Scale and invert to make bright cells with dark borders */
                float color_val = 1.0f - (min_dist * 3.5f);
                if (color_val < 0.0f) color_val = 0.0f;
                color = (GLubyte)(color_val * 255.0f);
            } else if (type == 4) { /* Rainbow */
                float n = fast_perlin(fx, fy);
                float r_arg = (fx + n) * 10.0f;
                float g_arg = (fy + n) * 15.0f;
                float b_arg = ((fx * fy) + n) * 20.0f;
                
                float r = sin(r_arg) * 127.5f + 127.5f;
                float g = sin(g_arg) * 127.5f + 127.5f;
                float b = sin(b_arg) * 127.5f + 127.5f;
                
                data[idx] = (GLubyte)r;
                data[idx+1] = (GLubyte)g;
                data[idx+2] = (GLubyte)b;
                continue;
            } else if (type == 5) { /* Circuit Board Cellular */
                float cells = 16.0f;
                float px = fx * cells;
                float py = fy * cells;
                int ix = (int)px;
                int iy = (int)py;
                float rx = px - ix;
                float ry = py - iy;
                
                int h = (ix * 37 + iy * 113) % 100;
                float val = 0.15f;
                
                if (rx < 0.1f || ry < 0.1f || rx > 0.9f || ry > 0.9f) {
                    val = 0.15f;
                } else if (h < 20) { /* Mount hole */
                    float dist_sq = (rx - 0.5f)*(rx - 0.5f) + (ry - 0.5f)*(ry - 0.5f);
                    if (dist_sq < 0.1f && dist_sq > 0.02f) val = 1.0f;
                    else if (dist_sq <= 0.02f) val = 0.15f;
                } else if (h < 60) { /* Horizontal trace */
                    if (ry > 0.35f && ry < 0.65f) val = 0.9f;
                    if (h < 30 && rx > 0.35f && rx < 0.65f) val = 1.0f; /* Solder pad */
                } else { /* Vertical trace */
                    if (rx > 0.35f && rx < 0.65f) val = 0.9f;
                    if (h > 80 && ry > 0.35f && ry < 0.65f) val = 1.0f; /* Solder pad */
                }
                
                if (val == 0.15f) val += smooth_noise_fast(fx * 4.0f, fy * 4.0f) * 0.1f;
                color = (GLubyte)(val * 255.0f);
            } else if (type == 6) { /* Camouflage */
                float n = fast_perlin(fx * 4.0f, fy * 4.0f);
                if (n < 0.35f) {
                    /* Brightened Dark Green */
                    data[idx] = 75; data[idx+1] = 100; data[idx+2] = 60;
                } else if (n < 0.55f) {
                    /* Brightened Olive Green */
                    data[idx] = 130; data[idx+1] = 145; data[idx+2] = 95;
                } else if (n < 0.75f) {
                    /* Brightened Brown */
                    data[idx] = 155; data[idx+1] = 115; data[idx+2] = 80;
                } else {
                    /* Brightened Khaki Tan */
                    data[idx] = 220; data[idx+1] = 205; data[idx+2] = 165;
                }
                continue;
            }
            
            data[idx] = color;
            data[idx+1] = color;
            data[idx+2] = color;
        }
    }
}

static void TexGenCB(Widget w, XtPointer client_data, XtPointer call_data) {
    int type = (int)client_data;
    if (!textureImage) {
        textureImage = (TK_RGBImageRec *)malloc(sizeof(TK_RGBImageRec));
        textureImage->sizeX = 128;
        textureImage->sizeY = 128;
        textureImage->data = (GLubyte *)malloc(128 * 128 * 3);
    }
    GenerateProceduralTexture(type, textureImage->data, 128, 128);
    texture_needs_upload = 1;
    
    if (!texturing_enabled) {
        texturing_enabled = 1;
        if (toggleTex) XmToggleButtonSetState(toggleTex, True, False);
    }
    
    if (context && window_mapped) { XMesaMakeCurrent(context); RedrawGL(); }
}

static void ResetLightCB(Widget w, XtPointer client_data, XtPointer call_data) {
    suppress_redraws = 1;
    if (scaleLA) XmScaleSetValue(scaleLA, 20);
    if (scaleLD) XmScaleSetValue(scaleLD, 100);
    if (scalePX) XmScaleSetValue(scalePX, 50);
    if (scalePY) XmScaleSetValue(scalePY, 50);
    if (scalePZ) XmScaleSetValue(scalePZ, 50);
    
    light_ambient = 0.2;
    light_diffuse = 1.0;
    light_pos_x = 5.0;
    light_pos_y = 5.0;
    light_pos_z = 5.0;
    
    suppress_redraws = 0;
    
    if (lightContext && window_mapped) {
        LightExposeCB(NULL, NULL, NULL);
    }
    if (context && window_mapped) { 
        XMesaMakeCurrent(context); 
        RedrawGL(); 
    }
}

static void ResetColorCB(Widget w, XtPointer client_data, XtPointer call_data) {
    suppress_redraws = 1;
    if (scaleR) XmScaleSetValue(scaleR, 100);
    if (scaleG) XmScaleSetValue(scaleG, 100);
    if (scaleB) XmScaleSetValue(scaleB, 100);
    
    current_color[0] = 1.0;
    current_color[1] = 1.0;
    current_color[2] = 1.0;
    
    suppress_redraws = 0;
    
    if (colorContext && window_mapped) {
        XMesaMakeCurrent(colorContext);
        glClearColor(current_color[0], current_color[1], current_color[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        XMesaSwapBuffers();
    }
    if (context && window_mapped) { 
        XMesaMakeCurrent(context); 
        RedrawGL(); 
    }
}

static void ShapeSelectCB(Widget w, XtPointer client_data, XtPointer call_data) {
    current_shape = (int)client_data;
    if (context && window_mapped && !suppress_redraws) { XMesaMakeCurrent(context); RedrawGL(); }
}

static void MaterialSelectCB(Widget w, XtPointer client_data, XtPointer call_data) {
    mat_type = (int)client_data;
    if (context && window_mapped && !suppress_redraws) { XMesaMakeCurrent(context); RedrawGL(); }
}

static void ToggleCB(Widget w, XtPointer client_data, XtPointer call_data) {
    XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct *)call_data;
    int flag = (int)client_data;
    if (flag == 1) lighting_enabled = cbs->set;
    if (flag == 2) fog_enabled = cbs->set;
    if (flag == 3) { texturing_enabled = cbs->set; texture_needs_upload = 1; }
    if (flag == 4) smooth_shading = cbs->set;
    if (flag == 5) blend_materials = cbs->set;
    if (flag == 6) {
        auto_spin = cbs->set;
        if (auto_spin && xrot_speed == 0.0 && yrot_speed == 0.0 && zrot_speed == 0.0) {
            xrot_speed = 1.0;
            yrot_speed = 0.5;
            zrot_speed = 0.2;
            if (scaleX_w) XmScaleSetValue(scaleX_w, 36);
            if (scaleY_w) XmScaleSetValue(scaleY_w, 18);
            if (scaleZ_w) XmScaleSetValue(scaleZ_w, 7);
        }
    }
    
    if (flag >= 10 && flag <= 12) poly_mode = flag - 10;
    
    if (flag == 13) {
        rainbow_mode = cbs->set;
        if (!rainbow_mode) {
            current_color[0] = 1.0; current_color[1] = 1.0; current_color[2] = 1.0;
            suppress_redraws = 1;
            if (scaleR) XmScaleSetValue(scaleR, 100);
            if (scaleG) XmScaleSetValue(scaleG, 100);
            if (scaleB) XmScaleSetValue(scaleB, 100);
            suppress_redraws = 0;
            if (colorContext && window_mapped) {
                XMesaMakeCurrent(colorContext);
                glClearColor(1.0, 1.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);
                XMesaSwapBuffers();
            }
        }
    }
    
    if (context && window_mapped && !suppress_redraws) { XMesaMakeCurrent(context); RedrawGL(); }
}

static void ColorScaleCB(Widget w, XtPointer client_data, XtPointer call_data) {
    XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *)call_data;
    int axis = (int)client_data;
    if (axis == 0) current_color[0] = (GLfloat)cbs->value / 100.0;
    if (axis == 1) current_color[1] = (GLfloat)cbs->value / 100.0;
    if (axis == 2) current_color[2] = (GLfloat)cbs->value / 100.0;
    
    if (colorContext && window_mapped && !suppress_redraws) {
        XMesaMakeCurrent(colorContext);
        glClearColor(current_color[0], current_color[1], current_color[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        XMesaSwapBuffers();
    }
    if (context && window_mapped && !suppress_redraws) { XMesaMakeCurrent(context); RedrawGL(); }
}

static void BgColorCB(Widget w, XtPointer client_data, XtPointer call_data) {
    int val = (int)client_data;
    bg_color_value = (GLfloat)val / 100.0;
    if (context && window_mapped && !suppress_redraws) {
        XMesaMakeCurrent(context);
        glClearColor(bg_color_value, bg_color_value, bg_color_value, 1.0);
        RedrawGL();
    }
}

static void LightExposeCB(Widget w, XtPointer client_data, XtPointer call_data) {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat position[4];
    GLfloat mat_ambient[4] = {0.2, 0.2, 0.2, 1.0};
    GLfloat mat_diffuse[4] = {0.8, 0.8, 0.8, 1.0};

    ambient[0] = light_ambient; ambient[1] = light_ambient; ambient[2] = light_ambient; ambient[3] = 1.0;
    diffuse[0] = light_diffuse; diffuse[1] = light_diffuse; diffuse[2] = light_diffuse; diffuse[3] = 1.0;
    position[0] = light_pos_x; position[1] = light_pos_y; position[2] = light_pos_z; position[3] = 1.0;

    if (!lightContext || !window_mapped) return;

    XMesaMakeCurrent(lightContext);

    glViewport(0, 0, 100, 100);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -5.0);

    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    auxSolidSphere(1.0);

    XMesaSwapBuffers();
}

static void LightScaleCB(Widget w, XtPointer client_data, XtPointer call_data) {
    XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *)call_data;
    int prop = (int)client_data;
    if (prop == 0) light_ambient = (GLfloat)cbs->value / 100.0;
    if (prop == 1) light_diffuse = (GLfloat)cbs->value / 100.0;
    if (prop == 2) light_pos_x = (GLfloat)cbs->value / 10.0;
    if (prop == 3) light_pos_y = (GLfloat)cbs->value / 10.0;
    if (prop == 4) light_pos_z = (GLfloat)cbs->value / 10.0;
    
    if (lightContext && window_mapped && !suppress_redraws) {
        LightExposeCB(NULL, NULL, NULL);
    }
    if (context && window_mapped && !suppress_redraws) { XMesaMakeCurrent(context); RedrawGL(); }
}

static void FileDialogOkCB(Widget w, XtPointer client_data, XtPointer call_data) {
    XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *)call_data;
    char *filename = NULL;
    
    if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &filename)) {
        return;
    }
    
    printf("Loading texture: %s\n", filename);
    if (textureImage) {
        free(textureImage->data);
        free(textureImage);
    }
    
    textureImage = tkRGBImageLoad(filename);
    if (textureImage) {
        XMesaMakeCurrent(context);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, textureImage->sizeX, textureImage->sizeY,
                          GL_RGB, GL_UNSIGNED_BYTE, textureImage->data);
        printf("Texture loaded successfully.\n");
    } else {
        printf("Failed to load texture.\n");
    }
    XtFree(filename);
    XtUnmanageChild(w);
    if (context && window_mapped && !suppress_redraws) { XMesaMakeCurrent(context); RedrawGL(); }
}

static void FileDialogCancelCB(Widget w, XtPointer client_data, XtPointer call_data) {
    XtUnmanageChild(w);
}

static void LoadTextureCB(Widget w, XtPointer client_data, XtPointer call_data) {
    if (!fileDialog) {
        fileDialog = XmCreateFileSelectionDialog(topLevel, "fileDialog", NULL, 0);
        XtAddCallback(fileDialog, XmNokCallback, FileDialogOkCB, NULL);
        XtAddCallback(fileDialog, XmNcancelCallback, FileDialogCancelCB, NULL);
    }
    XtManageChild(fileDialog);
}

static void ResetTextureCB(Widget w, XtPointer client_data, XtPointer call_data) {
    if (textureImage) {
        free(textureImage->data);
        free(textureImage);
        textureImage = NULL;
        makeCheckerImage();
        texture_needs_upload = 1;
        if (context && window_mapped) { XMesaMakeCurrent(context); RedrawGL(); }
    }
}

/* GL Initialization */
static void init_gl(Display *dpy, Window win)
{
    glClearColor(bg_color_value, bg_color_value, bg_color_value, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glPointSize(2.0);
    glLineWidth(2.0);
    glEnable(GL_NORMALIZE);
    
    /* Setup Texture */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    /* Setup Light */
    glEnable(GL_LIGHT0);
    /* Fog is set up in RedrawGL now */
    {
        GLfloat fogColor[4] = {0.2, 0.2, 0.3, 1.0};
        glFogi(GL_FOG_MODE, GL_EXP);
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, 0.05);
        glHint(GL_FOG_HINT, GL_DONT_CARE);
    }
    
    quadObj = gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    gluQuadricTexture(quadObj, GL_TRUE);
}

static void resize_gl(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)width/(GLfloat)height, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

static void apply_materials()
{
    GLfloat mat_ambient[4], mat_diffuse[4], mat_specular[4], shininess;
    
    if (mat_type == 4) { /* Normal */
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glColor3fv(current_color);
        return;
    } else {
        glDisable(GL_COLOR_MATERIAL);
    }
    
    if (mat_type == 0) { /* Brass */
        mat_ambient[0]=0.329412; mat_ambient[1]=0.223529; mat_ambient[2]=0.027451; mat_ambient[3]=1.0;
        mat_diffuse[0]=0.780392; mat_diffuse[1]=0.568627; mat_diffuse[2]=0.113725; mat_diffuse[3]=1.0;
        mat_specular[0]=0.992157; mat_specular[1]=0.941176; mat_specular[2]=0.807843; mat_specular[3]=1.0;
        shininess = 27.8974;
    } else if (mat_type == 1) { /* Chrome */
        mat_ambient[0]=0.25; mat_ambient[1]=0.25; mat_ambient[2]=0.25; mat_ambient[3]=1.0;
        mat_diffuse[0]=0.4; mat_diffuse[1]=0.4; mat_diffuse[2]=0.4; mat_diffuse[3]=1.0;
        mat_specular[0]=0.774597; mat_specular[1]=0.774597; mat_specular[2]=0.774597; mat_specular[3]=1.0;
        shininess = 76.8;
    } else if (mat_type == 2) { /* Emerald */
        mat_ambient[0]=0.0215; mat_ambient[1]=0.1745; mat_ambient[2]=0.0215; mat_ambient[3]=1.0;
        mat_diffuse[0]=0.07568; mat_diffuse[1]=0.61424; mat_diffuse[2]=0.07568; mat_diffuse[3]=1.0;
        mat_specular[0]=0.633; mat_specular[1]=0.727811; mat_specular[2]=0.633; mat_specular[3]=1.0;
        shininess = 76.8;
    } else if (mat_type == 5) { /* Ruby */
        mat_ambient[0]=0.1745; mat_ambient[1]=0.01175; mat_ambient[2]=0.01175; mat_ambient[3]=1.0;
        mat_diffuse[0]=0.61424; mat_diffuse[1]=0.04136; mat_diffuse[2]=0.04136; mat_diffuse[3]=1.0;
        mat_specular[0]=0.727811; mat_specular[1]=0.626959; mat_specular[2]=0.626959; mat_specular[3]=1.0;
        shininess = 76.8;
    } else if (mat_type == 6) { /* Sapphire */
        mat_ambient[0]=0.01175; mat_ambient[1]=0.01175; mat_ambient[2]=0.1745; mat_ambient[3]=1.0;
        mat_diffuse[0]=0.04136; mat_diffuse[1]=0.04136; mat_diffuse[2]=0.61424; mat_diffuse[3]=1.0;
        mat_specular[0]=0.626959; mat_specular[1]=0.626959; mat_specular[2]=0.727811; mat_specular[3]=1.0;
        shininess = 76.8;
    } else { /* Rubber */
        mat_ambient[0]=0.02; mat_ambient[1]=0.02; mat_ambient[2]=0.02; mat_ambient[3]=1.0;
        mat_diffuse[0]=0.01; mat_diffuse[1]=0.01; mat_diffuse[2]=0.01; mat_diffuse[3]=1.0;
        mat_specular[0]=0.4; mat_specular[1]=0.4; mat_specular[2]=0.4; mat_specular[3]=1.0;
        shininess = 10.0;
    }
    
    if (blend_materials) {
        mat_ambient[0] = (mat_ambient[0] + current_color[0]) * 0.5f;
        mat_ambient[1] = (mat_ambient[1] + current_color[1]) * 0.5f;
        mat_ambient[2] = (mat_ambient[2] + current_color[2]) * 0.5f;
        
        mat_diffuse[0] = (mat_diffuse[0] + current_color[0]) * 0.5f;
        mat_diffuse[1] = (mat_diffuse[1] + current_color[1]) * 0.5f;
        mat_diffuse[2] = (mat_diffuse[2] + current_color[2]) * 0.5f;
    }
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    
    glColor3fv(current_color);
}

static void DrawingAreaExposeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (!context) return;
    window_mapped = 1;
    XMesaMakeCurrent(context);
    RedrawGL();
}

static void ColorExposeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (!colorContext) return;
    XMesaMakeCurrent(colorContext);
    glClearColor(current_color[0], current_color[1], current_color[2], 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    XMesaSwapBuffers();
    if (context) XMesaMakeCurrent(context);
}

static void DrawingAreaResizeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *)call_data;
    Dimension width, height;
    if (!context) return;
    XtVaGetValues(w, XmNwidth, &width, XmNheight, &height, NULL);
    XMesaMakeCurrent(context);
    resize_gl(width, height);
    if (window_mapped) RedrawGL();
}

static void RedrawGL(void)
{
    if (!window_mapped) return;

    /* Generate the dynamic checkerboard if we need it */
    if (texturing_enabled && textureImage == NULL) {
        makeCheckerImage();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (lighting_enabled) {
        GLfloat amb[4];
        GLfloat diff[4];
        GLfloat pos[4];
        amb[0] = light_ambient; amb[1] = light_ambient; amb[2] = light_ambient; amb[3] = 1.0;
        diff[0] = light_diffuse; diff[1] = light_diffuse; diff[2] = light_diffuse; diff[3] = 1.0;
        pos[0] = light_pos_x; pos[1] = light_pos_y; pos[2] = light_pos_z; pos[3] = 1.0;
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        
        /* Also push to global light model to ensure a massive effect */
        GLfloat model_amb[4];
        model_amb[0] = light_ambient * 0.5;
        model_amb[1] = light_ambient * 0.5;
        model_amb[2] = light_ambient * 0.5;
        model_amb[3] = 1.0;
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_amb);
        
        glEnable(GL_LIGHTING);
    } else {
        glDisable(GL_LIGHTING);
    }

    /* Enable Evaluator Normals for Teapot */
    glEnable(GL_AUTO_NORMAL);

    if (fog_enabled) glEnable(GL_FOG); else glDisable(GL_FOG);
    if (texturing_enabled) {
        glEnable(GL_TEXTURE_2D);
        if (texture_needs_upload) {
            if (textureImage) {
                glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage->sizeX, textureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage->data);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, 3, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, checkerImage);
            }
            texture_needs_upload = 0;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    } else {
        glDisable(GL_TEXTURE_2D);
    }
    if (smooth_shading) glShadeModel(GL_SMOOTH); else glShadeModel(GL_FLAT);
    
    glPushMatrix();
    glTranslatef(0.0, 0.0, -5.0);
    glScalef(zoom_level, zoom_level, zoom_level);
    glRotatef(xrot, 1.0, 0.0, 0.0);
    glRotatef(yrot, 0.0, 1.0, 0.0);
    glRotatef(zrot, 0.0, 0.0, 1.0);
    
    apply_materials();
    
    if (poly_mode == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if (poly_mode == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    if (texturing_enabled && texture_wrap_mode > 0) {
        if (texture_wrap_mode == 1) { /* Linear */
            GLfloat planes_s[] = {1.0f, 0.0f, 0.0f, 0.0f};
            GLfloat planes_t[] = {0.0f, 1.0f, 0.0f, 0.0f};
            if (current_shape == 4) { /* Teapot gets scaled linear */
                planes_s[0] = 0.166666f; planes_s[3] = 0.5f;
                planes_t[1] = 0.166666f; planes_t[3] = 0.5f;
            }
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGenfv(GL_S, GL_OBJECT_PLANE, planes_s);
            glTexGenfv(GL_T, GL_OBJECT_PLANE, planes_t);
        } else if (texture_wrap_mode == 2) { /* Sphere */
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        }
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
    }
    
    switch (current_shape) {
        case 0: gluSphere(quadObj, 1.0, 32, 32); break;
        case 1: auxSolidCube(1.5); break;
        case 2: glTranslatef(0.0, 0.0, -0.75); gluCylinder(quadObj, 0.5, 0.5, 1.5, 32, 16); break;
        case 3: auxSolidTorus(0.3, 0.8); break;
        case 4: auxSolidTeapot(1.0); break;
        case 5: auxSolidDodecahedron(1.0); break;
        case 6: auxSolidIcosahedron(1.0); break;
        case 7: auxSolidOctahedron(1.0); break;
        case 8: auxSolidTetrahedron(1.0); break;
        case 9: glTranslatef(0.0, 0.0, -0.75); gluCylinder(quadObj, 1.0, 0.0, 1.5, 32, 16); break; /* Cone */
    }

    if (texturing_enabled && texture_wrap_mode > 0) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }
    
    glPopMatrix();
    
    XMesaSwapBuffers();
}

static void TransformCB(Widget w, XtPointer client_data, XtPointer call_data) {
    XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *)call_data;
    int axis = (int)client_data;
    if (auto_spin) {
        if (axis == 0) xrot_speed = (GLfloat)cbs->value / 36.0;
        if (axis == 1) yrot_speed = (GLfloat)cbs->value / 36.0;
        if (axis == 2) zrot_speed = (GLfloat)cbs->value / 36.0;
    } else {
        if (axis == 0) xrot = (GLfloat)cbs->value;
        if (axis == 1) yrot = (GLfloat)cbs->value;
        if (axis == 2) zrot = (GLfloat)cbs->value;
    }
    if (axis == 3) zoom_level = (GLfloat)cbs->value / 100.0;
    
    if (context && window_mapped && !suppress_redraws) {
        XMesaMakeCurrent(context);
        RedrawGL();
    }
}

static void CenterModelCB(Widget w, XtPointer client_data, XtPointer call_data) {
    xrot = 0.0;
    yrot = 0.0;
    zrot = 0.0;
    xrot_speed = 0.0;
    yrot_speed = 0.0;
    zrot_speed = 0.0;
    zoom_level = 1.0;
    
    suppress_redraws = 1;
    if (scaleX_w) XmScaleSetValue(scaleX_w, 0);
    if (scaleY_w) XmScaleSetValue(scaleY_w, 0);
    if (scaleZ_w) XmScaleSetValue(scaleZ_w, 0);
    if (zoomScale) XmScaleSetValue(zoomScale, 100);
    suppress_redraws = 0;
    
    if (context && window_mapped && !suppress_redraws) {
        XMesaMakeCurrent(context);
        RedrawGL();
    }
}

static Boolean IdleFunc(XtPointer client_data)
{
    int redraw_needed = 0;
    
    if (rainbow_mode) {
        frame_counter++;
        float r = sin(frame_counter * 0.05f) * 0.5f + 0.5f;
        float g = sin(frame_counter * 0.05f + 2.094f) * 0.5f + 0.5f;
        float b = sin(frame_counter * 0.05f + 4.188f) * 0.5f + 0.5f;
        
        current_color[0] = r;
        current_color[1] = g;
        current_color[2] = b;
        
        suppress_redraws = 1;
        if (scaleR) XmScaleSetValue(scaleR, (int)(r * 100.0f));
        if (scaleG) XmScaleSetValue(scaleG, (int)(g * 100.0f));
        if (scaleB) XmScaleSetValue(scaleB, (int)(b * 100.0f));
        suppress_redraws = 0;
        
        if (colorContext && window_mapped) {
            XMesaMakeCurrent(colorContext);
            glClearColor(r, g, b, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
            XMesaSwapBuffers();
        }
        redraw_needed = 1;
    }

    if (auto_spin) {
        xrot += xrot_speed;
        yrot += yrot_speed;
        zrot += zrot_speed;
        
        if (xrot >= 360.0) xrot -= 360.0; else if (xrot < 0.0) xrot += 360.0;
        if (yrot >= 360.0) yrot -= 360.0; else if (yrot < 0.0) yrot += 360.0;
        if (zrot >= 360.0) zrot -= 360.0; else if (zrot < 0.0) zrot += 360.0;
        
        if (xrot_speed != 0.0 || yrot_speed != 0.0 || zrot_speed != 0.0) {
            redraw_needed = 1;
        }
    }
    
    if (redraw_needed && context && window_mapped) {
        XMesaMakeCurrent(context);
        RedrawGL();
    }
    return False;
}

int main(int argc, char **argv)
{
    XtAppContext app;
    Widget mainForm, drawArea, leftPane;
    Widget colorLabel, lightLabel;
    Display *dpy;
    
    int scr;
    Colormap cmap;

    topLevel = XtVaAppInitialize(&app, "X11 RenderBench", NULL, 0,
                                 &argc, argv, NULL, 
                                 XmNwidth, 800, XmNheight, 625,
                                 NULL);
    dpy = XtDisplay(topLevel);
    scr = DefaultScreen(dpy);
    
    XFontStruct *fs = XLoadQueryFont(dpy, "fixed");
    if (fs) global_font = XmFontListCreate(fs, XmSTRING_DEFAULT_CHARSET);
    
    visinfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));
    if (!XMatchVisualInfo(dpy, scr, 24, TrueColor, visinfo)) {
        if (!XMatchVisualInfo(dpy, scr, 8, PseudoColor, visinfo)) {
            fprintf(stderr, "Could not get 24-bit TrueColor or 8-bit PseudoColor visual for RGB mode.\n");
            exit(1);
        }
    }
    
    if (visinfo->visual == DefaultVisual(dpy, scr)) {
        cmap = DefaultColormap(dpy, scr);
    } else {
        cmap = XCreateColormap(dpy, RootWindow(dpy, scr), visinfo->visual, AllocNone);
    }
    
    XtVaSetValues(topLevel,
                  XmNvisual, visinfo->visual,
                  XmNdepth, visinfo->depth,
                  XmNcolormap, cmap,
                  NULL);

    mainForm = XtVaCreateManagedWidget("mainForm", xmFormWidgetClass, topLevel, NULL);
    
    /* Left Pane Form */
    {
        leftPane = XtVaCreateManagedWidget("leftPane", xmFormWidgetClass, mainForm,
            XmNtopAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNleftAttachment, XmATTACH_FORM, NULL);
            
        /* Bottom Area container for LightColor and Background */
        Widget bottomArea = XtVaCreateManagedWidget("bottomArea", xmRowColumnWidgetClass, leftPane,
            XmNorientation, XmVERTICAL, XmNspacing, 5, XmNmarginWidth, 0, XmNmarginHeight, 0,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM, NULL);
            
        /* Light/Color (Bottom) - 4 Column Layout */
        Widget lightColorRC = XtVaCreateManagedWidget("lightColorRC", xmRowColumnWidgetClass, bottomArea,
            XmNorientation, XmHORIZONTAL, XmNspacing, 2, XmNmarginWidth, 2, NULL);
            
        /* Column 1: Lighting */
        {
            Widget lightCol = XtVaCreateManagedWidget("lightCol", xmRowColumnWidgetClass, lightColorRC, XmNorientation, XmVERTICAL, XmNspacing, 2, NULL);
            XmString str = XmStringCreateLtoR("Lighting:", XmSTRING_DEFAULT_CHARSET);
            lightLabel = XtVaCreateManagedWidget("lightLabel", xmLabelWidgetClass, lightCol, XmNfontList, global_font, XmNlabelString, str, NULL);
            XmStringFree(str);
            
            scaleLA = XtVaCreateManagedWidget("Boost Ambient", xmScaleWidgetClass, lightCol, XmNfontList, global_font,
                                             XtVaTypedArg, XmNtitleString, XmRString, "Ambient", 8,
                                             XmNminimum, 0, XmNmaximum, 300, XmNvalue, 20, XmNshowValue, False, XmNscaleHeight, 20, XmNorientation, XmHORIZONTAL, XmNwidth, 100, NULL);
            XtAddCallback(scaleLA, XmNvalueChangedCallback, LightScaleCB, (XtPointer)0);
            XtAddCallback(scaleLA, XmNdragCallback, LightScaleCB, (XtPointer)0);
            
            scalePX = XtVaCreateManagedWidget("X Move", xmScaleWidgetClass, lightCol, XmNfontList, global_font,
                                             XtVaTypedArg, XmNtitleString, XmRString, "X Move", 7,
                                             XmNminimum, -500, XmNmaximum, 500, XmNvalue, 50, XmNshowValue, False, XmNscaleHeight, 20, XmNorientation, XmHORIZONTAL, XmNwidth, 100, NULL);
            XtAddCallback(scalePX, XmNvalueChangedCallback, LightScaleCB, (XtPointer)2);
            XtAddCallback(scalePX, XmNdragCallback, LightScaleCB, (XtPointer)2);
            
            scalePY = XtVaCreateManagedWidget("Y Move", xmScaleWidgetClass, lightCol, XmNfontList, global_font,
                                             XtVaTypedArg, XmNtitleString, XmRString, "Y Move", 7,
                                             XmNminimum, -500, XmNmaximum, 500, XmNvalue, 50, XmNshowValue, False, XmNscaleHeight, 20, XmNorientation, XmHORIZONTAL, XmNwidth, 100, NULL);
            XtAddCallback(scalePY, XmNvalueChangedCallback, LightScaleCB, (XtPointer)3);
            XtAddCallback(scalePY, XmNdragCallback, LightScaleCB, (XtPointer)3);
            
            scalePZ = XtVaCreateManagedWidget("Z Move", xmScaleWidgetClass, lightCol, XmNfontList, global_font,
                                             XtVaTypedArg, XmNtitleString, XmRString, "Z Move", 7,
                                             XmNminimum, -500, XmNmaximum, 500, XmNvalue, 50, XmNshowValue, False, XmNscaleHeight, 20, XmNorientation, XmHORIZONTAL, XmNwidth, 100, NULL);
            XtAddCallback(scalePZ, XmNvalueChangedCallback, LightScaleCB, (XtPointer)4);
            XtAddCallback(scalePZ, XmNdragCallback, LightScaleCB, (XtPointer)4);
            
            scaleLD = XtVaCreateManagedWidget("Light Strength", xmScaleWidgetClass, lightCol, XmNfontList, global_font,
                                             XtVaTypedArg, XmNtitleString, XmRString, "Strength", 9,
                                             XmNminimum, 0, XmNmaximum, 100, XmNvalue, 100, XmNshowValue, False, XmNscaleHeight, 20, XmNorientation, XmHORIZONTAL, XmNwidth, 100, NULL);
            XtAddCallback(scaleLD, XmNvalueChangedCallback, LightScaleCB, (XtPointer)1);
            XtAddCallback(scaleLD, XmNdragCallback, LightScaleCB, (XtPointer)1);
            
        }
        
        /* Middle Area: Unifies Reset Col and Mat Col to guarantee preview alignment */
        Widget middleArea = XtVaCreateManagedWidget("middleArea", xmRowColumnWidgetClass, lightColorRC, 
            XmNorientation, XmVERTICAL, XmNspacing, 5, NULL);
            
        Widget controlsRC = XtVaCreateManagedWidget("controlsRC", xmRowColumnWidgetClass, middleArea, 
            XmNorientation, XmHORIZONTAL, XmNspacing, 10, NULL);
            
        /* Column 2: Reset Buttons */
        {
            Widget resetCol = XtVaCreateManagedWidget("resetCol", xmRowColumnWidgetClass, controlsRC, XmNorientation, XmVERTICAL, XmNspacing, 10, NULL);
            
            /* Spacer to push buttons exactly to the middle */
            XmString strSpace = XmStringCreateLtoR(" ", XmSTRING_DEFAULT_CHARSET);
            Widget sp = XtVaCreateManagedWidget("spacer", xmLabelWidgetClass, resetCol, XmNfontList, global_font, XmNlabelString, strSpace, NULL);
            XtVaSetValues(sp, XmNheight, 40, NULL);
            XmStringFree(strSpace);
            
            Widget btnResetLight = XtVaCreateManagedWidget("Reset Light", xmPushButtonWidgetClass, resetCol, XmNfontList, global_font, XmNwidth, 60, NULL);
            XtAddCallback(btnResetLight, XmNactivateCallback, ResetLightCB, NULL);
            
            Widget btnResetColor = XtVaCreateManagedWidget("Reset RGB", xmPushButtonWidgetClass, resetCol, XmNfontList, global_font, XmNwidth, 60, NULL);
            XtAddCallback(btnResetColor, XmNactivateCallback, ResetColorCB, NULL);
            
        }
        
        /* Column 3: Materials */
        {
            Widget matCol = XtVaCreateManagedWidget("matCol", xmRowColumnWidgetClass, controlsRC, XmNorientation, XmVERTICAL, XmNspacing, 2, NULL);
            XmString str = XmStringCreateLtoR("Materials RGB:", XmSTRING_DEFAULT_CHARSET);
            colorLabel = XtVaCreateManagedWidget("colorLabel", xmLabelWidgetClass, matCol, XmNfontList, global_font, XmNlabelString, str, NULL);
            XmStringFree(str);
            
            Widget rcR = XtVaCreateManagedWidget("rcR", xmRowColumnWidgetClass, matCol, XmNorientation, XmHORIZONTAL, XmNspacing, 2, XmNmarginHeight, 0, NULL);
            XmString strR = XmStringCreateLtoR("R:", XmSTRING_DEFAULT_CHARSET);
            XtVaCreateManagedWidget("lblR", xmLabelWidgetClass, rcR, XmNlabelString, strR, XmNfontList, global_font, XmNwidth, 20, NULL);
            XmStringFree(strR);
            scaleR = XtVaCreateManagedWidget("Red", xmScaleWidgetClass, rcR, XmNfontList, global_font,
                                             XmNminimum, 0, XmNmaximum, 100, XmNvalue, 100, XmNshowValue, False, XmNscaleHeight, 20, XmNorientation, XmHORIZONTAL, XmNwidth, 76, NULL);
            XtAddCallback(scaleR, XmNvalueChangedCallback, ColorScaleCB, (XtPointer)0);
            XtAddCallback(scaleR, XmNdragCallback, ColorScaleCB, (XtPointer)0);
            
            Widget rcG = XtVaCreateManagedWidget("rcG", xmRowColumnWidgetClass, matCol, XmNorientation, XmHORIZONTAL, XmNspacing, 2, XmNmarginHeight, 0, NULL);
            XmString strG = XmStringCreateLtoR("G:", XmSTRING_DEFAULT_CHARSET);
            XtVaCreateManagedWidget("lblG", xmLabelWidgetClass, rcG, XmNlabelString, strG, XmNfontList, global_font, XmNwidth, 20, NULL);
            XmStringFree(strG);
            scaleG = XtVaCreateManagedWidget("Green", xmScaleWidgetClass, rcG, XmNfontList, global_font,
                                             XmNminimum, 0, XmNmaximum, 100, XmNvalue, 100, XmNshowValue, False, XmNscaleHeight, 20, XmNorientation, XmHORIZONTAL, XmNwidth, 76, NULL);
            XtAddCallback(scaleG, XmNvalueChangedCallback, ColorScaleCB, (XtPointer)1);
            XtAddCallback(scaleG, XmNdragCallback, ColorScaleCB, (XtPointer)1);
            
            Widget rcB = XtVaCreateManagedWidget("rcB", xmRowColumnWidgetClass, matCol, XmNorientation, XmHORIZONTAL, XmNspacing, 2, XmNmarginHeight, 0, NULL);
            XmString strB = XmStringCreateLtoR("B:", XmSTRING_DEFAULT_CHARSET);
            XtVaCreateManagedWidget("lblB", xmLabelWidgetClass, rcB, XmNlabelString, strB, XmNfontList, global_font, XmNwidth, 20, NULL);
            XmStringFree(strB);
            scaleB = XtVaCreateManagedWidget("Blue", xmScaleWidgetClass, rcB, XmNfontList, global_font,
                                             XmNminimum, 0, XmNmaximum, 100, XmNvalue, 100, XmNshowValue, False, XmNscaleHeight, 20, XmNorientation, XmHORIZONTAL, XmNwidth, 76, NULL);
            XtAddCallback(scaleB, XmNvalueChangedCallback, ColorScaleCB, (XtPointer)2);
            XtAddCallback(scaleB, XmNdragCallback, ColorScaleCB, (XtPointer)2);
            
            Widget toggleRainbow = XtVaCreateManagedWidget("Rainbow Mode", xmToggleButtonWidgetClass, matCol, XmNfontList, global_font, XmNset, False, XmNwidth, 100, XmNrecomputeSize, False, NULL);
            XtAddCallback(toggleRainbow, XmNvalueChangedCallback, ToggleCB, (XtPointer)13);
            
        }
        
        /* Unified Previews Container */
        Widget previewsRC = XtVaCreateManagedWidget("previewsRC", xmRowColumnWidgetClass, middleArea,
            XmNorientation, XmHORIZONTAL, XmNspacing, 15, NULL);
            
        lightPreviewArea = XtVaCreateManagedWidget("lightPreviewArea", xmDrawingAreaWidgetClass, previewsRC,
                                                   XmNwidth, 100, XmNheight, 100, NULL);
        XtAddCallback(lightPreviewArea, XmNexposeCallback, LightExposeCB, NULL);
        
        colorPreviewArea = XtVaCreateManagedWidget("colorPreviewArea", xmDrawingAreaWidgetClass, previewsRC,
                                                   XmNwidth, 100, XmNheight, 100, NULL);
        XtAddCallback(colorPreviewArea, XmNexposeCallback, ColorExposeCB, NULL);
        
        /* Column 4: TexGen */
        {
            Widget texCol = XtVaCreateManagedWidget("texCol", xmRowColumnWidgetClass, lightColorRC, XmNorientation, XmVERTICAL, XmNspacing, 4, NULL);
            XmString str = XmStringCreateLtoR("TexGen:", XmSTRING_DEFAULT_CHARSET);
            XtVaCreateManagedWidget("texLabel", xmLabelWidgetClass, texCol, XmNfontList, global_font, XmNlabelString, str, NULL);
            XmStringFree(str);
            
            Widget btnChecker = XtVaCreateManagedWidget("Clouds", xmPushButtonWidgetClass, texCol, XmNfontList, global_font, XmNwidth, 120, NULL);
            XtAddCallback(btnChecker, XmNactivateCallback, TexGenCB, (XtPointer)0);
            
            Widget btnWood = XtVaCreateManagedWidget("Wood Grain", xmPushButtonWidgetClass, texCol, XmNfontList, global_font, XmNwidth, 120, NULL);
            XtAddCallback(btnWood, XmNactivateCallback, TexGenCB, (XtPointer)1);
            
            Widget btnMarble = XtVaCreateManagedWidget("Marble Swirl", xmPushButtonWidgetClass, texCol, XmNfontList, global_font, XmNwidth, 120, NULL);
            XtAddCallback(btnMarble, XmNactivateCallback, TexGenCB, (XtPointer)2);
            
            Widget btnVoronoi = XtVaCreateManagedWidget("Voronoi Cells", xmPushButtonWidgetClass, texCol, XmNfontList, global_font, XmNwidth, 120, NULL);
            XtAddCallback(btnVoronoi, XmNactivateCallback, TexGenCB, (XtPointer)3);
            
            Widget btnRainbow = XtVaCreateManagedWidget("Psych Rainbow", xmPushButtonWidgetClass, texCol, XmNfontList, global_font, XmNwidth, 120, NULL);
            XtAddCallback(btnRainbow, XmNactivateCallback, TexGenCB, (XtPointer)4);
            
            Widget btnCircuit = XtVaCreateManagedWidget("Circuit Board", xmPushButtonWidgetClass, texCol, XmNfontList, global_font, XmNwidth, 120, NULL);
            XtAddCallback(btnCircuit, XmNactivateCallback, TexGenCB, (XtPointer)5);
            
            Widget btnCamo = XtVaCreateManagedWidget("Camouflage", xmPushButtonWidgetClass, texCol, XmNfontList, global_font, XmNwidth, 120, NULL);
            XtAddCallback(btnCamo, XmNactivateCallback, TexGenCB, (XtPointer)6);
        }
        
        /* BACKGROUND COLOR SECTION (Moved from Right Pane) */
        {
            Widget bgRC = XtVaCreateManagedWidget("bgRC", xmRowColumnWidgetClass, bottomArea,
                XmNorientation, XmHORIZONTAL, XmNspacing, 6, XmNmarginWidth, 10, XmNmarginHeight, 5, NULL);
            
            XmString strBg = XmStringCreateLtoR("Background Color:", XmSTRING_DEFAULT_CHARSET);
            XtVaCreateManagedWidget("lblBg", xmLabelWidgetClass, bgRC, XmNlabelString, strBg, XmNfontList, global_font, NULL);
            XmStringFree(strBg);
            
            Widget btnBgBlack = XtVaCreateManagedWidget("Black", xmPushButtonWidgetClass, bgRC, XmNfontList, global_font, XmNwidth, 75, XmNrecomputeSize, False, NULL);
            XtAddCallback(btnBgBlack, XmNactivateCallback, BgColorCB, (XtPointer)0);
            
            Widget btnBgGrey = XtVaCreateManagedWidget("50% Grey", xmPushButtonWidgetClass, bgRC, XmNfontList, global_font, XmNwidth, 75, XmNrecomputeSize, False, NULL);
            XtAddCallback(btnBgGrey, XmNactivateCallback, BgColorCB, (XtPointer)50);
            
            Widget btnBgGrey75 = XtVaCreateManagedWidget("75% Grey", xmPushButtonWidgetClass, bgRC, XmNfontList, global_font, XmNwidth, 75, XmNrecomputeSize, False, NULL);
            XtAddCallback(btnBgGrey75, XmNactivateCallback, BgColorCB, (XtPointer)75);
            
            Widget btnBgWhite = XtVaCreateManagedWidget("White", xmPushButtonWidgetClass, bgRC, XmNfontList, global_font, XmNwidth, 75, XmNrecomputeSize, False, NULL);
            XtAddCallback(btnBgWhite, XmNactivateCallback, BgColorCB, (XtPointer)100);
        }
            
        /* Draw Area (Top) */
        drawArea = XtVaCreateManagedWidget("drawArea", xmDrawingAreaWidgetClass, leftPane,
            XmNtopAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_WIDGET, XmNbottomWidget, bottomArea,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);
        XtAddCallback(drawArea, XmNexposeCallback, DrawingAreaExposeCB, NULL);
        XtAddCallback(drawArea, XmNresizeCallback, DrawingAreaResizeCB, NULL);
        gl_widget = drawArea;
    }
    
    /* Right Pane (Grid) */
    Widget rightPaneScroll = XtVaCreateManagedWidget("rightPaneScroll", xmScrolledWindowWidgetClass, mainForm,
        XmNtopAttachment, XmATTACH_FORM, XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget, leftPane,
        XmNrightAttachment, XmATTACH_FORM,
        XmNwidth, 335,
        XmNscrollingPolicy, XmAUTOMATIC, NULL);
        
    Widget rightPaneRC = XtVaCreateManagedWidget("rightPaneRC", xmRowColumnWidgetClass, rightPaneScroll,
        XmNorientation, XmVERTICAL, XmNspacing, 2, XmNmarginHeight, 2, NULL);
        
    /* TOP SECTION 1 */
    Widget topRC1 = XtVaCreateManagedWidget("topRC1", xmRowColumnWidgetClass, rightPaneRC,
        XmNorientation, XmHORIZONTAL, XmNspacing, 4, XmNmarginHeight, 0, NULL);
        
    Widget leftCol1 = XtVaCreateManagedWidget("leftCol1", xmRowColumnWidgetClass, topRC1, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
    Widget rightCol1 = XtVaCreateManagedWidget("rightCol1", xmRowColumnWidgetClass, topRC1, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
    
    /* [Left Column 1] GL States */
    {
        XmString str = XmStringCreateLtoR("GL States:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblStates", xmLabelWidgetClass, leftCol1, XmNfontList, global_font, XmNfontList, global_font, XmNlabelString, str, NULL);
        XmStringFree(str);
        
        toggleLight = XtVaCreateManagedWidget("Enable Lighting", xmToggleButtonWidgetClass, leftCol1, XmNfontList, global_font, XmNfontList, global_font, XmNset, True, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(toggleLight, XmNvalueChangedCallback, ToggleCB, (XtPointer)1);
        
        toggleFog = XtVaCreateManagedWidget("Enable Fog", xmToggleButtonWidgetClass, leftCol1, XmNfontList, global_font, XmNfontList, global_font, XmNset, False, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(toggleFog, XmNvalueChangedCallback, ToggleCB, (XtPointer)2);
        
        toggleTex = XtVaCreateManagedWidget("Enable Texture", xmToggleButtonWidgetClass, leftCol1, XmNfontList, global_font, XmNfontList, global_font, XmNset, False, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(toggleTex, XmNvalueChangedCallback, ToggleCB, (XtPointer)3);
    }
    
    /* [Right Column 1] Texture Wrap */
    {
        XmString strWrap = XmStringCreateLtoR("Texture Wrap:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblWrap", xmLabelWidgetClass, rightCol1, XmNfontList, global_font, XmNfontList, global_font, XmNlabelString, strWrap, NULL);
        XmStringFree(strWrap);
        
        Widget radioWrapBox = XmCreateRadioBox(rightCol1, "radioWrapBox", NULL, 0);
        XtVaSetValues(radioWrapBox, XmNmarginHeight, 0, XmNmarginWidth, 0, XmNspacing, 1, NULL);
        XtManageChild(radioWrapBox);
        
        Widget toggleNative = XtVaCreateManagedWidget("Native UVs", xmToggleButtonWidgetClass, radioWrapBox, XmNfontList, global_font, XmNfontList, global_font, XmNset, True, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(toggleNative, XmNvalueChangedCallback, TextureWrapCB, (XtPointer)0);
        
        Widget toggleLinear = XtVaCreateManagedWidget("Linear Project", xmToggleButtonWidgetClass, radioWrapBox, XmNfontList, global_font, XmNfontList, global_font, XmNset, False, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(toggleLinear, XmNvalueChangedCallback, TextureWrapCB, (XtPointer)1);
        
        Widget toggleSphere = XtVaCreateManagedWidget("Sphere Map", xmToggleButtonWidgetClass, radioWrapBox, XmNfontList, global_font, XmNfontList, global_font, XmNset, False, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(toggleSphere, XmNvalueChangedCallback, TextureWrapCB, (XtPointer)2);
    }

    XtVaCreateManagedWidget("sep1", xmSeparatorWidgetClass, rightPaneRC, NULL);
    
    /* TOP SECTION 2 */
    Widget topRC2 = XtVaCreateManagedWidget("topRC2", xmRowColumnWidgetClass, rightPaneRC,
        XmNorientation, XmHORIZONTAL, XmNspacing, 4, XmNmarginHeight, 0, NULL);
        
    Widget leftCol2 = XtVaCreateManagedWidget("leftCol2", xmRowColumnWidgetClass, topRC2, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
    Widget rightCol2 = XtVaCreateManagedWidget("rightCol2", xmRowColumnWidgetClass, topRC2, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
    
    /* [Left Column 2] Texture Load */
    {
        XmString str = XmStringCreateLtoR("Texture Load:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblTexLoad", xmLabelWidgetClass, leftCol2, XmNfontList, global_font, XmNlabelString, str, NULL);
        XmStringFree(str);
        
        Widget btnLoadTex = XtVaCreateManagedWidget("Load Texture...", xmPushButtonWidgetClass, leftCol2, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(btnLoadTex, XmNactivateCallback, LoadTextureCB, NULL);
    }
    
    /* [Right Column 2] Reset Texture */
    {
        XmString strSpace = XmStringCreateLtoR(" ", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblTexSpacer", xmLabelWidgetClass, rightCol2, XmNfontList, global_font, XmNlabelString, strSpace, NULL);
        XmStringFree(strSpace);

        Widget resetTexBtn = XtVaCreateManagedWidget("Reset Texture", xmPushButtonWidgetClass, rightCol2, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(resetTexBtn, XmNactivateCallback, ResetTextureCB, NULL);
    }

    XtVaCreateManagedWidget("sep2", xmSeparatorWidgetClass, rightPaneRC, NULL);
    
    /* TOP SECTION 3 */
    Widget topRC3 = XtVaCreateManagedWidget("topRC3", xmRowColumnWidgetClass, rightPaneRC,
        XmNorientation, XmHORIZONTAL, XmNspacing, 4, XmNmarginHeight, 0, NULL);
        
    Widget leftCol3 = XtVaCreateManagedWidget("leftCol3", xmRowColumnWidgetClass, topRC3, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
    Widget rightCol3 = XtVaCreateManagedWidget("rightCol3", xmRowColumnWidgetClass, topRC3, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
    
    /* [Left Column 3] Render Mode */
    {
        XmString str = XmStringCreateLtoR("Render Mode:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblModes", xmLabelWidgetClass, leftCol3, XmNfontList, global_font, XmNlabelString, str, NULL);
        XmStringFree(str);
        
        Widget toggleSmooth = XtVaCreateManagedWidget("Smooth Shade", xmToggleButtonWidgetClass, leftCol3, XmNfontList, global_font, XmNset, True, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(toggleSmooth, XmNvalueChangedCallback, ToggleCB, (XtPointer)4);
        
        Widget toggleBlend = XtVaCreateManagedWidget("Blend Mats", xmToggleButtonWidgetClass, leftCol3, XmNfontList, global_font, XmNset, False, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(toggleBlend, XmNvalueChangedCallback, ToggleCB, (XtPointer)5);

        Widget rcModes = XtVaCreateManagedWidget("rcModes", xmRowColumnWidgetClass, leftCol3, 
            XmNorientation, XmVERTICAL, XmNradioBehavior, True, 
            XmNmarginHeight, 0, XmNmarginWidth, 0, XmNspacing, 1, NULL);
        
        Widget polyFill = XtVaCreateManagedWidget("Solid", xmToggleButtonWidgetClass, rcModes, XmNfontList, global_font, XmNset, True, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(polyFill, XmNvalueChangedCallback, ToggleCB, (XtPointer)12);
        Widget polyLine = XtVaCreateManagedWidget("Wireframe", xmToggleButtonWidgetClass, rcModes, XmNfontList, global_font, XmNset, False, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(polyLine, XmNvalueChangedCallback, ToggleCB, (XtPointer)11);
        Widget polyPoint = XtVaCreateManagedWidget("Dots", xmToggleButtonWidgetClass, rcModes, XmNfontList, global_font, XmNset, False, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(polyPoint, XmNvalueChangedCallback, ToggleCB, (XtPointer)10);
    }
    
    /* [Right Column 3] Materials */
    {
        XmString str = XmStringCreateLtoR("Materials:", XmSTRING_DEFAULT_CHARSET);
        Widget matLabel = XtVaCreateManagedWidget("matLabel", xmLabelWidgetClass, rightCol3, XmNfontList, global_font, XmNlabelString, str, NULL);
        XmStringFree(str);

        Widget b0 = XtVaCreateManagedWidget("Brass", xmPushButtonWidgetClass, rightCol3, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b0, XmNactivateCallback, MaterialSelectCB, (XtPointer)0);
        Widget b1 = XtVaCreateManagedWidget("Chrome", xmPushButtonWidgetClass, rightCol3, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b1, XmNactivateCallback, MaterialSelectCB, (XtPointer)1);
        Widget gemRC = XtVaCreateManagedWidget("gemRC", xmRowColumnWidgetClass, rightCol3, XmNorientation, XmHORIZONTAL, XmNspacing, 3, XmNmarginHeight, 0, XmNmarginWidth, 0, NULL);
        Widget b2 = XtVaCreateManagedWidget("Emerald", xmPushButtonWidgetClass, gemRC, XmNfontList, global_font, XmNwidth, 50, XmNrecomputeSize, False, NULL); XtAddCallback(b2, XmNactivateCallback, MaterialSelectCB, (XtPointer)2);
        Widget bRuby = XtVaCreateManagedWidget("Ruby", xmPushButtonWidgetClass, gemRC, XmNfontList, global_font, XmNwidth, 44, XmNrecomputeSize, False, NULL); XtAddCallback(bRuby, XmNactivateCallback, MaterialSelectCB, (XtPointer)5);
        Widget bSapph = XtVaCreateManagedWidget("Sapphire", xmPushButtonWidgetClass, gemRC, XmNfontList, global_font, XmNwidth, 55, XmNrecomputeSize, False, NULL); XtAddCallback(bSapph, XmNactivateCallback, MaterialSelectCB, (XtPointer)6);
        Widget b3 = XtVaCreateManagedWidget("Rubber", xmPushButtonWidgetClass, rightCol3, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b3, XmNactivateCallback, MaterialSelectCB, (XtPointer)3);
        Widget b4 = XtVaCreateManagedWidget("Normal", xmPushButtonWidgetClass, rightCol3, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b4, XmNactivateCallback, MaterialSelectCB, (XtPointer)4);
    }
    
    XtVaCreateManagedWidget("sepTop", xmSeparatorWidgetClass, rightPaneRC, NULL);

    /* SHAPES SECTION */
    {
        XmString str = XmStringCreateLtoR("Shapes:", XmSTRING_DEFAULT_CHARSET);
        Widget shapeLabel = XtVaCreateManagedWidget("shapeLabel", xmLabelWidgetClass, rightPaneRC, XmNfontList, global_font, XmNfontList, global_font, XmNlabelString, str, NULL);
        XmStringFree(str);
        
        Widget shapesSectionRC = XtVaCreateManagedWidget("shapesSectionRC", xmRowColumnWidgetClass, rightPaneRC,
            XmNorientation, XmHORIZONTAL, XmNspacing, 4, XmNmarginHeight, 0, NULL);

        Widget shapesLeftCol = XtVaCreateManagedWidget("shapesLeftCol", xmRowColumnWidgetClass, shapesSectionRC, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
        Widget shapesRightCol = XtVaCreateManagedWidget("shapesRightCol", xmRowColumnWidgetClass, shapesSectionRC, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
        
        Widget rcL = shapesLeftCol;
        Widget b0 = XtVaCreateManagedWidget("Sphere", xmPushButtonWidgetClass, rcL, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b0, XmNactivateCallback, ShapeSelectCB, (XtPointer)0);
        Widget b1 = XtVaCreateManagedWidget("Cube", xmPushButtonWidgetClass, rcL, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b1, XmNactivateCallback, ShapeSelectCB, (XtPointer)1);
        Widget b2 = XtVaCreateManagedWidget("Cylinder", xmPushButtonWidgetClass, rcL, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b2, XmNactivateCallback, ShapeSelectCB, (XtPointer)2);
        Widget b3 = XtVaCreateManagedWidget("Torus", xmPushButtonWidgetClass, rcL, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b3, XmNactivateCallback, ShapeSelectCB, (XtPointer)3);
        Widget b4 = XtVaCreateManagedWidget("Teapot", xmPushButtonWidgetClass, rcL, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b4, XmNactivateCallback, ShapeSelectCB, (XtPointer)4);

        Widget rcR = shapesRightCol;
        Widget b5 = XtVaCreateManagedWidget("Dodecahedron", xmPushButtonWidgetClass, rcR, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b5, XmNactivateCallback, ShapeSelectCB, (XtPointer)5);
        Widget b6 = XtVaCreateManagedWidget("Icosahedron", xmPushButtonWidgetClass, rcR, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b6, XmNactivateCallback, ShapeSelectCB, (XtPointer)6);
        Widget b7 = XtVaCreateManagedWidget("Octahedron", xmPushButtonWidgetClass, rcR, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b7, XmNactivateCallback, ShapeSelectCB, (XtPointer)7);
        Widget b8 = XtVaCreateManagedWidget("Tetrahedron", xmPushButtonWidgetClass, rcR, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b8, XmNactivateCallback, ShapeSelectCB, (XtPointer)8);
        Widget b9 = XtVaCreateManagedWidget("Cone", xmPushButtonWidgetClass, rcR, XmNfontList, global_font, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL); XtAddCallback(b9, XmNactivateCallback, ShapeSelectCB, (XtPointer)9);
    }
    
    XtVaCreateManagedWidget("sepShapes", xmSeparatorWidgetClass, rightPaneRC, NULL);

    /* ORIENTATION SECTION */
    {
        XmString strOrient = XmStringCreateLtoR("Orientation:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblOrient", xmLabelWidgetClass, rightPaneRC, XmNfontList, global_font, XmNlabelString, strOrient, NULL);
        XmStringFree(strOrient);

        Widget orientSectionRC = XtVaCreateManagedWidget("orientSectionRC", xmRowColumnWidgetClass, rightPaneRC,
            XmNorientation, XmHORIZONTAL, XmNspacing, 4, XmNmarginHeight, 0, NULL);
            
        Widget orientLeft = XtVaCreateManagedWidget("orientLeft", xmRowColumnWidgetClass, orientSectionRC, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);
        Widget orientRight = XtVaCreateManagedWidget("orientRight", xmRowColumnWidgetClass, orientSectionRC, XmNorientation, XmVERTICAL, XmNspacing, 1, XmNmarginHeight, 0, NULL);

        Widget autoSpinToggle = XtVaCreateManagedWidget("Auto Spin ON", xmToggleButtonWidgetClass, orientLeft, XmNfontList, global_font, XmNset, False, XmNwidth, 155, NULL);
        XtAddCallback(autoSpinToggle, XmNvalueChangedCallback, ToggleCB, (XtPointer)6);
        
        Widget rcX = XtVaCreateManagedWidget("rcX", xmRowColumnWidgetClass, orientLeft, XmNorientation, XmHORIZONTAL, XmNspacing, 2, XmNmarginHeight, 0, XmNmarginWidth, 0, NULL);
        XmString strX = XmStringCreateLtoR("X:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblX", xmLabelWidgetClass, rcX, XmNlabelString, strX, XmNfontList, global_font, NULL);
        XmStringFree(strX);
        scaleX_w = XtVaCreateManagedWidget("X-Rot", xmScaleWidgetClass, rcX, XmNfontList, global_font,
            XmNorientation, XmHORIZONTAL, XmNminimum, -360, XmNmaximum, 360, XmNvalue, 0, XmNshowValue, False, XmNscaleHeight, 20, XmNwidth, 130, NULL);
        XtAddCallback(scaleX_w, XmNvalueChangedCallback, TransformCB, (XtPointer)0);
        XtAddCallback(scaleX_w, XmNdragCallback, TransformCB, (XtPointer)0);

        Widget rcY = XtVaCreateManagedWidget("rcY", xmRowColumnWidgetClass, orientLeft, XmNorientation, XmHORIZONTAL, XmNspacing, 2, XmNmarginHeight, 0, XmNmarginWidth, 0, NULL);
        XmString strY = XmStringCreateLtoR("Y:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblY", xmLabelWidgetClass, rcY, XmNlabelString, strY, XmNfontList, global_font, NULL);
        XmStringFree(strY);
        scaleY_w = XtVaCreateManagedWidget("Y-Rot", xmScaleWidgetClass, rcY, XmNfontList, global_font,
            XmNorientation, XmHORIZONTAL, XmNminimum, -360, XmNmaximum, 360, XmNvalue, 0, XmNshowValue, False, XmNscaleHeight, 20, XmNwidth, 130, NULL);
        XtAddCallback(scaleY_w, XmNvalueChangedCallback, TransformCB, (XtPointer)1);
        XtAddCallback(scaleY_w, XmNdragCallback, TransformCB, (XtPointer)1);

        Widget btnReset = XtVaCreateManagedWidget("Reset View", xmPushButtonWidgetClass, orientRight, XmNfontList, global_font, XmNwidth, 155, NULL);
        XtAddCallback(btnReset, XmNactivateCallback, CenterModelCB, NULL);

        Widget rcZoom = XtVaCreateManagedWidget("rcZoom", xmRowColumnWidgetClass, orientRight, XmNorientation, XmHORIZONTAL, XmNspacing, 2, XmNmarginHeight, 0, XmNmarginWidth, 0, NULL);
        XmString strZoom = XmStringCreateLtoR("Zm:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblZoom", xmLabelWidgetClass, rcZoom, XmNlabelString, strZoom, XmNfontList, global_font, NULL);
        XmStringFree(strZoom);
        zoomScale = XtVaCreateManagedWidget("Zoom", xmScaleWidgetClass, rcZoom, XmNfontList, global_font,
            XmNorientation, XmHORIZONTAL, XmNminimum, 10, XmNmaximum, 300, XmNvalue, 100, XmNshowValue, False, XmNscaleHeight, 20, XmNwidth, 120, NULL);
        XtAddCallback(zoomScale, XmNvalueChangedCallback, TransformCB, (XtPointer)3);
        XtAddCallback(zoomScale, XmNdragCallback, TransformCB, (XtPointer)3);

        Widget rcZ = XtVaCreateManagedWidget("rcZ", xmRowColumnWidgetClass, orientRight, XmNorientation, XmHORIZONTAL, XmNspacing, 2, XmNmarginHeight, 0, XmNmarginWidth, 0, NULL);
        XmString strZ = XmStringCreateLtoR("Z:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblZ", xmLabelWidgetClass, rcZ, XmNlabelString, strZ, XmNfontList, global_font, NULL);
        XmStringFree(strZ);
        scaleZ_w = XtVaCreateManagedWidget("Z-Rot", xmScaleWidgetClass, rcZ, XmNfontList, global_font,
            XmNorientation, XmHORIZONTAL, XmNminimum, -360, XmNmaximum, 360, XmNvalue, 0, XmNshowValue, False, XmNscaleHeight, 20, XmNwidth, 130, NULL);
        XtAddCallback(scaleZ_w, XmNvalueChangedCallback, TransformCB, (XtPointer)2);
        XtAddCallback(scaleZ_w, XmNdragCallback, TransformCB, (XtPointer)2);
    }
    XtVaCreateManagedWidget("sepOrient", xmSeparatorWidgetClass, rightPaneRC, NULL);

    /* APP SECTION */
    {
        XmString str = XmStringCreateLtoR("Application:", XmSTRING_DEFAULT_CHARSET);
        XtVaCreateManagedWidget("lblApp", xmLabelWidgetClass, rightPaneRC, XmNfontList, global_font, XmNlabelString, str, NULL);
        XmStringFree(str);

        Widget appSectionRC = XtVaCreateManagedWidget("appSectionRC", xmRowColumnWidgetClass, rightPaneRC,
            XmNorientation, XmHORIZONTAL, XmNspacing, 4, XmNmarginHeight, 0, NULL);

        Widget btnResetDefaults = XtVaCreateManagedWidget("Reset Defaults", xmPushButtonWidgetClass, appSectionRC, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(btnResetDefaults, XmNactivateCallback, ResetDefaultsCB, NULL);
        
        Widget btnQuit = XtVaCreateManagedWidget("Quit", xmPushButtonWidgetClass, appSectionRC, XmNfontList, global_font, XmNwidth, 155, XmNrecomputeSize, False, NULL);
        XtAddCallback(btnQuit, XmNactivateCallback, QuitCB, NULL);
    }

    XtAddEventHandler(topLevel, StructureNotifyMask, False, MapHandler, NULL);
    
    XtRealizeWidget(topLevel);
    
    context = XMesaCreateContext(dpy, visinfo,
                                 GL_TRUE,   /* rgb_flag */
                                 GL_FALSE,  /* alpha_flag */
                                 GL_TRUE,   /* db_flag */
                                 16,        /* depth_size */
                                 0,         /* stencil_size */
                                 0,         /* accum_size */
                                 GL_FALSE,  /* ximage_flag */
                                 NULL);     /* share_list */
    if (!context) {
        fprintf(stderr, "XMesaCreateContext failed!\n");
        exit(1);
    }
    
    colorContext = XMesaCreateContext(dpy, visinfo,
                                 GL_TRUE,   /* rgb_flag */
                                 GL_FALSE,  /* alpha_flag */
                                 GL_TRUE,   /* db_flag */
                                 0,         /* depth_size */
                                 0,         /* stencil_size */
                                 0,         /* accum_size */
                                 GL_FALSE,  /* ximage_flag */
                                 NULL);     /* share_list */

    lightContext = XMesaCreateContext(dpy, visinfo,
                                 GL_TRUE,   /* rgb_flag */
                                 GL_FALSE,  /* alpha_flag */
                                 GL_TRUE,   /* db_flag */
                                 16,        /* depth_size */
                                 0,         /* stencil_size */
                                 0,         /* accum_size */
                                 GL_FALSE,  /* ximage_flag */
                                 NULL);     /* share_list */

    XMesaBindWindow(context, XtWindow(drawArea));
    if (colorContext) {
        XMesaBindWindow(colorContext, XtWindow(colorPreviewArea));
    }
    if (lightContext) {
        XMesaBindWindow(lightContext, XtWindow(lightPreviewArea));
    }
    XMesaMakeCurrent(context);
    
    init_gl(dpy, XtWindow(drawArea));
    
    {
        Dimension width, height;
        XtVaGetValues(drawArea, XmNwidth, &width, XmNheight, &height, NULL);
        resize_gl(width, height);
    }
    
    work_proc_id = XtAppAddWorkProc(app, IdleFunc, NULL);
    
    XtAppMainLoop(app);
    return 0;
}
