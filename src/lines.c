/* lines.c */

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
$Id: lines.c,v 1.36 1996/02/12 19:08:45 brianp Exp $

$Log: lines.c,v $
 * Revision 1.36  1996/02/12  19:08:45  brianp
 * applied Bill's Feb 8 patches: change eye Z's to clip W's
 *
 * Revision 1.35  1996/02/01  00:55:40  brianp
 * added some DEFARRAY(),UNDEFARRY() macros for Macintosh
 *
 * Revision 1.34  1996/01/22  15:29:16  brianp
 * replaced gl_interpolate_4fixed() with gl_interpolate_rgba()
 * changed a few variables from GLuint to GLint
 *
 * Revision 1.33  1995/12/30  17:17:22  brianp
 * use new gl_interp_texcoords function, per Bill Triggs
 *
 * Revision 1.32  1995/12/30  00:55:22  brianp
 * used fixed point vertex colors instead of floating point
 *
 * Revision 1.31  1995/12/20  15:26:14  brianp
 * changed VB color indexes to GLuint
 *
 * Revision 1.30  1995/12/19  22:17:05  brianp
 * removed 0.5 offset from window coordinates thanks to CC.RasterOffsetX/Y
 *
 * Revision 1.29  1995/12/18  17:26:15  brianp
 * replaced MAX_DEPTH with DEPTH_SCALE
 *
 * Revision 1.28  1995/12/12  21:48:21  brianp
 * call GL_INTERPOLATE_Z instead of GL_INTERPOLATE_I
 *
 * Revision 1.27  1995/10/23  21:28:30  brianp
 * use new gl_interpolate_4ub function
 *
 * Revision 1.26  1995/10/17  21:40:58  brianp
 * removed simple_ci/rgba_line() functions because of new device driver
 *
 * Revision 1.25  1995/09/21  14:07:29  brianp
 * more new DD prototyping
 *
 * Revision 1.24  1995/09/20  18:20:39  brianp
 * prototype device driver changes described
 *
 * Revision 1.23  1995/09/13  14:49:34  brianp
 * use CC.NewState convention
 * replaced VB.Vs and VB.Vt with VB.TexCoord
 *
 * Revision 1.22  1995/07/25  18:36:32  brianp
 * convert window coords from floats to ints by rounding, not truncating
 *
 * Revision 1.21  1995/07/15  14:03:49  brianp
 * added texture mapped lines
 *
 * Revision 1.20  1995/07/07  12:41:51  brianp
 * use CLAMP macro in glLineStipple, upper limit being 256, not 255
 *
 * Revision 1.19  1995/06/20  16:21:34  brianp
 * do float-to-int depth scaling here instead of in draw.c
 *
 * Revision 1.18  1995/06/12  15:39:09  brianp
 * changed color arrays to GLubyte
 * implement GL_LINE_RESET_TOKEN for feedback
 * new interpolation functions
 *
 * Revision 1.17  1995/06/07  14:47:04  brianp
 * faster width=2 lines
 *
 * Revision 1.16  1995/06/05  20:27:09  brianp
 * removed PB.clipflag stuff
 *
 * Revision 1.15  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.14  1995/05/12  19:26:43  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.13  1995/05/12  16:26:50  brianp
 * let PB do clipping of pixels for lines wider than 1
 *
 * Revision 1.12  1995/04/18  15:48:23  brianp
 * fixed assignment of NULL to function pointers to prevent warnings on Suns
 *
 * Revision 1.11  1995/04/12  15:36:15  brianp
 * updated to use DD.draw_* function pointers
 *
 * Revision 1.10  1995/03/27  20:31:53  brianp
 * new Texture.Enabled scheme
 *
 * Revision 1.9  1995/03/24  15:33:25  brianp
 * introduced VB
 *
 * Revision 1.8  1995/03/07  14:20:50  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.7  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.6  1995/03/04  19:16:57  brianp
 * added width clamp
 *
 * Revision 1.5  1995/03/02  19:18:20  brianp
 * new RasterMask logic
 *
 * Revision 1.4  1995/02/27  22:48:54  brianp
 * modified for PB
 *
 * Revision 1.3  1995/02/27  15:08:08  brianp
 * added Vcolor/Vindex scheme
 *
 * Revision 1.2  1995/02/24  15:15:53  brianp
 * changed && to & in gl_set_line_func
 *
 * Revision 1.1  1995/02/24  14:23:09  brianp
 * Initial revision
 *
 */


#include "bresenhm.h"
#include "context.h"
#include "dd.h"
#include "feedback.h"
#include "interp.h"
#include "list.h"
#include "macros.h"
#include "pb.h"
#include "vb.h"


#ifndef NULL
#  define NULL 0
#endif


void glLineWidth( GLfloat width )
{
   if (CC.CompileFlag) {
      gl_save_linewidth( width );
   }
   if (CC.ExecuteFlag) {
      if (width<=0.0) {
	 gl_error( GL_INVALID_VALUE, "glLineWidth" );
	 return;
      }
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glLineWidth" );
	 return;
      }

      CC.Line.Width = width;
      CC.NewState = GL_TRUE;
   }
}



void glLineStipple( GLint factor, GLushort pattern )
{
   if (CC.CompileFlag) {
      gl_save_linestipple( factor, pattern );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glLineStipple" );
	 return;
      }

      CC.Line.StippleFactor = CLAMP( factor, 1, 256 );
      CC.Line.StipplePattern = pattern;
   }
}



/**********************************************************************/
/*****                    Rasterization                           *****/
/**********************************************************************/


/*
 * There are 4 pairs (RGBA, CI) of line drawing functions:
 *   1. simple:  width=1 and no special rasterization functions (fastest)
 *   2. flat:  width=1, non-stippled, flat-shaded, any raster operations
 *   3. smooth:  width=1, non-stippled, smooth-shaded, any raster operations
 *   4. general:  any other kind of line (slowest)
 */


/*
 * All line drawing functions have the same arguments:
 * v1, v2 - indexes of first and second endpoints into vertex buffer arrays
 * pv     - provoking vertex: which vertex color/index to use for flat shading.
 */



static void feedback_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLfloat x1, y1, z1, w1;
   GLfloat x2, y2, z2, w2;
   GLfloat tex1[4], tex2[4];  /* texture coord */

   x1 = VB.Win[v1][0] - CC.RasterOffsetX;
   y1 = VB.Win[v1][1] - CC.RasterOffsetY;
   z1 = VB.Win[v1][2];
   w1 = VB.Clip[v1][3];

   x2 = VB.Win[v2][0] - CC.RasterOffsetX;
   y2 = VB.Win[v2][1] - CC.RasterOffsetY;
   z2 = VB.Win[v2][2];
   w2 = VB.Clip[v2][3];

   if (CC.StippleCounter==0) {
      APPEND_TOKEN( (GLfloat) GL_LINE_RESET_TOKEN );
   }
   else {
      APPEND_TOKEN( (GLfloat) GL_LINE_TOKEN );
   }
   if (CC.Light.ShadeModel==GL_FLAT) {
      GLfloat color[4];
      /* convert color from integer back to a float in [0,1] */
      color[0] = (GLfloat) VB.Color[pv][0] / CC.RedScale;
      color[1] = (GLfloat) VB.Color[pv][1] / CC.GreenScale;
      color[2] = (GLfloat) VB.Color[pv][2] / CC.BlueScale;
      color[3] = (GLfloat) VB.Color[pv][3] / CC.AlphaScale;
      gl_feedback_vertex( x1,y1,z1,w1, color, (GLfloat) VB.Index[pv], tex1 );
      gl_feedback_vertex( x2,y2,z2,w2, color, (GLfloat) VB.Index[pv], tex2 );
   }
   else {
      GLfloat color[4];
      /* convert color from integer back to a float in [0,1] */
      color[0] = (GLfloat) (VB.Color[v1][0] >> CC.ColorShift) / CC.RedScale;
      color[1] = (GLfloat) (VB.Color[v1][1] >> CC.ColorShift) / CC.GreenScale;
      color[2] = (GLfloat) (VB.Color[v1][2] >> CC.ColorShift) / CC.BlueScale;
      color[3] = (GLfloat) (VB.Color[v1][3] >> CC.ColorShift) / CC.AlphaScale;
      gl_feedback_vertex( x1,y1,z1,w1, color, (GLfloat) VB.Index[v1], tex1 );
      /* convert color from integer back to a float in [0,1] */
      color[0] = (GLfloat) (VB.Color[v2][0] >> CC.ColorShift) / CC.RedScale;
      color[1] = (GLfloat) (VB.Color[v2][1] >> CC.ColorShift) / CC.GreenScale;
      color[2] = (GLfloat) (VB.Color[v2][2] >> CC.ColorShift) / CC.BlueScale;
      color[3] = (GLfloat) (VB.Color[v2][3] >> CC.ColorShift) / CC.AlphaScale;
      gl_feedback_vertex( x2,y2,z2,w2, color, (GLfloat) VB.Index[v2], tex2 );
   }
   CC.StippleCounter++;
}



static void select_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLfloat z1 = VB.Win[v1][2];
   GLfloat z2 = VB.Win[v2][2];

   CC.HitFlag = GL_TRUE;
   if (z1 < CC.HitMinZ) {
      CC.HitMinZ = z1;
   }
   if (z1 > CC.HitMaxZ) {
      CC.HitMaxZ = z1;
   }
   if (z2 < CC.HitMinZ) {
      CC.HitMinZ = z2;
   }
   if (z2 > CC.HitMaxZ) {
      CC.HitMaxZ = z2;
   }
}



#if MAX_WIDTH > MAX_HEIGHT
#  define MAXPOINTS MAX_WIDTH
#else
#  define MAXPOINTS MAX_HEIGHT
#endif


/*
 * Flat shaded, width=1, non-stippled, color index line.
 */
static void flat_ci_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v1][0];
   GLint y1 = (GLint) VB.Win[v1][1];
   GLint x2 = (GLint) VB.Win[v2][0];
   GLint y2 = (GLint) VB.Win[v2][1];
   GLint n;

   PB_SET_INDEX( VB.Index[pv] );

   /* compute pixel locations */
   n = gl_bresenham( x1, y1, x2, y2, PB.x+PB.count, PB.y+PB.count );

   /* interpolate z values */
   if (CC.Depth.Test) {
      GLdepth *zptr = PB.z + PB.count;
      GLint z1 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
      GLint z2 = (GLint) (VB.Win[v2][2] * DEPTH_SCALE);
      GL_INTERPOLATE_Z( n, z1, z2, zptr );
   }

   PB.count += n;
   PB_CHECK_FLUSH
}


/*
 * Flat-shaded, width=1, non-stippled, rgba line.
 */
static void flat_rgba_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v1][0];
   GLint y1 = (GLint) VB.Win[v1][1];
   GLint x2 = (GLint) VB.Win[v2][0];
   GLint y2 = (GLint) VB.Win[v2][1];
   GLint n;

   /* Note that color components are ints, not fixed point here */
   PB_SET_COLOR( VB.Color[pv][0], VB.Color[pv][1],
                 VB.Color[pv][2], VB.Color[pv][3] );

   /* compute pixel locations */
   n = gl_bresenham( x1, y1, x2, y2, PB.x+PB.count, PB.y+PB.count );

   /* interpolate z values */
   if (CC.Depth.Test) {
      GLdepth *zptr = PB.z + PB.count;
      GLint z1 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
      GLint z2 = (GLint) (VB.Win[v2][2] * DEPTH_SCALE);
      GL_INTERPOLATE_Z( n, z1, z2, zptr );
   }

   PB.count += n;
   PB_CHECK_FLUSH
}



/*
 * Smooth-shaded, width=1, non-stippled, color index line.
 */
static void smooth_ci_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v1][0];
   GLint y1 = (GLint) VB.Win[v1][1];
   GLint x2 = (GLint) VB.Win[v2][0];
   GLint y2 = (GLint) VB.Win[v2][1];
   GLint n;

   /* compute pixel locations */
   n = gl_bresenham( x1, y1, x2, y2, PB.x+PB.count, PB.y+PB.count );

   /* interpolate z values */
   if (CC.Depth.Test) {
      GLdepth *zptr = PB.z + PB.count;
      GLint z1 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
      GLint z2 = (GLint) (VB.Win[v2][2] * DEPTH_SCALE);
      GL_INTERPOLATE_Z( n, z1, z2, zptr );
   }

   /* interpolate index */
   gl_interpolate_i( n, (GLint) VB.Index[v1], (GLint) VB.Index[v2],
                     (GLint *) PB.i+PB.count );

   PB.count += n;
   PB_CHECK_FLUSH
}


/*
 * Smooth-shaded, width=1, non-stippled, RGBA line.
 */
static void smooth_rgba_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLint x1 = (GLint) VB.Win[v1][0];
   GLint y1 = (GLint) VB.Win[v1][1];
   GLint x2 = (GLint) VB.Win[v2][0];
   GLint y2 = (GLint) VB.Win[v2][1];
   GLint n;

   /* compute pixel locations */
   n = gl_bresenham( x1, y1, x2, y2, PB.x+PB.count, PB.y+PB.count );

   /* interpolate z values */
   if (CC.Depth.Test) {
      GLdepth *zptr = PB.z + PB.count;
      GLint z1 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
      GLint z2 = (GLint) (VB.Win[v2][2] * DEPTH_SCALE);
      GL_INTERPOLATE_Z( n, z1, z2, zptr );
   }

   /* interpolate color, VB.Colors are in fixed point */
   gl_interpolate_rgba( n,
                        VB.Color[v1][0], VB.Color[v2][0], PB.r+PB.count,
                        VB.Color[v1][1], VB.Color[v2][1], PB.g+PB.count,
                        VB.Color[v1][2], VB.Color[v2][2], PB.b+PB.count,
                        VB.Color[v1][3], VB.Color[v2][3], PB.a+PB.count );

   PB.count += n;
   PB_CHECK_FLUSH
}



/*
 * General CI line:  any width, smooth or flat, stippled, any raster ops.
 */
static void general_ci_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLint x1, y1, x2, y2;
   GLint x[MAXPOINTS], y[MAXPOINTS];
   GLdepth z[MAXPOINTS];
   GLubyte mask[MAXPOINTS];
   GLuint index[MAXPOINTS];
   GLint i, n;
   GLint dx, dy;

   x1 = (GLint) VB.Win[v1][0];
   y1 = (GLint) VB.Win[v1][1];
   x2 = (GLint) VB.Win[v2][0];
   y2 = (GLint) VB.Win[v2][1];

   /* compute pixel locations */
   if (CC.Line.StippleFlag) {
      n = gl_stippled_bresenham( x1, y1, x2, y2, x, y, mask );
   }
   else {
      n = gl_bresenham( x1, y1, x2, y2, x, y );
      for (i=0;i<n;i++) {
	 mask[i] = 1;
      }
   }

   if (CC.Depth.Test) {
      /* interpolate z */
      GLint z1 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
      GLint z2 = (GLint) (VB.Win[v2][2] * DEPTH_SCALE);
      GL_INTERPOLATE_Z( n, z1, z2, z );
   }

   if (CC.Light.ShadeModel==GL_FLAT) {
      GLuint indx = VB.Index[pv];
      for (i=0;i<n;i++) {
	 index[i] = indx;
      }
   }
   else {
      /* interpolate index */
      gl_interpolate_i( n, (GLint) VB.Index[v1], (GLint) VB.Index[v2],
		        (GLint *) index );
   }

   /* compute delta x and delta y */
   if (x1>x2) {
      dx = x1 - x2;
   }
   else {
      dx = x2 - x1;
   }
   if (y1>y2) {
      dy = y1 - y2;
   }
   else {
      dy = y2 - y1;
   }


   /* render */
   if (CC.Line.Width==2.0F) {
      /* special case, easy to optimize */
      if (dx>dy) {
	 /* X-major: duplicate pixels in Y direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       PB_WRITE_CI_PIXEL( x[i], y[i]-1, z[i], index[i] );
	       PB_WRITE_CI_PIXEL( x[i], y[i], z[i], index[i] );
	    }
	 }
      }
      else {
	 /* Y-major: duplicate pixels in X direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       PB_WRITE_CI_PIXEL( x[i]-1, y[i], z[i], index[i] );
	       PB_WRITE_CI_PIXEL( x[i], y[i], z[i], index[i] );
	    }
	 }
      }
      PB_CHECK_FLUSH
   }
   else {
      GLint width, w0, w1;
      width = (GLint) CLAMP( CC.Line.Width, MIN_LINE_WIDTH, MAX_LINE_WIDTH );
      w0 = -width / 2;
      w1 = w0 + width - 1;

      if (dx>dy) {
	 /* X-major: duplicate pixels in Y direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       GLint yy;
	       GLint y0 = y[i] + w0;
	       GLint y1 = y[i] + w1;
	       for (yy=y0;yy<=y1;yy++) {
		  PB_WRITE_CI_PIXEL( x[i], yy, z[i], index[i] );
	       }
	       PB_CHECK_FLUSH
	    }
	 }
      }
      else {
	 /* Y-major: duplicate pixels in X direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       GLint xx;
	       GLint x0 = x[i] + w0;
	       GLint x1 = x[i] + w1;
	       for (xx=x0;xx<=x1;xx++) {
		  PB_WRITE_CI_PIXEL( xx, y[i], z[i], index[i] );
	       }
	       PB_CHECK_FLUSH
	    }
	 }
      }
   }
}


/*
 * General RGBA line:  any width, smooth or flat, stippled, any raster ops.
 */
static void general_rgba_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLint x1, y1, x2, y2;
   GLint x[MAXPOINTS], y[MAXPOINTS];
   GLdepth z[MAXPOINTS];
   GLubyte mask[MAXPOINTS];
   GLubyte red[MAXPOINTS], green[MAXPOINTS], blue[MAXPOINTS], alpha[MAXPOINTS];
   GLint i, n;
   GLint dx, dy;

   x1 = (GLint) VB.Win[v1][0];
   y1 = (GLint) VB.Win[v1][1];
   x2 = (GLint) VB.Win[v2][0];
   y2 = (GLint) VB.Win[v2][1];

   /* compute the line */
   if (CC.Line.StippleFlag) {
      n = gl_stippled_bresenham( x1, y1, x2, y2, x, y, mask );
   }
   else {
      n = gl_bresenham( x1, y1, x2, y2, x, y );
      for (i=0;i<n;i++) {
	 mask[i] = 1;
      }
   }

   if (CC.Depth.Test) {
      GLint z1 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
      GLint z2 = (GLint) (VB.Win[v2][2] * DEPTH_SCALE);
      GL_INTERPOLATE_Z( n, z1, z2, z );
   }

   if (CC.Light.ShadeModel==GL_FLAT) {
      GLint r, g, b, a;
      r = VB.Color[pv][0];  /* colors are ints, not fixed point */
      g = VB.Color[pv][1];
      b = VB.Color[pv][2];
      a = VB.Color[pv][3];
      for (i=0;i<n;i++) {
	 red[i]   = r;
	 green[i] = g;
	 blue[i]  = b;
	 alpha[i] = a;
      }
   }
   else {
      /* interpolate color, VB.Colors are in fixed point */
      gl_interpolate_rgba( n,
                           VB.Color[v1][0], VB.Color[v2][0], red,
                           VB.Color[v1][1], VB.Color[v2][1], green,
                           VB.Color[v1][2], VB.Color[v2][2], blue,
                           VB.Color[v1][3], VB.Color[v2][3], alpha );
   }

   /* compute delta x and delta y */
   if (x1>x2) {
      dx = x1 - x2;
   }
   else {
      dx = x2 - x1;
   }
   if (y1>y2) {
      dy = y1 - y2;
   }
   else {
      dy = y2 - y1;
   }

   /* render */
   if (CC.Line.Width==2.0F) {
      /* special case, easy to optimize */
      if (dx>dy) {
	 /* X-major: duplicate pixels in Y direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       PB_WRITE_RGBA_PIXEL( x[i], y[i]-1, z[i],
				    red[i], green[i], blue[i], alpha[i] );
	       PB_WRITE_RGBA_PIXEL( x[i], y[i], z[i],
				    red[i], green[i], blue[i], alpha[i] );
	    }
	 }
      }
      else {
	 /* Y-major: duplicate pixels in X direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       PB_WRITE_RGBA_PIXEL( x[i]-1, y[i], z[i],
				    red[i], green[i], blue[i], alpha[i] );
	       PB_WRITE_RGBA_PIXEL( x[i], y[i], z[i],
				    red[i], green[i], blue[i], alpha[i] );
	    }
	 }
      }
      PB_CHECK_FLUSH
   }
   else {
      GLint width, w0, w1;
      width = (GLint) CLAMP( CC.Line.Width, MIN_LINE_WIDTH, MAX_LINE_WIDTH );
      w0 = -width / 2;
      w1 = w0 + width - 1;

      if (dx>dy) {
	 /* X-major: duplicate pixels in Y direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       GLint yy;
	       GLint y0 = y[i] + w0;
	       GLint y1 = y[i] + w1;
	       for (yy=y0;yy<=y1;yy++) {
		  PB_WRITE_RGBA_PIXEL( x[i], yy, z[i],
				       red[i], green[i], blue[i], alpha[i] );
	       }
	       PB_CHECK_FLUSH
	    }
	 }
      }
      else {
	 /* Y-major: duplicate pixels in X direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       GLint xx;
	       GLint x0 = x[i] + w0;
	       GLint x1 = x[i] + w1;
	       for (xx=x0;xx<=x1;xx++) {
		  PB_WRITE_RGBA_PIXEL( xx, y[i], z[i],
				       red[i], green[i], blue[i], alpha[i] );
	       }
	       PB_CHECK_FLUSH
	    }
	 }
      }
   }
}



/*
 * Textured RGBA line:  any width, smooth or flat, stippled, any raster ops
 * with texturing.
 */
static void textured_rgba_line( GLuint v1, GLuint v2, GLuint pv )
{
   GLint x1, y1, x2, y2;
   GLint x[MAXPOINTS], y[MAXPOINTS];
   GLdepth z[MAXPOINTS];
   GLubyte mask[MAXPOINTS];
   /* Allocate arrays dynamically on Mac */
   DEFARRAY(GLubyte,red,MAXPOINTS);
   DEFARRAY(GLubyte,green,MAXPOINTS);
   DEFARRAY(GLubyte,blue,MAXPOINTS);
   DEFARRAY(GLubyte,alpha,MAXPOINTS);
   DEFARRAY(GLfloat,s,MAXPOINTS);
   DEFARRAY(GLfloat,t,MAXPOINTS);
   DEFARRAY(GLfloat,u,MAXPOINTS);
   DEFARRAY(GLfloat,v,MAXPOINTS);
   GLint i, n;
   GLint dx, dy;

   x1 = (GLint) VB.Win[v1][0];
   y1 = (GLint) VB.Win[v1][1];
   x2 = (GLint) VB.Win[v2][0];
   y2 = (GLint) VB.Win[v2][1];

   /* compute the line */
   if (CC.Line.StippleFlag) {
      n = gl_stippled_bresenham( x1, y1, x2, y2, x, y, mask );
   }
   else {
      n = gl_bresenham( x1, y1, x2, y2, x, y );
      for (i=0;i<n;i++) {
	 mask[i] = 1;
      }
   }

   if (CC.Depth.Test) {
      GLint z1 = (GLint) (VB.Win[v1][2] * DEPTH_SCALE);
      GLint z2 = (GLint) (VB.Win[v2][2] * DEPTH_SCALE);
      GL_INTERPOLATE_Z( n, z1, z2, z );
   }

   if (CC.Light.ShadeModel==GL_FLAT) {
      GLint r, g, b, a;
      r = VB.Color[pv][0];  /* colors are ints, not in fixed point */
      g = VB.Color[pv][1];
      b = VB.Color[pv][2];
      a = VB.Color[pv][3];
      for (i=0;i<n;i++) {
	 red[i]   = r;
	 green[i] = g;
	 blue[i]  = b;
	 alpha[i] = a;
      }
   }
   else {
      /* interpolate color, VB.Colors are in fixed point */
      gl_interpolate_rgba( n,
                           VB.Color[v1][0], VB.Color[v2][0], red,
                           VB.Color[v1][1], VB.Color[v2][1], green,
                           VB.Color[v1][2], VB.Color[v2][2], blue,
                           VB.Color[v1][3], VB.Color[v2][3], alpha );
   }

   /* interpolate texture coordinates */
   gl_interp_texcoords( n, GL_FALSE, VB.Clip[v1][3],VB.Clip[v2][3],
		        VB.TexCoord[v1][0], VB.TexCoord[v2][0],
                        VB.TexCoord[v1][1], VB.TexCoord[v2][1],
                        VB.TexCoord[v1][2], VB.TexCoord[v2][2],
                        VB.TexCoord[v1][3], VB.TexCoord[v2][3],
		        s, t, u, v, NULL );

   /* compute delta x and delta y */
   if (x1>x2) {
      dx = x1 - x2;
   }
   else {
      dx = x2 - x1;
   }
   if (y1>y2) {
      dy = y1 - y2;
   }
   else {
      dy = y2 - y1;
   }

   /* render */
   if (CC.Line.Width==2.0F) {
      /* special case, easy to optimize */
      if (dx>dy) {
	 /* X-major: duplicate pixels in Y direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       PB_WRITE_TEX_PIXEL( x[i], y[i]-1, z[i],
			  red[i], green[i], blue[i], alpha[i], s[i], t[i] );
	       PB_WRITE_TEX_PIXEL( x[i], y[i], z[i],
			  red[i], green[i], blue[i], alpha[i], s[i], t[i] );
	    }
	 }
      }
      else {
	 /* Y-major: duplicate pixels in X direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       PB_WRITE_TEX_PIXEL( x[i]-1, y[i], z[i],
			  red[i], green[i], blue[i], alpha[i], s[i], t[i] );
	       PB_WRITE_TEX_PIXEL( x[i], y[i], z[i],
			  red[i], green[i], blue[i], alpha[i], s[i], t[i] );
	    }
	 }
      }
      PB_CHECK_FLUSH
   }
   else {
      GLint width, w0, w1;
      width = (GLint) CLAMP( CC.Line.Width, MIN_LINE_WIDTH, MAX_LINE_WIDTH );
      w0 = -width / 2;
      w1 = w0 + width - 1;

      if (dx>dy) {
	 /* X-major: duplicate pixels in Y direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       GLint yy;
	       GLint y0 = y[i] + w0;
	       GLint y1 = y[i] + w1;
	       for (yy=y0;yy<=y1;yy++) {
		  PB_WRITE_TEX_PIXEL( x[i], yy, z[i],
			     red[i], green[i], blue[i], alpha[i], s[i], t[i] );
	       }
	       PB_CHECK_FLUSH
	    }
	 }
      }
      else {
	 /* Y-major: duplicate pixels in X direction */
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       GLint xx;
	       GLint x0 = x[i] + w0;
	       GLint x1 = x[i] + w1;
	       for (xx=x0;xx<=x1;xx++) {
		  PB_WRITE_TEX_PIXEL( xx, y[i], z[i],
			     red[i], green[i], blue[i], alpha[i], s[i], t[i] );
	       }
	       PB_CHECK_FLUSH
	    }
	 }
      }
   }

   /* Deallocate dynamic arrays on Mac */
   UNDEFARRAY(red);
   UNDEFARRAY(green);
   UNDEFARRAY(blue);
   UNDEFARRAY(alpha);
   UNDEFARRAY(s);
   UNDEFARRAY(t);
   UNDEFARRAY(u);
   UNDEFARRAY(v); 
}




/*
 * Determine which line drawing function to use given the current
 * rendering context.
 */
void gl_set_line_function( void )
{
   /* TODO: antialiased lines */

   if (CC.RenderMode==GL_RENDER) {
      CC.LineFunc = (*DD.get_line_func)();
      if (CC.LineFunc) {
         /* Device driver will draw lines. */
      }
      else if (CC.Texture.Enabled) {
	 CC.LineFunc = textured_rgba_line;
      }
      else if (CC.Line.Width!=1.0 || CC.Line.StippleFlag || CC.Line.SmoothFlag
	       || CC.Texture.Enabled) {
	 CC.LineFunc = CC.RGBAflag ? general_rgba_line : general_ci_line;
      }
      else {
	 if (CC.Light.ShadeModel==GL_SMOOTH) {
	    /* Width==1, non-stippled, smooth-shaded, any raster ops */
	    CC.LineFunc = CC.RGBAflag ? smooth_rgba_line : smooth_ci_line;
	 }
         else {
	    /* Width==1, non-stippled, flat-shaded, any raster ops */
	    CC.LineFunc = CC.RGBAflag ? flat_rgba_line : flat_ci_line;
         }
      }
   }
   else if (CC.RenderMode==GL_FEEDBACK) {
      CC.LineFunc = feedback_line;
   }
   else {
      /* GL_SELECT mode */
      CC.LineFunc = select_line;
   }
}


