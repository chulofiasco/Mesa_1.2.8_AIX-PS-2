/* points.c */

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
$Id: points.c,v 1.24 1995/12/30 17:18:40 brianp Exp $

$Log: points.c,v $
 * Revision 1.24  1995/12/30  17:18:40  brianp
 * divide texture S and T by Q, per Bill Triggs
 *
 * Revision 1.23  1995/12/30  00:58:12  brianp
 * use integer vertex colors instead of floating point
 *
 * Revision 1.22  1995/12/20  15:26:05  brianp
 * changed VB color indexes to GLuint
 *
 * Revision 1.21  1995/12/19  22:17:05  brianp
 * removed 0.5 offset from window coordinates thanks to CC.RasterOffsetX/Y
 *
 * Revision 1.20  1995/10/17  21:41:56  brianp
 * removed simple_ci/rgba_points() functions because of new device driver
 *
 * Revision 1.19  1995/09/28  19:39:10  brianp
 * replaced ClipFlag[] with Unclipped[]
 *
 * Revision 1.18  1995/09/20  18:20:39  brianp
 * prototype device driver changes described
 *
 * Revision 1.17  1995/09/13  14:50:38  brianp
 * render an array of points rather than single points
 *
 * Revision 1.16  1995/07/25  18:36:32  brianp
 * convert window coords from floats to ints by rounding, not truncating
 *
 * Revision 1.15  1995/07/15  14:03:58  brianp
 * added texture mapped points
 *
 * Revision 1.14  1995/06/20  16:20:50  brianp
 * do float-to-int depth scaling here instead of in draw.c
 *
 * Revision 1.13  1995/06/05  20:27:26  brianp
 * better clipping of points with size > 1
 *
 * Revision 1.12  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.11  1995/05/12  16:57:22  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.10  1995/04/18  15:48:23  brianp
 * fixed assignment of NULL to function pointers to prevent warnings on Suns
 *
 * Revision 1.9  1995/04/12  15:36:15  brianp
 * updated to use DD.draw_* function pointers
 *
 * Revision 1.8  1995/03/24  15:33:32  brianp
 * introduced VB
 *
 * Revision 1.7  1995/03/07  14:20:55  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.6  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.5  1995/03/04  19:16:47  brianp
 * added size clamp
 *
 * Revision 1.4  1995/03/02  19:18:34  brianp
 * new RasterMask logic
 *
 * Revision 1.3  1995/02/27  22:48:59  brianp
 * modified for PB
 *
 * Revision 1.2  1995/02/27  15:08:12  brianp
 * added Vcolor/Vindex scheme
 *
 * Revision 1.1  1995/02/24  14:26:49  brianp
 * Initial revision
 *
 */


#include "context.h"
#include "dd.h"
#include "feedback.h"
#include "list.h"
#include "macros.h"
#include "pb.h"
#include "span.h"
#include "vb.h"




void glPointSize( GLfloat size )
{
   if (CC.CompileFlag) {
      gl_save_pointsize( size );
   }
   if (CC.ExecuteFlag) {
      if (size<=0.0) {
	 gl_error( GL_INVALID_VALUE, "glPointSize" );
	 return;
      }
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glPointSize" );
	 return;
      }

      CC.Point.Size = size;
      CC.NewState = GL_TRUE;
   }
}



/**********************************************************************/
/*****                    Rasterization                           *****/
/**********************************************************************/


/*
 * There are 3 pairs (RGBA, CI) of point rendering functions:
 *   1. simple:  size=1 and no special rasterization functions (fastest)
 *   2. size1:  size=1 and any rasterization functions
 *   3. general:  any size and rasterization functions (slowest)
 *
 * All point rendering functions take the same two arguments: first and
 * last which specify that the points specified by VB[first] through
 * VB[last] are to be rendered.
 */



/*
 * Put points in feedback buffer.
 */
static void feedback_points( GLuint first, GLuint last )
{
   GLuint i;
   GLint shift = CC.ColorShift;

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLfloat x, y, z, w;
         GLfloat color[4];

         x = VB.Win[i][0] - CC.RasterOffsetX;
         y = VB.Win[i][1] - CC.RasterOffsetY;
         z = VB.Win[i][2];
         w = VB.Clip[i][3];

         /* convert color from integer back to a float in [0,1] */
         color[0] = (GLfloat) (VB.Color[i][0] >> shift) / CC.RedScale;
         color[1] = (GLfloat) (VB.Color[i][1] >> shift) / CC.GreenScale;
         color[2] = (GLfloat) (VB.Color[i][2] >> shift) / CC.BlueScale;
         color[3] = (GLfloat) (VB.Color[i][3] >> shift) / CC.AlphaScale;

         APPEND_TOKEN( (GLfloat) GL_POINT_TOKEN );
         gl_feedback_vertex( x, y, z, w, color,
                             (GLfloat) VB.Index[i], VB.TexCoord[i] );
      }
   }
}



/*
 * Put points in selection buffer.
 */
static void select_points( GLuint first, GLuint last )
{
   GLuint i;

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLfloat z = VB.Win[i][2];

         CC.HitFlag = GL_TRUE;
         if (z < CC.HitMinZ) {
            CC.HitMinZ = z;
         }
         if (z < CC.HitMinZ) {
            CC.HitMaxZ = z;
         }
      }
   }
}


/*
 * CI points with size == 1.0
 */
static void size1_ci_points( GLuint first, GLuint last )
{
   GLuint i;

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);
         PB_WRITE_CI_PIXEL( x, y, z, VB.Index[i] );
      }
   }
   PB_CHECK_FLUSH
}


/*
 * RGBA points with size == 1.0
 */
static void size1_rgba_points( GLuint first, GLuint last )
{
   GLuint i;
   GLint shift = CC.ColorShift;

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint red, green, blue, alpha;

         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         red   = VB.Color[i][0] >> shift;
         green = VB.Color[i][1] >> shift;
         blue  = VB.Color[i][2] >> shift;
         alpha = VB.Color[i][3] >> shift;

         PB_WRITE_RGBA_PIXEL( x, y, z, red, green, blue, alpha );
      }
   }
   PB_CHECK_FLUSH
}



/*
 * General CI points.
 */
static void general_ci_points( GLuint first, GLuint last )
{
   GLuint i;
   GLint isize;

   isize = (GLint) (CLAMP(CC.Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }

         PB_SET_INDEX( VB.Index[i] );

         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_PIXEL( ix, iy, z );
            }
         }
         PB_CHECK_FLUSH
      }
   }
}


/*
 * General RGBA points.
 */
static void general_rgba_points( GLuint first, GLuint last )
{
   GLuint i;
   GLint isize;
   GLint shift = CC.ColorShift;

   isize = (GLint) (CLAMP(CC.Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }

         PB_SET_COLOR( VB.Color[i][0] >> shift,
                       VB.Color[i][1] >> shift,
                       VB.Color[i][2] >> shift,
                       VB.Color[i][3] >> shift );

         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_PIXEL( ix, iy, z );
            }
         }
         PB_CHECK_FLUSH
      }
   }
}




/*
 * Textured RGBA points.
 */
static void textured_rgba_points( GLuint first, GLuint last )
{
   GLuint i;
   GLint shift = CC.ColorShift;

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;
         GLint isize;
         GLint red, green, blue, alpha;
         GLfloat s, t;

         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         isize = (GLint)
                   (CLAMP(CC.Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);
         if (isize<1) {
            isize = 1;
         }

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }

         red   = VB.Color[i][0] >> shift;
         green = VB.Color[i][1] >> shift;
         blue  = VB.Color[i][2] >> shift;
         alpha = VB.Color[i][3] >> shift;
         s = VB.TexCoord[i][0] / VB.TexCoord[i][3];
         t = VB.TexCoord[i][1] / VB.TexCoord[i][3];

/*    don't think this is needed
         PB_SET_COLOR( red, green, blue, alpha );
*/

         for (iy=y0;iy<=y1;iy++) {
            for (ix=x0;ix<=x1;ix++) {
               PB_WRITE_TEX_PIXEL( ix, iy, z, red, green, blue, alpha, s, t );
            }
         }
         PB_CHECK_FLUSH
      }
   }
}




/*
 * Examine the current context to determine which point drawing function
 * should be used.
 */
void gl_set_point_function( void )
{
   /* TODO: antialiased points */

   if (CC.RenderMode==GL_RENDER) {
      CC.PointsFunc = (*DD.get_points_func)();
      if (CC.PointsFunc) {
         /* Device driver will draw points. */
      }
      else if (CC.Texture.Enabled) {
	 CC.PointsFunc = textured_rgba_points;
      }
      else if (CC.Point.Size==1.0) {
         /* size=1, any raster ops */
         CC.PointsFunc = CC.RGBAflag ? size1_rgba_points : size1_ci_points;
      }
      else {
	 /* every other kind of point rendering */
	 CC.PointsFunc = CC.RGBAflag ? general_rgba_points : general_ci_points;
      }
   }
   else if (CC.RenderMode==GL_FEEDBACK) {
      CC.PointsFunc = feedback_points;
   }
   else {
      /* GL_SELECT mode */
      CC.PointsFunc = select_points;
   }

}


