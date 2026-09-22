/* $Id: readpix.c,v 1.14 1996/05/01 15:49:44 brianp Exp $ */

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
$Log: readpix.c,v $
 * Revision 1.14  1996/05/01  15:49:44  brianp
 * optimized glReadPixels() for GLuint and GLushort GL_DEPTH reading
 *
 * Revision 1.13  1996/04/25  20:41:45  brianp
 * replaced gl_read_depth_span() call with DD.read_depth_span()
 *
 * Revision 1.12  1996/04/15  14:13:22  brianp
 * better pixel mapping
 *
 * Revision 1.11  1996/02/19  21:50:00  brianp
 * added support for software alpha buffering
 *
 * Revision 1.10  1995/10/14  16:27:03  brianp
 * use DD.set_buffer
 *
 * Revision 1.9  1995/10/04  22:08:52  brianp
 * replaced i with j in byte swap function calls
 *
 * Revision 1.8  1995/08/31  21:26:39  brianp
 * use DD.read_*_span instead of dd_read_*_span
 *
 * Revision 1.7  1995/07/24  20:34:16  brianp
 * replaced memset() with MEMSET() and memcpy() with MEMCPY()
 *
 * Revision 1.6  1995/06/12  15:52:41  brianp
 * renamed from readpixels.c to readpix.c
 *
 * Revision 1.5  1995/06/12  15:42:37  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.4  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.3  1995/03/13  20:54:53  brianp
 * added read buffer logic
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:27:03  brianp
 * Initial revision
 *
 */


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "alphabuf.h"
#include "context.h"
#include "depth.h"
#include "dd.h"
#include "feedback.h"
#include "list.h"
#include "macros.h"
#include "span.h"
#include "stencil.h"




/*
 * Compute ceiling of integer quotient of A divided by B:
 */
#define CEILING( A, B )  ( (A) % (B) == 0 ? (A)/(B) : (A)/(B)+1 )



/*
 * Flip the order of the 4 bytes in each word in the given array.
 */
static void swap4( GLuint *p, GLuint n )
{
   register GLuint i, a, b;

   for (i=0;i<n;i++) {
      b = p[i];
      a =  (b >> 24)
	| ((b >> 8) & 0xff00)
	| ((b << 8) & 0xff0000)
	| ((b << 24) & 0xff000000);
      p[i] = a;
   }
}


/*
 * Flip the order of the 2 bytes in each word in the given array.
 */
static void swap2( GLushort *p, GLuint n )
{
   register GLuint i;

   for (i=0;i<n;i++) {
      p[i] = (p[i] >> 8) | ((p[i] << 8) & 0xff00);
   }
}






/**********************************************************************/
/*****                         glReadPixels                       *****/
/**********************************************************************/


/*
 * Read a block of color index pixels.
 */
static void read_index_pixels( GLint x, GLint y,
			       GLsizei width, GLsizei height,
			       GLenum type, GLvoid *pixels )
{
   GLint i, j;
   GLuint a, s, k, l, start;

   /* error checking */
   if (CC.RGBAflag) {
      gl_error( GL_INVALID_OPERATION, "glReadPixels" );
      return;
   }

   /* Size of each component */
   switch (type) {
      case GL_UNSIGNED_BYTE:	s = sizeof(GLubyte);	break;
      case GL_BYTE:		s = sizeof(GLbyte);	break;
      case GL_UNSIGNED_SHORT:	s = sizeof(GLushort);	break;
      case GL_SHORT:		s = sizeof(GLshort);	break;
      case GL_UNSIGNED_INT:	s = sizeof(GLuint);	break;
      case GL_INT:		s = sizeof(GLint);	break;
      case GL_FLOAT:		s = sizeof(GLfloat);	break;
      default:
	 gl_error( GL_INVALID_ENUM, "glReadPixels(type)" );
	 return;
   }

   /* Compute packing parameters */
   a = CC.PackAlignment;
   if (CC.PackRowLength>0) {
      l = CC.PackRowLength;
   }
   else {
      l = width;
   }
   /* k = offset between rows in components */
   if (s>=a) {
      k = l;
   }
   else {
      k = a/s * CEILING( s*l, a );
   }

   /* offset to first component returned */
   start = CC.PackSkipRows * k + CC.PackSkipPixels;

   /* process image row by row */
   for (j=0;j<height;j++,y++) {
      GLuint index[MAX_WIDTH];
      (*DD.read_index_span)( width, x, y, index );

      if (CC.Pixel.IndexShift!=0 || CC.Pixel.IndexOffset!=0) {
	 GLuint s;
	 if (CC.Pixel.IndexShift<0) {
	    /* right shift */
	    s = -CC.Pixel.IndexShift;
	    for (i=0;i<width;i++) {
	       index[i] = (index[i] >> s) + CC.Pixel.IndexOffset;
	    }
	 }
	 else {
	    /* left shift */
	    s = CC.Pixel.IndexShift;
	    for (i=0;i<width;i++) {
	       index[i] = (index[i] << s) + CC.Pixel.IndexOffset;
	    }
	 }
      }

      if (CC.Pixel.MapColorFlag) {
	 for (i=0;i<width;i++) {
	    index[i] = CC.Pixel.MapItoI[ index[i] ];
	 }
      }

      switch (type) {
	 case GL_UNSIGNED_BYTE:
	    {
	       GLubyte *dst = (GLubyte *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLubyte) index[i];
	       }
	    }
	    break;
	 case GL_BYTE:
	    {
	       GLbyte *dst = (GLbyte *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLbyte) index[i];
	       }
	    }
	    break;
	 case GL_UNSIGNED_SHORT:
	    {
	       GLushort *dst = (GLushort *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLushort) index[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap2( (GLushort *) pixels + start + j * k, width );
	       }
	    }
	    break;
	 case GL_SHORT:
	    {
	       GLshort *dst = (GLshort *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLshort) index[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap2( (GLushort *) pixels + start + j * k, width );
	       }
	    }
	    break;
	 case GL_UNSIGNED_INT:
	    {
	       GLuint *dst = (GLuint *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLuint) index[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start + j * k, width );
	       }
	    }
	    break;
	 case GL_INT:
	    {
	       GLint *dst = (GLint *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLint) index[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start + j * k, width );
	       }
	    }
	    break;
	 case GL_FLOAT:
	    {
	       GLfloat *dst = (GLfloat *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLfloat) index[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start + j * k, width );
	       }
	    }
	    break;
      }

   }
}



static void read_depth_pixels( GLint x, GLint y,
			       GLsizei width, GLsizei height,
			       GLenum type, GLvoid *pixels )
{
   GLint i, j;
   GLuint a, s, k, l, start;
   GLboolean bias_or_scale;

   /* Error checking */
   if (!CC.DepthBuffer) {
      gl_error( GL_INVALID_OPERATION, "glReadPixels" );
      return;
   }

   bias_or_scale = CC.Pixel.DepthBias!=0.0 || CC.Pixel.DepthScale!=1.0;

   /* Size of each component */
   switch (type) {
      case GL_UNSIGNED_BYTE:	s = sizeof(GLubyte);	break;
      case GL_BYTE:		s = sizeof(GLbyte);	break;
      case GL_UNSIGNED_SHORT:	s = sizeof(GLushort);	break;
      case GL_SHORT:		s = sizeof(GLshort);	break;
      case GL_UNSIGNED_INT:	s = sizeof(GLuint);	break;
      case GL_INT:		s = sizeof(GLint);	break;
      case GL_FLOAT:		s = sizeof(GLfloat);	break;
      default:
	 gl_error( GL_INVALID_ENUM, "glReadPixels(type)" );
	 return;
   }

   /* Compute packing parameters */
   a = CC.PackAlignment;
   if (CC.PackRowLength>0) {
      l = CC.PackRowLength;
   }
   else {
      l = width;
   }
   /* k = offset between rows in components */
   if (s>=a) {
      k = l;
   }
   else {
      k = a/s * CEILING( s*l, a );
   }

   /* offset to first component returned */
   start = CC.PackSkipRows * k + CC.PackSkipPixels;

   if (type==GL_UNSIGNED_SHORT && sizeof(GLdepth)==sizeof(GLushort)
       && !bias_or_scale && !CC.PackSwapBytes) {
      /* Special case: directly read 16-bit unsigned depth values. */
      for (j=0;j<height;j++,y++) {
         GLushort *dst = (GLushort *) pixels + start + j * k;
         (*DD.read_depth_span_int)( width, x, y, (GLdepth *) dst );
      }
   }
   else if (type==GL_UNSIGNED_INT && sizeof(GLdepth)==sizeof(GLuint)
            && !bias_or_scale && !CC.PackSwapBytes) {
      /* Special case: directly read 32-bit unsigned depth values. */
      /* Compute shift value to scale depth values up to 32-bit uints. */
      GLuint shift = 0;
      GLuint max = MAX_DEPTH;
      while ((max&0x80000000)==0) {
         max = max << 1;
         shift++;
      }
      for (j=0;j<height;j++,y++) {
         GLuint *dst = (GLuint *) pixels + start + j * k;
         (*DD.read_depth_span_int)( width, x, y, (GLdepth *) dst );
         for (i=0;i<width;i++) {
            dst[i] = dst[i] << shift;
         }
      }
   }
   else {
      /* General case (slow) */
      for (j=0;j<height;j++,y++) {
         GLfloat depth[MAX_WIDTH];

         (*DD.read_depth_span_float)( width, x, y, depth );

         if (bias_or_scale) {
            for (i=0;i<width;i++) {
               GLfloat d;
               d = depth[i] * CC.Pixel.DepthScale + CC.Pixel.DepthBias;
               depth[i] = CLAMP( d, 0.0, 1.0 );
            }
         }

         switch (type) {
            case GL_UNSIGNED_BYTE:
               {
                  GLubyte *dst = (GLubyte *) pixels + start + j * k;
                  for (i=0;i<width;i++) {
                     *dst++ = FLOAT_TO_UBYTE( depth[i] );
                  }
               }
               break;
            case GL_BYTE:
               {
                  GLbyte *dst = (GLbyte *) pixels + start + j * k;
                  for (i=0;i<width;i++) {
                     *dst++ = FLOAT_TO_BYTE( depth[i] );
                  }
               }
               break;
            case GL_UNSIGNED_SHORT:
               {
                  GLushort *dst = (GLushort *) pixels + start + j * k;
                  for (i=0;i<width;i++) {
                     *dst++ = FLOAT_TO_USHORT( depth[i] );
                  }
                  if (CC.PackSwapBytes) {
                     swap2( (GLushort *) pixels + start + j * k, width );
                  }
               }
               break;
            case GL_SHORT:
               {
                  GLshort *dst = (GLshort *) pixels + start + j * k;
                  for (i=0;i<width;i++) {
                     *dst++ = FLOAT_TO_SHORT( depth[i] );
                  }
                  if (CC.PackSwapBytes) {
                     swap2( (GLushort *) pixels + start + j * k, width );
                  }
               }
               break;
            case GL_UNSIGNED_INT:
               {
                  GLuint *dst = (GLuint *) pixels + start + j * k;
                  for (i=0;i<width;i++) {
                     *dst++ = FLOAT_TO_UINT( depth[i] );
                  }
                  if (CC.PackSwapBytes) {
                     swap4( (GLuint *) pixels + start + j * k, width );
                  }
               }
               break;
            case GL_INT:
               {
                  GLint *dst = (GLint *) pixels + start + j * k;
                  for (i=0;i<width;i++) {
                     *dst++ = FLOAT_TO_INT( depth[i] );
                  }
                  if (CC.PackSwapBytes) {
                     swap4( (GLuint *) pixels + start + j * k, width );
                  }
               }
               break;
            case GL_FLOAT:
               {
                  GLfloat *dst = (GLfloat *) pixels + start + j * k;
                  for (i=0;i<width;i++) {
                     *dst++ = depth[i];
                  }
                  if (CC.PackSwapBytes) {
                     swap4( (GLuint *) pixels + start + j * k, width );
                  }
               }
               break;
         }
      }
   }
}




static void read_stencil_pixels( GLint x, GLint y,
				 GLsizei width, GLsizei height,
				 GLenum type, GLvoid *pixels )
{
   GLint i, j;
   GLuint a, s, k, l, start;
   GLboolean shift_or_offset;

   if (!CC.StencilBuffer) {
      gl_error( GL_INVALID_OPERATION, "glReadPixels" );
      return;
   }

   shift_or_offset = CC.Pixel.IndexShift!=0 || CC.Pixel.IndexOffset!=0;

   /* Size of each component */
   switch (type) {
      case GL_UNSIGNED_BYTE:	s = sizeof(GLubyte);	break;
      case GL_BYTE:		s = sizeof(GLbyte);	break;
      case GL_UNSIGNED_SHORT:	s = sizeof(GLushort);	break;
      case GL_SHORT:		s = sizeof(GLshort);	break;
      case GL_UNSIGNED_INT:	s = sizeof(GLuint);	break;
      case GL_INT:		s = sizeof(GLint);	break;
      case GL_FLOAT:		s = sizeof(GLfloat);	break;
      default:
	 gl_error( GL_INVALID_ENUM, "glReadPixels(type)" );
	 return;
   }

   /* Compute packing parameters */
   a = CC.PackAlignment;
   if (CC.PackRowLength>0) {
      l = CC.PackRowLength;
   }
   else {
      l = width;
   }
   /* k = offset between rows in components */
   if (s>=a) {
      k = l;
   }
   else {
      k = a/s * CEILING( s*l, a );
   }

   /* offset to first component returned */
   start = CC.PackSkipRows * k + CC.PackSkipPixels;

   /* process image row by row */
   for (j=0;j<height;j++,y++) {
      GLubyte stencil[MAX_WIDTH];

      gl_read_stencil_span( width, x, y, stencil );

      if (shift_or_offset) {
	 GLuint s;
	 if (CC.Pixel.IndexShift<0) {
	    /* right shift */
	    s = -CC.Pixel.IndexShift;
	    for (i=0;i<width;i++) {
	       stencil[i] = (stencil[i] >> s) + CC.Pixel.IndexOffset;
	    }
	 }
	 else {
	    /* left shift */
	    s = CC.Pixel.IndexShift;
	    for (i=0;i<width;i++) {
	       stencil[i] = (stencil[i] << s) + CC.Pixel.IndexOffset;
	    }
	 }
      }

      if (CC.Pixel.MapStencilFlag) {
	 for (i=0;i<width;i++) {
	    stencil[i] = CC.Pixel.MapStoS[ stencil[i] ];
	 }
      }

      switch (type) {
	 case GL_UNSIGNED_BYTE:
	    {
	       GLubyte *dst = (GLubyte *) pixels + start + j * k;
	       MEMCPY( dst, stencil, width );
	    }
	    break;
	 case GL_BYTE:
	    {
	       GLbyte *dst = (GLbyte  *) pixels + start + j * k;
	       MEMCPY( dst, stencil, width );
	    }
	    break;
	 case GL_UNSIGNED_SHORT:
	    {
	       GLushort *dst = (GLushort *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLushort) stencil[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap2( (GLushort *) pixels + start +j * k, width );
	       }
	    }
	    break;
	 case GL_SHORT:
	    {
	       GLshort *dst = (GLshort *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLshort) stencil[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap2( (GLushort *) pixels + start +j * k, width );
	       }
	    }
	    break;
	 case GL_UNSIGNED_INT:
	    {
	       GLuint *dst = (GLuint *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLuint) stencil[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start +j * k, width );
	       }
	    }
	    break;
	 case GL_INT:
	    {
	       GLint *dst = (GLint *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLint) stencil[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start +j * k, width );
	       }
	    }
	    break;
	 case GL_FLOAT:
	    {
	       GLfloat *dst = (GLfloat *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  *dst++ = (GLfloat) stencil[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start +j * k, width );
	       }
	    }
	    break;
      }

   }
}



/*
 * Test if scaling or biasing of colors is needed.
 */
static GLboolean scale_or_bias_rgba( void )
{
   if (CC.Pixel.RedScale!=1.0F   || CC.Pixel.RedBias!=0.0F ||
       CC.Pixel.GreenScale!=1.0F || CC.Pixel.GreenBias!=0.0F ||
       CC.Pixel.BlueScale!=1.0F  || CC.Pixel.BlueBias!=0.0F ||
       CC.Pixel.AlphaScale!=1.0F || CC.Pixel.AlphaBias!=0.0F) {
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}



/*
 * Apply scale and bias factors to an array of RGBA pixels.
 */
static void scale_and_bias_rgba( GLint n,
				 GLfloat red[], GLfloat green[],
				 GLfloat blue[], GLfloat alpha[] )
{
   register GLint i;
   register GLfloat r, g, b, a;

   for (i=0;i<n;i++) {
      r = red[i]   * CC.Pixel.RedScale   + CC.Pixel.RedBias;
      g = green[i] * CC.Pixel.GreenScale + CC.Pixel.GreenBias;
      b = blue[i]  * CC.Pixel.BlueScale  + CC.Pixel.BlueBias;
      a = alpha[i] * CC.Pixel.AlphaScale + CC.Pixel.AlphaBias;
      red[i]   = CLAMP( r, 0.0F, 1.0F );
      green[i] = CLAMP( g, 0.0F, 1.0F );
      blue[i]  = CLAMP( b, 0.0F, 1.0F );
      alpha[i] = CLAMP( a, 0.0F, 1.0F );
   }
}



/*
 * Apply pixel mapping to an array of RGBA pixels.
 */
static void map_rgba( GLint n,
		      GLfloat red[], GLfloat green[],
		      GLfloat blue[], GLfloat alpha[] )
{
   GLfloat rscale = CC.Pixel.MapRtoRsize-1;
   GLfloat gscale = CC.Pixel.MapGtoGsize-1;
   GLfloat bscale = CC.Pixel.MapBtoBsize-1;
   GLfloat ascale = CC.Pixel.MapAtoAsize-1;
   GLint i;

   for (i=0;i<n;i++) {
      red[i]   = CC.Pixel.MapRtoR[ (GLint) (red[i]   * rscale) ];
      green[i] = CC.Pixel.MapGtoG[ (GLint) (green[i] * gscale) ];
      blue[i]  = CC.Pixel.MapBtoB[ (GLint) (blue[i]  * bscale) ];
      alpha[i] = CC.Pixel.MapAtoA[ (GLint) (alpha[i] * ascale) ];
   }
}




/*
 * Read R, G, B, A, RGB, L, or LA pixels.
 */
static void read_color_pixels( GLint x, GLint y,
			       GLsizei width, GLsizei height,
			       GLenum format, GLenum type, GLvoid *pixels )
{
   GLint i, j, n, a, s, l, k;
   GLboolean scale_or_bias;
   GLfloat red[MAX_WIDTH], green[MAX_WIDTH], blue[MAX_WIDTH], alpha[MAX_WIDTH];
   GLboolean r_flag, g_flag, b_flag, a_flag, l_flag;
   GLuint start;

   scale_or_bias = scale_or_bias_rgba();

   /* Determine how many / which components to return */
   r_flag = g_flag = b_flag = a_flag = l_flag = GL_FALSE;
   switch (format) {
      case GL_RED:				r_flag = GL_TRUE;  n = 1;  break;
      case GL_GREEN:				g_flag = GL_TRUE;  n = 1;  break;
      case GL_BLUE:				b_flag = GL_TRUE;  n = 1;  break;
      case GL_ALPHA:				a_flag = GL_TRUE;  n = 1;  break;
      case GL_LUMINANCE:			l_flag = GL_TRUE;  n = 1;  break;
      case GL_LUMINANCE_ALPHA:	       l_flag = a_flag = GL_TRUE;  n = 2;  break;
      case GL_RGB:	      r_flag = g_flag = b_flag = GL_TRUE;  n = 3;  break;
      case GL_RGBA:  r_flag = g_flag = b_flag = a_flag = GL_TRUE;  n = 4;  break;
      default:
	 gl_error( GL_INVALID_ENUM, "glReadPixels(format)" );
   }

   /* Size of each component */
   switch (type) {
      case GL_UNSIGNED_BYTE:	s = sizeof(GLubyte);	break;
      case GL_BYTE:		s = sizeof(GLbyte);	break;
      case GL_UNSIGNED_SHORT:	s = sizeof(GLushort);	break;
      case GL_SHORT:		s = sizeof(GLshort);	break;
      case GL_UNSIGNED_INT:	s = sizeof(GLuint);	break;
      case GL_INT:		s = sizeof(GLint);	break;
      case GL_FLOAT:		s = sizeof(GLfloat);	break;
      default:
	 gl_error( GL_INVALID_ENUM, "glReadPixels(type)" );
	 return;
   }

   /* Compute packing parameters */
   a = CC.PackAlignment;
   if (CC.PackRowLength>0) {
      l = CC.PackRowLength;
   }
   else {
      l = width;
   }
   /* k = offset between rows in components */
   if (s>=a) {
      k = n * l;
   }
   else {
      k = a/s * CEILING( s*n*l, a );
   }

   /* offset to first component returned */
   start = CC.PackSkipRows * k + CC.PackSkipPixels * n;

   /* process image row by row */
   for (j=0;j<height;j++,y++) {

      /*
       * Read the pixels from frame buffer
       */
      if (CC.RGBAflag) {
	 GLubyte r[MAX_WIDTH], g[MAX_WIDTH], b[MAX_WIDTH], a[MAX_WIDTH];
	 GLfloat rscale = 1.0F / CC.RedScale;
	 GLfloat gscale = 1.0F / CC.GreenScale;
	 GLfloat bscale = 1.0F / CC.BlueScale;
	 GLfloat ascale = 1.0F / CC.AlphaScale;

	 /* read colors and convert to floats */
	 (*DD.read_color_span)( width, x, y, r, g, b, a );
         if (CC.RasterMask & ALPHABUF_BIT) {
            gl_read_alpha_span( width, x, y, a );
         }
	 for (i=0;i<width;i++) {
	    red[i]   = r[i] * rscale;
	    green[i] = g[i] * gscale;
	    blue[i]  = b[i] * bscale;
	    alpha[i] = a[i] * ascale;
	 }

	 if (scale_or_bias) {
	    scale_and_bias_rgba( width, red, green, blue, alpha );
	 }
	 if (CC.Pixel.MapColorFlag) {
	    map_rgba( width, red, green, blue, alpha );
	 }
      }
      else {
	 /* convert CI values to RGBA */
	 GLuint index[MAX_WIDTH];
	 (*DD.read_index_span)( width, x, y, index );

	 if (CC.Pixel.IndexShift!=0 || CC.Pixel.IndexOffset!=0) {
	    GLuint s;
	    if (CC.Pixel.IndexShift<0) {
	       /* right shift */
	       s = -CC.Pixel.IndexShift;
	       for (i=0;i<width;i++) {
		  index[i] = (index[i] >> s) + CC.Pixel.IndexOffset;
	       }
	    }
	    else {
	       /* left shift */
	       s = CC.Pixel.IndexShift;
	       for (i=0;i<width;i++) {
		  index[i] = (index[i] << s) + CC.Pixel.IndexOffset;
	       }
	    }
	 }

	 for (i=0;i<width;i++) {
	    red[i]   = CC.Pixel.MapItoR[ index[i] ];
	    green[i] = CC.Pixel.MapItoG[ index[i] ];
	    blue[i]  = CC.Pixel.MapItoB[ index[i] ];
	    alpha[i] = CC.Pixel.MapItoA[ index[i] ];
	 }
      }

      /*
       * Pack/transfer/store the pixels
       */

      switch (type) {
	 case GL_UNSIGNED_BYTE:
	    {
	       GLubyte *dst = (GLubyte *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  if (r_flag)  *dst++ = FLOAT_TO_UBYTE( red[i] );
		  if (g_flag)  *dst++ = FLOAT_TO_UBYTE( green[i] );
		  if (b_flag)  *dst++ = FLOAT_TO_UBYTE( blue[i] );
		  if (l_flag)  *dst++ = FLOAT_TO_UBYTE(red[i]+green[i]+blue[i]);
		  if (a_flag)  *dst++ = FLOAT_TO_UBYTE( alpha[i] );
	       }
	    }
	    break;
	 case GL_BYTE:
	    {
	       GLbyte *dst = (GLbyte *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  if (r_flag)  *dst++ = FLOAT_TO_BYTE( red[i] );
		  if (g_flag)  *dst++ = FLOAT_TO_BYTE( green[i] );
		  if (b_flag)  *dst++ = FLOAT_TO_BYTE( blue[i] );
		  if (l_flag)  *dst++ = FLOAT_TO_BYTE(red[i]+green[i]+blue[i]);
		  if (a_flag)  *dst++ = FLOAT_TO_BYTE( alpha[i] );
	       }
	    }
	    break;
	 case GL_UNSIGNED_SHORT:
	    {
	       GLushort *dst = (GLushort *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  if (r_flag)  *dst++ = FLOAT_TO_USHORT( red[i] );
		  if (g_flag)  *dst++ = FLOAT_TO_USHORT( green[i] );
		  if (b_flag)  *dst++ = FLOAT_TO_USHORT( blue[i] );
		  if (l_flag)  *dst++ = FLOAT_TO_USHORT(red[i]+green[i]+blue[i]);
		  if (a_flag)  *dst++ = FLOAT_TO_USHORT( alpha[i] );
	       }
	    }
	    if (CC.PackSwapBytes) {
	       swap2( (GLushort *) pixels + start + j * k, width*n );
	    }
	    break;
	 case GL_SHORT:
	    {
	       GLshort *dst = (GLshort *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  if (r_flag)  *dst++ = FLOAT_TO_SHORT( red[i] );
		  if (g_flag)  *dst++ = FLOAT_TO_SHORT( green[i] );
		  if (b_flag)  *dst++ = FLOAT_TO_SHORT( blue[i] );
		  if (l_flag)  *dst++ = FLOAT_TO_SHORT(red[i]+green[i]+blue[i]);
		  if (a_flag)  *dst++ = FLOAT_TO_SHORT( alpha[i] );
	       }
	       if (CC.PackSwapBytes) {
		  swap2( (GLushort *) pixels + start + j * k, width*n );
	       }
	    }
	    break;
	 case GL_UNSIGNED_INT:
	    {
	       GLuint *dst = (GLuint *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  if (r_flag)  *dst++ = FLOAT_TO_UINT( red[i] );
		  if (g_flag)  *dst++ = FLOAT_TO_UINT( green[i] );
		  if (b_flag)  *dst++ = FLOAT_TO_UINT( blue[i] );
		  if (l_flag)  *dst++ = FLOAT_TO_UINT(red[i]+green[i]+blue[i]);
		  if (a_flag)  *dst++ = FLOAT_TO_UINT( alpha[i] );
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start + j * k, width*n );
	       }
	    }
	    break;
	 case GL_INT:
	    {
	       GLint *dst = (GLint *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  if (r_flag)  *dst++ = FLOAT_TO_INT( red[i] );
		  if (g_flag)  *dst++ = FLOAT_TO_INT( green[i] );
		  if (b_flag)  *dst++ = FLOAT_TO_INT( blue[i] );
		  if (l_flag)  *dst++ = FLOAT_TO_INT(red[i]+green[i]+blue[i]);
		  if (a_flag)  *dst++ = FLOAT_TO_INT( alpha[i] );
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start + j * k, width*n );
	       }
	    }
	    break;
	 case GL_FLOAT:
	    {
	       GLfloat *dst = (GLfloat *) pixels + start + j * k;
	       for (i=0;i<width;i++) {
		  if (r_flag)  *dst++ = red[i];
		  if (g_flag)  *dst++ = green[i];
		  if (b_flag)  *dst++ = blue[i];
		  if (l_flag)  *dst++ = red[i]+green[i]+blue[i];
		  if (a_flag)  *dst++ = alpha[i];
	       }
	       if (CC.PackSwapBytes) {
		  swap4( (GLuint *) pixels + start + j * k, width*n );
	       }
	    }
	    break;
      }

   }
}



void glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height,
		   GLenum format, GLenum type, GLvoid *pixels )
{
   (void) (*DD.set_buffer)( CC.Pixel.ReadBuffer );

   switch (format) {
      case GL_COLOR_INDEX:
         read_index_pixels( x, y, width, height, type, pixels );
	 break;
      case GL_STENCIL_INDEX:
	 read_stencil_pixels( x, y, width, height, type, pixels );
         break;
      case GL_DEPTH_COMPONENT:
	 read_depth_pixels( x, y, width, height, type, pixels );
	 break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_RGBA:
	 read_color_pixels( x, y, width, height, format, type, pixels );
	 break;
      default:
	 gl_error( GL_INVALID_ENUM, "glReadPixels(format)" );
   }

   (void) (*DD.set_buffer)( CC.Color.DrawBuffer );
}
