/* $Id: copypix.c,v 1.23 1996/05/01 15:48:03 brianp Exp $ */

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
$Log: copypix.c,v $
 * Revision 1.23  1996/05/01  15:48:03  brianp
 * replaced gl_read_depth_span() with (*DD.read_depth_span_float)()
 *
 * Revision 1.22  1996/02/26  15:06:42  brianp
 * replaced CC.Current.Color with CC.Current.IntColor
 *
 * Revision 1.21  1996/02/09  22:22:14  brianp
 * removed #include "gamma.h"
 *
 * Revision 1.20  1996/02/06  03:23:54  brianp
 * removed gamma correction code
 *
 * Revision 1.19  1995/12/30  00:48:53  brianp
 * use CC.Current.IntColor
 *
 * Revision 1.18  1995/12/18  17:15:27  brianp
 * replaced MAX_DEPTH with DEPTH_SCALE, use new GLdepth type
 *
 * Revision 1.17  1995/11/03  17:40:29  brianp
 * fixed a varname typo, removed unused variables
 *
 * Revision 1.16  1995/10/19  15:47:12  brianp
 * added gamma support
 *
 * Revision 1.15  1995/10/16  15:25:49  brianp
 * added zooming for stencil drawing/copying
 *
 * Revision 1.14  1995/10/14  16:28:21  brianp
 * added glPixelZoom support
 *
 * Revision 1.13  1995/09/15  18:43:25  brianp
 * broke gl_copypixels into sub-functions
 * directly access DD.write_color_span under the right conditions
 *
 * Revision 1.12  1995/09/07  14:15:52  brianp
 * use CC.NewState convention
 * use DEFARRAY macro for MacIntosh support
 *
 * Revision 1.11  1995/08/04  13:07:28  brianp
 * changed some types involved in scale and bias operation
 *
 * Revision 1.10  1995/07/24  19:00:17  brianp
 * convert real window coords to ints with rounding, not truncating
 * changed calls to CLAMP() to prevent type warnings on Suns
 *
 * Revision 1.9  1995/06/12  15:52:07  brianp
 * renamed from copypixels.c to copypix.c
 *
 * Revision 1.8  1995/06/12  15:38:01  brianp
 * changed color arrays to GLubyte
 * added feedback and selection support
 *
 * Revision 1.7  1995/05/31  14:58:45  brianp
 * check for valid rasterpos, use gl_read_color|index_span()
 *
 * Revision 1.6  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.5  1995/05/12  19:24:13  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.4  1995/04/18  15:47:59  brianp
 * added a cast to prevent warning on Suns
 *
 * Revision 1.3  1995/03/13  20:54:53  brianp
 * added read buffer logic
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:20:20  brianp
 * Initial revision
 *
 */


#include <string.h>
#include "context.h"
#include "dd.h"
#include "depth.h"
#include "feedback.h"
#include "list.h"
#include "macros.h"
#include "pixel.h"
#include "span.h"
#include "stencil.h"


#ifndef NULL
#  define NULL 0
#endif




static void copy_rgb_pixels( GLint srcx, GLint srcy, GLint width, GLint height,
                             GLint destx, GLint desty )
{
   DEFARRAY( GLdepth, zspan, MAX_WIDTH );
   DEFARRAY( GLubyte, red, MAX_WIDTH );
   DEFARRAY( GLubyte, green, MAX_WIDTH );
   DEFARRAY( GLubyte, blue, MAX_WIDTH );
   DEFARRAY( GLubyte, alpha, MAX_WIDTH );
   GLboolean scale_or_bias, quick_draw, zoom;
   GLint sy, dy, stepy;
   GLint i, j;
   GLboolean setbuffer;

   if (CC.Pixel.ZoomX==1.0F && CC.Pixel.ZoomY==1.0F) {
      zoom = GL_FALSE;
   }
   else {
      zoom = GL_TRUE;
   }

   /* Determine if copy should be done bottom-to-top or top-to-bottom */
   if (srcy<desty) {
      /* top-down  max-to-min */
      sy = srcy + height - 1;
      dy = desty + height - 1;
      stepy = -1;
   }
   else {
      /* bottom-up  min-to-max */
      sy = srcy;
      dy = desty;
      stepy = 1;
   }

   scale_or_bias = CC.Pixel.RedScale!=1.0 || CC.Pixel.RedBias!=0.0
                || CC.Pixel.GreenScale!=1.0 || CC.Pixel.GreenBias!=0.0
		|| CC.Pixel.BlueScale!=1.0 || CC.Pixel.BlueBias!=0.0
		|| CC.Pixel.AlphaScale!=1.0 || CC.Pixel.AlphaBias!=0.0;

   if (CC.Depth.Test) {
      /* fill in array of z values */
      GLint z = (GLint) (CC.Current.RasterPos[2] * DEPTH_SCALE);
      for (i=0;i<width;i++) {
         zspan[i] = z;
      }
   }

   if (CC.RasterMask==0 && destx>=0 && destx+width<=CC.BufferWidth && !zoom) {
      quick_draw = GL_TRUE;
   }
   else {
      quick_draw = GL_FALSE;
   }

   /* If read and draw buffer are different we must do buffer switching */
   setbuffer = CC.Pixel.ReadBuffer!=CC.Color.DrawBuffer;

   for (j=0; j<height; j++, sy+=stepy, dy+=stepy) {
      /* read */

      if (setbuffer) {
         (*DD.set_buffer)( CC.Pixel.ReadBuffer );
      }
      gl_read_color_span( width, srcx, sy, red, green, blue, alpha );

      if (scale_or_bias) {
         GLfloat rbias = CC.Pixel.RedBias   * CC.RedScale;
         GLfloat gbias = CC.Pixel.GreenBias * CC.GreenScale;
         GLfloat bbias = CC.Pixel.BlueBias  * CC.BlueScale;
         GLfloat abias = CC.Pixel.AlphaBias * CC.AlphaScale;
         GLint rmax = (GLint) CC.RedScale;
         GLint gmax = (GLint) CC.GreenScale;
         GLint bmax = (GLint) CC.BlueScale;
         GLint amax = (GLint) CC.AlphaScale;
         for (i=0;i<width;i++) {
            GLint r = red[i]   * CC.Pixel.RedScale   + rbias;
            GLint g = green[i] * CC.Pixel.GreenScale + gbias;
            GLint b = blue[i]  * CC.Pixel.BlueScale  + bbias;
            GLint a = alpha[i] * CC.Pixel.AlphaScale + abias;
            red[i]   = CLAMP( r, 0, rmax );
            green[i] = CLAMP( g, 0, gmax );
            blue[i]  = CLAMP( b, 0, bmax );
            alpha[i] = CLAMP( a, 0, amax );
         }
      }

      if (CC.Pixel.MapColorFlag) {
         GLfloat r = (CC.Pixel.MapRtoRsize-1) / CC.RedScale;
         GLfloat g = (CC.Pixel.MapGtoGsize-1) / CC.GreenScale;
         GLfloat b = (CC.Pixel.MapBtoBsize-1) / CC.BlueScale;
         GLfloat a = (CC.Pixel.MapAtoAsize-1) / CC.AlphaScale;
         for (i=0;i<width;i++) {
            GLint ir = red[i] * r;
            GLint ig = green[i] * g;
            GLint ib = blue[i] * b;
            GLint ia = alpha[i] * a;
            red[i]   = (GLint) (CC.Pixel.MapRtoR[ir] * CC.RedScale);
            green[i] = (GLint) (CC.Pixel.MapGtoG[ig] * CC.GreenScale);
            blue[i]  = (GLint) (CC.Pixel.MapBtoB[ib] * CC.BlueScale);
            alpha[i] = (GLint) (CC.Pixel.MapAtoA[ia] * CC.AlphaScale);
         }
      }

      /* write */
      if (setbuffer) {
         (*DD.set_buffer)( CC.Color.DrawBuffer );
      }
      if (quick_draw && dy>=0 && dy<CC.BufferHeight) {
         (*DD.write_color_span)( width, destx, dy,
                                 red, green, blue, alpha, NULL);
      }
      else if (zoom) {
         gl_write_zoomed_color_span( width, destx, dy, zspan,
                                     red, green, blue, alpha, desty );
      }
      else {
         gl_write_color_span( width, destx, dy, zspan,
                              red, green, blue, alpha, GL_BITMAP );
      }
   }
   UNDEFARRAY( zspan );
   UNDEFARRAY( red );
   UNDEFARRAY( green );
   UNDEFARRAY( blue );
   UNDEFARRAY( alpha );
}



static void copy_ci_pixels( GLint srcx, GLint srcy, GLint width, GLint height,
                             GLint destx, GLint desty )
{
   GLdepth zspan[MAX_WIDTH];
   GLuint indx[MAX_WIDTH];
   GLint sy, dy, stepy;
   GLint i, j;
   GLboolean setbuffer, zoom;

   if (CC.Pixel.ZoomX==1.0F && CC.Pixel.ZoomY==1.0F) {
      zoom = GL_FALSE;
   }
   else {
      zoom = GL_TRUE;
   }

   /* Determine if copy should be bottom-to-top or top-to-bottom */
   if (srcy<desty) {
      /* top-down  max-to-min */
      sy = srcy + height - 1;
      dy = desty + height - 1;
      stepy = -1;
   }
   else {
      /* bottom-up  min-to-max */
      sy = srcy;
      dy = desty;
      stepy = 1;
   }

   if (CC.Depth.Test) {
      /* fill in array of z values */
      GLint z = (GLint) (CC.Current.RasterPos[2] * DEPTH_SCALE);
      for (i=0;i<width;i++) {
         zspan[i] = z;
      }
   }

   /* If read and draw buffer are different we must do buffer switching */
   setbuffer = CC.Pixel.ReadBuffer!=CC.Color.DrawBuffer;

   for (j=0; j<height; j++, sy+=stepy, dy+=stepy) {
      /* read */
      if (setbuffer) {
         (*DD.set_buffer)( CC.Pixel.ReadBuffer );
      }
      gl_read_index_span( width, srcx, sy, indx );

      /* shift, offset */
      if (CC.Pixel.IndexShift || CC.Pixel.IndexOffset) {
         if (CC.Pixel.IndexShift<0) {
            for (i=0;i<width;i++) {
               indx[i] = (indx[i] >> -CC.Pixel.IndexShift)
                         + CC.Pixel.IndexOffset;
            }
         }
         else {
            for (i=0;i<width;i++) {
               indx[i] = (indx[i] << CC.Pixel.IndexShift)
                         + CC.Pixel.IndexOffset;
            }
         }
      }

      /* mapping */
      if (CC.Pixel.MapColorFlag) {
         for (i=0;i<width;i++) {
            if (indx[i] < CC.Pixel.MapItoIsize) {
               indx[i] = CC.Pixel.MapItoI[ indx[i] ];
            }
         }
      }

      /* write */
      if (setbuffer) {
         (*DD.set_buffer)( CC.Color.DrawBuffer );
      }
      if (zoom) {
         gl_write_zoomed_index_span( width, destx, dy, zspan, indx, desty );
      }
      else {
         gl_write_index_span( width, destx, dy, zspan, indx, GL_BITMAP );
      }
   }
}



/*
 * TODO: Optimize!!!!
 */
static void copy_depth_pixels( GLint srcx, GLint srcy,
                               GLint width, GLint height,
                               GLint destx, GLint desty )
{
   GLfloat depth[MAX_WIDTH];
   GLdepth zspan[MAX_WIDTH];
   GLuint indx[MAX_WIDTH];
   GLubyte red[MAX_WIDTH], green[MAX_WIDTH];
   GLubyte blue[MAX_WIDTH], alpha[MAX_WIDTH];
   GLint sy, dy, stepy;
   GLint i, j;
   GLboolean zoom;

   if (!CC.DepthBuffer) {
      gl_error( GL_INVALID_OPERATION, "glCopyPixels" );
      return;
   }

   if (CC.Pixel.ZoomX==1.0F && CC.Pixel.ZoomY==1.0F) {
      zoom = GL_FALSE;
   }
   else {
      zoom = GL_TRUE;
   }

   /* Determine if copy should be bottom-to-top or top-to-bottom */
   if (srcy<desty) {
      /* top-down  max-to-min */
      sy = srcy + height - 1;
      dy = desty + height - 1;
      stepy = -1;
   }
   else {
      /* bottom-up  min-to-max */
      sy = srcy;
      dy = desty;
      stepy = 1;
   }

   /* setup colors or indexes */
   if (CC.RGBAflag) {
      GLubyte r, g, b, a;
      r = CC.Current.IntColor[0];
      g = CC.Current.IntColor[1];
      b = CC.Current.IntColor[2];
      a = CC.Current.IntColor[3];
      MEMSET( red,   (int) r, width );
      MEMSET( green, (int) g, width );
      MEMSET( blue,  (int) b, width );
      MEMSET( alpha, (int) a, width );
   }
   else {
      for (i=0;i<width;i++) {
         indx[i] = CC.Current.Index;
      }
   }

   for (j=0; j<height; j++, sy+=stepy, dy+=stepy) {
      /* read */
      (*DD.read_depth_span_float)( width, srcx, sy, depth );
      /* scale, bias, clamp */
      for (i=0;i<width;i++) {
         GLfloat d = depth[i] * CC.Pixel.DepthScale + CC.Pixel.DepthBias;
         zspan[i] = (GLint) (CLAMP( d, 0.0, 1.0 ) * DEPTH_SCALE);
      }
      /* write */
      if (CC.RGBAflag) {
         if (zoom) {
            gl_write_zoomed_color_span( width, destx, dy, zspan,
                                        red, green, blue, alpha, desty );
         }
         else {
            gl_write_color_span( width, destx, dy, zspan,
                                 red, green, blue, alpha, GL_BITMAP );
         }
      }
      else {
         if (zoom) {
            gl_write_zoomed_index_span( width, destx, dy, zspan, indx, desty);
         }
         else {
            gl_write_index_span( width, destx, dy, zspan, indx, GL_BITMAP );
         }
      }
   }
}



static void copy_stencil_pixels( GLint srcx, GLint srcy,
                                 GLint width, GLint height,
                                 GLint destx, GLint desty )
{
   GLubyte stencil[MAX_WIDTH];
   GLint sy, dy, stepy;
   GLint i, j;
   GLboolean zoom;

   if (!CC.StencilBuffer) {
      gl_error( GL_INVALID_OPERATION, "glCopyPixels" );
      return;
   }

   if (CC.Pixel.ZoomX==1.0F && CC.Pixel.ZoomY==1.0F) {
      zoom = GL_FALSE;
   }
   else {
      zoom = GL_TRUE;
   }

   /* Determine if copy should be bottom-to-top or top-to-bottom */
   if (srcy<desty) {
      /* top-down  max-to-min */
      sy = srcy + height - 1;
      dy = desty + height - 1;
      stepy = -1;
   }
   else {
      /* bottom-up  min-to-max */
      sy = srcy;
      dy = desty;
      stepy = 1;
   }

   for (j=0; j<height; j++, sy+=stepy, dy+=stepy) {
      /* read */
      gl_read_stencil_span( width, srcx, sy, stencil );
      /* shift, offset */
      if (CC.Pixel.IndexShift<0) {
         for (i=0;i<width;i++) {
            stencil[i] = (stencil[i] >> -CC.Pixel.IndexShift)
                         + CC.Pixel.IndexOffset;
         }
      }
      else {
         for (i=0;i<width;i++) {
            stencil[i] = (stencil[i] << CC.Pixel.IndexShift)
			 + CC.Pixel.IndexOffset;
         }
      }
      /* mapping */
      if (CC.Pixel.MapStencilFlag) {
         for (i=0;i<width;i++) {
            if ((GLint) stencil[i] < CC.Pixel.MapStoSsize) {
               stencil[i] = CC.Pixel.MapStoS[ stencil[i] ];
            }
         }
      }
      /* write */
      if (zoom) {
         gl_write_zoomed_stencil_span( width, destx, dy, stencil, desty );
      }
      else {
         gl_write_stencil_span( width, destx, dy, stencil );
      }
   }
}




void gl_copypixels( GLint srcx, GLint srcy, GLsizei width, GLsizei height,
		    GLenum type )
{
   GLint destx, desty;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glCopyPixels" );
      return;
   }

   if (width<0 || height<0) {
      gl_error( GL_INVALID_VALUE, "glCopyPixels" );
      return;
   }

   if (CC.NewState) {
      gl_update_state();
   }

   if (CC.RenderMode==GL_RENDER) {
      /* Destination of copy: */
      if (!CC.Current.RasterPosValid) {
	 return;
      }
      destx = (GLint) (CC.Current.RasterPos[0] + 0.5F);
      desty = (GLint) (CC.Current.RasterPos[1] + 0.5F);

      if (type==GL_COLOR && CC.RGBAflag) {
         copy_rgb_pixels( srcx, srcy, width, height, destx, desty );
      }
      else if (type==GL_COLOR && !CC.RGBAflag) {
         copy_ci_pixels( srcx, srcy, width, height, destx, desty );
      }
      else if (type==GL_DEPTH) {
         copy_depth_pixels( srcx, srcy, width, height, destx, desty );
      }
      else if (type==GL_STENCIL) {
         copy_stencil_pixels( srcx, srcy, width, height, destx, desty );
      }
      else {
	 gl_error( GL_INVALID_ENUM, "glCopyPixels" );
      }
   }
   else if (CC.RenderMode==GL_FEEDBACK) {
      GLfloat color[4];
      color[0] = CC.Current.IntColor[0] / CC.RedScale;
      color[1] = CC.Current.IntColor[1] / CC.GreenScale;
      color[2] = CC.Current.IntColor[2] / CC.BlueScale;
      color[3] = CC.Current.IntColor[3] / CC.AlphaScale;
      APPEND_TOKEN( (GLfloat) GL_COPY_PIXEL_TOKEN );
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



void glCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height,
		   GLenum type )
{
   if (CC.CompileFlag) {
      gl_save_copypixels( x, y, width, height, type );
   }
   if (CC.ExecuteFlag) {
      gl_copypixels( x, y, width, height, type );
   }
}


