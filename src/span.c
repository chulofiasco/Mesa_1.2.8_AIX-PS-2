/* $Id: span.c,v 1.31 1996/04/25 20:41:06 brianp Exp $ */

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
$Log: span.c,v $
 * Revision 1.31  1996/04/25  20:41:06  brianp
 * replaced gl_depth_test_span() with DD.depth_test_span()
 *
 * Revision 1.30  1996/03/22  20:54:47  brianp
 * replaced CC.ClipSpans with WINCLIP_BIT RasterMask flag
 *
 * Revision 1.29  1996/02/19  21:50:00  brianp
 * added support for software alpha buffering
 *
 * Revision 1.28  1996/02/06  03:23:54  brianp
 * removed gamma correction code
 *
 * Revision 1.27  1996/01/22  15:33:53  brianp
 * use new CC.MutablePixels variable instead of CC.MutableColors
 *
 * Revision 1.26  1996/01/09  19:51:57  brianp
 * fixed a bug in clip_span()
 *
 * Revision 1.25  1995/12/30  01:01:05  brianp
 * gl_write_monocolor_span now takes integer color, not floating point
 *
 * Revision 1.24  1995/12/18  17:27:22  brianp
 * use new GLdepth datatype
 *
 * Revision 1.23  1995/11/30  00:19:52  brianp
 * make copy of span colors if they may be modified and reused
 *
 * Revision 1.22  1995/10/19  15:49:30  brianp
 * added gamma support
 *
 * Revision 1.21  1995/10/13  22:41:16  brianp
 * removed dithering code, added color/index masking code
 *
 * Revision 1.20  1995/09/18  14:21:29  brianp
 * use NULL mask if writing all pixels in a color span
 *
 * Revision 1.19  1995/08/31  21:33:29  brianp
 * use *DD.read_*_span instead of dd_read_*_span
 *
 * Revision 1.18  1995/07/26  15:03:48  brianp
 * replaced some literals with variables for SunOS 4.x per Asif Khan
 *
 * Revision 1.17  1995/07/24  18:58:10  brianp
 * added CC.ClipSpans logic
 *
 * Revision 1.16  1995/06/12  15:42:53  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.15  1995/05/31  14:57:36  brianp
 * added gl_read_index_span() and gl_read_color_span()
 *
 * Revision 1.14  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.13  1995/05/17  13:52:37  brianp
 * implemented glIndexMask(0) and glColorMask(0,0,0,0)
 *
 * Revision 1.12  1995/05/17  13:17:22  brianp
 * changed default CC.Mode value to allow use of real OpenGL headers
 * removed need for CC.MajorMode variable
 *
 * Revision 1.11  1995/05/12  16:28:09  brianp
 * renamed texture functions
 * added clipping for glDrawPixels
 *
 * Revision 1.10  1995/04/11  14:03:27  brianp
 * changed (*CC.write...) to (*DD.write...)
 *
 * Revision 1.9  1995/03/30  21:06:50  brianp
 * updated to use pointers to CC.write_* functions
 *
 * Revision 1.8  1995/03/27  20:32:17  brianp
 * new Texture.Enabled scheme
 *
 * Revision 1.7  1995/03/08  15:10:02  brianp
 * support for dd_logicop
 *
 * Revision 1.6  1995/03/07  19:02:32  brianp
 * updated for new logic blend function names
 *
 * Revision 1.5  1995/03/07  14:21:14  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.4  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.3  1995/03/01  17:44:31  brianp
 * added stenciling for PB
 *
 * Revision 1.2  1995/02/27  22:49:03  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  14:28:31  brianp
 * Initial revision
 *
 */


/*
 * pixel span rasterization:
 * These functions simulate the rasterization pipeline.
 */


#include <string.h>
#include "alpha.h"
#include "alphabuf.h"
#include "blend.h"
#include "context.h"
#include "depth.h"
#include "dd.h"
#include "fog.h"
#include "logic.h"
#include "macros.h"
#include "masking.h"
#include "scissor.h"
#include "span.h"
#include "stencil.h"
#include "texture.h"


#ifndef NULL
#  define NULL 0
#endif


/*
 * Apply the current polygon stipple pattern to a span of pixels.
 */
static void stipple_polygon_span( GLuint n, GLint x, GLint y, GLubyte mask[] )
{
   register GLuint i, m, stipple, highbit=0x80000000;

   stipple = CC.PolygonStipple[y % 32];
   m = highbit >> (GLuint) (x % 32);

   for (i=0;i<n;i++) {
      if ((m & stipple)==0) {
	 mask[i] = 0;
      }
      m = m >> 1;
      if (m==0) {
	 m = 0x80000000;
      }
   }
}



/*
 * Clip a pixel span to the current buffer/window boundaries.
 * Return:  0 = all pixels clipped
 *          1 = at least one pixel is visible
 */
static GLuint clip_span( GLint n, GLint x, GLint y, GLubyte mask[] )
{
   GLint i;

   /* Clip to top and bottom */
   if (y<0 || y>=CC.BufferHeight) {
      return 0;
   }

   /* Clip to left and right */
   if (x>=0 && x+n<=CC.BufferWidth) {
      /* no clipping needed */
      return 1;
   }
   else if (x+n<=0) {
      /* completely off left side */
      return 0;
   }
   else if (x>=CC.BufferWidth) {
      /* completely off right side */
      return 0;
   }
   else {
      /* clip-test each pixel, this could be done better */
      for (i=0;i<n;i++) {
         if (x+i<0 || x+i>=CC.BufferWidth) {
            mask[i] = 0;
         }
      }
      return 1;
   }
}



/*
 * Write a horizontal span of color index pixels to the frame buffer.
 * Stenciling, Depth-testing, etc. are done as needed.
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in the span
 *         z - array of [n] z-values
 *         index - array of [n] color indexes
 *         primitive - either GL_POINT, GL_LINE, GL_POLYGON, or GL_BITMAP
 */
void gl_write_index_span( GLuint n, GLint x, GLint y, GLdepth z[],
			  GLuint index[], GLenum primitive )
{
   GLuint i;
   GLubyte mask[MAX_WIDTH];

   /* init mask to 1's (all pixels are to be written) */
   for (i=0;i<n;i++)
      mask[i] = 1;

   if ((CC.RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span(n,x,y,mask)==0) {
	 return;
      }
   }

   /* Per-pixel fog */
   if (CC.Fog.Enabled && (CC.Hint.Fog==GL_NICEST || primitive==GL_BITMAP)) {
      gl_fog_index_pixels( n, z, index );
   }

   /* Do the scissor test */
   if (CC.Scissor.Enabled) {
      if (gl_scissor_span( n, x, y, mask )==0) {
	 return;
      }
   }

   /* Polygon Stippling */
   if (CC.Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( n, x, y, mask );
   }

   if (CC.Stencil.Enabled) {
      /* first stencil test */
      if (gl_stencil_span( n, x, y, mask )==0) {
	 return;
      }
      /* depth buffering w/ stencil */
      gl_depth_stencil_span( n, x, y, z, mask );
   }
   else if (CC.Depth.Test) {
      /* regular depth testing */
      if ((*DD.depth_test_span)( n, x, y, z, mask )==0)  return;
   }

   if (CC.Color.IndexMask==0) {
      /* write no pixels */
      return;
   }

   /* logic op */
   if (CC.Color.SWLogicOpEnabled) {
      gl_logic_span( n, x, y, index, mask );
   }

   if (CC.Color.SWmasking) {
      gl_mask_index_span( n, x, y, index );
   }

   /* write pixels */
   (*DD.write_index_span)( n, x, y, index, mask );
}




void gl_write_monoindex_span( GLuint n, GLint x, GLint y, GLdepth z[],
			      GLuint index, GLenum primitive )
{
   GLuint i;
   GLubyte mask[MAX_WIDTH];

   /* init mask to 1's (all pixels are to be written) */
   for (i=0;i<n;i++)
      mask[i] = 1;

   if ((CC.RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span(n,x,y,mask)==0) {
	 return;
      }
   }

   /* Do the scissor test */
   if (CC.Scissor.Enabled) {
      if (gl_scissor_span( n, x, y, mask )==0) {
	 return;
      }
   }

   /* Polygon Stippling */
   if (CC.Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( n, x, y, mask );
   }

   if (CC.Stencil.Enabled) {
      /* first stencil test */
      if (gl_stencil_span( n, x, y, mask )==0) {
	 return;
      }
      /* depth buffering w/ stencil */
      gl_depth_stencil_span( n, x, y, z, mask );
   }
   else if (CC.Depth.Test) {
      /* regular depth testing */
      if ((*DD.depth_test_span)( n, x, y, z, mask )==0)  return;
   }

   if (CC.Color.IndexMask==0) {
      /* write no pixels */
      return;
   }

   if ((CC.Fog.Enabled && (CC.Hint.Fog==GL_NICEST || primitive==GL_BITMAP))
        || CC.Color.SWLogicOpEnabled || CC.Color.SWmasking) {
      GLuint ispan[MAX_WIDTH];
      for (i=0;i<n;i++) {
	 ispan[i] = index;
      }

      if (CC.Fog.Enabled && (CC.Hint.Fog==GL_NICEST || primitive==GL_BITMAP)) {
	 gl_fog_index_pixels( n, z, ispan );
      }

      if (CC.Color.SWLogicOpEnabled) {
	 gl_logic_span( n, x, y, ispan, mask );
      }

      if (CC.Color.SWmasking) {
         gl_mask_index_span( n, x, y, ispan );
      }

      (*DD.write_index_span)( n, x, y, ispan, mask );
   }
   else {
      (*DD.write_monoindex_span)( n, x, y, mask );
   }
}



void gl_write_color_span( GLuint n, GLint x, GLint y, GLdepth z[],
			  GLubyte r[], GLubyte g[],
			  GLubyte b[], GLubyte a[],
			  GLenum primitive )
{
   GLuint i;
   GLubyte mask[MAX_WIDTH];
   GLboolean write_all = GL_TRUE;
   GLubyte rtmp[MAX_WIDTH], gtmp[MAX_WIDTH], btmp[MAX_WIDTH], atmp[MAX_WIDTH];
   GLubyte *red, *green, *blue, *alpha;

   /* init mask to 1's (all pixels are to be written) */
   for (i=0;i<n;i++)
      mask[i] = 1;

   if ((CC.RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span(n,x,y,mask)==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if (primitive==GL_BITMAP && CC.MutablePixels) {
      /* must make a copy of the colors since they may be modified */
      MEMCPY( rtmp, r, n * sizeof(GLubyte) );
      MEMCPY( gtmp, g, n * sizeof(GLubyte) );
      MEMCPY( btmp, b, n * sizeof(GLubyte) );
      MEMCPY( atmp, a, n * sizeof(GLubyte) );
      red = rtmp;
      green = gtmp;
      blue = btmp;
      alpha = atmp;
   }
   else {
      red   = r;
      green = g;
      blue  = b;
      alpha = a;
   }

   /* Per-pixel fog */
   if (CC.Fog.Enabled && (CC.Hint.Fog==GL_NICEST || primitive==GL_BITMAP)) {
      gl_fog_color_pixels( n, z, red, green, blue, alpha );
   }

   /* Do the scissor test */
   if (CC.Scissor.Enabled) {
      if (gl_scissor_span( n, x, y, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Polygon Stippling */
   if (CC.Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( n, x, y, mask );
      write_all = GL_FALSE;
   }

   /* Do the alpha test */
   if (CC.Color.AlphaEnabled) {
      if (gl_alpha_test( n, alpha, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if (CC.Stencil.Enabled) {
      /* first stencil test */
      if (gl_stencil_span( n, x, y, mask )==0) {
	 return;
      }
      /* depth buffering w/ stencil */
      gl_depth_stencil_span( n, x, y, z, mask );
      write_all = GL_FALSE;
   }
   else if (CC.Depth.Test) {
      /* regular depth testing */
      GLuint m = (*DD.depth_test_span)( n, x, y, z, mask );
      if (m==0) {
         return;
      }
      if (m<n) {
         write_all = GL_FALSE;
      }
   }

   if (CC.Color.ColorMask==0) {
      /* write no pixels */
      return;
   }

   /* blending */
   if (CC.Color.BlendEnabled) {
      gl_blend_span( n, x, y, red, green, blue, alpha, mask );
   }

   /* Color component masking */
   if (CC.Color.SWmasking) {
      gl_mask_color_span( n, x, y, red, green, blue, alpha );
   }

   /* write pixels */
   (*DD.write_color_span)( n, x, y, red, green, blue, alpha,
                           write_all ? NULL : mask );

   if (CC.RasterMask & ALPHABUF_BIT) {
      gl_write_alpha_span( n, x, y, alpha, write_all ? NULL : mask );
   }
}



/*
 * Write a horizontal span of color pixels to the frame buffer.
 * The color is initially constant for the whole span.
 * Alpha-testing, stenciling, depth-testing, and blending are done as needed.
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in the span
 *         z - array of [n] z-values
 *         r, g, b, a - the color of the pixels
 *         primitive - either GL_POINT, GL_LINE, GL_POLYGON or GL_BITMAP.
 */
void gl_write_monocolor_span( GLuint n, GLint x, GLint y, GLdepth z[],
			      GLint r, GLint g, GLint b, GLint a,
                              GLenum primitive )
{
   GLuint i;
   GLubyte mask[MAX_WIDTH];
   GLboolean write_all = GL_TRUE;

   /* init mask to 1's (all pixels are to be written) */
   for (i=0;i<n;i++)
      mask[i] = 1;

   if ((CC.RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span(n,x,y,mask)==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Do the scissor test */
   if (CC.Scissor.Enabled) {
      if (gl_scissor_span( n, x, y, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Polygon Stippling */
   if (CC.Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( n, x, y, mask );
      write_all = GL_FALSE;
   }

   /* Do the alpha test */
   if (CC.Color.AlphaEnabled) {
      GLubyte alpha[MAX_WIDTH];
      for (i=0;i<n;i++) {
         alpha[i] = a;
      }
      if (gl_alpha_test( n, alpha, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if (CC.Stencil.Enabled) {
      /* first stencil test */
      if (gl_stencil_span( n, x, y, mask )==0) {
	 return;
      }
      /* depth buffering w/ stencil */
      gl_depth_stencil_span( n, x, y, z, mask );
      write_all = GL_FALSE;
   }
   else if (CC.Depth.Test) {
      /* regular depth testing */
      GLuint m = (*DD.depth_test_span)( n, x, y, z, mask );
      if (m==0) {
         return;
      }
      if (m<n) {
         write_all = GL_FALSE;
      }
   }

   if (CC.Color.ColorMask==0) {
      /* write no pixels */
      return;
   }

   if (CC.Color.BlendEnabled || CC.Color.SWmasking) {
      /* assign same color to each pixel */
      GLubyte red[MAX_WIDTH], green[MAX_WIDTH];
      GLubyte blue[MAX_WIDTH], alpha[MAX_WIDTH];
      for (i=0;i<n;i++) {
	 if (mask[i]) {
	    red[i]   = r;
	    green[i] = g;
	    blue[i]  = b;
	    alpha[i] = a;
	 }
      }

      if (CC.Color.BlendEnabled) {
         gl_blend_span( n, x, y, red, green, blue, alpha, mask );
      }

      /* Color component masking */
      if (CC.Color.SWmasking) {
         gl_mask_color_span( n, x, y, red, green, blue, alpha );
      }

      /* write pixels */
      (*DD.write_color_span)( n, x, y, red, green, blue, alpha,
                              write_all ? NULL : mask );
      if (CC.RasterMask & ALPHABUF_BIT) {
         gl_write_alpha_span( n, x, y, alpha, write_all ? NULL : mask );
      }
   }
   else {
      (*DD.write_monocolor_span)( n, x, y, mask );
      if (CC.RasterMask & ALPHABUF_BIT) {
         gl_write_mono_alpha_span( n, x, y, a, write_all ? NULL : mask );
      }
   }
}



/*
 * Write a horizontal span of textured pixels to the frame buffer.
 * The color of each pixel is different.
 * Alpha-testing, stenciling, depth-testing, and blending are done
 * as needed.
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in the span
 *         z - array of [n] z-values
 *         s, t - array of (s,t) texture coordinates for each pixel
 *         red, green, blue, alpha - array of [n] color components
 *         primitive - either GL_POINT, GL_LINE, GL_POLYGON or GL_BITMAP.
 */
void gl_write_texture_span( GLuint n, GLint x, GLint y, GLdepth z[],
			    GLfloat s[], GLfloat t[],
			    GLubyte r[], GLubyte g[],
			    GLubyte b[], GLubyte a[],
			    GLenum primitive )
{
   GLuint i;
   GLubyte mask[MAX_WIDTH];
   GLboolean write_all = GL_TRUE;
   GLubyte rtmp[MAX_WIDTH], gtmp[MAX_WIDTH], btmp[MAX_WIDTH], atmp[MAX_WIDTH];
   GLubyte *red, *green, *blue, *alpha;

   /* init mask to 1's (all pixels are to be written) */
   for (i=0;i<n;i++)
      mask[i] = 1;

   if ((CC.RasterMask & WINCLIP_BIT) || primitive==GL_BITMAP) {
      if (clip_span(n,x,y,mask)==0) {
	 return;
      }
      write_all = GL_FALSE;
   }


   if (primitive==GL_BITMAP) {
      /* must make a copy of the colors since they may be modified */
      MEMCPY( rtmp, r, n * sizeof(GLubyte) );
      MEMCPY( gtmp, g, n * sizeof(GLubyte) );
      MEMCPY( btmp, b, n * sizeof(GLubyte) );
      MEMCPY( atmp, a, n * sizeof(GLubyte) );
      red = rtmp;
      green = gtmp;
      blue = btmp;
      alpha = atmp;
   }
   else {
      red   = r;
      green = g;
      blue  = b;
      alpha = a;
   }

   /* Texture */
   if (CC.Texture.Enabled & 2) {
      gl_texture_pixels_2d( n, s, t, red, green, blue, alpha );
   }
   else if (CC.Texture.Enabled & 1) {
      gl_texture_pixels_1d( n, s, red, green, blue, alpha );
   }


   /* Per-pixel fog */
   if (CC.Fog.Enabled && (CC.Hint.Fog==GL_NICEST || primitive==GL_BITMAP)) {
      gl_fog_color_pixels( n, z, red, green, blue, alpha );
   }

   /* Do the scissor test */
   if (CC.Scissor.Enabled) {
      if (gl_scissor_span( n, x, y, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   /* Polygon Stippling */
   if (CC.Polygon.StippleFlag && primitive==GL_POLYGON) {
      stipple_polygon_span( n, x, y, mask );
      write_all = GL_FALSE;
   }

   /* Do the alpha test */
   if (CC.Color.AlphaEnabled) {
      if (gl_alpha_test( n, alpha, mask )==0) {
	 return;
      }
      write_all = GL_FALSE;
   }

   if (CC.Stencil.Enabled) {
      /* first stencil test */
      if (gl_stencil_span( n, x, y, mask )==0) {
	 return;
      }
      /* depth buffering w/ stencil */
      gl_depth_stencil_span( n, x, y, z, mask );
      write_all = GL_FALSE;
   }
   else if (CC.Depth.Test) {
      /* regular depth testing */
      GLuint m = (*DD.depth_test_span)( n, x, y, z, mask );
      if (m==0) {
         return;
      }
      if (m<n) {
         write_all = GL_FALSE;
      }
   }

   if (CC.Color.ColorMask==0) {
      /* write no pixels */
      return;
   }

   /* blending */
   if (CC.Color.BlendEnabled) {
      gl_blend_span( n, x, y, red, green, blue, alpha, mask );
   }

   if (CC.Color.SWmasking) {
      gl_mask_color_span( n, x, y, red, green, blue, alpha );
   }

   /* write pixels */
   (*DD.write_color_span)( n, x, y, red, green, blue, alpha,
                           write_all ? NULL : mask );
   if (CC.RasterMask & ALPHABUF_BIT) {
      gl_write_alpha_span( n, x, y, alpha, write_all ? NULL : mask );
   }
}



/*
 * Read RGBA pixels from frame buffer.  Clipping will be done to prevent
 * reading ouside the buffer's boundaries.
 */
void gl_read_color_span( GLuint n, GLint x, GLint y,
			 GLubyte red[], GLubyte green[],
			 GLubyte blue[], GLubyte alpha[] )
{
   register GLuint i;

   if (y<0 || y>=CC.BufferHeight || x>=CC.BufferWidth) {
      /* completely above, below, or right */
      for (i=0;i<n;i++) {
	 red[i] = green[i] = blue[i] = alpha[i] = 0;
      }
   }
   else {
      if (x>=0 && x+n<=CC.BufferWidth) {
	 /* OK */
	 (*DD.read_color_span)( n, x, y, red, green, blue, alpha );
         if (CC.RasterMask & ALPHABUF_BIT) {
            gl_read_alpha_span( n, x, y, alpha );
         }
      }
      else {
	 i = 0;
	 if (x<0) {
	    while (x<0 && n>0) {
	       red[i] = green[i] =  blue[i] = alpha[i] = 0;
	       x++;
	       n--;
	       i++;
	    }
	 }
	 n = MIN2( n, CC.BufferWidth - x );
	 (*DD.read_color_span)( n, x, y, red+i, green+i, blue+i, alpha+i );
         if (CC.RasterMask & ALPHABUF_BIT) {
            gl_read_alpha_span( n, x, y, alpha+i );
         }
      }
   }
}




/*
 * Read CI pixels from frame buffer.  Clipping will be done to prevent
 * reading ouside the buffer's boundaries.
 */
void gl_read_index_span( GLuint n, GLint x, GLint y, GLuint indx[] )
{
   register GLuint i;

   if (y<0 || y>=CC.BufferHeight || x>=CC.BufferWidth) {
      /* completely above, below, or right */
      for (i=0;i<n;i++) {
	 indx[i] = 0;
      }
   }
   else {
      if (x>=0 && x+n<=CC.BufferWidth) {
	 /* OK */
	 (*DD.read_index_span)( n, x, y, indx );
      }
      else {
	 i = 0;
	 if (x<0) {
	    while (x<0 && n>0) {
	       indx[i] = 0;
	       x++;
	       n--;
	       i++;
	    }
	 }
	 n = MIN2( n, CC.BufferWidth - x );
	 (*DD.read_index_span)( n, x, y, indx+i );
      }
   }
}


