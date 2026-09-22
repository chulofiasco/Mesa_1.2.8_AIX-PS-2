/* $Id: Amigamesa.h,v 1.0 1996/02/21 11:09:45 StefanZ Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  1.2
 * Copyright (C) 1995-1996  Brian Paul  (brianp@ssec.wisc.edu)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
$Log: amesa.h,v $
 * Revision 1.0  1996/02/21  11:09:45  brianp
 * A copy of amesa.h version 1.4 in a brave atempt to make a amiga interface
 *
 */


/*
 * Mesa/Amiga interface by Stephan Zivkovic (d94sz@efd.lth.se)
 */


/* Example usage:
 *
 * 1. Make a window using Intuition calls
 *
 * 2. Call AMesaCreateContext() to make a rendering context and attach it
 *    to the window made in step 1.
 *
 * 3. Call AMesaMakeCurrent() to make the context the active one.
 *
 * 4. Make gl* calls to render your graphics.
 *
 * 5. When exiting, call AMesaDestroyContext().
 */



#ifndef AMIGAMESA_H
#define AMIGAMESA_H


#include <intuition/intuition.h>
#include "GL/gl.h"


/*
 * This is the Amiga/Mesa context structure.  This usually contains
 * info about what window/buffer we're rendering too, the current
 * drawing color, etc.
 */
struct amigamesa_context
{
	struct gl_context *gl_ctx;	/* the core library context */
	GLboolean db_flag;		/* double buffered? */
	GLboolean rgb_flag;		/* RGB mode? */
	struct amigamesa_contex * sharelist;

	unsigned long pixel;	/* current color index or RGBA pixel value */
	unsigned long clearpixel;  /* pixel for clearing the color buffers */

	/* etc... */
	struct Window *window;		/* the Intuition window */
        struct RastPort *front_rp;	/* front rastport */
        struct RastPort *back_rp;	/* back rastport (NULL if SB or RGB) */
	struct RastPort *rp;		/* current rastport */
	struct TmpRas *tmpras;		/* tmpras rastport */
	GLuint depth;			/* bits per pixel (1, 8, 24, etc) */

	GLuint width, height;		/* drawable area */
	GLint left, bottom;		/* offsets due to window border */
	
	int mypen[256];
	UBYTE penconv[256];	/* when allocating index 13 with a color */
				/* penconv[13] is the actual system color. */
				/* penconv[] is changed if */
				/* auxSetOneColor(index,r,g,b); is called */ 

	UBYTE *imageline;	/* One Line for WritePixelRow renders */
	GLuint *rgb_buffer;	/* back buffer when in RGBA mode  OLD DElete?*/
};



typedef struct amigamesa_context *AmigaMesaContext;

void amiga_setup_DD_pointers(void);


/**********************************************************************/
/*****              Some Usefull code                             *****/
/**********************************************************************/

int RGBA(GLubyte r,GLubyte g,GLubyte b,GLubyte a);    /* returns an allocated color thts nearest the wanted one */


/**********************************************************************/
/*****               Amiga/Mesa API Functions                     *****/
/**********************************************************************/

__asm __saveds struct amigamesa_context *AmigaMesaCreateContext(register __a1 struct Window *window,register __d0 GLboolean rgb_flag,register __d1 GLboolean db_flag );
__asm __saveds void AmigaMesaDestroyContext(register __a0 struct amigamesa_context *c );
__asm __saveds void AmigaMesaMakeCurrent(register __a0 struct amigamesa_context *c );
__asm __saveds void AmigaMesaSwapBuffers();



#endif
