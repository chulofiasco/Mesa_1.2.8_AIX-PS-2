/* Discription:        */
static char versionstring[]="\0$VER:       1.0 (dd-mm-yy)";
#include <stdio.h>
#define INTUI_V36_NAMES_ONLY

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>


#include "qv:mesa/include/gl/AmigaMesa.h"

#define AMIGA
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include "glaux.h"


#ifdef LATTICE
int CXBRK(void)    { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

struct Library *IntuitionBase;
struct Library *GfxBase;
struct Library *glBase;
struct amigamesa_context *glcont;

#define IDCMPS IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_NEWSIZE


void handle_window_events(struct Window *win);
void rita(struct Window *win,struct RastPort *rport);
void Init(void);



void Init(void)
{
/*    GLint i;*/

/*    glClearColor(0.0, 0.0, 0.0, 0.0);*/
	glClearIndex(0);

    glMatrixMode (GL_PROJECTION);				/*  prepare for and then	*/ 
    glLoadIdentity ();							/*  define the projection	*/
    glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);/*  transformation  		*/
    glMatrixMode (GL_MODELVIEW);				/*  back to modelview matrix*/
    glViewport (0, 0, 200, 200);				/*  define the viewport		*/

    glShadeModel (GL_FLAT);

    glLoadIdentity ();	/*  clear the matrix	*/
/*    glTranslatef (0.0, 3.0, -5.0);*/	/*  viewing transformation	*/
    glTranslatef (0.0, 0.0, -5.0);	/*  viewing transformation	*/
    glScalef (2.0, 2.0, 2.0);	/*  modeling transformation	*/

}


void display (void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f (1.0, 1.0, 1.0);

	glRotatef(1.0, 2.0,0,1);
    auxWireCube(1.0);	/*  draw the cube	*/
    glFlush();
/*    AmigaMesaSwapBuffers();*/

}







/*
** Open a simple window on the default public screen,
** then leave it open until the user selects the close gadget.
*/
VOID main(int argc, char **argv)
{
struct Window *test_window = NULL;
struct Screen *test_screen = NULL;
if(glBase=OpenLibrary("gl.library",0))
{

if (IntuitionBase = OpenLibrary("intuition.library",37))
	{
	if (GfxBase = OpenLibrary("graphics.library",37))
		{									/* get a lock on the default public screen */
		if (test_screen = LockPubScreen(NULL))
			{
										/* open the window on the public screen */


			test_window = OpenWindowTags(NULL,
						WA_Left,  10,    	WA_Top,    20,
						WA_InnerWidth, 200,	WA_InnerHeight, 200,
						WA_DragBar,         TRUE,
						WA_CloseGadget,     TRUE,
						WA_SizeGadget,		TRUE,
						WA_DepthGadget,		TRUE,
						WA_SmartRefresh,    TRUE,
						WA_ReportMouse,		TRUE,
						WA_NoCareRefresh,   TRUE,
						WA_SizeBBottom,		TRUE,
						WA_MinWidth,100,WA_MinHeight,14,
						WA_MaxWidth,-1,WA_MaxHeight,-1,
						WA_IDCMP,           IDCMPS,
						WA_Flags,			WFLG_SIZEGADGET | WFLG_DRAGBAR,
						WA_Title,           "MesaTest",
						WA_PubScreen,       test_screen,
						TAG_END);
											/* Unlock the screen.  The window now acts as a lock on
											** the screen, and we do not need the screen after the
											** window has been closed.  */
			UnlockPubScreen(NULL, test_screen);
            								/* if we have a valid window open, run the rest of the
            								** program, then clean up when done. */

			if (test_window)
        		{
				printf("Window is up\n");
				glcont=AmigaMesaCreateContext(test_window,GL_FALSE,GL_FALSE);
				if (glcont)
					{
					printf("Gl context created\n");
					AmigaMesaMakeCurrent(glcont);
					printf("Gl is up\n");

					Init();
					printf("Init... Done\n");
					display();


					handle_window_events(test_window);
					AmigaMesaDestroyContext(glcont);
					}
				else
					{
					printf("GL Context faild\n");
					}
				CloseWindow(test_window);
				}
			}
		CloseLibrary(GfxBase);
		}	
	CloseLibrary(IntuitionBase);
	}
CloseLibrary(glBase);
}
}

/*  Wait for the user to select the close gadget. */
void handle_window_events(struct Window *win)
{
struct IntuiMessage *msg;
BOOL done = FALSE;

while (! done)
	{
								/* We have no other ports of signals to wait on,
								** so we'll just use WaitPort() instead of Wait()	*/
/*	WaitPort(win->UserPort);*/
	display();

	while ( (! done) &&
		(msg = (struct IntuiMessage *)GetMsg(win->UserPort)))
		{
								/* use a switch statement if looking for multiple event types */
		switch(msg->Class)
			{
			case	IDCMP_CLOSEWINDOW:
				done = TRUE;
				break;
			case	IDCMP_NEWSIZE:
		    	glViewport (0, 0, win->Width-win->BorderLeft-win->BorderRight, win->Height-win->BorderTop-win->BorderBottom);	/*  define the viewport	*/
				display();
				break;
			}

		ReplyMsg((struct Message *)msg);
        }
    }
}

#define	drawOneLine(x1,y1,x2,y2) glBegin(GL_LINES); \
	glVertex2f ((x1),(y1)); glVertex2f ((x2),(y2)); glEnd();


GLenum rgb, doubleBuffer, directRender, windType;


GLenum mode1, mode2;
GLint size;
float pntA[3] = {
    -160.0, 0.0, 0.0
};
float pntB[3] = {
    -130.0, 0.0, 0.0
};
float pntC[3] = {
    -40.0, -50.0, 0.0
};
float pntD[3] = {
    30.0, 60.0, 0.0
};




void rita(struct Window *win,struct RastPort *rport)
	{
/*		SetABPenDrMd(rport,2,1,JAM1);
		Move(rport,(win->BorderLeft),(win->BorderTop));
		Draw(rport,win->BorderLeft+200,win->BorderTop+200);  */
	
/*	GLint i;*/

/*    glClear (GL_COLOR_BUFFER_BIT);*/
/*  draw all lines in white	*/
/*    glColor3f (1.0, 1.0, 1.0);*/

/*  in 1st row, 3 lines drawn, each with a different stipple	*/
/*    glEnable (GL_LINE_STIPPLE);*/
/*    glLineStipple (1, 0x0101);	  dotted	*/
    drawOneLine (50.0, 125.0, 150.0, 125.0);
	display();
/*
    for (i = 0; i < 360; i += 5)
    	{
		glRotatef(5.0, 0,0,1);

		glIndexi(1);
		
		glBegin(GL_LINE_STRIP);
	    	glVertex3fv(pntA);
	    	glVertex3fv(pntB);
		glEnd();

		glPointSize(1);

		glBegin(GL_POINTS);
	    	glVertex3fv(pntA);
	    	glVertex3fv(pntB);
		glEnd();
    	}
*/
/*
      glPopMatrix();
	  glFlush();
*/
	}
	
