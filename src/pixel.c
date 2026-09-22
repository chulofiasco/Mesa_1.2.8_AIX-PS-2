/* $Id: pixel.c,v 1.18 1996/04/15 14:12:48 brianp Exp $ */

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
$Log: pixel.c,v $
 * Revision 1.18  1996/04/15  14:12:48  brianp
 * inserted missing CC.Pixel.Map?to?size assignments
 *
 * Revision 1.17  1996/02/14  15:40:15  brianp
 * replaced ABS with ABSF
 *
 * Revision 1.16  1996/01/22  15:33:10  brianp
 * check if zoomed span length is <=0, per Frederic Devernay
 * check for CC.EightBitColor in update_drawpixels_state()
 *
 * Revision 1.15  1996/01/19  19:16:33  brianp
 * fixed pixel zoom problems, per Frederic Devernay
 *
 * Revision 1.14  1995/12/19  16:42:44  brianp
 * added gl_pixel_transfer()
 *
 * Revision 1.13  1995/12/18  17:26:35  brianp
 * use new GLdepth datatype
 *
 * Revision 1.12  1995/11/30  00:19:31  brianp
 * include stdio.h
 *
 * Revision 1.11  1995/11/17  14:30:41  brianp
 * in gl_write_zoomed_*() limit spans to MAX_WIDTH, removed mallocs/frees
 *
 * Revision 1.10  1995/10/16  15:26:34  brianp
 * added gl_write_zoomed_stencil_span
 *
 * Revision 1.9  1995/10/14  16:28:56  brianp
 * added glPixelZoom support
 *
 * Revision 1.8  1995/09/15  18:39:07  brianp
 * added update_drawpixels_state()
 *
 * Revision 1.7  1995/08/01  20:53:50  brianp
 * added gl_save_pixelzoom()
 *
 * Revision 1.6  1995/07/24  20:35:20  brianp
 * replaced memset() with MEMSET() and memcpy() with MEMCPY()
 *
 * Revision 1.5  1995/05/29  21:22:42  brianp
 * added glGetPixelMap*() functions
 *
 * Revision 1.4  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.3  1995/05/12  16:57:22  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:25:08  brianp
 * Initial revision
 *
 */


/*
 * glPixelStore, glPixelTransfer, glPixelMap, glPixelZoom, etc.
 */



#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "list.h"
#include "macros.h"
#include "pixel.h"
#include "span.h"
#include "stencil.h"




/*
 * Determine if we can use the optimized glDrawPixels function.
 */
static void update_drawpixels_state( void )
{
   if (CC.RGBAflag==GL_TRUE &&
       CC.EightBitColor &&
       CC.Pixel.RedBias==0.0   && CC.Pixel.RedScale==1.0 &&
       CC.Pixel.GreenBias==0.0 && CC.Pixel.GreenScale==1.0 &&
       CC.Pixel.BlueBias==0.0  && CC.Pixel.BlueScale==1.0 &&
       CC.Pixel.AlphaBias==0.0 && CC.Pixel.AlphaScale==1.0 &&
       CC.Pixel.MapColorFlag==GL_FALSE &&
       CC.Pixel.ZoomX==1.0 && CC.Pixel.ZoomY==1.0 &&
/*       CC.UnpackAlignment==4 &&*/
       CC.UnpackRowLength==0 &&
       CC.UnpackSkipPixels==0 &&
       CC.UnpackSkipRows==0 &&
       CC.UnpackSwapBytes==0 &&
       CC.UnpackLSBFirst==0) {
      CC.FastDrawPixels = GL_TRUE;
   }
   else {
      CC.FastDrawPixels = GL_FALSE;
   }
}




/**********************************************************************/
/*****                    glPixelZoom                             *****/
/**********************************************************************/


/*
 * Write a span of pixels to the frame buffer while applying a pixel zoom.
 * This is only used by glDrawPixels and glCopyPixels.
 * Input:  n - number of pixels in input row
 *         x, y - destination of the span
 *         z - depth values for the span
 *         red, green, blue, alpha - array of colors
 *         y0 - location of first row in the image we're drawing.
 */
void
gl_write_zoomed_color_span( GLuint n, GLint x, GLint y, const GLdepth z[],
                            const GLubyte red[], const GLubyte green[],
                            const GLubyte blue[], const GLubyte alpha[],
                            GLint y0 )
{
   GLint m;
   GLint r0, r1, row, r;
   GLint i, j, skipcol;
   GLubyte zred[MAX_WIDTH], zgreen[MAX_WIDTH];  /* zoomed pixel colors */
   GLubyte zblue[MAX_WIDTH], zalpha[MAX_WIDTH];
   GLdepth zdepth[MAX_WIDTH];  /* zoomed depth values */
   GLint maxwidth = MIN2( CC.BufferWidth, MAX_WIDTH );

   /* compute width of output row */
   m = (GLint) ABSF( n * CC.Pixel.ZoomX );
   if (m==0) {
      return;
   }
   if (CC.Pixel.ZoomX<0.0) {
      /* adjust x coordinate for left/right mirroring */
      x = x - m;
   }

   /* compute which rows to draw */
   row = y-y0;
   r0 = y0 + (GLint) (row * CC.Pixel.ZoomY);
   r1 = y0 + (GLint) ((row+1) * CC.Pixel.ZoomY);
   if (r0==r1) {
      return;
   }
   else if (r1<r0) {
      GLint rtmp = r1;
      r1 = r0;
      r0 = rtmp;
   }

   /* return early if r0...r1 is above or below window */
   if (r0<0 && r1<0) {
      /* below window */
      return;
   }
   if (r0>=CC.BufferHeight && r1>=CC.BufferHeight) {
      /* above window */
      return;
   }

   /* check if left edge is outside window */
   skipcol = 0;
   if (x<0) {
      skipcol = -x;
      m += x;
   }
   /* make sure span isn't too long or short */
   if (m>maxwidth) {
      m = maxwidth;
   }
   else if (m<=0) {
      return;
   }

   assert( m <= MAX_WIDTH );

   /* zoom the span horizontally */
   if (CC.Pixel.ZoomX==-1.0F) {
      /* n==m */
      for (j=0;j<m;j++) {
         i = n - (j+skipcol) - 1;
         zred[j]   = red[i];
         zgreen[j] = green[i];
         zblue[j]  = blue[i];
         zalpha[j] = alpha[i];
         zdepth[j] = z[i];
      }
   }
   else {
      GLfloat xscale = 1.0F / CC.Pixel.ZoomX;
      for (j=0;j<m;j++) {
         i = (j+skipcol) * xscale;
         if (i<0)  i = n + i - 1;
         zred[j]   = red[i];
         zgreen[j] = green[i];
         zblue[j]  = blue[i];
         zalpha[j] = alpha[i];
         zdepth[j] = z[i];
      }
   }

   /* write the span */
   for (r=r0; r<r1; r++) {
      gl_write_color_span( m, x+skipcol, r, zdepth,
                           zred, zgreen, zblue, zalpha, GL_BITMAP );
   }
}



/*
 * As above, but write CI pixels.
 */
void
gl_write_zoomed_index_span( GLuint n, GLint x, GLint y, const GLdepth z[],
                            const GLuint indexes[], GLint y0 )
{
   GLint m;
   GLint r0, r1, row, r;
   GLint i, j, skipcol;
   GLuint zindexes[MAX_WIDTH];  /* zoomed color indexes */
   GLdepth zdepth[MAX_WIDTH];  /* zoomed depth values */
   GLint maxwidth = MIN2( CC.BufferWidth, MAX_WIDTH );

   /* compute width of output row */
   m = (GLint) ABSF( n * CC.Pixel.ZoomX );
   if (m==0) {
      return;
   }
   if (CC.Pixel.ZoomX<0.0) {
      /* adjust x coordinate for left/right mirroring */
      x = x - m;
   }

   /* compute which rows to draw */
   row = y-y0;
   r0 = y0 + (GLint) (row * CC.Pixel.ZoomY);
   r1 = y0 + (GLint) ((row+1) * CC.Pixel.ZoomY);
   if (r0==r1) {
      return;
   }
   else if (r1<r0) {
      GLint rtmp = r1;
      r1 = r0;
      r0 = rtmp;
   }

   /* return early if r0...r1 is above or below window */
   if (r0<0 && r1<0) {
      /* below window */
      return;
   }
   if (r0>=CC.BufferHeight && r1>=CC.BufferHeight) {
      /* above window */
      return;
   }

   /* check if left edge is outside window */
   skipcol = 0;
   if (x<0) {
      skipcol = -x;
      m += x;
   }
   /* make sure span isn't too long or short */
   if (m>maxwidth) {
      m = maxwidth;
   }
   else if (m<=0) {
      return;
   }

   assert( m <= MAX_WIDTH );

   /* zoom the span horizontally */
   if (CC.Pixel.ZoomX==-1.0F) {
      /* n==m */
      for (j=0;j<m;j++) {
         i = n - (j+skipcol) - 1;
         zindexes[j] = indexes[i];
         zdepth[j]   = z[i];
      }
   }
   else {
      GLfloat xscale = 1.0F / CC.Pixel.ZoomX;
      for (j=0;j<m;j++) {
         i = (j+skipcol) * xscale;
         if (i<0)  i = n + i - 1;
         zindexes[j] = indexes[i];
         zdepth[j] = z[i];
      }
   }

   /* write the span */
   for (r=r0; r<r1; r++) {
      gl_write_index_span( m, x+skipcol, r, zdepth, zindexes, GL_BITMAP );
   }
}



/*
 * As above, but write stencil values.
 */
void
gl_write_zoomed_stencil_span( GLuint n, GLint x, GLint y,
                              const GLubyte stencil[], GLint y0 )
{
   GLint m;
   GLint r0, r1, row, r;
   GLint i, j, skipcol;
   GLubyte zstencil[MAX_WIDTH];  /* zoomed stencil values */
   GLint maxwidth = MIN2( CC.BufferWidth, MAX_WIDTH );

   /* compute width of output row */
   m = (GLint) ABSF( n * CC.Pixel.ZoomX );
   if (m==0) {
      return;
   }
   if (CC.Pixel.ZoomX<0.0) {
      /* adjust x coordinate for left/right mirroring */
      x = x - m;
   }

   /* compute which rows to draw */
   row = y-y0;
   r0 = y0 + (GLint) (row * CC.Pixel.ZoomY);
   r1 = y0 + (GLint) ((row+1) * CC.Pixel.ZoomY);
   if (r0==r1) {
      return;
   }
   else if (r1<r0) {
      GLint rtmp = r1;
      r1 = r0;
      r0 = rtmp;
   }

   /* return early if r0...r1 is above or below window */
   if (r0<0 && r1<0) {
      /* below window */
      return;
   }
   if (r0>=CC.BufferHeight && r1>=CC.BufferHeight) {
      /* above window */
      return;
   }

   /* check if left edge is outside window */
   skipcol = 0;
   if (x<0) {
      skipcol = -x;
      m += x;
   }
   /* make sure span isn't too long or short */
   if (m>maxwidth) {
      m = maxwidth;
   }
   else if (m<=0) {
      return;
   }

   assert( m <= MAX_WIDTH );

   /* zoom the span horizontally */
   if (CC.Pixel.ZoomX==-1.0F) {
      /* n==m */
      for (j=0;j<m;j++) {
         i = n - (j+skipcol) - 1;
         zstencil[j] = stencil[i];
      }
   }
   else {
      GLfloat xscale = 1.0F / CC.Pixel.ZoomX;
      for (j=0;j<m;j++) {
         i = (j+skipcol) * xscale;
         if (i<0)  i = n + i - 1;
         zstencil[j] = stencil[i];
      }
   }

   /* write the span */
   for (r=r0; r<r1; r++) {
      gl_write_stencil_span( m, x+skipcol, r, zstencil );
   }
}




void glPixelZoom( GLfloat xfactor, GLfloat yfactor )
{
   if (CC.CompileFlag) {
      gl_save_pixelzoom( xfactor, yfactor );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glPixelZoom" );
	 return;
      }
      CC.Pixel.ZoomX = xfactor;
      CC.Pixel.ZoomY = yfactor;
      update_drawpixels_state();
   }
}



/**********************************************************************/
/*****                    glPixelStore                            *****/
/**********************************************************************/


void glPixelStorei( GLenum pname, GLint param )
{
   /* NOTE: this call can't be compiled into the display list */

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glPixelStore" );
      return;
   }

   switch (pname) {
      case GL_PACK_SWAP_BYTES:
         CC.PackSwapBytes = param ? GL_TRUE : GL_FALSE;
	 break;
      case GL_PACK_LSB_FIRST:
         CC.PackLSBFirst = param ? GL_TRUE : GL_FALSE;
	 break;
      case GL_PACK_ROW_LENGTH:
	 if (param<0) {
	    gl_error( GL_INVALID_VALUE, "glPixelStore(param)" );
	 }
	 else {
	    CC.PackRowLength = param;
	 }
	 break;
      case GL_PACK_SKIP_PIXELS:
	 if (param<0) {
	    gl_error( GL_INVALID_VALUE, "glPixelStore(param)" );
	 }
	 else {
	    CC.PackSkipPixels = param;
	 }
	 break;
      case GL_PACK_SKIP_ROWS:
	 if (param<0) {
	    gl_error( GL_INVALID_VALUE, "glPixelStore(param)" );
	 }
	 else {
	    CC.PackSkipRows = param;
	 }
	 break;
      case GL_PACK_ALIGNMENT:
         if (param==1 || param==2 || param==4 || param==8) {
	    CC.PackAlignment = param;
	 }
	 else {
	    gl_error( GL_INVALID_VALUE, "glPixelStore(param)" );
	 }
	 break;
      case GL_UNPACK_SWAP_BYTES:
	 CC.UnpackSwapBytes = param ? GL_TRUE : GL_FALSE;
         break;
      case GL_UNPACK_LSB_FIRST:
	 CC.UnpackLSBFirst = param ? GL_TRUE : GL_FALSE;
	 break;
      case GL_UNPACK_ROW_LENGTH:
	 if (param<0) {
	    gl_error( GL_INVALID_VALUE, "glPixelStore(param)" );
	 }
	 else {
	    CC.UnpackRowLength = param;
	 }
	 break;
      case GL_UNPACK_SKIP_PIXELS:
	 if (param<0) {
	    gl_error( GL_INVALID_VALUE, "glPixelStore(param)" );
	 }
	 else {
	    CC.UnpackSkipPixels = param;
	 }
	 break;
      case GL_UNPACK_SKIP_ROWS:
	 if (param<0) {
	    gl_error( GL_INVALID_VALUE, "glPixelStore(param)" );
	 }
	 else {
	    CC.UnpackSkipRows = param;
	 }
	 break;
      case GL_UNPACK_ALIGNMENT:
         if (param==1 || param==2 || param==4 || param==8) {
	    CC.UnpackAlignment = param;
	 }
	 else {
	    gl_error( GL_INVALID_VALUE, "glPixelStore" );
	 }
	 break;
      default:
	 gl_error( GL_INVALID_ENUM, "glPixelStore" );
   }
   update_drawpixels_state();
}



void glPixelStoref( GLenum pname, GLfloat param )
{
   glPixelStorei( pname, (GLint) param );
}



/**********************************************************************/
/*****                         glPixelMap                         *****/
/**********************************************************************/



void gl_pixel_map( GLenum map, GLint mapsize, const GLfloat *values )
{
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glPixelMapfv" );
      return;
   }

   if (mapsize<0 || mapsize>MAX_PIXEL_MAP_TABLE) {
      gl_error( GL_INVALID_VALUE, "glPixelMapfv(mapsize)" );
      return;
   }

   if (map>=GL_PIXEL_MAP_S_TO_S && map<=GL_PIXEL_MAP_I_TO_A) {
      /* test that mapsize is a power of two */
      GLuint p;
      GLboolean ok = GL_FALSE;
      for (p=1; p<=MAX_PIXEL_MAP_TABLE; p=p<<1) {
	 if ( (p&mapsize) == p ) {
	    ok = GL_TRUE;
	    break;
	 }
      }
      if (!ok) {
	 gl_error( GL_INVALID_VALUE, "glPixelMapfv(mapsize)" );
      }
   }

   switch (map) {
      case GL_PIXEL_MAP_S_TO_S:
         CC.Pixel.MapStoSsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapStoS[i] = (GLint) values[i];
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_I:
         CC.Pixel.MapItoIsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapItoI[i] = (GLint) values[i];
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_R:
         CC.Pixel.MapItoRsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapItoR[i] = CLAMP( values[i], 0.0, 1.0 );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_G:
         CC.Pixel.MapItoGsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapItoG[i] = CLAMP( values[i], 0.0, 1.0 );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_B:
         CC.Pixel.MapItoBsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapItoB[i] = CLAMP( values[i], 0.0, 1.0 );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_A:
         CC.Pixel.MapItoAsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapItoA[i] = CLAMP( values[i], 0.0, 1.0 );
	 }
	 break;
      case GL_PIXEL_MAP_R_TO_R:
         CC.Pixel.MapRtoRsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapRtoR[i] = CLAMP( values[i], 0.0, 1.0 );
	 }
	 break;
      case GL_PIXEL_MAP_G_TO_G:
         CC.Pixel.MapGtoGsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapGtoG[i] = CLAMP( values[i], 0.0, 1.0 );
	 }
	 break;
      case GL_PIXEL_MAP_B_TO_B:
         CC.Pixel.MapBtoBsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapBtoB[i] = CLAMP( values[i], 0.0, 1.0 );
	 }
	 break;
      case GL_PIXEL_MAP_A_TO_A:
         CC.Pixel.MapAtoAsize = mapsize;
         for (i=0;i<mapsize;i++) {
	    CC.Pixel.MapAtoA[i] = CLAMP( values[i], 0.0, 1.0 );
	 }
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glPixelMapfv(map)" );
   }
}



void glPixelMapfv( GLenum map, GLint mapsize, const GLfloat *values )
{
   if (CC.CompileFlag) {
      gl_save_pixelmap( map, mapsize, values );
   }
   if (CC.ExecuteFlag) {
      gl_pixel_map( map, mapsize, values );
   }
}




void glPixelMapuiv( GLenum map, GLint mapsize, const GLuint *values )
{
   GLfloat fvalues[MAX_PIXEL_MAP_TABLE];
   GLuint i;

   if (map==GL_PIXEL_MAP_I_TO_I || map==GL_PIXEL_MAP_S_TO_S) {
      for (i=0;i<mapsize;i++) {
	 fvalues[i] = (GLfloat) values[i];
      }
   }
   else {
      for (i=0;i<mapsize;i++) {
	 fvalues[i] = UINT_TO_FLOAT( values[i] );
      }
   }
   glPixelMapfv( map, mapsize, fvalues );
}



void glPixelMapusv( GLenum map, GLint mapsize, const GLushort *values )
{
   GLfloat fvalues[MAX_PIXEL_MAP_TABLE];
   GLuint i;

   if (map==GL_PIXEL_MAP_I_TO_I || map==GL_PIXEL_MAP_S_TO_S) {
      for (i=0;i<mapsize;i++) {
	 fvalues[i] = (GLfloat) values[i];
      }
   }
   else {
      for (i=0;i<mapsize;i++) {
	 fvalues[i] = USHORT_TO_FLOAT( values[i] );
      }
   }
   glPixelMapfv( map, mapsize, fvalues );
}



void glGetPixelMapfv( GLenum map, GLfloat *values )
{
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetPixelMapfv" );
      return;
   }
   switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
         for (i=0;i<CC.Pixel.MapItoIsize;i++) {
	    values[i] = (GLfloat) CC.Pixel.MapItoI[i];
	 }
	 break;
      case GL_PIXEL_MAP_S_TO_S:
         for (i=0;i<CC.Pixel.MapStoSsize;i++) {
	    values[i] = (GLfloat) CC.Pixel.MapStoS[i];
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_R:
         MEMCPY(values,CC.Pixel.MapItoR,CC.Pixel.MapItoRsize*sizeof(GLfloat));
	 break;
      case GL_PIXEL_MAP_I_TO_G:
         MEMCPY(values,CC.Pixel.MapItoG,CC.Pixel.MapItoGsize*sizeof(GLfloat));
	 break;
      case GL_PIXEL_MAP_I_TO_B:
         MEMCPY(values,CC.Pixel.MapItoB,CC.Pixel.MapItoBsize*sizeof(GLfloat));
	 break;
      case GL_PIXEL_MAP_I_TO_A:
         MEMCPY(values,CC.Pixel.MapItoA,CC.Pixel.MapItoAsize*sizeof(GLfloat));
	 break;
      case GL_PIXEL_MAP_R_TO_R:
         MEMCPY(values,CC.Pixel.MapRtoR,CC.Pixel.MapRtoRsize*sizeof(GLfloat));
	 break;
      case GL_PIXEL_MAP_G_TO_G:
         MEMCPY(values,CC.Pixel.MapGtoG,CC.Pixel.MapGtoGsize*sizeof(GLfloat));
	 break;
      case GL_PIXEL_MAP_B_TO_B:
         MEMCPY(values,CC.Pixel.MapBtoB,CC.Pixel.MapBtoBsize*sizeof(GLfloat));
	 break;
      case GL_PIXEL_MAP_A_TO_A:
         MEMCPY(values,CC.Pixel.MapAtoA,CC.Pixel.MapAtoAsize*sizeof(GLfloat));
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetPixelMapfv" );
   }
}


void glGetPixelMapuiv( GLenum map, GLuint *values )
{
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetPixelMapfv" );
      return;
   }
   switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
         MEMCPY(values, CC.Pixel.MapItoI, CC.Pixel.MapItoIsize*sizeof(GLint));
	 break;
      case GL_PIXEL_MAP_S_TO_S:
         MEMCPY(values, CC.Pixel.MapStoS, CC.Pixel.MapStoSsize*sizeof(GLint));
	 break;
      case GL_PIXEL_MAP_I_TO_R:
	 for (i=0;i<CC.Pixel.MapItoRsize;i++) {
	    values[i] = FLOAT_TO_UINT( CC.Pixel.MapItoR[i] );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_G:
	 for (i=0;i<CC.Pixel.MapItoGsize;i++) {
	    values[i] = FLOAT_TO_UINT( CC.Pixel.MapItoG[i] );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_B:
	 for (i=0;i<CC.Pixel.MapItoBsize;i++) {
	    values[i] = FLOAT_TO_UINT( CC.Pixel.MapItoB[i] );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_A:
	 for (i=0;i<CC.Pixel.MapItoAsize;i++) {
	    values[i] = FLOAT_TO_UINT( CC.Pixel.MapItoA[i] );
	 }
	 break;
      case GL_PIXEL_MAP_R_TO_R:
	 for (i=0;i<CC.Pixel.MapRtoRsize;i++) {
	    values[i] = FLOAT_TO_UINT( CC.Pixel.MapRtoR[i] );
	 }
	 break;
      case GL_PIXEL_MAP_G_TO_G:
	 for (i=0;i<CC.Pixel.MapGtoGsize;i++) {
	    values[i] = FLOAT_TO_UINT( CC.Pixel.MapGtoG[i] );
	 }
	 break;
      case GL_PIXEL_MAP_B_TO_B:
	 for (i=0;i<CC.Pixel.MapBtoBsize;i++) {
	    values[i] = FLOAT_TO_UINT( CC.Pixel.MapBtoB[i] );
	 }
	 break;
      case GL_PIXEL_MAP_A_TO_A:
	 for (i=0;i<CC.Pixel.MapAtoAsize;i++) {
	    values[i] = FLOAT_TO_UINT( CC.Pixel.MapAtoA[i] );
	 }
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetPixelMapfv" );
   }
}


void glGetPixelMapusv( GLenum map, GLushort *values )
{
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetPixelMapfv" );
      return;
   }
   switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
	 for (i=0;i<CC.Pixel.MapItoIsize;i++) {
	    values[i] = (GLushort) CC.Pixel.MapItoI[i];
	 }
	 break;
      case GL_PIXEL_MAP_S_TO_S:
	 for (i=0;i<CC.Pixel.MapStoSsize;i++) {
	    values[i] = (GLushort) CC.Pixel.MapStoS[i];
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_R:
	 for (i=0;i<CC.Pixel.MapItoRsize;i++) {
	    values[i] = FLOAT_TO_USHORT( CC.Pixel.MapItoR[i] );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_G:
	 for (i=0;i<CC.Pixel.MapItoGsize;i++) {
	    values[i] = FLOAT_TO_USHORT( CC.Pixel.MapItoG[i] );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_B:
	 for (i=0;i<CC.Pixel.MapItoBsize;i++) {
	    values[i] = FLOAT_TO_USHORT( CC.Pixel.MapItoB[i] );
	 }
	 break;
      case GL_PIXEL_MAP_I_TO_A:
	 for (i=0;i<CC.Pixel.MapItoAsize;i++) {
	    values[i] = FLOAT_TO_USHORT( CC.Pixel.MapItoA[i] );
	 }
	 break;
      case GL_PIXEL_MAP_R_TO_R:
	 for (i=0;i<CC.Pixel.MapRtoRsize;i++) {
	    values[i] = FLOAT_TO_USHORT( CC.Pixel.MapRtoR[i] );
	 }
	 break;
      case GL_PIXEL_MAP_G_TO_G:
	 for (i=0;i<CC.Pixel.MapGtoGsize;i++) {
	    values[i] = FLOAT_TO_USHORT( CC.Pixel.MapGtoG[i] );
	 }
	 break;
      case GL_PIXEL_MAP_B_TO_B:
	 for (i=0;i<CC.Pixel.MapBtoBsize;i++) {
	    values[i] = FLOAT_TO_USHORT( CC.Pixel.MapBtoB[i] );
	 }
	 break;
      case GL_PIXEL_MAP_A_TO_A:
	 for (i=0;i<CC.Pixel.MapAtoAsize;i++) {
	    values[i] = FLOAT_TO_USHORT( CC.Pixel.MapAtoA[i] );
	 }
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetPixelMapfv" );
   }
}



/**********************************************************************/
/*****                       glPixelTransfer                      *****/
/**********************************************************************/


/*
 * Implements glPixelTransfer[fi] whether called immediately or from a
 * display list.
 */
void gl_pixel_transfer( GLenum pname, GLfloat param )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glPixelTransfer" );
      return;
   }

   switch (pname) {
      case GL_MAP_COLOR:
         CC.Pixel.MapColorFlag = param ? GL_TRUE : GL_FALSE;
	 break;
      case GL_MAP_STENCIL:
         CC.Pixel.MapStencilFlag = param ? GL_TRUE : GL_FALSE;
	 break;
      case GL_INDEX_SHIFT:
         CC.Pixel.IndexShift = (GLint) param;
	 break;
      case GL_INDEX_OFFSET:
         CC.Pixel.IndexOffset = (GLint) param;
	 break;
      case GL_RED_SCALE:
         CC.Pixel.RedScale = param;
	 break;
      case GL_RED_BIAS:
         CC.Pixel.RedBias = param;
	 break;
      case GL_GREEN_SCALE:
         CC.Pixel.GreenScale = param;
	 break;
      case GL_GREEN_BIAS:
         CC.Pixel.GreenBias = param;
	 break;
      case GL_BLUE_SCALE:
         CC.Pixel.BlueScale = param;
	 break;
      case GL_BLUE_BIAS:
         CC.Pixel.BlueBias = param;
	 break;
      case GL_ALPHA_SCALE:
         CC.Pixel.AlphaScale = param;
	 break;
      case GL_ALPHA_BIAS:
         CC.Pixel.AlphaBias = param;
	 break;
      case GL_DEPTH_SCALE:
         CC.Pixel.DepthScale = param;
	 break;
      case GL_DEPTH_BIAS:
         CC.Pixel.DepthBias = param;
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glPixelTransfer(pname)" );
         return;
   }
   update_drawpixels_state();
}



void glPixelTransferf( GLenum pname, GLfloat param )
{
   if (CC.CompileFlag) {
      gl_save_pixeltransfer( pname, param );
   }
   if (CC.ExecuteFlag) {
      gl_pixel_transfer( pname, param );
   }
}



void glPixelTransferi( GLenum pname, GLint param )
{
   if (CC.CompileFlag) {
      gl_save_pixeltransfer( pname, (GLfloat) param );
   }
   if (CC.ExecuteFlag) {
      gl_pixel_transfer( pname, (GLfloat) param );
   }
}



/**********************************************************************/
/*****                 Pixel packing/unpacking                    *****/
/**********************************************************************/



/*
 * Unpack pixel data according to parameters set by glPixelStore.
	GLint PackAlignment;
	GLint PackRowLength;
	GLint PackSkipPixels;
	GLint PackSkipRows;
	GLboolean PackSwapBytes;
	GLboolean PackLSBFirst;

	GLint UnpackAlignment;
	GLint UnpackRowLength;
	GLint UnpackSkipPixels;
	GLint UnpackSkipRows;
	GLboolean UnpackSwapBytes;
	GLboolean UnpackLSBFirst;
 */



/*
 * Flip the 8 bits in each byte of the given array.
 */
void gl_flip_bytes( GLubyte *p, GLuint n )
{
   register GLuint i, a, b;

   for (i=0;i<n;i++) {
      b = (GLuint) p[i];
      a = ((b & 0x01) << 7) |
	  ((b & 0x02) << 5) |
	  ((b & 0x04) << 3) |
	  ((b & 0x08) << 1) |
	  ((b & 0x10) >> 1) |
	  ((b & 0x20) >> 3) |
	  ((b & 0x40) >> 5) |
	  ((b & 0x80) >> 7);
      p[i] = (GLubyte) a;
   }
}


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


/*
 * Compute ceiling of integer quotient of A divided by B:
 */
#define CEILING( A, B )  ( (A) % (B) == 0 ? (A)/(B) : (A)/(B)+1 )




/*
 * Unpack the given 2-D pixel array data.  The unpacked format will be con-
 * tiguous (no "empty" bytes) with byte/bit swapping applied as needed.
 * Input:  same as glDrawPixels
 * Output:  pointer to block of pixel data in same format and type as input
 *          or NULL if error.
 */
GLvoid *gl_unpack( GLsizei width, GLsizei height, GLenum format, GLenum type,
	           const GLvoid *pixels )
{
   GLuint i, s, a, n, l, k;
   GLuint bytes, width_in_bytes;
   GLubyte *dst, *src, *buffer;

   /* Compute bytes per component */
   switch (type) {
      case GL_UNSIGNED_BYTE:
         s = sizeof(GLubyte);
	 break;
      case GL_BYTE:
	 s = sizeof(GLbyte);
	 break;
      case GL_BITMAP:
	 s = 0;  /* special case */
	 break;
      case GL_UNSIGNED_SHORT:
	 s = sizeof(GLushort);
	 break;
      case GL_SHORT:
	 s = sizeof(GLshort);
	 break;
      case GL_UNSIGNED_INT:
	 s = sizeof(GLuint);
	 break;
      case GL_INT:
	 s = sizeof(GLint);
	 break;
      case GL_FLOAT:
	 s = sizeof(GLfloat);
	 break;
      default:
	 gl_error( GL_INVALID_ENUM, "internal error in gl_unpack(type)" );
	 return NULL;
   }

   /* Check if unpacking really is needed */
#ifdef LEAVEOUT
   if (!CC.UnpackSwapBytes && !CC.UnpackLSBFirst
       && CC.UnpackRowLength==0 && CC.UnpackSkipPixels==0
       && CC.UnpackSkipRows==0 && s>=CC.UnpackAlignment) {
      /* No unpacking has to be done */
      return (GLvoid *) pixels;
   }
#endif

   /* Compute number of components per pixel */
   switch (format) {
      case GL_COLOR_INDEX:
      case GL_STENCIL_INDEX:
      case GL_DEPTH_COMPONENT:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
         n = 1;
	 break;
      case GL_LUMINANCE_ALPHA:
	 n = 2;
	 break;
      case GL_RGB:
	 n = 3;
	 break;
      case GL_RGBA:
	 n = 4;
	 break;
      default:
	 gl_error( GL_INVALID_ENUM, "internal error in gl_unpack(format)" );
	 return NULL;
   }

   /* Compute alignment and row length */
   a = CC.UnpackAlignment;
   if (CC.UnpackRowLength>0) {
      l = CC.UnpackRowLength;
   }
   else {
      l = width;
   }

   /*
    * Unpack!
    */
   if (type==GL_BITMAP) {
      /* BITMAP data */

      k = 8 * a * CEILING( n*l, 8*a );

      /* allocate storage for unpacked pixel data */
      bytes = CEILING( width * height , 8 );
      buffer = (GLubyte *) malloc( bytes );
      if (!buffer) {
	 return NULL;
      }

      /* Copy/unpack pixel data to buffer */
      width_in_bytes = CEILING( width, 8 );
      src = (GLubyte *) pixels
	    + CC.UnpackSkipRows * k
            + CC.UnpackSkipPixels / 8;
      dst = buffer;
      for (i=0;i<height;i++) {
	 MEMCPY( dst, src, width_in_bytes );
	 dst += width_in_bytes;
	 src += k * s;
      }
      if (CC.UnpackLSBFirst) {
	 /* reverse order of 8 bits in each byte */
	 gl_flip_bytes( buffer, bytes );
      }
   }
   else {
      /* Non-BITMAP data */

      if (s>=a) {
	 k = n * l;
      }
      else {
	 k = a/s * CEILING( s*n*l, a );
      }

      /* allocate storage for unpacked pixel data */
      bytes = width * height * n * s;
      buffer = (GLubyte *) malloc( bytes );
      if (!buffer) {
	 return NULL;
      }

      /* Copy/unpack pixel data to buffer */
      width_in_bytes = width * n * s;
      src = (GLubyte *) pixels
	    + CC.UnpackSkipRows * k * s
            + CC.UnpackSkipPixels * n * s;
      dst = buffer;
      for (i=0;i<height;i++) {
	 MEMCPY( dst, src, width_in_bytes );
	 dst += width_in_bytes;
	 src += k * s;
      }

      if (CC.UnpackSwapBytes && s>1) {
	 if (s==2) {
	    swap2( (GLushort *) buffer, bytes/2 );
	 }
	 else if (s==4) {
	    swap4( (GLuint *) buffer, bytes/4 );
	 }
      }
   }

   return (GLvoid *) buffer;
}




/*
   if (s>=a) {
      k = n * l;
   }
   else {  *s<a*
      k = (a/s) * ceil( s*n*l / a );
   }

   s = size in bytes of a single component
   a = alignment
   n = number of components in a pixel
   l = number of pixels in a row

   k = number of components or indices between first pixel in each row in mem.
*/

