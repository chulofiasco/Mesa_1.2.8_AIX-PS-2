/* $Id: accum.c,v 1.17 1996/05/21 14:58:49 brianp Exp $ */

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
$Log: accum.c,v $
 * Revision 1.17  1996/05/21  14:58:49  brianp
 * added rounding to glAccum( GL_RETURN, r ) per Alex De Bruyn
 *
 * Revision 1.16  1995/12/20  20:30:19  brianp
 * allow larger than 2-byte accumulation buffer components, per James Johnson
 *
 * Revision 1.15  1995/12/18  17:24:43  brianp
 * replaced ACC_TYPE with GLaccum
 *
 * Revision 1.14  1995/10/13  22:42:57  brianp
 * use new DD.set_buffer function
 *
 * Revision 1.13  1995/09/18  14:20:50  brianp
 * call DD.write_span_color with NULL mask, temporary hack
 *
 * Revision 1.12  1995/08/31  21:26:18  brianp
 * use DD.read_color_span instead of dd_read_color_span
 *
 * Revision 1.11  1995/07/24  20:34:16  brianp
 * replaced memset() with MEMSET() and memcpy() with MEMCPY()
 *
 * Revision 1.10  1995/06/12  15:35:00  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.9  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.8  1995/05/12  19:22:23  brianp
 * added #include "macros.h"
 *
 * Revision 1.7  1995/05/12  19:20:19  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.6  1995/04/11  14:03:27  brianp
 * changed (*CC.write...) to (*DD.write...)
 *
 * Revision 1.5  1995/03/30  21:06:50  brianp
 * updated to use pointers to CC.write_* functions
 *
 * Revision 1.4  1995/03/24  15:28:22  brianp
 * replaced ACCUM_BITS with ACC_TYPE
 *
 * Revision 1.3  1995/03/13  20:54:27  brianp
 * added read buffer logic
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:14:02  brianp
 * Initial revision
 *
 */


#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "dd.h"
#include "list.h"
#include "macros.h"




void glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
   if (CC.CompileFlag) {
      gl_save_clearaccum( red, green, blue, alpha );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glAccum" );
	 return;
      }
      CC.Accum.ClearColor[0] = CLAMP( red, -1.0, 1.0 );
      CC.Accum.ClearColor[1] = CLAMP( green, -1.0, 1.0 );
      CC.Accum.ClearColor[2] = CLAMP( blue, -1.0, 1.0 );
      CC.Accum.ClearColor[3] = CLAMP( alpha, -1.0, 1.0 );
   }
}




void gl_accum( GLenum op, GLfloat value )
{
   GLuint xpos, ypos, width, height;
   GLfloat acc_scale;

   if (sizeof(GLaccum)==1) {
      acc_scale = 127.0;
   }
   else if (sizeof(GLaccum)==2) {
      acc_scale = 32767.0;
   }
   else {
      /* sizeof(GLaccum) > 2 (Cray) */
      acc_scale = (float) SHRT_MAX;
   }

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glAccum" );
      return;
   }

   if (!CC.AccumBuffer) {
      /* allocate accumulation buffer if not already present */
      CC.AccumBuffer = (GLaccum *)
	    malloc( CC.BufferWidth * CC.BufferHeight * 4 * sizeof(GLaccum) );
      if (!CC.AccumBuffer) {
	 /* unable to setup accumulation buffer */
	 gl_error( GL_OUT_OF_MEMORY, "glAccum" );
	 return;
      }
   }

   /* Determine region to operate upon. */
   if (CC.Scissor.Enabled) {
      xpos = CC.Scissor.X;
      ypos = CC.Scissor.Y;
      width = CC.Scissor.Width;
      height = CC.Scissor.Height;
   }
   else {
      /* whole window */
      xpos = 0;
      ypos = 0;
      width = CC.BufferWidth;
      height = CC.BufferHeight;
   }

   switch (op) {
      case GL_ADD:
         {
	    GLaccum ival, *acc;
	    GLuint i, j;

	    ival = (GLaccum) (value * acc_scale);
	    for (j=0;j<height;j++) {
	       acc = CC.AccumBuffer + ( ypos * CC.BufferWidth + xpos ) * 4;
	       for (i=0;i<width;i++) {
		  *acc += ival;	  acc++;   /* red */
		  *acc += ival;	  acc++;   /* green */
		  *acc += ival;	  acc++;   /* blue */
		  *acc += ival;	  acc++;   /* alpha */
	       }
	       ypos++;
	    }
	 }
	 break;
      case GL_MULT:
	 {
	    GLaccum *acc;
	    GLuint i, j;

	    for (j=0;j<height;j++) {
	       acc = CC.AccumBuffer + ( ypos * CC.BufferWidth + xpos ) * 4;
	       for (i=0;i<width;i++) {
		  *acc = (GLaccum) ( (GLfloat) *acc * value );	  acc++; /*r*/
		  *acc = (GLaccum) ( (GLfloat) *acc * value );	  acc++; /*g*/
		  *acc = (GLaccum) ( (GLfloat) *acc * value );	  acc++; /*g*/
		  *acc = (GLaccum) ( (GLfloat) *acc * value );	  acc++; /*a*/
	       }
	       ypos++;
	    }
	 }
	 break;
      case GL_ACCUM:
	 {
	    GLaccum *acc;
	    GLubyte red[MAX_WIDTH], green[MAX_WIDTH];
	    GLubyte blue[MAX_WIDTH], alpha[MAX_WIDTH];
	    GLfloat rscale, gscale, bscale, ascale;
	    GLuint i, j;

	    (void) (*DD.set_buffer)( CC.Pixel.ReadBuffer );

	    /* Accumulate */
	    rscale = value * acc_scale / CC.RedScale;
	    gscale = value * acc_scale / CC.GreenScale;
	    bscale = value * acc_scale / CC.BlueScale;
	    ascale = value * acc_scale / CC.AlphaScale;
	    for (j=0;j<height;j++) {
	       (*DD.read_color_span)( width, xpos, ypos,
                                      red, green, blue, alpha);
	       acc = CC.AccumBuffer + ( ypos * CC.BufferWidth + xpos ) * 4;
	       for (i=0;i<width;i++) {
		  *acc += (GLaccum) ( (GLfloat) red[i]   * rscale );  acc++;
		  *acc += (GLaccum) ( (GLfloat) green[i] * gscale );  acc++;
		  *acc += (GLaccum) ( (GLfloat) blue[i]  * bscale );  acc++;
		  *acc += (GLaccum) ( (GLfloat) alpha[i] * ascale );  acc++;
	       }
	       ypos++;
	    }

	    (void) (*DD.set_buffer)( CC.Color.DrawBuffer );
	 }
	 break;
      case GL_LOAD:
	 {
	    GLaccum *acc;
	    GLubyte red[MAX_WIDTH], green[MAX_WIDTH];
	    GLubyte blue[MAX_WIDTH], alpha[MAX_WIDTH];
	    GLfloat rscale, gscale, bscale, ascale;
	    GLuint i, j;

	    (void) (*DD.set_buffer)( CC.Pixel.ReadBuffer );

	    /* Load accumulation buffer */
	    rscale = value * acc_scale / CC.RedScale;
	    gscale = value * acc_scale / CC.GreenScale;
	    bscale = value * acc_scale / CC.BlueScale;
	    ascale = value * acc_scale / CC.AlphaScale;
	    for (j=0;j<height;j++) {
	       (*DD.read_color_span)( width, xpos, ypos,
                                      red, green, blue, alpha);
	       acc = CC.AccumBuffer + ( ypos * CC.BufferWidth + xpos ) * 4;
	       for (i=0;i<width;i++) {
		  *acc++ = (GLaccum) ( (GLfloat) red[i]   * rscale );
		  *acc++ = (GLaccum) ( (GLfloat) green[i] * gscale );
		  *acc++ = (GLaccum) ( (GLfloat) blue[i]  * bscale );
		  *acc++ = (GLaccum) ( (GLfloat) alpha[i] * ascale );
	       }
	       ypos++;
	    }

	    (void) (*DD.set_buffer)( CC.Color.DrawBuffer );
	 }
	 break;
      case GL_RETURN:
	 {
	    GLubyte red[MAX_WIDTH], green[MAX_WIDTH];
	    GLubyte blue[MAX_WIDTH], alpha[MAX_WIDTH];
	    GLaccum *acc;
	    GLfloat rscale, gscale, bscale, ascale;
	    GLint rmax, gmax, bmax, amax;
	    GLuint i, j;

	    rscale = value / acc_scale * CC.RedScale;
	    gscale = value / acc_scale * CC.GreenScale;
	    bscale = value / acc_scale * CC.BlueScale;
	    ascale = value / acc_scale * CC.AlphaScale;
	    rmax = (GLint) CC.RedScale;
	    gmax = (GLint) CC.GreenScale;
	    bmax = (GLint) CC.BlueScale;
	    amax = (GLint) CC.AlphaScale;
	    for (j=0;j<height;j++) {
	       acc = CC.AccumBuffer + ( ypos * CC.BufferWidth + xpos ) * 4;
	       for (i=0;i<width;i++) {
		  GLint r, g, b, a;
		  r = (GLint) ( (GLfloat) (*acc++) * rscale + 0.5F );
		  g = (GLint) ( (GLfloat) (*acc++) * gscale + 0.5F );
		  b = (GLint) ( (GLfloat) (*acc++) * bscale + 0.5F );
		  a = (GLint) ( (GLfloat) (*acc++) * ascale + 0.5F );
		  red[i]   = CLAMP( r, 0, rmax );
		  green[i] = CLAMP( g, 0, gmax );
		  blue[i]  = CLAMP( b, 0, bmax );
		  alpha[i] = CLAMP( a, 0, amax );
	       }
	       (*DD.write_color_span)( width, xpos, ypos,
				       red, green, blue, alpha, NULL );
	       ypos++;
	    }
	 }
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glAccum" );
   }
}




void glAccum( GLenum op, GLfloat value )
{
   if (CC.ExecuteFlag) {
      gl_accum( op, value );
   }
   if (CC.CompileFlag) {
      gl_save_accum( op, value );
   }
}



/*
 * Clear the accumulation buffer.
 */
void gl_clear_accum_buffer( void )
{
   GLuint buffersize;
   GLfloat acc_scale;

   if (sizeof(GLaccum)==1) {
      acc_scale = 127.0;
   }
   else if (sizeof(GLaccum)==2) {
      acc_scale = 32767.0;
   }
   else {
      /* sizeof(GLaccum) > 2 (Cray) */
      acc_scale = (float) SHRT_MAX;
   }

   buffersize = CC.BufferWidth * CC.BufferHeight;  /* number of pixels */

   if (!CC.AccumBuffer) {
      /* try to alloc accumulation buffer */
      CC.AccumBuffer = (GLaccum *)
	                   malloc( buffersize * 4 * sizeof(GLaccum) );
   }

   if (CC.AccumBuffer) {
      if (CC.Scissor.Enabled) {
	 /* Limit clear to scissor box */
	 GLaccum r, g, b, a;
	 GLint x, y, i;
	 r = (GLaccum) (CC.Accum.ClearColor[0] * acc_scale);
	 g = (GLaccum) (CC.Accum.ClearColor[1] * acc_scale);
	 b = (GLaccum) (CC.Accum.ClearColor[2] * acc_scale);
	 a = (GLaccum) (CC.Accum.ClearColor[3] * acc_scale);
	 for (y=CC.Scissor.Ymin; y<=CC.Scissor.Ymax; y++) {
	    for (x=CC.Scissor.Xmin; x<=CC.Scissor.Xmax; x++) {
	       i = 4 * (y * CC.BufferWidth + x);
	       CC.AccumBuffer[i+0] = r;
	       CC.AccumBuffer[i+1] = g;
	       CC.AccumBuffer[i+2] = b;
	       CC.AccumBuffer[i+3] = a;
	    }
	 }
      }
      else {
	 /* clear whole buffer */
	 if (CC.Accum.ClearColor[0]==0.0 &&
	     CC.Accum.ClearColor[1]==0.0 &&
	     CC.Accum.ClearColor[2]==0.0 &&
	     CC.Accum.ClearColor[3]==0.0) {
	    /* Black */
	    MEMSET( CC.AccumBuffer, 0, buffersize * 4 * sizeof(GLaccum) );
	 }
	 else {
	    /* Not black */
	    GLaccum *acc, r, g, b, a;
	    GLuint i;

	    acc = CC.AccumBuffer;
	    r = (GLaccum) (CC.Accum.ClearColor[0] * acc_scale);
	    g = (GLaccum) (CC.Accum.ClearColor[1] * acc_scale);
	    b = (GLaccum) (CC.Accum.ClearColor[2] * acc_scale);
	    a = (GLaccum) (CC.Accum.ClearColor[3] * acc_scale);
	    for (i=0;i<buffersize;i++) {
	       *acc++ = r;
	       *acc++ = g;
	       *acc++ = b;
	       *acc++ = a;
	    }
	 }
      }
   }
}
