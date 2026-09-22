/*
 * (c) Copyright 1993, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */

/* Mesa tkAmiga by Stefan Z (d94sz@efd.lth.se)*/
/*
History:

1.0 960315 Now almost evetything is implemented
1.1 960425 Fixed problem with double closeclicks (Thanx to Daniel Jönsson)

TODO:
Exposefunc
Middle & Righmousebutton


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>


#include "gltk.h"
#include "gl/Amigamesa.h"

#define static

#if defined(__cplusplus) || defined(c_plusplus)
#define class c_class
#endif

#if DBG
#define TKASSERT(x)                                     \
if ( !(x) ) {                                           \
    PrintMessage("%s(%d) Assertion failed %s\n",        \
        __FILE__, __LINE__, #x);                        \
}
#else
#define TKASSERT(x)
#endif  /* DBG */

/******************************************************************************/

struct wnd 
	{
	int x, y, width, height;
	GLenum type;
	struct amigamesa_context *context;
	};

struct wnd win={0,0,100,100,TK_INDEX,NULL};





/*
struct Library *IntuitionBase;
struct Library *GfxBase;
*/
struct Screen *screen = NULL;
struct Window *tkhwnd = NULL;




GLboolean tkPopupEnable = TRUE;

/* Fixed palette support.  */
/*
#define BLACK   PALETTERGB(0,0,0)
#define WHITE   PALETTERGB(255,255,255)
#define NUM_STATIC_COLORS   (COLOR_BTNHIGHLIGHT - COLOR_SCROLLBAR + 1)
*/
static void (*ExposeFunc)(int, int)              = NULL;
static void (*ReshapeFunc)(GLsizei, GLsizei)     = NULL;
static void (*DisplayFunc)(void)                 = NULL;
static GLenum (*KeyDownFunc)(int, GLenum)        = NULL;
static GLenum (*MouseDownFunc)(int, int, GLenum) = NULL;
static GLenum (*MouseUpFunc)(int, int, GLenum)   = NULL;
static GLenum (*MouseMoveFunc)(int, int, GLenum) = NULL;
static void (*IdleFunc)(void)                    = NULL;

/*
 *  Prototypes for the debugging functions go here
 */

#define DBGFUNC 0
#if DBGFUNC

static void DbgPrintf( const char *Format, ... );
static void pwi( void );
static void pwr(RECT *pr);
/*static void ShowPixelFormat(HDC hdc);*/

#endif


float tkRGBMap[8][3] = 
	{
    {	0, 0, 0 },
    {	1, 0, 0 },
    {	0, 1, 0 },
    {	1, 1, 0 },
    {	0, 0, 1 },
    {	1, 0, 1 },
    {	0, 1, 1 },
    {	1, 1, 1 }
	};

int atk_setIDCMPs(void)
	{
	int mask;
	mask=IDCMP_CLOSEWINDOW;
	if (ReshapeFunc)
		mask |=IDCMP_NEWSIZE;
	if (KeyDownFunc)
		mask |=(IDCMP_RAWKEY | IDCMP_VANILLAKEY);
	if (MouseDownFunc||MouseUpFunc)
		mask |=IDCMP_MOUSEBUTTONS;
	if (MouseMoveFunc)
		mask |=	IDCMP_MOUSEMOVE;

	return(mask);
	}

/* TODO (*ExposeFunc)(int, int)   */

void atk_modyfyIDCMP(void)
	{
	if(tkhwnd)
		{
		ModifyIDCMP( tkhwnd, atk_setIDCMPs() );
		}
	}


int atk_FixKeyRAW(char c)
	{
	int key;
	switch (c) 
		{
		/*
	  case XK_Return: 	key = TK_RETURN;	break;
	  case XK_Escape: 	key = TK_ESCAPE;	break;
		*/
	  case 'O':		key = TK_LEFT;		break;
	  case 'L':		key = TK_UP;			break;
	  case 'N':  	key = TK_RIGHT;		break;
	  case 'M':		key = TK_DOWN;		break;
	  default: 		key = GL_FALSE;		break;
	}
	return(key);
	}


/***************************************************************
 *                                                             *
 *  Exported Functions go here                                 *
 *                                                             *
 ***************************************************************/


void tkNewCursor(GLint id, GLubyte *shapeBuf, GLubyte *maskBuf, GLenum fgColor,
     GLenum bgColor, GLint hotX, GLint hotY)
{
/* cursor.c */ 
}

void tkSetCursor(GLint id)
{
/* cursor.s*/
}

void tkErrorPopups(GLboolean bEnable)
{
    tkPopupEnable = bEnable;
}

void tkCloseWindow(void)
	{
	if (win.context) 
		{
		AmigaMesaDestroyContext(win.context);
		win.context=NULL;
		}
	if (tkhwnd)
		{
		CloseWindow(tkhwnd);
		tkhwnd=NULL;
		}
	if (GfxBase)
		{
		CloseLibrary(GfxBase);
		GfxBase=NULL;
		}
	if (IntuitionBase)
		{
		CloseLibrary(IntuitionBase);
		IntuitionBase=NULL;
		}
	}




void tkExec(void)
	{
	struct IntuiMessage *msg;
	BOOL done = FALSE;
	BOOL press=FALSE;
	int	key,id;
/*		printf("tkExec\n");*/

	if (ReshapeFunc)
		{
		(*ReshapeFunc)(win.width,win.height);
		}

	if (DisplayFunc)
		{
		(*DisplayFunc)();
		}


	while (!done)
		{
			
		while ( (! done) && (msg = (struct IntuiMessage *)GetMsg(tkhwnd->UserPort)))
			{
								/* use a switch statement if looking for multiple event types */
			id=msg->Class;
		/*	printf("msg->Class=0x%x\n",id);*/
			ReplyMsg((struct Message *)msg);
			switch (id)
				{
				case IDCMP_NEWSIZE:
					if (ReshapeFunc)
						(*ReshapeFunc)(win.width,win.height);
					if (DisplayFunc)
						(*DisplayFunc)();
					break;
				case IDCMP_RAWKEY:
					key= atk_FixKeyRAW(msg->Code);
					if (key && KeyDownFunc)
						{
						GLenum mask;
						mask = 0;
						/*
    					if (current.xkey.state & ControlMask)
    						mask |= TK_CONTROL;  /*TODO Fixa ctrl och shift check */
    					if (current.xkey.state & ShiftMask)
	    					mask |= TK_SHIFT;
	    				*/ 
						(*KeyDownFunc)(key, mask);
						}
					break;
				case IDCMP_VANILLAKEY:
					if (KeyDownFunc);
						{
						GLenum mask;
						mask = 0;
						/*
		    			if (current.xkey.state & ControlMask)
							mask |= TK_CONTROL;   /*TODO Fixa ctrl och shift check */
		    			if (current.xkey.state & ShiftMask)
							mask |= TK_SHIFT;
		    			*/
						(*KeyDownFunc)(msg->Code, mask);
						}
					break;
				case IDCMP_MOUSEMOVE:
					if (MouseMoveFunc)
						(*MouseMoveFunc)(msg->MouseX,msg->MouseY,press);
					break;					
				case IDCMP_MOUSEBUTTONS:
				printf("code=%x\n",(WORD)msg->Code);
					if (!((WORD)msg->Code & 0x80))  // Mouse Press  (hack)
						{
						press=TK_LEFTBUTTON;
						if (MouseDownFunc)
							{
							GLenum mask;
	    					mask = 0;    /* TODO check right & middlebutton */
	   					mask = TK_LEFTBUTTON;
							/*
	    					if (current.xbutton.button == 1) { mask |= TK_LEFTBUTTON; }
							if (current.xbutton.button == 2) { mask |= TK_MIDDLEBUTTON;}
							if (current.xbutton.button == 3) { mask |= TK_RIGHTBUTTON;}
							*/
							(*MouseDownFunc)(msg->MouseX,msg->MouseY,mask);
							}
						}
					else
						{
						if (MouseUpFunc)
							{
							GLenum mask;
	    					mask = 0;
	    					mask |= TK_LEFTBUTTON;
							/*
    						if (current.xbutton.button == 1) { mask |= TK_LEFTBUTTON; }
							if (current.xbutton.button == 2) { mask |= TK_MIDDLEBUTTON;}
							if (current.xbutton.button == 3) { mask |= TK_RIGHTBUTTON;}
							*/
							(*MouseUpFunc)(msg->MouseX,msg->MouseY,mask);
							}
						press=0;
						}
					break;
				case IDCMP_CLOSEWINDOW:
						done = TRUE;
						break;
					}
								/* TODO Fill this with tests and call apropriate functions */
								/* TODO (*ExposeFunc)(int, int) ??????? What should it do? */
				}
		if(!done)
			{
			if (IdleFunc)
				{
				(*IdleFunc)();
				if (DisplayFunc)
					{
					(*DisplayFunc)();
					}
				}
			else
				{
				WaitPort(tkhwnd->UserPort);
				}
			}
		}
	/*	tkCloseWindow();*/
	tkQuit();      /* if something else is neaded freeing*/
}


void tkExposeFunc(void (*Func)(int, int))
	{
/*	printf("ExposeFunc\n");*/
	ExposeFunc = Func;
	atk_modyfyIDCMP();
	}

void tkReshapeFunc(void (*Func)(GLsizei, GLsizei))
	{
/*	printf("ReshapeFunc\n");*/
	ReshapeFunc = Func;
	atk_modyfyIDCMP();
	}

void tkDisplayFunc(void (*Func)(void))
	{
/*	printf("DisplayFunc\n");*/
	DisplayFunc = Func;
	}

void tkKeyDownFunc(GLenum (*Func)(int, GLenum))
	{
/*	printf("KeyDownFunc\n");*/
	KeyDownFunc = Func;
	atk_modyfyIDCMP();
	}

void tkMouseDownFunc(GLenum (*Func)(int, int, GLenum))
	{
/*	printf("MouseDownFunc\n");*/
	MouseDownFunc = Func;
	atk_modyfyIDCMP();
	}

void tkMouseUpFunc(GLenum (*Func)(int, int, GLenum))
	{
/*	printf("MouseUpFunc\n");*/
	MouseUpFunc = Func;
	atk_modyfyIDCMP();
	}

void tkMouseMoveFunc(GLenum (*Func)(int, int, GLenum))
	{
/*	printf("MouseMoveFunc\n");*/
	MouseMoveFunc = Func;
	atk_modyfyIDCMP();
	}

void tkIdleFunc(void (*Func)(void))
	{
/*	printf("IdleFunc\n");*/
	IdleFunc = Func;
	}

void tkInitPosition(int x, int y, int width, int height)
	{
	win.x=x;
	win.y=y;
	win.width=width;
	win.height=height;
	}

void tkInitDisplayMode(GLenum type)
	{
	win.type = type;
	}

GLenum tkInitWindow(char *title)
	{
	GLenum   Result = GL_FALSE,
					RGB_Flag=GL_TRUE,
					DB_Flag=GL_FALSE;


	if (IntuitionBase = OpenLibrary("intuition.library",37))
		{
		if (GfxBase = OpenLibrary("graphics.library",37))
			{
			if (!(screen = LockPubScreen("MESA")))
				screen = LockPubScreen(NULL);
			if (screen)
				{
											/* open the window on the public screen */


				tkhwnd = OpenWindowTags(NULL,
							WA_Left,  win.x,    		WA_Top,    win.x,
							WA_InnerWidth, win.width,	WA_InnerHeight, win.height,
							WA_DragBar,				TRUE,
							WA_CloseGadget,		TRUE,
							WA_SizeGadget,			TRUE,
							WA_DepthGadget,		TRUE,
							WA_SmartRefresh,		TRUE,
							WA_ReportMouse,		TRUE,
							WA_NoCareRefresh,		TRUE,
							WA_SizeBBottom,		TRUE,
							WA_MinWidth,100,		WA_MinHeight,30,
							WA_MaxWidth,-1,		WA_MaxHeight,-1,
							WA_IDCMP,				atk_setIDCMPs(),
							WA_Flags,				WFLG_SIZEGADGET | WFLG_DRAGBAR,
							WA_Title,				title,
							WA_PubScreen,			screen,
							TAG_END);
											/* Unlock the screen.  The window now acts as a lock on
											** the screen, and we do not need the screen after the
											** window has been closed.  */
				UnlockPubScreen(NULL, screen);
				if (tkhwnd)
					{
					if (win.type & TK_INDEX)
						{
						RGB_Flag=GL_FALSE;
						}
					if (win.type & TK_DOUBLE)
						{
						DB_Flag=GL_TRUE;
						}

					win.context=AmigaMesaCreateContext(tkhwnd,RGB_Flag,DB_Flag);
    			AmigaMesaMakeCurrent(win.context);
    			return GL_TRUE;
					
					}
				else
					{
					printf("Failed to open window.\n");
					return(Result);
					}
				}
			}	
		}
}



/******************************************************************************/

/*
 * You cannot just call DestroyWindow() here.  The programs do not expect
 * tkQuit() to return;  DestroyWindow() just sends a WM_DESTROY message
 */

void tkQuit(void)
{
	tkCloseWindow();
   exit(0);    /*TODO*/
}

/******************************************************************************/

void tkSetOneColor(int index, float r, float g, float b)
{
	win.context->penconv[index]=RGBA(r*255,g*255,b*255,255);
}

void tkSetFogRamp(int density, int startIndex)
{
/* TODO */
printf("tkSetFogRamp(%d,%d) TODO\n",density,startIndex);
}

void tkSetGreyRamp(void)
{
 /* TODO */
printf("tkSetFogRamp() TODO\n");

}

void tkSetRGBMap( int Size, float *Values )
{
/* TODO */
printf("tkSetRGBMap(%d,%f) TODO\n",Size,*Values);
}

void tkSetOverlayMap(int size, float *rgb)
{
printf("tkSetOverlayMap(%d,%f) TODO\n",size,rgb);
}
/******************************************************************************/

void tkSwapBuffers(void)
{
  AmigaMesaSwapBuffers();
}

/******************************************************************************/

GLint tkGetColorMapSize(void)
{
 /* TODO */
	printf("tkGetColorMapSize() TODO\n");
	return(255);
}

void tkGetMouseLoc(int *x, int *y)
{
	*x = tkhwnd->MouseX;
	*y = tkhwnd->MouseY;
}


GLenum tkGetDisplayMode(void)
{
    return win.type;
}

GLenum tkSetWindowLevel(GLenum level)
{
printf("tkSetWindowLevel(%d) TODO\n",level);
}

/*
TK_RGBImageRec *tkRGBImageLoad(char *fileName)
{
printf("tkRGBImageLoad(%s) TODO\n",fileName);
}
*/

void tkGetSystem(TKenum type, void *ptr)
{
printf("tkGetSystem(%d,*ptr) TODO\n",type);
/* getset.c */
}
