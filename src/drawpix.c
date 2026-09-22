/* $Id: drawpix.c,v 1.25 1996/05/15 16:37:28 brianp Exp $ */

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
$Log: drawpix.c,v $
 * Revision 1.25  1996/05/15  16:37:28  brianp
 * added optimization for GL_LUMINANCE images from Frederic Devernay
 *
 * Revision 1.24  1996/05/01  15:48:57  brianp
 * optimized glDrawPixels() for GLuint and GLushort GL_DEPTH drawing
 *
 * Revision 1.23  1996/04/25  20:41:30  brianp
 * added support for DD.draw_pixels()
 *
 * Revision 1.22  1996/04/15  14:13:46  brianp
 * better pixel mapping
 *
 * Revision 1.21  1996/02/26  15:07:16  brianp
 * replaced CC.Current.Color with CC.Current.IntColor
 *
 * Revision 1.20  1995/12/19  16:43:05  brianp
 * added quickdraw_rgb() function
 *
 * Revision 1.19  1995/12/18  17:27:05  brianp
 * use new GLdepth datatype
 *
 * Revision 1.18  1995/11/09  15:15:29  brianp
 * allow glDrawPixels width and height to be zero
 *
 * Revision 1.17  1995/11/03  17:41:17  brianp
 * fixed a few things for C++ compilation
 *
 * Revision 1.16  1995/10/19  15:48:51  brianp
 * replaced a for loop with MEMSET
 *
 * Revision 1.15  1995/10/16  15:25:49  brianp
 * added zooming for stencil drawing/copying
 *
 * Revision 1.14  1995/10/14  16:28:07  brianp
 * added glPixelZoom support
 *
 * Revision 1.13  1995/09/28  16:18:48  brianp
 * replaced a loop in draw_color_pixels with a MEMSET
 *
 * Revision 1.12  1995/09/15  18:46:10  brianp
 * directly call DD.write_color_span under right conditions
 *
 * Revision 1.11  1995/07/24  20:35:20  brianp
 * replaced memset() with MEMSET() and memcpy() with MEMCPY()
 *
 * Revision 1.10  1995/07/24  18:59:42  brianp
 * convert real window coords to ints with rounding, not truncating
 *
 * Revision 1.9  1995/06/12  15:53:40  brianp
 * removed garbage chars from end of file
 *
 * Revision 1.8  1995/06/12  15:50:44  brianp
 * renamed from drawpixels.[ch] to drawpix.[ch]
 *
 * Revision 1.7  1995/06/12  15:38:55  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.6  1995/05/31  14:58:26  brianp
 * check that rasterpos is valid before drawing
 *
 * Revision 1.5  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.4  1995/05/12  19:26:43  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.3  1995/05/10  18:43:58  brianp
 * removed extraneous parenthesis
 * moved clip_pixels() to span.c
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:20:50  brianp
 * Initial revision
 *
 */


#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "dd.h"
#include "feedback.h"
#include "list.h"
#include "macros.h"
#include "pixel.h"
#include "span.h"
#include "stencil.h"




/* TODO:  apply texture mapping to fragments */



static void draw_index_pixels( GLsizei width, GLsizei height,
			       GLenum type, const GLvoid *pixels )
{
   GLint x, y, desty;
   GLuint i, j;
   GLdepth zspan[MAX_WIDTH];
   GLboolean zoom;

   zoom = CC.Pixel.ZoomX!=1.0 || CC.Pixel.ZoomY!=1.0;

   /* Position, depth of pixels */
   x = (GLint) (CC.Current.RasterPos[0] + 0.5F);
   y = (GLint) (CC.Current.RasterPos[1] + 0.5F);
   desty = y;
   if (CC.Depth.Test) {
      GLdepth zval = (GLdepth) (CC.Current.RasterPos[2] * DEPTH_SCALE);
      for (i=0;i<width;i++) {
	 zspan[i] = zval;
      }
   }

   /* process the image row by row */
   for (i=0;i<height;i++,y++) {
      GLuint ispan[MAX_WIDTH];

      /* convert to uints */
      switch (type) {
	 case GL_UNSIGNED_BYTE:
	    {
	       GLubyte *src = (GLubyte *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  ispan[j] = (GLuint) *src++;
	       }
	    }
	    break;
	 case GL_BYTE:
	    {
	       GLbyte *src = (GLbyte *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  ispan[j] = (GLuint) *src++;
	       }
	    }
	    break;
	 case GL_UNSIGNED_SHORT:
	    {
	       GLushort *src = (GLushort *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  ispan[j] = (GLuint) *src++;
	       }
	    }
	    break;
	 case GL_SHORT:
	    {
	       GLshort *src = (GLshort *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  ispan[j] = (GLuint) *src++;
	       }
	    }
	    break;
	 case GL_UNSIGNED_INT:
	    {
	       GLuint *src = (GLuint *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  ispan[j] = *src++;
	       }
	    }
	    break;
	 case GL_INT:
	    {
	       GLint *src = (GLint *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  ispan[j] = (GLuint) *src++;
	       }
	    }
	    break;
	 case GL_BITMAP:
	    /* TODO */
	    break;
	 case GL_FLOAT:
	    {
	       GLfloat *src = (GLfloat *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  ispan[j] = (GLuint) (GLint) *src++;
	       }
	    }
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "Internal: draw_index_pixels" );
      }

      /* apply shift and offset */
      if (CC.Pixel.IndexOffset || CC.Pixel.IndexShift) {
	 if (CC.Pixel.IndexShift>=0) {
	    for (j=0;j<width;j++) {
	       ispan[j] = (ispan[j] << CC.Pixel.IndexShift)
		          + CC.Pixel.IndexOffset;
	    }
	 }
	 else {
	    for (j=0;j<width;j++) {
	       ispan[j] = (ispan[j] >> -CC.Pixel.IndexShift)
		          + CC.Pixel.IndexOffset;
	    }
	 }
      }

      if (CC.RGBAflag) {
	 /* Convert index to RGBA and write to frame buffer */
	 GLubyte red[MAX_WIDTH], green[MAX_WIDTH];
	 GLubyte blue[MAX_WIDTH], alpha[MAX_WIDTH];
	 for (j=0;j<width;j++) {
	    red[j]   = (GLint) (CC.Pixel.MapItoR[ispan[j]] * CC.RedScale);
	    green[j] = (GLint) (CC.Pixel.MapItoG[ispan[j]] * CC.GreenScale);
	    blue[j]  = (GLint) (CC.Pixel.MapItoB[ispan[j]] * CC.BlueScale);
	    alpha[j] = (GLint) (CC.Pixel.MapItoA[ispan[j]] * CC.AlphaScale);
	 }
         if (zoom) {
            gl_write_zoomed_color_span( width, x, y, zspan,
                                        red, green, blue, alpha, desty );
         }
         else {
            gl_write_color_span( width, x, y, zspan,
                                 red, green, blue, alpha, GL_BITMAP );
         }
      }
      else {
	 /* optionally apply index map then write to frame buffer */
	 if (CC.Pixel.MapColorFlag) {
	    for (j=0;j<width;j++) {
	       ispan[j] = CC.Pixel.MapItoI[ispan[j]];
	    }
	 }
         if (zoom) {
            gl_write_zoomed_index_span( width, x, y, zspan, ispan, desty );
         }
         else {
            gl_write_index_span( width, x, y, zspan, ispan, GL_BITMAP );
         }
      }
   }

}



static void draw_stencil_pixels( GLsizei width, GLsizei height,
			         GLenum type, const GLvoid *pixels )
{
   GLint x, y, desty;
   GLuint i, j;
   GLboolean zoom;

   zoom = CC.Pixel.ZoomX!=1.0 || CC.Pixel.ZoomY!=1.0;

   /* Position, depth of pixels */
   x = (GLint) (CC.Current.RasterPos[0] + 0.5F);
   y = (GLint) (CC.Current.RasterPos[1] + 0.5F);
   desty = y;

   /* process the image row by row */
   for (i=0;i<height;i++,y++) {
      GLubyte stencil[MAX_WIDTH];

      /* convert to ubytes */
      switch (type) {
	 case GL_UNSIGNED_BYTE:
	    {
	       GLubyte *src = (GLubyte *) pixels + i * width;
	       MEMCPY( stencil, src, width );
	    }
	    break;
	 case GL_BYTE:
	    {
	       GLbyte *src = (GLbyte *) pixels + i * width;
	       MEMCPY( stencil, src, width );
	    }
	    break;
	 case GL_UNSIGNED_SHORT:
	    {
	       GLushort *src = (GLushort *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  stencil[j] = (GLubyte) ((*src++) & 0xff);
	       }
	    }
	    break;
	 case GL_SHORT:
	    {
	       GLshort *src = (GLshort *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  stencil[j] = (GLubyte) ((*src++) & 0xff);
	       }
	    }
	    break;
	 case GL_UNSIGNED_INT:
	    {
	       GLuint *src = (GLuint *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  stencil[j] = (GLubyte) ((*src++) & 0xff);
	       }
	    }
	    break;
	 case GL_INT:
	    {
	       GLint *src = (GLint *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  stencil[j] = (GLubyte) ((*src++) & 0xff);
	       }
	    }
	    break;
	 case GL_BITMAP:
	    /* TODO */
	    break;
	 case GL_FLOAT:
	    {
	       GLfloat *src = (GLfloat *) pixels + i * width;
	       for (j=0;j<width;j++) {
		  stencil[j] = (GLubyte) (((GLint) *src++) & 0xff);
	       }
	    }
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "Internal: draw_stencil_pixels" );
      }

      /* apply shift and offset */
      if (CC.Pixel.IndexOffset || CC.Pixel.IndexShift) {
	 if (CC.Pixel.IndexShift>=0) {
	    for (j=0;j<width;j++) {
	       stencil[j] = (stencil[j] << CC.Pixel.IndexShift)
		          + CC.Pixel.IndexOffset;
	    }
	 }
	 else {
	    for (j=0;j<width;j++) {
	       stencil[j] = (stencil[j] >> -CC.Pixel.IndexShift)
		          + CC.Pixel.IndexOffset;
	    }
	 }
      }

      /* mapping */
      if (CC.Pixel.MapStencilFlag) {
	 for (j=0;j<width;j++) {
	    stencil[j] = CC.Pixel.MapStoS[ stencil[j] ];
	 }
      }

      /* write stencil values to stencil buffer */
      if (zoom) {
         gl_write_zoomed_stencil_span( (GLuint) width, x, y, stencil, desty );
      }
      else {
         gl_write_stencil_span( (GLuint) width, x, y, stencil );
      }
   }
}



static void draw_depth_pixels( GLsizei width, GLsizei height,
			       GLenum type, const GLvoid *pixels )
{
   GLint x, y, desty;
   GLubyte red[MAX_WIDTH], green[MAX_WIDTH], blue[MAX_WIDTH], alpha[MAX_WIDTH];
   GLuint ispan[MAX_WIDTH];
   GLboolean bias_or_scale;
   GLboolean zoom;

   bias_or_scale = CC.Pixel.DepthBias!=0.0 || CC.Pixel.DepthScale!=1.0;
   zoom = CC.Pixel.ZoomX!=1.0 || CC.Pixel.ZoomY!=1.0;

   /* Position, depth of pixels */
   x = (GLint) (CC.Current.RasterPos[0] + 0.5F);
   y = (GLint) (CC.Current.RasterPos[1] + 0.5F);
   desty = y;

   /* Color or index */
   if (CC.RGBAflag) {
      GLint r, g, b, a;
      r = (GLint) (CC.Current.RasterColor[0] * CC.RedScale);
      g = (GLint) (CC.Current.RasterColor[1] * CC.GreenScale);
      b = (GLint) (CC.Current.RasterColor[2] * CC.BlueScale);
      a = (GLint) (CC.Current.RasterColor[3] * CC.AlphaScale);
      MEMSET( red,   r, width );
      MEMSET( green, g, width );
      MEMSET( blue,  b, width );
      MEMSET( alpha, a, width );
   }
   else {
      GLuint i;
      for (i=0;i<width;i++) {
	 ispan[i] = CC.Current.RasterIndex;
      }
   }

   if (type==GL_UNSIGNED_SHORT && sizeof(GLdepth)==sizeof(GLushort)
       && !bias_or_scale && !zoom && CC.RGBAflag) {
      /* Special case: directly write 16-bit depth values */
      GLuint j;
      for (j=0;j<height;j++,y++) {
         GLdepth *zptr = (GLdepth *) pixels + j * width;
         gl_write_color_span( width, x, y, zptr,
                              red, green, blue, alpha, GL_BITMAP );
      }
   }
   else if (type==GL_UNSIGNED_INT && sizeof(GLdepth)==sizeof(GLuint)
       && !bias_or_scale && !zoom && CC.RGBAflag) {
      /* Special case: directly write 32-bit depth values */
      GLuint i, j;
      /* Compute shift value to scale 32-bit uints down to depth values. */
      GLuint shift = 0;
      GLuint max = MAX_DEPTH;
      while ((max&0x80000000)==0) {
         max = max << 1;
         shift++;
      }
      for (j=0;j<height;j++,y++) {
         GLdepth zspan[MAX_WIDTH];
         GLuint *zptr = (GLuint *) pixels + j * width;
         for (i=0;i<width;i++) {
            zspan[i] = zptr[i] >> shift;
         }
         gl_write_color_span( width, x, y, zspan,
                              red, green, blue, alpha, GL_BITMAP );
      }
   }
   else {
      /* General case (slower) */
      GLuint i, j;

      /* process image row by row */
      for (i=0;i<height;i++,y++) {
         GLfloat depth[MAX_WIDTH];
         GLdepth zspan[MAX_WIDTH];

         switch (type) {
            case GL_UNSIGNED_BYTE:
               {
                  GLubyte *src = (GLubyte *) pixels + i * width;
                  for (j=0;j<width;j++) {
                     depth[j] = UBYTE_TO_FLOAT( *src++ );
                  }
               }
               break;
            case GL_BYTE:
               {
                  GLbyte *src = (GLbyte *) pixels + i * width;
                  for (j=0;j<width;j++) {
                     depth[j] = BYTE_TO_FLOAT( *src++ );
                  }
               }
               break;
            case GL_UNSIGNED_SHORT:
               {
                  GLushort *src = (GLushort *) pixels + i * width;
                  for (j=0;j<width;j++) {
                     depth[j] = USHORT_TO_FLOAT( *src++ );
                  }
               }
               break;
            case GL_SHORT:
               {
                  GLshort *src = (GLshort *) pixels + i * width;
                  for (j=0;j<width;j++) {
                     depth[j] = SHORT_TO_FLOAT( *src++ );
                  }
               }
               break;
            case GL_UNSIGNED_INT:
               {
                  GLuint *src = (GLuint *) pixels + i * width;
                  for (j=0;j<width;j++) {
                     depth[j] = UINT_TO_FLOAT( *src++ );
                  }
               }
               break;
            case GL_INT:
               {
                  GLint *src = (GLint *) pixels + i * width;
                  for (j=0;j<width;j++) {
                     depth[j] = INT_TO_FLOAT( *src++ );
                  }
               }
               break;
            case GL_FLOAT:
               {
                  GLfloat *src = (GLfloat *) pixels + i * width;
                  for (j=0;j<width;j++) {
                     depth[j] = *src++;
                  }
               }
               break;
         }

         /* apply depth scale and bias */
         if (CC.Pixel.DepthScale!=1.0 || CC.Pixel.DepthBias!=0.0) {
            for (j=0;j<width;j++) {
               depth[j] = depth[j] * CC.Pixel.DepthScale + CC.Pixel.DepthBias;
            }
         }

         /* clamp depth values to [0,1] and convert from floats to integers */
         for (j=0;j<width;j++) {
            zspan[j] = (GLdepth) (CLAMP( depth[j], 0.0F, 1.0F ) * DEPTH_SCALE);
         }

         if (CC.RGBAflag) {
            if (zoom) {
               gl_write_zoomed_color_span( width, x, y, zspan,
                                           red, green, blue, alpha, desty );
            }
            else {
               gl_write_color_span( width, x, y, zspan,
                                    red, green, blue, alpha, GL_BITMAP );
            }
         }
         else {
            if (zoom) {
               gl_write_zoomed_index_span( width, x, y, zspan,
                                           ispan, GL_BITMAP );
            }
            else {
               gl_write_index_span( width, x, y, zspan, ispan, GL_BITMAP );
            }
         }

      }
   }
}



static void draw_color_pixels( GLsizei width, GLsizei height, GLenum format,
			       GLenum type, const GLvoid *pixels )
{
   GLuint i, j;
   GLint x, y, desty;
   GLdepth zspan[MAX_WIDTH];
   GLboolean scale_or_bias, quick_draw;
   GLboolean zoom;

   zoom = CC.Pixel.ZoomX!=1.0 || CC.Pixel.ZoomY!=1.0;

   /* Position, depth of pixels */
   x = (GLint) (CC.Current.RasterPos[0] + 0.5F);
   y = (GLint) (CC.Current.RasterPos[1] + 0.5F);
   desty = y;
   if (CC.Depth.Test) {
      /* fill in array of z values */
      GLdepth z = (GLdepth) (CC.Current.RasterPos[2] * DEPTH_SCALE);
      for (i=0;i<width;i++) {
	 zspan[i] = z;
      }
   }

   /* Determine if scaling and/or biasing is needed */
   if (CC.Pixel.RedScale!=1.0F   || CC.Pixel.RedBias!=0.0F ||
       CC.Pixel.GreenScale!=1.0F || CC.Pixel.GreenBias!=0.0F ||
       CC.Pixel.BlueScale!=1.0F  || CC.Pixel.BlueBias!=0.0F ||
       CC.Pixel.AlphaScale!=1.0F || CC.Pixel.AlphaBias!=0.0F) {
      scale_or_bias = GL_TRUE;
   }
   else {
      scale_or_bias = GL_FALSE;
   }

   /* Determine if we can directly call the device driver function */
   if (CC.RasterMask==0 && !zoom && x>=0 && y>=0
       && x+width<=CC.BufferWidth && y+height<=CC.BufferHeight) {
      quick_draw = GL_TRUE;
   }
   else {
      quick_draw = GL_FALSE;
   }

   /* First check for common cases */
   if (type==GL_UNSIGNED_BYTE && (format==GL_RGB || format == GL_LUMINANCE)
       && !CC.Pixel.MapColorFlag && !scale_or_bias
       && CC.RedScale==255.0 && CC.GreenScale==255.0 && CC.BlueScale==255.0) {
      DEFARRAY( GLubyte, alpha, MAX_WIDTH );
      GLubyte *src = (GLubyte *) pixels;
      /* constant alpha */
      MEMSET( alpha, (GLint) CC.AlphaScale, width );
      if (format == GL_RGB) {
	 /* 8-bit RGB pixels */
	 DEFARRAY( GLubyte, red, MAX_WIDTH );
	 DEFARRAY( GLubyte, green, MAX_WIDTH );
	 DEFARRAY( GLubyte, blue, MAX_WIDTH );
	 for (i=0;i<height;i++,y++) {
	    for (j=0;j<width;j++) {
	       red[j]   = *src++;
	       green[j] = *src++;
	       blue[j]  = *src++;
	    }
	    if (quick_draw) {
	       (*DD.write_color_span)( width, x,y, red, green, blue, alpha, NULL);
	    }
	    else if (zoom) {
	       gl_write_zoomed_color_span( (GLuint) width, x, y, zspan,
					   red, green, blue, alpha, desty );
	    }
	    else {
	       gl_write_color_span( (GLuint) width, x, y, zspan,
				    red, green, blue, alpha, GL_BITMAP );
	    }
	 }
	 UNDEFARRAY( red );
	 UNDEFARRAY( green );
	 UNDEFARRAY( blue );
      }
      else {
	 /* 8-bit Luminance pixels */
	 GLubyte *lum = (GLubyte *) pixels;
	 for (i=0;i<height;i++,y++,lum+=width) {
	    if (quick_draw) {
	       (*DD.write_color_span)( width, x,y, lum, lum, lum, alpha, NULL);
	    }
	    else if (zoom) {
	       gl_write_zoomed_color_span( (GLuint) width, x, y, zspan,
					   lum, lum, lum, alpha, desty );
	    }
	    else {
	       gl_write_color_span( (GLuint) width, x, y, zspan,
				    lum, lum, lum, alpha, GL_BITMAP );
	    }
	 }
      }
      UNDEFARRAY( alpha );
   }
   else {
      /* General solution */
      GLboolean r_flag, g_flag, b_flag, a_flag, l_flag;
      GLuint components;

      r_flag = g_flag = b_flag = a_flag = l_flag = GL_FALSE;
      switch (format) {
	 case GL_RED:
	    r_flag = GL_TRUE;
	    components = 1;
	    break;
	 case GL_GREEN:
	    g_flag = GL_TRUE;
	    components = 1;
	    break;
	 case GL_BLUE:
	    b_flag = GL_TRUE;
	    components = 1;
	    break;
	 case GL_ALPHA:
	    a_flag = GL_TRUE;
	    components = 1;
	    break;
	 case GL_RGB:
	    r_flag = g_flag = b_flag = GL_TRUE;
	    components = 3;
	    break;
	 case GL_LUMINANCE:
	    l_flag = GL_TRUE;
	    components = 1;
	    break;
	 case GL_LUMINANCE_ALPHA:
	    l_flag = a_flag = GL_TRUE;
	    components = 2;
	    break;
	 case GL_RGBA:
	    r_flag = g_flag = b_flag = a_flag = GL_TRUE;
	    components = 4;
	    break;
      }

      /* process the image row by row */
      for (i=0;i<height;i++,y++) {
	 GLfloat rf[MAX_WIDTH], gf[MAX_WIDTH], bf[MAX_WIDTH], af[MAX_WIDTH];
	 GLubyte red[MAX_WIDTH], green[MAX_WIDTH];
	 GLubyte blue[MAX_WIDTH], alpha[MAX_WIDTH];

	 /* convert to floats */
	 switch (type) {
	    case GL_UNSIGNED_BYTE:
	       {
		  GLubyte *src = (GLubyte *) pixels + i * width * components;
		  for (j=0;j<width;j++) {
		     if (l_flag) {
			rf[j] = gf[j] = bf[j] = UBYTE_TO_FLOAT(*src++);
		     }
		     else {
			rf[j] = r_flag ? UBYTE_TO_FLOAT(*src++) : 0.0;
			gf[j] = g_flag ? UBYTE_TO_FLOAT(*src++) : 0.0;
			bf[j] = b_flag ? UBYTE_TO_FLOAT(*src++) : 0.0;
		     }
		     af[j] = a_flag ? UBYTE_TO_FLOAT(*src++) : 1.0;
		  }
	       }
	       break;
	    case GL_BYTE:
	       {
		  GLbyte *src = (GLbyte *) pixels + i * width * components;
		  for (j=0;j<width;j++) {
		     if (l_flag) {
			rf[j] = gf[j] = bf[j] = BYTE_TO_FLOAT(*src++);
		     }
		     else {
			rf[j] = r_flag ? BYTE_TO_FLOAT(*src++) : 0.0;
			gf[j] = g_flag ? BYTE_TO_FLOAT(*src++) : 0.0;
			bf[j] = b_flag ? BYTE_TO_FLOAT(*src++) : 0.0;
		     }
		     af[j] = a_flag ? BYTE_TO_FLOAT(*src++) : 1.0;
		  }
	       }
	       break;
	    case GL_BITMAP:
	       /* special case */
	       break;
	    case GL_UNSIGNED_SHORT:
	       {
		  GLushort *src = (GLushort *) pixels + i * width * components;
		  for (j=0;j<width;j++) {
		     if (l_flag) {
			rf[j] = gf[j] = bf[j] = USHORT_TO_FLOAT(*src++);
		     }
		     else {
			rf[j] = r_flag ? USHORT_TO_FLOAT(*src++) : 0.0;
			gf[j] = g_flag ? USHORT_TO_FLOAT(*src++) : 0.0;
			bf[j] = b_flag ? USHORT_TO_FLOAT(*src++) : 0.0;
		     }
		     af[j] = a_flag ? USHORT_TO_FLOAT(*src++) : 1.0;
		  }
	       }
	       break;
	    case GL_SHORT:
	       {
		  GLshort *src = (GLshort *) pixels + i * width * components;
		  for (j=0;j<width;j++) {
		     if (l_flag) {
			rf[j] = gf[j] = bf[j] = SHORT_TO_FLOAT(*src++);
		     }
		     else {
			rf[j] = r_flag ? SHORT_TO_FLOAT(*src++) : 0.0;
			gf[j] = g_flag ? SHORT_TO_FLOAT(*src++) : 0.0;
			bf[j] = b_flag ? SHORT_TO_FLOAT(*src++) : 0.0;
		     }
		     af[j] = a_flag ? SHORT_TO_FLOAT(*src++) : 1.0;
		  }
	       }
	       break;
	    case GL_UNSIGNED_INT:
	       {
		  GLuint *src = (GLuint *) pixels + i * width * components;
		  for (j=0;j<width;j++) {
		     if (l_flag) {
			rf[j] = gf[j] = bf[j] = UINT_TO_FLOAT(*src++);
		     }
		     else {
			rf[j] = r_flag ? UINT_TO_FLOAT(*src++) : 0.0;
			gf[j] = g_flag ? UINT_TO_FLOAT(*src++) : 0.0;
			bf[j] = b_flag ? UINT_TO_FLOAT(*src++) : 0.0;
		     }
		     af[j] = a_flag ? af[j] = UINT_TO_FLOAT(*src++) : 1.0;
		  }
	       }
	       break;
	    case GL_INT:
	       {
		  GLint *src = (GLint *) pixels + i * width * components;
		  for (j=0;j<width;j++) {
		     if (l_flag) {
			rf[j] = gf[j] = bf[j] = INT_TO_FLOAT(*src++);
		     }
		     else {
			rf[j] = r_flag ? INT_TO_FLOAT(*src++) : 0.0;
			gf[j] = g_flag ? INT_TO_FLOAT(*src++) : 0.0;
			bf[j] = b_flag ? INT_TO_FLOAT(*src++) : 0.0;
		     }
		     af[j] = a_flag ? INT_TO_FLOAT(*src++) : 1.0;
		  }
	       }
	       break;
	    case GL_FLOAT:
	       {
		  GLfloat *src = (GLfloat *) pixels + i * width * components;
		  for (j=0;j<width;j++) {
		     if (l_flag) {
			rf[j] = gf[j] = bf[j] = *src++;
		     }
		     else {
			rf[j] = r_flag ? *src++ : 0.0;
			gf[j] = g_flag ? *src++ : 0.0;
			bf[j] = b_flag ? *src++ : 0.0;
		     }
		     af[j] = a_flag ? *src++ : 1.0;
		  }
	       }
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glDrawPixels" );
	       return;
	 }

	 /* apply scale and bias */
	 if (scale_or_bias) {
	    for (j=0;j<width;j++) {
	       GLfloat r, g, b, a;
	       r = rf[j] * CC.Pixel.RedScale   + CC.Pixel.RedBias;
	       g = gf[j] * CC.Pixel.GreenScale + CC.Pixel.GreenBias;
	       b = bf[j] * CC.Pixel.BlueScale  + CC.Pixel.BlueBias;
	       a = af[j] * CC.Pixel.AlphaScale + CC.Pixel.AlphaBias;
	       rf[j] = CLAMP( r, 0.0, 1.0 );
	       gf[j] = CLAMP( g, 0.0, 1.0 );
	       bf[j] = CLAMP( b, 0.0, 1.0 );
	       af[j] = CLAMP( a, 0.0, 1.0 );
	    }
	 }

	 /* apply pixel mappings */
	 if (CC.Pixel.MapColorFlag) {
            GLfloat rscale = CC.Pixel.MapRtoRsize-1;
            GLfloat gscale = CC.Pixel.MapGtoGsize-1;
            GLfloat bscale = CC.Pixel.MapBtoBsize-1;
            GLfloat ascale = CC.Pixel.MapAtoAsize-1;
	    for (j=0;j<width;j++) {
	       rf[j] = CC.Pixel.MapRtoR[ (GLint) (rf[j] * rscale) ];
	       gf[j] = CC.Pixel.MapGtoG[ (GLint) (gf[j] * gscale) ];
	       bf[j] = CC.Pixel.MapBtoB[ (GLint) (bf[j] * bscale) ];
	       af[j] = CC.Pixel.MapAtoA[ (GLint) (af[j] * ascale) ];
	    }
	 }

	 /* convert to integers */
	 for (j=0;j<width;j++) {
	    red[j]   = (GLint) (rf[j] * CC.RedScale);
	    green[j] = (GLint) (gf[j] * CC.GreenScale);
	    blue[j]  = (GLint) (bf[j] * CC.BlueScale);
	    alpha[j] = (GLint) (af[j] * CC.AlphaScale);
	 }

	 /* write to frame buffer */
         if (quick_draw) {
            (*DD.write_color_span)( width, x,y, red, green, blue, alpha, NULL);
         }
         else if (zoom) {
            gl_write_zoomed_color_span( width, x, y, zspan,
                                        red, green, blue, alpha, desty );
         }
         else {
            gl_write_color_span( (GLuint) width, x, y, zspan,
                                 red, green, blue, alpha, GL_BITMAP );
         }
      }
   }

}



/*
 * Do a glDrawPixels( w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels ) optimized
 * for the case of no pixel mapping, no scale, no bias, no zoom, default
 * storage mode, no raster ops, and no pixel clipping.
 * Return:  GL_TRUE if success
 *          GL_FALSE if conditions weren't met for optimized drawing
 */
static GLboolean quickdraw_rgb( GLsizei width, GLsizei height,
                                const void *pixels )
{
   DEFARRAY( GLubyte, red, MAX_WIDTH );
   DEFARRAY( GLubyte, green, MAX_WIDTH );
   DEFARRAY( GLubyte, blue, MAX_WIDTH );
   DEFARRAY( GLubyte, alpha, MAX_WIDTH );
   GLint i, j;
   GLint x, y;
   GLint bytes_per_row;

   bytes_per_row = width * 3 + (width % CC.UnpackAlignment);

   if (!CC.Current.RasterPosValid) {
      return GL_TRUE;  /* this is success */
   }

   x = (GLint) (CC.Current.RasterPos[0] + 0.5F);
   y = (GLint) (CC.Current.RasterPos[1] + 0.5F);

   if (x<0 || y<0 || x+width>CC.BufferWidth || y+height>CC.BufferHeight) {
      return GL_FALSE;
   }

   /* constant alpha */
   for (j=0;j<width;j++) {
      alpha[j] = (GLint) CC.AlphaScale;
   }

   /* write directly to device driver */
   for (i=0;i<height;i++) {
      /* each row of pixel data starts at 4-byte boundary */
      GLubyte *src = (GLubyte *) pixels + i * bytes_per_row;
      for (j=0;j<width;j++) {
         red[j]   = *src++;
         green[j] = *src++;
         blue[j]  = *src++;
      }
      (*DD.write_color_span)( width, x, y+i, red, green, blue, alpha, NULL);
   }

   UNDEFARRAY( red );
   UNDEFARRAY( green );
   UNDEFARRAY( blue );
   UNDEFARRAY( alpha );

   return GL_TRUE;
}



/*
 * Implements general glDrawPixels operation.
 */
void gl_drawpixels( GLsizei width, GLsizei height,
		    GLenum format, GLenum type, const GLvoid *pixels )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glDrawPixels" );
      return;
   }

   if (CC.NewState) {
      gl_update_state();
   }

   if (CC.RenderMode==GL_RENDER) {
      if (!CC.Current.RasterPosValid) {
	 return;
      }
      switch (format) {
	 case GL_COLOR_INDEX:
            draw_index_pixels( width, height, type, pixels );
	    break;
	 case GL_STENCIL_INDEX:
	    draw_stencil_pixels( width, height, type, pixels );
	    break;
	 case GL_DEPTH_COMPONENT:
	    draw_depth_pixels( width, height, type, pixels );
	    break;
	 case GL_RED:
	 case GL_GREEN:
	 case GL_BLUE:
	 case GL_ALPHA:
	 case GL_RGB:
	 case GL_LUMINANCE:
	 case GL_LUMINANCE_ALPHA:
	 case GL_RGBA:
            draw_color_pixels( width, height, format, type, pixels );
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glDrawPixels" );
      }
   }
   else if (CC.RenderMode==GL_FEEDBACK) {
      GLfloat color[4];
      color[0] = CC.Current.IntColor[0] / CC.RedScale;
      color[1] = CC.Current.IntColor[1] / CC.GreenScale;
      color[2] = CC.Current.IntColor[2] / CC.BlueScale;
      color[3] = CC.Current.IntColor[3] / CC.AlphaScale;
      APPEND_TOKEN( (GLfloat) GL_DRAW_PIXEL_TOKEN );
      gl_feedback_vertex( CC.Current.RasterPos[0],
			  CC.Current.RasterPos[1],
			  CC.Current.RasterPos[2],
			  CC.Current.RasterPos[3],
			  color, CC.Current.Index,
			  CC.Current.TexCoord );
   }
   else if (CC.RenderMode==GL_SELECT) {
      /* TODO: verify that this is correct */
      CC.HitFlag = GL_TRUE;
      if (CC.Current.RasterPos[2] < CC.HitMinZ) {
	 CC.HitMinZ = CC.Current.RasterPos[2];
      }
      if (CC.Current.RasterPos[2] > CC.HitMaxZ) {
	 CC.HitMaxZ = CC.Current.RasterPos[2];
      }
   }
}



void glDrawPixels( GLsizei width, GLsizei height,
		   GLenum format, GLenum type, const GLvoid *pixels )
{
   GLvoid *image;

   if (width<0 || height<0) {
      gl_error( GL_INVALID_VALUE, "glDrawPixels" );
      return;
   }

   /* Let the device driver take a crack at glDrawPixels */
   if (!CC.CompileFlag && DD.draw_pixels) {
      GLint x = (GLint) (CC.Current.RasterPos[0] + 0.5F);
      GLint y = (GLint) (CC.Current.RasterPos[1] + 0.5F);
      if ((*DD.draw_pixels)( x, y, width, height, format, type, GL_FALSE,
                             pixels )) {
         /* Device driver did the job */
         return;
      }
   }

   if (format==GL_RGB && type==GL_UNSIGNED_BYTE && CC.FastDrawPixels
       && !CC.CompileFlag && CC.RenderMode==GL_RENDER && CC.RasterMask==0) {
      /* optimized path */
      if (quickdraw_rgb( width, height, pixels )) {
         /* success */
         return;
      }
   }

   /* take the general path */
   image = gl_unpack( width, height, format, type, pixels );
   if (!image) {
      gl_error( GL_OUT_OF_MEMORY, "glDrawPixels" );
      return;
   }

   if (CC.CompileFlag) {
      gl_save_drawpixels( width, height, format, type, image );
   }
   if (CC.ExecuteFlag) {
      gl_drawpixels( width, height, format, type, image );
      if (!CC.CompileFlag) {
         /* may discard unpacked image now */
         free( image );
      }
   }
}


