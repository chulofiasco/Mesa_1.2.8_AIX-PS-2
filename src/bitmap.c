/* bitmap.c */

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
 * glBitmap()
 */


/*
$Id: bitmap.c,v 1.18 1996/02/26 15:06:42 brianp Exp $

$Log: bitmap.c,v $
 * Revision 1.18  1996/02/26  15:06:42  brianp
 * replaced CC.Current.Color with CC.Current.IntColor
 *
 * Revision 1.17  1996/01/22  15:26:03  brianp
 * replaced gl_init_pb() with PB_INIT macro
 *
 * Revision 1.16  1996/01/16  15:06:38  brianp
 * call gl_init_pb() if CC.NewState is set
 *
 * Revision 1.15  1995/12/30  00:46:10  brianp
 * scale raster color to ints before calling PB_SET_COLOR
 *
 * Revision 1.14  1995/12/18  17:14:21  brianp
 * replaced MAX_DEPTH with DEPTH_SCALE
 *
 * Revision 1.13  1995/09/07  19:46:56  brianp
 * use CC.NewState convention
 * truncate, don't round raster position to integer position
 *
 * Revision 1.12  1995/07/24  20:34:16  brianp
 * replaced memset() with MEMSET() and memcpy() with MEMCPY()
 *
 * Revision 1.11  1995/07/11  15:29:33  brianp
 * round rasterpos to nearest integer before drawing
 *
 * Revision 1.10  1995/06/20  16:19:10  brianp
 * removed PB clipflag stuff
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
 * Revision 1.6  1995/05/12  16:27:28  brianp
 * better bitmap clipping scheme
 *
 * Revision 1.5  1995/03/13  15:58:42  brianp
 * fixed bitmap bugs per Thorsten Ohl
 *
 * Revision 1.4  1995/03/07  14:19:41  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.3  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/27  22:48:16  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  14:16:45  brianp
 * Initial revision
 *
 */


#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "feedback.h"
#include "list.h"
#include "macros.h"
#include "pb.h"
#include "pixel.h"





void gl_bitmap( GLsizei width, GLsizei height,
	        GLfloat xorig, GLfloat yorig,
	        GLfloat xmove, GLfloat ymove,
	        const GLubyte *bitmap )
{
   /* Error checks */
   if (width<0 || height<0) {
      gl_error( GL_INVALID_VALUE, "glBitmap" );
      return;
   }
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glBitmap" );
      return;
   }
   if (CC.Current.RasterPosValid==GL_FALSE) {
      /* do nothing */
      return;
   }

   if (CC.NewState) {
      gl_update_state();
      PB_INIT( GL_BITMAP );
   }

   if (CC.RenderMode==GL_RENDER) {
      GLint bx, by;      /* bitmap position */
      GLint px, py, pz;  /* pixel position */
      GLubyte *ptr;

      if (CC.RGBAflag) {
         GLint r, g, b, a;
         r = (GLint) (CC.Current.RasterColor[0] * CC.RedScale);
         g = (GLint) (CC.Current.RasterColor[1] * CC.GreenScale);
         b = (GLint) (CC.Current.RasterColor[2] * CC.BlueScale);
         a = (GLint) (CC.Current.RasterColor[3] * CC.AlphaScale);
	 PB_SET_COLOR( r, g, b, a );
      }
      else {
	 PB_SET_INDEX( CC.Current.RasterIndex );
      }

      px = (GLint) ( (CC.Current.RasterPos[0] - xorig) + 0.0F );
      py = (GLint) ( (CC.Current.RasterPos[1] - yorig) + 0.0F );
      pz = (GLint) ( CC.Current.RasterPos[2] * DEPTH_SCALE );
      ptr = (GLubyte *) bitmap;

      for (by=0;by<height;by++) {
	 GLubyte bitmask;

	 /* do a row */
	 bitmask = 128;
	 for (bx=0;bx<width;bx++) {
	    if (*ptr&bitmask) {
	       PB_WRITE_PIXEL( px+bx, py+by, pz );
	    }
	    bitmask = bitmask >> 1;
	    if (bitmask==0) {
	       ptr++;
	       bitmask = 128;
	    }
	 }

	 PB_CHECK_FLUSH

	 /* get ready for next row */
	 if (width%8)  ptr++;
      }

      gl_flush_pb();

      /* update raster position */
      CC.Current.RasterPos[0] += xmove;
      CC.Current.RasterPos[1] += ymove;
   }
   else if (CC.RenderMode==GL_FEEDBACK) {
      GLfloat color[4];
      color[0] = CC.Current.IntColor[0] / CC.RedScale;
      color[1] = CC.Current.IntColor[1] / CC.GreenScale;
      color[2] = CC.Current.IntColor[2] / CC.BlueScale;
      color[3] = CC.Current.IntColor[3] / CC.AlphaScale;
      APPEND_TOKEN( (GLfloat) GL_BITMAP_TOKEN );
      /* TODO: Verify XYZW values are correct: */
      gl_feedback_vertex( CC.Current.RasterPos[0] - xorig,
			  CC.Current.RasterPos[1] - yorig,
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



void glBitmap( GLsizei width, GLsizei height,
	       GLfloat xorig, GLfloat yorig,
	       GLfloat xmove, GLfloat ymove,
	       const GLubyte *bitmap )
{
   GLubyte *data;  /* The unpacked bitmap data */

   /*
    * Data conversion:  we want:  unpack MSB first, unpack row length = width,
    * unpack skip pixels = 0, unpack skip rows = 0, unpack alignment = 1
    */
   if (CC.UnpackLSBFirst==GL_FALSE && CC.UnpackAlignment==1
       && CC.UnpackRowLength==0 && CC.UnpackSkipPixels==0
       && CC.UnpackSkipRows==0) {
      /* bitmap is already in desired form */
      data = (GLubyte *) bitmap;
   }
   else {
      /* Copy the data to a new buffer while applying all the glPixelStore
       * pixel unpacking options.
       */
      GLuint bytes, bytes_per_row, width_in_bytes, skip;
      GLuint i, j;
      GLubyte *src, *dst;

      width_in_bytes = (width+7)/8;
      bytes = width_in_bytes * height;

      /* allocate storage for unpacked bitmap data */
      data = (GLubyte *) malloc( bytes );
      if (!data) {
	 gl_error( GL_OUT_OF_MEMORY, "glBitmap" );
	 return;
      }

      if (CC.UnpackRowLength==0) {
	 bytes_per_row = width_in_bytes;
      }
      else {
	 bytes_per_row = (CC.UnpackRowLength+7)/8;
      }

      if (bytes_per_row % CC.UnpackAlignment) {
	 skip = CC.UnpackAlignment - bytes_per_row % CC.UnpackAlignment;
      }
      else {
	 skip = 0;
      }

      skip += bytes_per_row - width_in_bytes;

      src = (GLubyte *) bitmap + CC.UnpackSkipPixels/8
	                  + CC.UnpackSkipRows*bytes_per_row;
      dst = data;

      /* copy bytes from src to dst */
      for (i=0;i<height;i++) {
	 for (j=0;j<width_in_bytes;j++) {
	    *dst++ = *src++;
	 }
	 src += skip;
      }

      if (CC.UnpackLSBFirst) {
	 /* Flip the bits in each byte */
	 gl_flip_bytes( data, bytes );
      }
   }

   if (CC.CompileFlag) {
      if (data==bitmap) {
	 /* make a copy to save in the display list */
	 GLuint bytes = (width + 7) / 8 * height;
	 data = (GLubyte *) malloc( bytes );
	 if (data) {
	    MEMCPY( data, bitmap, bytes );
	 }
      }
      gl_save_bitmap( width, height, xorig, yorig, xmove, ymove, data );
   }
   if (CC.ExecuteFlag) {
      gl_bitmap( width, height, xorig, yorig, xmove, ymove, data );
      if (!CC.CompileFlag && data!=bitmap) {
	 /* Free the unpacked data */
	 free( data );
      }
   }
}


