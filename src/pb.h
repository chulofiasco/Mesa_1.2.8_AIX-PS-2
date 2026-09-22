/* pb.h */

/*
 * Mesa 3-D graphics library
 * Version:  1.2
 * Copyright (C) 1995  Brian Paul  (brianp@ssec.wisc.edu)
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
$Id: pb.h,v 1.13 1996/03/07 15:28:54 brianp Exp $

$Log: pb.h,v $
 * Revision 1.13  1996/03/07  15:28:54  brianp
 * define PB_SIZE to be 3*MAX_WIDTH per Wolfram Gloger
 *
 * Revision 1.12  1996/01/22  15:31:30  brianp
 * replaced gl_init_pb() with PB_INIT macro
 *
 * Revision 1.11  1995/12/30  00:57:46  brianp
 * use integer colors instead of floating point
 *
 * Revision 1.10  1995/12/18  17:28:20  brianp
 * use new GLdepth datatype
 *
 * Revision 1.9  1995/07/15  14:04:10  brianp
 * added PB_WRITE_TEX_PIXEL
 *
 * Revision 1.8  1995/06/20  16:18:39  brianp
 * removed clipflag
 *
 * Revision 1.7  1995/06/12  15:34:47  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.6  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.5  1995/05/17  13:17:22  brianp
 * changed default CC.Mode value to allow use of real OpenGL headers
 * removed need for CC.MajorMode variable
 *
 * Revision 1.4  1995/05/12  16:26:33  brianp
 * added pixel clipping
 *
 * Revision 1.3  1995/03/07  14:21:24  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.2  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  17:51:57  brianp
 * Initial revision
 *
 */


#ifndef PB_H
#define PB_H


#include "GL/gl.h"
#include "config.h"



/*
 * Pixel buffer size, must be larger than MAX_WIDTH.
 */
#define PB_SIZE (3*MAX_WIDTH)


struct pixel_buffer {
	GLint x[PB_SIZE];	/* X window coord in [0,MAX_WIDTH) */
	GLint y[PB_SIZE];	/* Y window coord in [0,MAX_HEIGHT) */
	GLdepth z[PB_SIZE];	/* Z window coord in [0,MAX_DEPTH] */
	GLubyte r[PB_SIZE];	/* Red */
	GLubyte g[PB_SIZE];	/* Green */
	GLubyte b[PB_SIZE];	/* Blue */
	GLubyte a[PB_SIZE];	/* Alpha */
	GLuint i[PB_SIZE];	/* Index */
	GLfloat s[PB_SIZE];	/* Texture S coordinate */
	GLfloat t[PB_SIZE];	/* Texture T coordinate */
	GLint color[4];		/* Mono color, integers! */
	GLuint index;		/* Mono index */
	GLuint count;		/* Number of pixels in buffer */
	GLboolean mono;		/* Same color or index for all pixels? */
	GLenum primitive;	/* GL_POINT, GL_LINE, GL_POLYGON or GL_BITMAP*/
};


extern struct pixel_buffer PB;


/*
 * Initialize the Pixel Buffer, specifying the type of primitive being drawn.
 */
#define PB_INIT( PRIM )		\
	PB.count = 0;		\
	PB.mono = GL_FALSE;	\
	PB.primitive = (PRIM);



/*
 * Set the color used for all subsequent pixels in the buffer.
 */
#define PB_SET_COLOR( R, G, B, A )			\
	if (PB.color[0]!=(R) || PB.color[1]!=(G)	\
	 || PB.color[2]!=(B) || PB.color[3]!=(A)	\
	 || !PB.mono) {					\
		gl_flush_pb();				\
	}						\
	PB.color[0] = R;				\
	PB.color[1] = G;				\
	PB.color[2] = B;				\
	PB.color[3] = A;				\
	PB.mono = GL_TRUE;


/*
 * Set the color index used for all subsequent pixels in the buffer.
 */
#define PB_SET_INDEX( I )			\
	if (PB.index!=(I) || !PB.mono) {	\
		gl_flush_pb();			\
	}					\
	PB.index = I;				\
	PB.mono = GL_TRUE;


/*
 * "write" a pixel using current color or index
 */
#define PB_WRITE_PIXEL( X, Y, Z )	\
	PB.x[PB.count] = X;		\
	PB.y[PB.count] = Y;		\
	PB.z[PB.count] = Z;		\
	PB.count++;


/*
 * "write" an RGBA pixel
 */
#define PB_WRITE_RGBA_PIXEL( X, Y, Z, R, G, B, A )	\
	PB.x[PB.count] = X;				\
	PB.y[PB.count] = Y;				\
	PB.z[PB.count] = Z;				\
	PB.r[PB.count] = R;				\
	PB.g[PB.count] = G;				\
	PB.b[PB.count] = B;				\
	PB.a[PB.count] = A;				\
	PB.count++;

/*
 * "write" a color-index pixel
 */
#define PB_WRITE_CI_PIXEL( X, Y, Z, I )	\
	PB.x[PB.count] = X;		\
	PB.y[PB.count] = Y;		\
	PB.z[PB.count] = Z;		\
	PB.i[PB.count] = I;		\
	PB.count++;


/*
 * "write" an RGBA pixel with texture coordinates
 */
#define PB_WRITE_TEX_PIXEL( X, Y, Z, R, G, B, A, S, T )	\
	PB.x[PB.count] = X;				\
	PB.y[PB.count] = Y;				\
	PB.z[PB.count] = Z;				\
	PB.r[PB.count] = R;				\
	PB.g[PB.count] = G;				\
	PB.b[PB.count] = B;				\
	PB.a[PB.count] = A;				\
	PB.s[PB.count] = S;				\
	PB.t[PB.count] = T;				\
	PB.count++;


/*
 * Call this function at least every MAX_WIDTH pixels:
 */
#define PB_CHECK_FLUSH				\
	if (PB.count>=PB_SIZE-MAX_WIDTH) {	\
	   gl_flush_pb();			\
	}


extern void gl_flush_pb( void );



#endif
