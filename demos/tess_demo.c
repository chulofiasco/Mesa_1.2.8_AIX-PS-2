/* tess_demo.c */

/*
 * A demo of the GLU polygon tesselation functions written by Bogdan Sikorski.
 * This demo isn't built by the Makefile because it needs GLUT.  After you've
 * installed GLUT you can try this demo.
 * Here's the command for IRIX, for example:
   cc -g -ansi -prototypes -fullwarn -float -I../include -DSHM tess_demo.c -L../lib -lglut -lMesaGLU -lMesaGL -lm -lX11 -lXext -lXmu -lfpe -lXext -o tess_demo
 */


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POINTS 200
#define MAX_CONTOURS 50

int menu;
typedef enum{ QUIT, TESSELATE, CLEAR } menu_entries;
typedef enum{ DEFINE, TESSELATED } mode_type;
struct
{
	GLint p[MAX_POINTS][2];
	GLuint point_cnt;
} contours[MAX_CONTOURS];
GLuint contour_cnt;
GLsizei width,height;
mode_type mode;

struct
{
	GLsizei no;
	GLfloat color[3];
	GLint p[3][2];
	GLclampf p_color[3][3];
} triangle;

void my_error(GLenum err)
{
	int len,i;
	char errbuf[64];
	const char *errstr;
	unsigned long err32 = (unsigned long)err;
	if (err32 > 0UL && err32 < 65536UL) {
		err32 |= 0x10000UL;
	}

	errstr = (const char *)gluErrorString((GLenum)err32);
	if (errstr != NULL) {
		sprintf(errbuf, "GLU Error: %s", errstr);
	} else {
		switch(err32) {
			case 100154: errstr = "duplicate vertex"; break;
			case 100155: errstr = "overlapping contours"; break;
			case 100156: errstr = "self intersecting contour"; break;
			case 100157: errstr = "contour orientation mismatch"; break;
			case 100158: errstr = "contour topology mismatch"; break;
			case 100159: errstr = "coplanar contour error"; break;
			default: errstr = "tessellation failed"; break;
		}
		sprintf(errbuf, "GLU Error %lu (%s)", err32, errstr);
	}

	glColor3f(1.0, 0.2, 0.2);
	glRasterPos2i(10, 20);
	len = strlen(errbuf);
	for (i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, errbuf[i]);
}

static GLenum current_tess_mode;
static GLint tess_verts[200][2];
static GLfloat tess_colors[200][3];
static GLfloat current_edge_color[3] = {1.0, 1.0, 0.5};
static int tess_vert_cnt = 0;

void begin_callback(GLenum mode)
{
	current_tess_mode = mode;
	tess_vert_cnt = 0;
}

void edge_callback(GLenum flag)
{
	if(flag == GL_TRUE)
	{
		current_edge_color[0] = 1.0;
		current_edge_color[1] = 1.0;
		current_edge_color[2] = 0.5;
	}
	else
	{
		current_edge_color[0] = 1.0;
		current_edge_color[1] = 0.0;
		current_edge_color[2] = 0.0;
	}
}

void vertex_callback(void *data)
{
	GLint *p = (GLint *)data;
	if (tess_vert_cnt < 200) {
		tess_verts[tess_vert_cnt][0] = p[0];
		tess_verts[tess_vert_cnt][1] = p[1];
		tess_colors[tess_vert_cnt][0] = current_edge_color[0];
		tess_colors[tess_vert_cnt][1] = current_edge_color[1];
		tess_colors[tess_vert_cnt][2] = current_edge_color[2];
		tess_vert_cnt++;
	}
}

void end_callback()
{
	int i;
	glBegin(GL_LINES);
	if (current_tess_mode == GL_TRIANGLES) {
		for (i = 0; i + 2 < tess_vert_cnt; i += 3) {
			glColor3fv(tess_colors[i]);   glVertex2iv(tess_verts[i]);   glVertex2iv(tess_verts[i+1]);
			glColor3fv(tess_colors[i+1]); glVertex2iv(tess_verts[i+1]); glVertex2iv(tess_verts[i+2]);
			glColor3fv(tess_colors[i+2]); glVertex2iv(tess_verts[i+2]); glVertex2iv(tess_verts[i]);
		}
	} else if (current_tess_mode == GL_TRIANGLE_FAN) {
		for (i = 1; i + 1 < tess_vert_cnt; i++) {
			glColor3fv(tess_colors[i]);   glVertex2iv(tess_verts[0]);   glVertex2iv(tess_verts[i]);
			glColor3fv(tess_colors[i]);   glVertex2iv(tess_verts[i]);   glVertex2iv(tess_verts[i+1]);
			glColor3fv(tess_colors[i+1]); glVertex2iv(tess_verts[i+1]); glVertex2iv(tess_verts[0]);
		}
	} else if (current_tess_mode == GL_TRIANGLE_STRIP) {
		for (i = 0; i + 2 < tess_vert_cnt; i++) {
			glColor3fv(tess_colors[i]);   glVertex2iv(tess_verts[i]);   glVertex2iv(tess_verts[i+1]);
			glColor3fv(tess_colors[i+1]); glVertex2iv(tess_verts[i+1]); glVertex2iv(tess_verts[i+2]);
			glColor3fv(tess_colors[i+2]); glVertex2iv(tess_verts[i+2]); glVertex2iv(tess_verts[i]);
		}
	}
	glEnd();
}

void set_screen_wh(GLsizei w, GLsizei h)
{
	width=w;
	height=h;
}

void tesse(void)
{
	GLUtriangulatorObj *tobj;
	GLdouble data[3];
	GLuint i,j,point_cnt;

	tobj=gluNewTess();
	if(tobj!=NULL)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f (0.7, 0.7, 0.0);
		gluTessCallback(tobj,GLU_BEGIN,(void (*)())glBegin);
		gluTessCallback(tobj,GLU_END,(void (*)())glEnd);
		gluTessCallback(tobj,GLU_ERROR,(void (*)())my_error);
		gluTessCallback(tobj,GLU_VERTEX,(void (*)())glVertex2iv);
		gluBeginPolygon(tobj);
		for(j=0;j<=contour_cnt;j++)
		{
			point_cnt=contours[j].point_cnt;
			if (point_cnt < 3) continue;
			gluNextContour(tobj,GLU_UNKNOWN);
			for(i=0;i<point_cnt;i++)
			{
				data[0]=(GLdouble)(contours[j].p[i][0]);
				data[1]=(GLdouble)(contours[j].p[i][1]);
				data[2]=0.0;
				gluTessVertex(tobj,data,contours[j].p[i]);
			}
		}
		gluEndPolygon(tobj);
		glLineWidth(2.0);
		gluTessCallback(tobj,GLU_BEGIN,(void (*)())begin_callback);
		gluTessCallback(tobj,GLU_END,(void (*)())end_callback);
		gluTessCallback(tobj,GLU_VERTEX,(void (*)())vertex_callback);
		gluTessCallback(tobj,GLU_EDGE_FLAG,(void (*)())edge_callback);
		gluBeginPolygon(tobj);
		for(j=0;j<=contour_cnt;j++)
		{
			point_cnt=contours[j].point_cnt;
			if (point_cnt < 3) continue;
			gluNextContour(tobj,GLU_UNKNOWN);
			for(i=0;i<point_cnt;i++)
			{
				data[0]=(GLdouble)(contours[j].p[i][0]);
				data[1]=(GLdouble)(contours[j].p[i][1]);
				data[2]=0.0;
				gluTessVertex(tobj,data,contours[j].p[i]);
			}
		}
		gluEndPolygon(tobj);
		gluDeleteTess(tobj);
		glutMouseFunc(NULL);
		glColor3f (1.0, 1.0, 0.0);
		glLineWidth(1.0);
		mode=TESSELATED;
	}
}

void left_down(int x1,int y1)
{
	GLint P[2];
	GLuint point_cnt;

	/* translate GLUT into GL coordinates */
	P[0]=x1;
	P[1]=height-y1;
	point_cnt=contours[contour_cnt].point_cnt;
	contours[contour_cnt].p[point_cnt][0]=P[0];
	contours[contour_cnt].p[point_cnt][1]=P[1];
    glBegin(GL_LINES);
    if(point_cnt)
    {
        glVertex2iv(contours[contour_cnt].p[point_cnt-1]);
        glVertex2iv(P);
    }
    else
    {
        glVertex2iv(P);
        glVertex2iv(P);
    }
    glEnd();
    glFinish();
	++(contours[contour_cnt].point_cnt);
}

void middle_down(int x1,int y1)
{
	GLuint point_cnt;

	point_cnt=contours[contour_cnt].point_cnt;
	if(point_cnt>2)
	{
		glBegin(GL_LINES);
		glVertex2iv(contours[contour_cnt].p[0]);
		glVertex2iv(contours[contour_cnt].p[point_cnt-1]);
		contours[contour_cnt].p[point_cnt][0]= -1;
		glEnd();
		glFinish();
		contour_cnt++;
		contours[contour_cnt].point_cnt=0;
	}
}

void mouse_clicked(int button,int state,int x,int y)
{
	x-= x%10;
	y-= y%10;
	switch(button)
	{
		case GLUT_LEFT_BUTTON:
			if(state==GLUT_DOWN)
				left_down(x,y);
			break;
		case GLUT_MIDDLE_BUTTON:
			if(state==GLUT_DOWN)
				middle_down(x,y);
			break;
	}
}

void display(void)
{
	GLuint i,j;
	GLuint point_cnt;

    glClear(GL_COLOR_BUFFER_BIT);
    switch(mode)
    {
    	case DEFINE:
			/* draw grid */
			glColor3f (0.6,0.5,0.5);
			glBegin(GL_LINES);
			for(i=0;i<width;i+=10)
				for(j=0;j<height;j+=10)
				{
					glVertex2i(0,j);
					glVertex2i(width,j);
					glVertex2i(i,height);
					glVertex2i(i,0);
			}
			glColor3f (1.0, 1.0, 0.0);
			for(i=0;i<=contour_cnt;i++)
			{
				point_cnt=contours[i].point_cnt;
				glBegin(GL_LINES);
				switch(point_cnt)
				{
					case 0:
						break;
					case 1:
						glVertex2iv(contours[i].p[0]);
						glVertex2iv(contours[i].p[0]);
						break;
					case 2:
						glVertex2iv(contours[i].p[0]);
						glVertex2iv(contours[i].p[1]);
						break;
					default:
						--point_cnt;
						for(j=0;j<point_cnt;j++)
						{
							glVertex2iv(contours[i].p[j]);
							glVertex2iv(contours[i].p[j+1]);
						}
						if(contours[i].p[j+1][0]== -1)
						{
							glVertex2iv(contours[i].p[0]);
							glVertex2iv(contours[i].p[j]);
						}
						break;
				}
				glEnd();
			}
			glFinish();
			break;
		case TESSELATED:
			/* draw lines */
			tesse();
			break;
	}

    glColor3f (1.0, 1.0, 0.0);
}

void clear( void )
{
    contour_cnt=0;
    contours[0].point_cnt=0;
	glutMouseFunc(mouse_clicked);
	mode=DEFINE;
	display();
}

void quit( void )
{
	exit(0);
}

void menu_selected(int entry)
{
	switch(entry)
	{
		case CLEAR:
			clear();
			break;
		case TESSELATE:
			tesse();
			break;
		case QUIT:
			quit();
			break;
	}
}

void key_pressed(unsigned char key,int x,int y)
{
	switch(key)
	{
		case 't':
		case 'T':
			tesse();
			glFinish();
			break;
		case 'q':
		case 'Q':
			quit();
			break;
		case 'c':
		case 'C':
			clear();
			break;
	}
}

void myinit (void) 
{
/*  clear background to gray	*/
    glClearColor (0.4, 0.4, 0.4, 0.0);
    glShadeModel (GL_FLAT);    

	menu=glutCreateMenu(menu_selected);
	glutAddMenuEntry("clear",CLEAR);
	glutAddMenuEntry("tesselate",TESSELATE);
	glutAddMenuEntry("quit",QUIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutMouseFunc(mouse_clicked);
	glutKeyboardFunc(key_pressed);
    contour_cnt=0;
    glPolygonMode(GL_FRONT,GL_FILL);
    mode=DEFINE;
}

static void reshape(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (GLdouble)w, 0.0, (GLdouble)h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    set_screen_wh(w,h);
}


static void usage( void )
{
   printf("Use left mouse button to place vertices.\n");
   printf("Press middle mouse button when done.\n");
   printf("Select tesselate from the pop-up menu.\n");
}


/*  Main Loop
 *  Open window with initial window size, title bar, 
 *  RGBA display mode, and handle input events.
 */
void main(int argc, char** argv)
{
   usage();
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (400, 400);
    glutCreateWindow (argv[0]);
    myinit ();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
}
