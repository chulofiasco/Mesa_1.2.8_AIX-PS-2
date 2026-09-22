/* $Id: xmesa3.c,v 1.25 1996/04/09 19:05:55 billh Exp $ */

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
$Log: xmesa3.c,v $
 * Revision 1.25  1996/04/09  19:05:55  billh
 * added a cast to line drawing functions to stop warning on Sun's compiler
 *
 * Revision 1.24  1996/03/22  20:56:03  brianp
 * removed CC.ClipSpans stuff
 *
 * Revision 1.23  1996/03/05  19:52:37  brianp
 * Set GC line width to 0, not 1, for best performance
 *
 * Revision 1.22  1996/03/02  21:41:05  brianp
 * added PF_8R8G8B pixel format
 *
 * Revision 1.21  1996/03/01  20:07:27  brianp
 * added optimized polygon rasterizers for non-depth-buffered rendering
 *
 * Revision 1.20  1996/02/26  16:23:09  brianp
 * added PF_5R6G5B pixel format from Joerg Hessdoerfer
 *
 * Revision 1.19  1996/02/23  17:13:54  brianp
 * cleaned up inner loops of polygon drawing functions
 *
 * Revision 1.18  1996/02/16  16:39:35  brianp
 * let wide lines be drawn by XDrawLine, suggested by Michael Pichler
 *
 * Revision 1.17  1996/02/06  03:25:22  brianp
 * removed #include "gamma.h"
 *
 * Revision 1.16  1996/01/26  20:32:25  brianp
 * use CC.ColorShift in encode_color()
 *
 * Revision 1.16  1996/01/26  20:32:25  brianp
 * use CC.ColorShift in encode_color()
 *
 * Revision 1.15  1996/01/12  22:31:43  brianp
 * added simple window clipping to line drawing functions
 *
 * Revision 1.14  1996/01/11  22:53:29  brianp
 * added more flat-shaded line drawing functions
 *
 * Revision 1.13  1996/01/10  22:36:25  brianp
 * made back XImage one pixel wider and higher than window
 * added more line drawing functions
 *
 * Revision 1.12  1996/01/05  01:16:52  brianp
 * added flat_LOOKUP8_z_polygon
 *
 * Revision 1.11  1996/01/02  22:12:40  brianp
 * added flat TRUECOLOR and HPCR polygon functions
 * removed old fixed-point macros
 *
 * Revision 1.10  1995/12/30  01:06:09  brianp
 * use fixed point vertex colors instead of floating point
 * added several flat-shaded polygon functions
 * use renamed DITHER macro
 *
 * Revision 1.8  1995/12/07  17:05:06  brianp
 * faster color interpolation by removing some bit shifting
 *
 * Revision 1.7  1995/11/30  00:21:36  brianp
 * added PF_GRAYSCALE support
 *
 * Revision 1.6  1995/11/14  21:49:09  brianp
 * optimized polygon rendering setup
 *
 * Revision 1.5  1995/11/08  22:08:22  brianp
 * fixed OFFSET4/1 bug in smooth_rgba_z_polygon_ximage()
 *
 * Revision 1.4  1995/11/04  20:09:32  brianp
 * added F suffix to floating point constants
 * replaced incorrect 0.05F with 0.5F
 *
 * Revision 1.3  1995/11/03  17:41:48  brianp
 * removed unused vars, fixed code for C++ compilation
 *
 * Revision 1.2  1995/10/30  15:50:41  brianp
 * make sure CC.ClipSpans is FALSE before using smooth_rgba_z_polygon_ximage
 *
 * Revision 1.1  1995/10/30  15:15:15  brianp
 * Initial revision
 *
 */


/*
 * Mesa/X11 interface, part 3.
 *
 * This file contains "accelerated" point, line, and polygon functions.
 * It should be fairly easy to write new special-purpose point, line or
 * polygon functions and hook them into this module.
 */



#include <sys/time.h>
#include <assert.h>


#include <stdlib.h>
#include <stdio.h>
#include "X11/Xlib.h"
#include "bresenhm.h"
#include "context.h"
#include "depth.h"
#include "dd.h"
#include "interp.h"
#include "macros.h"
#include "polygons.h"
#include "vb.h"
#include "xmesaP.h"


/*
 * Like PACK_8A8B8G8R() but don't use alpha.  This is usually an acceptable
 * shortcut.  If alpha is needed the MESA_ALPHA environment variable
 * should be defined as would be needed for any other visual type.
 */
#define PACK_8B8G8R( R, G, B )   ( ((B) << 16) | ((G) << 8) | (R) )



/*
 * Given a vertex number return the X pixel value for that vertex's color.
 * Note that if smooth shading is enabled we have to convert the vertex
 * colors from fixed point to integers by shifting by CC.ColorShift bits.
 */
static unsigned long encode_color( GLuint i )
{
   switch (XMesa->pixelformat) {
      case PF_INDEX:
         return (unsigned long) VB.Index[i];
      case PF_TRUECOLOR:
         {
            register int r, g, b;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            return PACK_RGB( r, g, b );
         }
      case PF_8A8B8G8R:
         {
            register int r, g, b, a;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            a = VB.Color[i][3] >> shift;
            return PACK_8A8B8G8R( r, g, b, a );
         }
      case PF_8R8G8B:
         {
            register int r, g, b;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            return PACK_8R8G8B( r, g, b );
         }
      case PF_5R6G5B:
         {
            register int r, g, b;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            return PACK_5R6G5B( r, g, b );
         }
      case PF_DITHER:
         {
            register int r, g, b;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            return DITHER( 0, 0, r, g, b );
         }
      case PF_LOOKUP:
         {
            register int r, g, b;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            return LOOKUP( r, g, b );
         }
      case PF_HPCR:
         {
            register int r, g, b;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            return DITHER_HPCR( 1, 1, r, g, b );
         }
      case PF_1BIT:
         {
            register int r, g, b;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            return (r+g+b) > 382;
         }
      case PF_GRAYSCALE:
         {
            register int r, g, b;
            int shift = CC.ColorShift;
            r = VB.Color[i][0] >> shift;
            g = VB.Color[i][1] >> shift;
            b = VB.Color[i][2] >> shift;
            return GRAY_RGB( r, g, b );
         }
      default:
         abort();
   }
   return 0;
}



/**********************************************************************/
/***                    Point rendering                             ***/
/**********************************************************************/


/*
 * Render an array of points into a pixmap, any pixel format.
 */
static void draw_points_ANY_pixmap( GLuint first, GLuint last )
{
   register GLuint i;
   if (VB.MonoColor) {
      /* all same color */
      XPoint p[VB_SIZE];
      int n = 0;
      for (i=first;i<=last;i++) {
         if (VB.Unclipped[i]) {
            p[n].x =       (GLint) VB.Win[i][0];
            p[n].y = FLIP( (GLint) VB.Win[i][1] );
            n++;
         }
      }
      XDrawPoints( XMesa->display, XMesa->buffer, XMesa->gc1, p, n,
                   CoordModeOrigin );
   }
   else {
      /* all different colors */
      for (i=first;i<=last;i++) {
         if (VB.Unclipped[i]) {
            register int x, y;
            XSetForeground( XMesa->display, XMesa->gc2, encode_color(i) );
            x =       (GLint) VB.Win[i][0];
            y = FLIP( (GLint) VB.Win[i][1] );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2, x, y);
         }
      }
   }
}



/*
 * Analyze current CC state to see if we can provide a fast points drawing
 * function, like those in points.c.  Otherwise, return NULL.
 */
points_func xmesa_get_points_func( void )
{
   if (CC.Point.Size==1.0F && !CC.Point.SmoothFlag && CC.RasterMask==0
       && !CC.Texture.Enabled) {
      if (XMesa->buffer==XIMAGE) {
         return NULL; /*draw_points_ximage;*/
      }
      else {
         return draw_points_ANY_pixmap;
      }
   }
   else {
      return NULL;
   }
}



/**********************************************************************/
/***                      Line rendering                            ***/
/**********************************************************************/

/*
 * Render a line into a pixmap, any pixel format.
 */
static void flat_line_pixmap( GLuint v0, GLuint v1, GLuint pv )
{
   register int x0, y0, x1, y1;
   GC gc;
   if (VB.MonoColor) {
      gc = XMesa->gc1;  /* use current color */
   }
   else {
      gc = XMesa->gc2;
      XSetForeground( XMesa->display, XMesa->gc2, encode_color(pv) );
   }
   x0 =       (GLint) VB.Win[v0][0];
   y0 = FLIP( (GLint) VB.Win[v0][1] );
   x1 =       (GLint) VB.Win[v1][0];
   y1 = FLIP( (GLint) VB.Win[v1][1] );
   XDrawLine( XMesa->display, XMesa->buffer, gc, x0, y0, x1, y1 );
}


/*
 * Despite being clipped to the view volume, the line's window coordinates
 * may just lie outside the window bounds.  That is, if the legal window
 * coordinates are [0,W-1][0,H-1], it's possible for x==W and/or y==H.
 * These quick and dirty macros take care of that possibility.
 */
#define WINCLIP_X(X1,X2)		\
   {					\
      GLint w = CC.BufferWidth;		\
      if (X1==w | X2==w) {		\
         if (X1==w & X2==w)  return;	\
         X1 -= X1==w;   X2 -= X2==w;	\
      }					\
   }

#define WINCLIP_Y(Y1,Y2)		\
   {					\
      GLint h = CC.BufferHeight;	\
      if (Y1==h | Y2==h) {		\
         if (Y1==h & Y2==h)  return;	\
         Y1 -= Y1==h;   Y2 -= Y2==h;	\
      }					\
   }


/*
 * Draw a flat-shaded, PF_8A8B8G8R line into an XImage.
 */
static void flat_8A8B8G8R_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0],  y1 = (GLint) VB.Win[v0][1];
   GLint x2 = (GLint) VB.Win[v1][0],  y2 = (GLint) VB.Win[v1][1];
   GLuint pixel;
   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);
   pixel = PACK_8B8G8R( VB.Color[pv][0], VB.Color[pv][1], VB.Color[pv][2] );

#define BRESENHAM_PLOT(X,Y)			\
	{					\
	   GLuint *ptr = PIXELADDR4(X,Y);	\
	   *ptr = pixel;			\
	}

   BRESENHAM( x1, y1, x2, y2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, PF_8R8G8B line into an XImage.
 */
static void flat_8R8G8B_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0],  y1 = (GLint) VB.Win[v0][1];
   GLint x2 = (GLint) VB.Win[v1][0],  y2 = (GLint) VB.Win[v1][1];
   GLuint pixel;
   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);
   pixel = PACK_8R8G8B( VB.Color[pv][0], VB.Color[pv][1], VB.Color[pv][2] );

#define BRESENHAM_PLOT(X,Y)			\
	{					\
	   GLuint *ptr = PIXELADDR4(X,Y);	\
	   *ptr = pixel;			\
	}

   BRESENHAM( x1, y1, x2, y2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, PF_5R6G5B line into an XImage.
 */
static void flat_5R6G5B_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0],  y1 = (GLint) VB.Win[v0][1];
   GLint x2 = (GLint) VB.Win[v1][0],  y2 = (GLint) VB.Win[v1][1];
   GLushort pixel;
   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);
   pixel = PACK_5R6G5B( VB.Color[pv][0], VB.Color[pv][1], VB.Color[pv][2] );

#define BRESENHAM_PLOT(X,Y)			\
	{					\
	   GLushort *ptr = PIXELADDR2(X,Y);	\
	   *ptr = pixel;			\
	}

   BRESENHAM( x1, y1, x2, y2 );

#undef BRESENHAM_PLOT
}



/*
 * Draw a flat-shaded, PF_TRUECOLOR line into an XImage.
 */
static void flat_TRUECOLOR_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   unsigned long pixel = PACK_RGB( VB.Color[pv][0], VB.Color[pv][1],
                                   VB.Color[pv][2] );
   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y)					\
        XPutPixel( XMesa->backimage, X, FLIP(Y), pixel );

   BRESENHAM( x1, y1, x2, y2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, PF_DITHER 8-bit line into an XImage.
 */
static void flat_DITHER8_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint r = VB.Color[pv][0], g = VB.Color[pv][1], b = VB.Color[pv][2];

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y)	       	\
	GLubyte *ptr = PIXELADDR1(X,Y);	\
	*ptr = DITHER( X, Y, r, g, b);

   BRESENHAM( x1, y1, x2, y2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, PF_LOOKUP 8-bit line into an XImage.
 */
static void flat_LOOKUP8_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLubyte pixel = LOOKUP( VB.Color[pv][0], VB.Color[pv][1], VB.Color[pv][2] );

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y)	       	\
	GLubyte *ptr = PIXELADDR1(X,Y);	\
	*ptr = pixel;

   BRESENHAM( x1, y1, x2, y2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, PF_HPCR line into an XImage.
 */
static void flat_HPCR_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint r = VB.Color[pv][0], g = VB.Color[pv][1], b = VB.Color[pv][2];

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y)			\
	GLubyte *ptr = PIXELADDR1(X,Y);		\
	*ptr = DITHER_HPCR( X, Y, r, g, b);

   BRESENHAM( x1, y1, x2, y2 );

#undef BRESENHAM_PLOT
}



/*
 * Draw a flat-shaded, Z-less, PF_TRUECOLOR line into an XImage.
 */
static void flat_TRUECOLOR_z_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint z1 = (GLint) (VB.Win[v0][2] * DEPTH_SCALE);
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint z2 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
   unsigned long pixel = PACK_RGB( VB.Color[pv][0], VB.Color[pv][1],
                                   VB.Color[pv][2] );
   GLdepth *zptr;

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y,Z)					\
	zptr = Z_ADDRESS(X,Y);					\
	if ((GLdepth)(Z) < *zptr) {				\
           XPutPixel( XMesa->backimage, X, FLIP(Y), pixel );	\
	   *zptr = (Z);						\
	}

   BRESENHAM_Z( x1, y1, z1, x2, y2, z2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, Z-less, PF_8A8B8G8R line into an XImage.
 */
static void flat_8A8B8G8R_z_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint z1 = (GLint) (VB.Win[v0][2] * DEPTH_SCALE);
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint z2 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
   GLuint pixel = PACK_8B8G8R( VB.Color[pv][0], VB.Color[pv][1],
                               VB.Color[pv][2] );
   GLdepth *zptr;

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y,Z)			\
	zptr = Z_ADDRESS(X,Y);			\
	if ((GLdepth)(Z) < *zptr) {		\
	   GLuint *ptr = PIXELADDR4(X,Y);	\
	   *ptr = pixel;			\
	   *zptr = (Z);				\
	}

   BRESENHAM_Z( x1, y1, z1, x2, y2, z2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, Z-less, PF_8R8G8B line into an XImage.
 */
static void flat_8R8G8B_z_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint z1 = (GLint) (VB.Win[v0][2] * DEPTH_SCALE);
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint z2 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
   GLuint pixel = PACK_8R8G8B( VB.Color[pv][0], VB.Color[pv][1],
                               VB.Color[pv][2] );
   GLdepth *zptr;

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y,Z)			\
	zptr = Z_ADDRESS(X,Y);			\
	if ((GLdepth)(Z) < *zptr) {		\
	   GLuint *ptr = PIXELADDR4(X,Y);	\
	   *ptr = pixel;			\
	   *zptr = (Z);				\
	}

   BRESENHAM_Z( x1, y1, z1, x2, y2, z2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, Z-less, PF_5R6G5B line into an XImage.
 */
static void flat_5R6G5B_z_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint z1 = (GLint) (VB.Win[v0][2] * DEPTH_SCALE);
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint z2 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
   GLuint pixel = PACK_5R6G5B( VB.Color[pv][0], VB.Color[pv][1],
                               VB.Color[pv][2] );
   GLdepth *zptr;

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y,Z)			\
	zptr = Z_ADDRESS(X,Y);			\
	if ((GLdepth)(Z) < *zptr) {		\
	   GLushort *ptr = PIXELADDR2(X,Y);	\
	   *ptr = pixel;			\
	   *zptr = (Z);				\
	}

   BRESENHAM_Z( x1, y1, z1, x2, y2, z2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, Z-less, PF_DITHER 8-bit line into an XImage.
 */
static void flat_DITHER8_z_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint z1 = (GLint) (VB.Win[v0][2] * DEPTH_SCALE);
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint z2 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
   GLint r = VB.Color[pv][0], g = VB.Color[pv][1], b = VB.Color[pv][2];
   GLdepth *zptr;

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y,Z)			\
	zptr = Z_ADDRESS(X,Y);			\
	if ((GLdepth)(Z) < *zptr) {		\
	   GLubyte *ptr = PIXELADDR1(X,Y);	\
	   *ptr = DITHER( X, Y, r, g, b);	\
	   *zptr = (Z);				\
	}

   BRESENHAM_Z( x1, y1, z1, x2, y2, z2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, Z-less, PF_LOOKUP 8-bit line into an XImage.
 */
static void flat_LOOKUP8_z_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint z1 = (GLint) (VB.Win[v0][2] * DEPTH_SCALE);
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint z2 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
   GLubyte pixel = LOOKUP( VB.Color[pv][0], VB.Color[pv][1], VB.Color[pv][2] );
   GLdepth *zptr;

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y,Z)			\
	zptr = Z_ADDRESS(X,Y);			\
	if ((GLdepth)(Z) < *zptr) {		\
	   GLubyte *ptr = PIXELADDR1(X,Y);	\
	   *ptr = pixel;			\
	   *zptr = (Z);				\
	}

   BRESENHAM_Z( x1, y1, z1, x2, y2, z2 );

#undef BRESENHAM_PLOT
}


/*
 * Draw a flat-shaded, Z-less, PF_HPCR line into an XImage.
 */
static void flat_HPCR_z_line_ximage( GLuint v0, GLuint v1, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v0][0], y1 = (GLint) VB.Win[v0][1];
   GLint z1 = (GLint) (VB.Win[v0][2] * DEPTH_SCALE);
   GLint x2 = (GLint) VB.Win[v1][0], y2 = (GLint) VB.Win[v1][1];
   GLint z2 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
   GLint r = VB.Color[pv][0], g = VB.Color[pv][1], b = VB.Color[pv][2];
   GLdepth *zptr;

   WINCLIP_X(x1,x2);
   WINCLIP_Y(y1,y2);

#define BRESENHAM_PLOT(X,Y,Z)			\
	zptr = Z_ADDRESS(X,Y);			\
	if ((GLdepth)(Z) < *zptr) {		\
	   GLubyte *ptr = PIXELADDR1(X,Y);	\
	   *ptr = DITHER_HPCR( X, Y, r, g, b);	\
	   *zptr = (Z);				\
	}

   BRESENHAM_Z( x1, y1, z1, x2, y2, z2 );

#undef BRESENHAM_PLOT
}



/*
 * Analyze current state to see if we can provide a fast line drawing
 * function, like those in lines.c.  Otherwise, return NULL.
 */
line_func xmesa_get_line_func( void )
{
   if (CC.Line.SmoothFlag)              return NULL;
   if (CC.Line.StippleFlag)             return NULL;
   if (CC.Texture.Enabled)              return NULL;
   if (CC.Light.ShadeModel!=GL_FLAT)    return NULL;

   if (XMesa->buffer==XIMAGE && CC.RasterMask==DEPTH_BIT
       && CC.Depth.Func==GL_LESS && CC.Depth.Mask==GL_TRUE
       && CC.Line.Width==1.0F) {
      switch (XMesa->pixelformat) {
         case PF_TRUECOLOR:
            return flat_TRUECOLOR_z_line_ximage;
         case PF_8A8B8G8R:
            return flat_8A8B8G8R_z_line_ximage;
         case PF_8R8G8B:
            return flat_8R8G8B_z_line_ximage;
         case PF_5R6G5B:
            return flat_5R6G5B_z_line_ximage;
         case PF_DITHER:
            return (XMesa->depth==8) ? flat_DITHER8_z_line_ximage : NULL;
         case PF_LOOKUP:
            return (XMesa->depth==8) ? flat_LOOKUP8_z_line_ximage : NULL;
         case PF_HPCR:
            return flat_HPCR_z_line_ximage;
         default:
            return NULL;
      }
   }
   if (XMesa->buffer==XIMAGE && CC.RasterMask==0 && CC.Line.Width==1.0F) {
      switch (XMesa->pixelformat) {
         case PF_TRUECOLOR:
            return flat_TRUECOLOR_line_ximage;
         case PF_8A8B8G8R:
            return flat_8A8B8G8R_line_ximage;
         case PF_8R8G8B:
            return flat_8R8G8B_line_ximage;
         case PF_5R6G5B:
            return flat_5R6G5B_line_ximage;
         case PF_DITHER:
            return (XMesa->depth==8) ? flat_DITHER8_line_ximage : NULL;
         case PF_LOOKUP:
            return (XMesa->depth==8) ? flat_LOOKUP8_line_ximage : NULL;
         case PF_HPCR:
            return flat_HPCR_line_ximage;
	 default:
	    return NULL;
      }
   }
   if (XMesa->buffer!=XIMAGE && CC.RasterMask==0) {
      XGCValues gcvals;
      gcvals.line_width = (int) CC.Line.Width;
      if (gcvals.line_width<2) {
         /* width=1 can be slow, width=0 is fastest */
         gcvals.line_width = 0;
      }
      XChangeGC( XMesa->display, XMesa->gc1, GCLineWidth, &gcvals );
      XChangeGC( XMesa->display, XMesa->gc2, GCLineWidth, &gcvals );
      return flat_line_pixmap;
   }
   return NULL;
}




/**********************************************************************/
/***                   Polygon rendering                            ***/
/**********************************************************************/

/*
 * Render a polygon into a pixmap, any pixel format.
 */
void draw_polygon_ANY_pixmap( GLuint n, GLuint vlist[], GLuint pv )
{
   GLuint i;
   XPoint p[VB_SIZE];
   GC gc;
   if (VB.MonoColor) {
      gc = XMesa->gc1;  /* use current color */
   }
   else {
      gc = XMesa->gc2;
      XSetForeground( XMesa->display, XMesa->gc2, encode_color(pv) );
   }
   for (i=0;i<n;i++) {
      GLuint j = vlist[i];
      /* The +1 offset is tricky, involves the CC.RasterOffset and X's */
      /* sampling policy. */
      p[i].x =       (GLint) VB.Win[j][0] + 1;
      p[i].y = FLIP( (GLint) (VB.Win[j][1]+1.0f) - 1 );
   }
   XFillPolygon( XMesa->display, XMesa->buffer, gc,
		 p, n, Convex, CoordModeOrigin );
}



static GLint lx[MAX_HEIGHT], rx[MAX_HEIGHT];	/* X bounds */
static GLfixed lr[MAX_HEIGHT], rr[MAX_HEIGHT];	/* Red */
static GLfixed lg[MAX_HEIGHT], rg[MAX_HEIGHT];	/* Green */
static GLfixed lb[MAX_HEIGHT], rb[MAX_HEIGHT];	/* Blue */





/*
 * XImage, smooth, depth-buffered, PF_TRUECOLOR polygon.
 */
static void smooth_TRUECOLOR_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_Z

#define INNER_CODE					\
   GLint i, yy = FLIP(y);				\
   for (i=0;i<len;i++,xmin++) {				\
      if (FixedToUns(fz) < zptr[i]) {			\
         unsigned long p;				\
         zptr[i] = FixedToUns(fz);			\
         p = PACK_RGB( FixedToInt(fr), FixedToInt(fg),	\
		       FixedToInt(fb) );		\
         XPutPixel( XMesa->backimage, xmin, yy, p );	\
      }							\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;		\
      fz += fdzdx;					\
   }

#include "polytemp.h"
}


/*
 * XImage, smooth, depth-buffered, PF_8A8B8G8R polygon.
 */
static void smooth_8A8B8G8R_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_Z

#define INNER_CODE						\
   GLuint *img = PIXELADDR4(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++) {					\
      if (FixedToUns(fz) < zptr[i]) {				\
         zptr[i] = FixedToUns(fz);				\
         img[i] = PACK_8B8G8R( FixedToInt(fr), FixedToInt(fg),	\
                               FixedToInt(fb) );		\
      }								\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx; 			\
      fz += fdzdx;						\
   }

#include "polytemp.h"
}


/*
 * XImage, smooth, depth-buffered, PF_8R8G8B polygon.
 */
static void smooth_8R8G8B_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_Z

#define INNER_CODE						\
   GLuint *img = PIXELADDR4(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++) {					\
      if (FixedToUns(fz) < zptr[i]) {				\
         zptr[i] = FixedToUns(fz);				\
         img[i] = PACK_8R8G8B( FixedToInt(fr), FixedToInt(fg),	\
                               FixedToInt(fb) );		\
      }								\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx; 			\
      fz += fdzdx;						\
   }

#include "polytemp.h"
}


/*
 * XImage, smooth, depth-buffered, PF_5R6G5B polygon.
 */
static void smooth_5R6G5B_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_Z

#define INNER_CODE						\
   GLushort *img = PIXELADDR2(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++) {					\
      if (FixedToUns(fz) < zptr[i]) {				\
         zptr[i] = FixedToUns(fz);				\
         img[i] = PACK_5R6G5B( FixedToInt(fr), FixedToInt(fg),	\
                               FixedToInt(fb) );		\
      }								\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;			\
      fz += fdzdx;						\
   }

#include "polytemp.h"
}


/*
 * XImage, smooth, depth-buffered, 8-bit PF_DITHER polygon.
 */
static void smooth_DITHER8_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_Z

#define INNER_CODE						\
   GLubyte *img = PIXELADDR1(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++,xmin++) {					\
      if (FixedToUns(fz) < zptr[i]) {				\
         zptr[i] = FixedToUns(fz);				\
         img[i] = DITHER( xmin, y, FixedToInt(fr),		\
                          FixedToInt(fg), FixedToInt(fb) );	\
      }								\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;			\
      fz += fdzdx;						\
   }

#include "polytemp.h"
}



/*
 * XImage, smooth, depth-buffered, PF_DITHER polygon.
 */
static void smooth_DITHER_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_Z

#define INNER_CODE					\
   GLint i, yy = FLIP(y);				\
   for (i=0;i<len;i++,xmin++) {				\
      if (FixedToUns(fz) < zptr[i]) {			\
	 unsigned long p;				\
         zptr[i] = FixedToUns(fz);			\
	 p = DITHER( xmin, y, FixedToInt(fr),		\
                     FixedToInt(fg), FixedToInt(fb) );	\
	 XPutPixel( XMesa->backimage, xmin, yy, p );	\
      }							\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;		\
      fz += fdzdx;					\
   }

#include "polytemp.h"
}



/*
 * XImage, smooth, depth-buffered, 8-bit PF_LOOKUP polygon.
 */
static void smooth_LOOKUP8_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_Z

#define INNER_CODE						\
   GLubyte *img = PIXELADDR1(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++) {					\
      if (FixedToUns(fz) < zptr[i]) {				\
         zptr[i] = FixedToUns(fz);				\
         img[i] = LOOKUP( FixedToInt(fr),			\
                          FixedToInt(fg), FixedToInt(fb) );	\
      }								\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;			\
      fz += fdzdx;						\
   }

#include "polytemp.h"
}




/*
 * XImage, smooth, depth-buffered, 8-bit PF_HPCR polygon.
 */
static void smooth_HPCR_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_Z

#define INNER_CODE						\
   GLubyte *img = PIXELADDR1(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++,xmin++) {					\
      if (FixedToUns(fz) < zptr[i]) {				\
         zptr[i] = FixedToUns(fz);				\
         img[i] = DITHER_HPCR( xmin, y, FixedToInt(fr),		\
                              FixedToInt(fg), FixedToInt(fb) );	\
      }								\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;			\
      fz += fdzdx;						\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, depth-buffered, PF_TRUECOLOR polygon.
 */
static void flat_TRUECOLOR_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_Z

#define SETUP_CODE					\
   unsigned long pixel = PACK_RGB( VB.Color[pv][0],	\
		 VB.Color[pv][1], VB.Color[pv][2] );

#define INNER_CODE						\
   GLint i, yy = FLIP(y);					\
   for (i=0;i<len;i++,xmin++) {					\
      if (FixedToUns(fz) < zptr[i]) {				\
         zptr[i] = FixedToUns(fz);				\
         XPutPixel( XMesa->backimage, xmin, yy, pixel );	\
      }								\
      fz += fdzdx;						\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, depth-buffered, PF_8A8B8G8R polygon.
 */
static void flat_8A8B8G8R_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_Z

#define SETUP_CODE				\
   GLuint pixel = PACK_8B8G8R( VB.Color[pv][0],	\
	     VB.Color[pv][1], VB.Color[pv][2] );

#define INNER_CODE			\
   GLuint *img = PIXELADDR4(xmin,y);	\
   GLint i;				\
   for (i=0;i<len;i++) {		\
      if (FixedToUns(fz) < zptr[i]) {	\
         zptr[i] = FixedToUns(fz);	\
         img[i] = pixel;		\
      }					\
      fz += fdzdx;			\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, depth-buffered, PF_8R8G8B polygon.
 */
static void flat_8R8G8B_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_Z

#define SETUP_CODE				\
   GLuint pixel = PACK_8R8G8B( VB.Color[pv][0],	\
	     VB.Color[pv][1], VB.Color[pv][2] );

#define INNER_CODE			\
   GLuint *img = PIXELADDR4(xmin,y);	\
   GLint i;				\
   for (i=0;i<len;i++) {		\
      if (FixedToUns(fz) < zptr[i]) {	\
         zptr[i] = FixedToUns(fz);	\
         img[i] = pixel;		\
      }					\
      fz += fdzdx;			\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, depth-buffered, PF_5R6G5B polygon.
 */
static void flat_5R6G5B_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_Z

#define SETUP_CODE					\
   GLushort pixel = PACK_5R6G5B( VB.Color[pv][0],	\
		 VB.Color[pv][1], VB.Color[pv][2] );

#define INNER_CODE			\
   GLushort *img = PIXELADDR2(xmin,y);	\
   GLint i;				\
   for (i=0;i<len;i++) {		\
      if (FixedToUns(fz) < zptr[i]) {	\
         zptr[i] = FixedToUns(fz);	\
         img[i] = pixel;		\
      }					\
      fz += fdzdx;			\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, depth-buffered, 8-bit PF_DITHER polygon.
 */
static void flat_DITHER8_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_Z

#define SETUP_CODE				\
   GLubyte r = VB.Color[pv][0];			\
   GLubyte g = VB.Color[pv][1];			\
   GLubyte b = VB.Color[pv][2];

#define INNER_CODE				\
   GLubyte *img = PIXELADDR1(xmin,y);		\
   GLint i;					\
   for (i=0;i<len;i++,xmin++) {			\
      if (FixedToUns(fz) < zptr[i]) {		\
         zptr[i] = FixedToUns(fz);		\
         img[i] = DITHER( xmin, y, r, g, b );	\
      }						\
      fz += fdzdx;				\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, depth-buffered, PF_DITHER polygon.
 */
static void flat_DITHER_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_Z

#define SETUP_CODE				\
   GLubyte r = VB.Color[pv][0];			\
   GLubyte g = VB.Color[pv][1];			\
   GLubyte b = VB.Color[pv][2];

#define INNER_CODE					\
   GLint i, yy = FLIP(y);				\
   for (i=0;i<len;i++,xmin++) {				\
      if (FixedToUns(fz) < zptr[i]) {			\
	 unsigned long p;				\
         zptr[i] = FixedToUns(fz);			\
         p = DITHER( xmin, yy, r, g, b );		\
         XPutPixel( XMesa->backimage, xmin, yy, p );	\
      }							\
      fz += fdzdx;					\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, depth-buffered, 8-bit PF_HPCR polygon.
 */
static void flat_HPCR_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_Z

#define SETUP_CODE				\
   GLubyte r = VB.Color[pv][0];			\
   GLubyte g = VB.Color[pv][1];			\
   GLubyte b = VB.Color[pv][2];

#define INNER_CODE					\
   GLubyte *img = PIXELADDR1(xmin,y);			\
   GLint i;						\
   for (i=0;i<len;i++,xmin++) {				\
      if (FixedToUns(fz) < zptr[i]) {			\
         zptr[i] = FixedToUns(fz);			\
         img[i] = DITHER_HPCR( xmin, y, r, g, b );	\
      }							\
      fz += fdzdx;					\
   }

#include "polytemp.h"
}




/*
 * XImage, flat, depth-buffered, 8-bit PF_LOOKUP polygon.
 */
static void flat_LOOKUP8_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_Z

#define SETUP_CODE				\
   GLubyte r = VB.Color[pv][0];			\
   GLubyte g = VB.Color[pv][1];			\
   GLubyte b = VB.Color[pv][2];			\
   GLubyte pixel = LOOKUP(r,g,b);

#define INNER_CODE				\
   GLubyte *img = PIXELADDR1(xmin,y);		\
   GLint i;					\
   for (i=0;i<len;i++) {			\
      if (FixedToUns(fz) < zptr[i]) {		\
         zptr[i] = FixedToUns(fz);		\
         img[i] = pixel;			\
      }						\
      fz += fdzdx;				\
   }

#include "polytemp.h"
}




/*
 * XImage, smooth, NON-depth-buffered, PF_TRUECOLOR polygon.
 */
static void smooth_TRUECOLOR_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR

#define INNER_CODE							\
   GLint i, yy = FLIP(y);						\
   for (i=0;i<len;i++,xmin++) {						\
      unsigned long p;							\
      p = PACK_RGB( FixedToInt(fr), FixedToInt(fg), FixedToInt(fb) );	\
      XPutPixel( XMesa->backimage, xmin, yy, p );			\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;				\
   }

#include "polytemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, PF_8A8B8G8R polygon.
 */
static void smooth_8A8B8G8R_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR

#define INNER_CODE						\
   GLuint *img = PIXELADDR4(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++) {					\
      img[i] = PACK_8B8G8R( FixedToInt(fr), FixedToInt(fg),	\
			    FixedToInt(fb) );			\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx; 			\
   }

#include "polytemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, PF_8R8G8B polygon.
 */
static void smooth_8R8G8B_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR

#define INNER_CODE						\
   GLuint *img = PIXELADDR4(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++) {					\
      img[i] = PACK_8R8G8B( FixedToInt(fr), FixedToInt(fg),	\
			    FixedToInt(fb) );			\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx; 			\
   }

#include "polytemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, PF_5R6G5B polygon.
 */
static void smooth_5R6G5B_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR

#define INNER_CODE						\
   GLushort *img = PIXELADDR2(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++) {					\
      img[i] = PACK_5R6G5B( FixedToInt(fr), FixedToInt(fg),	\
                            FixedToInt(fb) );			\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;			\
   }

#include "polytemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, 8-bit PF_DITHER polygon.
 */
static void smooth_DITHER8_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR

#define INNER_CODE						\
   GLubyte *img = PIXELADDR1(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++,xmin++) {					\
      img[i] = DITHER( xmin, y, FixedToInt(fr),			\
		       FixedToInt(fg), FixedToInt(fb) );	\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;			\
   }

#include "polytemp.h"
}



/*
 * XImage, smooth, NON-depth-buffered, PF_DITHER polygon.
 */
static void smooth_DITHER_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR

#define INNER_CODE					\
   GLint i, yy = FLIP(y);				\
   for (i=0;i<len;i++,xmin++) {				\
      unsigned long p;					\
      p = DITHER( xmin, y, FixedToInt(fr),		\
                  FixedToInt(fg), FixedToInt(fb) );	\
      XPutPixel( XMesa->backimage, xmin, yy, p );	\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;		\
   }

#include "polytemp.h"
}



/*
 * XImage, smooth, NON-depth-buffered, 8-bit PF_LOOKUP polygon.
 */
static void smooth_LOOKUP8_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR

#define INNER_CODE						\
   GLubyte *img = PIXELADDR1(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++) {					\
      img[i] = LOOKUP( FixedToInt(fr),				\
		       FixedToInt(fg), FixedToInt(fb) );	\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;			\
   }

#include "polytemp.h"
}



/*
 * XImage, smooth, NON-depth-buffered, 8-bit PF_HPCR polygon.
 */
static void smooth_HPCR_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR

#define INNER_CODE						\
   GLubyte *img = PIXELADDR1(xmin,y);				\
   GLint i;							\
   for (i=0;i<len;i++,xmin++) {					\
      img[i] = DITHER_HPCR( xmin, y, FixedToInt(fr),		\
                            FixedToInt(fg), FixedToInt(fb) );	\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;			\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, NON-depth-buffered, PF_TRUECOLOR polygon.
 */
static void flat_TRUECOLOR_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define SETUP_CODE					\
   unsigned long pixel = PACK_RGB( VB.Color[pv][0],	\
		 VB.Color[pv][1], VB.Color[pv][2] );

#define INNER_CODE					\
   GLint i, yy = FLIP(y);				\
   for (i=0;i<len;i++,xmin++) {				\
      XPutPixel( XMesa->backimage, xmin, yy, pixel );	\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, NON-depth-buffered, PF_8A8B8G8R polygon.
 */
static void flat_8A8B8G8R_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define SETUP_CODE					\
   GLuint pixel = PACK_8B8G8R( VB.Color[pv][0],		\
			VB.Color[pv][1], VB.Color[pv][2] );

#define INNER_CODE			\
   GLuint *img = PIXELADDR4(xmin,y);	\
   GLint i;				\
   for (i=0;i<len;i++) {		\
      img[i] = pixel;			\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, NON-depth-buffered, PF_8R8G8B polygon.
 */
static void flat_8R8G8B_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define SETUP_CODE					\
   GLuint pixel = PACK_8R8G8B( VB.Color[pv][0],		\
			VB.Color[pv][1], VB.Color[pv][2] );

#define INNER_CODE			\
   GLuint *img = PIXELADDR4(xmin,y);	\
   GLint i;				\
   for (i=0;i<len;i++) {		\
      img[i] = pixel;			\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, NON-depth-buffered, PF_5R6G5B polygon.
 */
static void flat_5R6G5B_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define SETUP_CODE					\
   GLushort pixel = PACK_5R6G5B( VB.Color[pv][0],	\
		 VB.Color[pv][1], VB.Color[pv][2] );

#define INNER_CODE			\
   GLushort *img = PIXELADDR2(xmin,y);	\
   GLint i;				\
   for (i=0;i<len;i++) {		\
      img[i] = pixel;			\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, NON-depth-buffered, 8-bit PF_DITHER polygon.
 */
static void flat_DITHER8_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define SETUP_CODE				\
   GLubyte r = VB.Color[pv][0];			\
   GLubyte g = VB.Color[pv][1];			\
   GLubyte b = VB.Color[pv][2];

#define INNER_CODE				\
   GLubyte *img = PIXELADDR1(xmin,y);		\
   GLint i;					\
   for (i=0;i<len;i++,xmin++) {			\
      img[i] = DITHER( xmin, y, r, g, b );	\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, NON-depth-buffered, PF_DITHER polygon.
 */
static void flat_DITHER_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define SETUP_CODE				\
   GLubyte r = VB.Color[pv][0];			\
   GLubyte g = VB.Color[pv][1];			\
   GLubyte b = VB.Color[pv][2];

#define INNER_CODE					\
   GLint i, yy = FLIP(y);				\
   for (i=0;i<len;i++,xmin++) {				\
      unsigned long p = DITHER( xmin, yy, r, g, b );	\
      XPutPixel( XMesa->backimage, xmin, yy, p );	\
   }

#include "polytemp.h"
}



/*
 * XImage, flat, NON-depth-buffered, 8-bit PF_HPCR polygon.
 */
static void flat_HPCR_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define SETUP_CODE				\
   GLubyte r = VB.Color[pv][0];			\
   GLubyte g = VB.Color[pv][1];			\
   GLubyte b = VB.Color[pv][2];

#define INNER_CODE				\
   GLubyte *img = PIXELADDR1(xmin,y);		\
   GLint i;					\
   for (i=0;i<len;i++,xmin++) {			\
      img[i] = DITHER_HPCR( xmin, y, r, g, b );	\
   }

#include "polytemp.h"
}




/*
 * XImage, flat, NON-depth-buffered, 8-bit PF_LOOKUP polygon.
 */
static void flat_LOOKUP8_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define SETUP_CODE				\
   GLubyte r = VB.Color[pv][0];			\
   GLubyte g = VB.Color[pv][1];			\
   GLubyte b = VB.Color[pv][2];			\
   GLubyte pixel = LOOKUP(r,g,b);

#define INNER_CODE			\
   GLubyte *img = PIXELADDR1(xmin,y);	\
   GLint i;				\
   for (i=0;i<len;i++) {		\
      img[i] = pixel;			\
   }

#include "polytemp.h"
}




/*
 * Analyze current CC and device driver state to see if we can provide a
 * fast polygon drawing function, like those in polygons.c.  Otherwise,
 * return NULL.
 */
polygon_func xmesa_get_polygon_func( void )
{
   static int first_time=1;
   if (first_time) {
      int i;
      for (i=0;i<MAX_HEIGHT;i++) {
         lx[i] = MAX_WIDTH;
         rx[i] = -1;
      }
      first_time=0;
   }

   if (CC.Polygon.SmoothFlag)     return NULL;
   if (CC.Polygon.StippleFlag)    return NULL;
   if (CC.Texture.Enabled)        return NULL;

   if (XMesa->buffer==XIMAGE) {
      if (   CC.Light.ShadeModel==GL_SMOOTH
          && CC.RasterMask==DEPTH_BIT
          && CC.Depth.Func==GL_LESS
          && CC.Depth.Mask==GL_TRUE) {
         switch (XMesa->pixelformat) {
            case PF_TRUECOLOR:
	       return smooth_TRUECOLOR_z_polygon;
            case PF_8A8B8G8R:
               return smooth_8A8B8G8R_z_polygon;
            case PF_8R8G8B:
               return smooth_8R8G8B_z_polygon;
            case PF_5R6G5B:
               return smooth_5R6G5B_z_polygon;
            case PF_HPCR:
	       return smooth_HPCR_z_polygon;
            case PF_DITHER:
               return (XMesa->depth==8) ? smooth_DITHER8_z_polygon
                                        : smooth_DITHER_z_polygon;
            case PF_LOOKUP:
               return (XMesa->depth==8) ? smooth_LOOKUP8_z_polygon : NULL;
            default:
               return NULL;
         }
      }
      if (   CC.Light.ShadeModel==GL_FLAT
          && CC.RasterMask==DEPTH_BIT
          && CC.Depth.Func==GL_LESS
          && CC.Depth.Mask==GL_TRUE) {
         switch (XMesa->pixelformat) {
            case PF_TRUECOLOR:
	       return flat_TRUECOLOR_z_polygon;
            case PF_8A8B8G8R:
               return flat_8A8B8G8R_z_polygon;
            case PF_8R8G8B:
               return flat_8R8G8B_z_polygon;
            case PF_5R6G5B:
               return flat_5R6G5B_z_polygon;
            case PF_HPCR:
	       return flat_HPCR_z_polygon;
            case PF_DITHER:
               return (XMesa->depth==8) ? flat_DITHER8_z_polygon
                                        : flat_DITHER_z_polygon;
            case PF_LOOKUP:
               return (XMesa->depth==8) ? flat_LOOKUP8_z_polygon : NULL;
            default:
               return NULL;
         }
      }
      if (   CC.RasterMask==0   /* no depth test */
          && CC.Light.ShadeModel==GL_SMOOTH) {
         switch (XMesa->pixelformat) {
            case PF_TRUECOLOR:
	       return smooth_TRUECOLOR_polygon;
            case PF_8A8B8G8R:
               return smooth_8A8B8G8R_polygon;
            case PF_8R8G8B:
               return smooth_8R8G8B_polygon;
            case PF_5R6G5B:
               return smooth_5R6G5B_polygon;
            case PF_HPCR:
	       return smooth_HPCR_polygon;
            case PF_DITHER:
               return (XMesa->depth==8) ? smooth_DITHER8_polygon
                                        : smooth_DITHER_polygon;
            case PF_LOOKUP:
               return (XMesa->depth==8) ? smooth_LOOKUP8_polygon : NULL;
            default:
               return NULL;
         }
      }
      if (   CC.RasterMask==0   /* no depth test */
          && CC.Light.ShadeModel==GL_FLAT) {
         switch (XMesa->pixelformat) {
            case PF_TRUECOLOR:
	       return flat_TRUECOLOR_polygon;
            case PF_8A8B8G8R:
               return flat_8A8B8G8R_polygon;
            case PF_8R8G8B:
               return flat_8R8G8B_polygon;
            case PF_5R6G5B:
               return flat_5R6G5B_polygon;
            case PF_HPCR:
	       return flat_HPCR_polygon;
            case PF_DITHER:
               return (XMesa->depth==8) ? flat_DITHER8_polygon
                                        : flat_DITHER_polygon;
            case PF_LOOKUP:
               return (XMesa->depth==8) ? flat_LOOKUP8_polygon : NULL;
            default:
               return NULL;
         }
      }
      return NULL;
   }
   else {
      /* pixmap */
      if (CC.Light.ShadeModel==GL_FLAT && CC.RasterMask==0) {
         return draw_polygon_ANY_pixmap;
      }
      return NULL;
   }
}

