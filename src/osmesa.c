/* $Id: osmesa.c,v 1.8 1996/04/25 20:43:15 brianp Exp $ */

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
$Log: osmesa.c,v $
 * Revision 1.8  1996/04/25  20:43:15  brianp
 * removed optional, unimplemented DD functions
 *
 * Revision 1.7  1996/03/22  20:54:47  brianp
 * replaced CC.ClipSpans with WINCLIP_BIT RasterMask flag
 *
 * Revision 1.6  1996/01/22  21:07:42  brianp
 * used PIXELADDR macros instead of OFFSET macro
 * added a few accelerated polygon drawing functions
 *
 * Revision 1.5  1995/11/03  17:40:00  brianp
 * added casts for C++ compilation
 *
 * Revision 1.4  1995/10/30  15:32:58  brianp
 * added mask argument to read_[color|index]_pixels functions
 *
 * Revision 1.3  1995/10/19  15:49:06  brianp
 * changed clear_color and set_color arguments to GLubytes
 * changed arguments to gl_new_context
 *
 * Revision 1.2  1995/10/13  22:41:58  brianp
 * renamed index to set_index and color to set_color
 * make index_mask and color_mask return GL_FALSE
 *
 * Revision 1.1  1995/10/12  17:00:03  brianp
 * Initial revision
 *
 */



/*
 * Off-Screen Mesa rendering / Rendering into client memory space
 */


#include <stdlib.h>
#include <string.h>
#include "GL/osmesa.h"
#include "context.h"
#include "dd.h"
#include "depth.h"
#include "macros.h"
#include "vb.h"
#include "xform.h"


struct osmesa_context {
   struct gl_context *gl_ctx;	/* The GL/Mesa context */
   GLenum format;		/* either GL_RGBA or GL_COLOR_INDEX */
   void *buffer;		/* the image buffer */
   GLint width, height;		/* size of image buffer */
   GLuint pixel;		/* current color index or RGBA pixel value */
   GLuint clearpixel;		/* pixel for clearing the color buffer */
};



static OSMesaContext Current = NULL;

static void osmesa_setup_DD_pointers( void );



/*
 * Create an Off-Screen Mesa rendering context.  The only attribute needed is
 * an RGBA vs Color-Index mode flag.
 *
 * Input:  format - either GL_RGBA or GL_COLOR_INDEX
 *         sharelist - specifies another OSMesaContext with which to share
 *                     display lists.  NULL indicates no sharing.
 * Return:  an OSMesaContext or 0 if error
 */
OSMesaContext OSMesaCreateContext( GLenum format, OSMesaContext sharelist )
{
   OSMesaContext ctx;

   if (format!=GL_RGBA && format!=GL_COLOR_INDEX) {
      return NULL;
   }

   ctx = (OSMesaContext) malloc( sizeof(struct osmesa_context) );
   if (ctx) {
      ctx->gl_ctx = gl_new_context( (format==GL_RGBA) ? GL_TRUE : GL_FALSE,
                                    255.0, 255.0, 255.0, 255.0,
                                    GL_FALSE,
                                    sharelist ? sharelist->gl_ctx : NULL );
      if (!ctx->gl_ctx) {
         free(ctx);
         return NULL;
      }
      ctx->format = format;
      ctx->buffer = NULL;
      ctx->width = 0;
      ctx->height = 0;
      ctx->pixel = 0;
      ctx->clearpixel = 0;
   }
   return ctx;
}



/*
 * Destroy an Off-Screen Mesa rendering context.
 *
 * Input:  ctx - the context to destroy
 */
void OSMesaDestroyContext( OSMesaContext ctx )
{
   if (ctx) {
      gl_destroy_context( ctx->gl_ctx );
      free( ctx );
   }
}



/*
 * Bind an OSMesaContext to an image buffer.  The image buffer is just a
 * block of memory which the client provides.  Its size must be at least
 * as large as width*height*sizeof(type).  Its address should be a multiple
 * of 4 if using RGBA mode.
 *
 * Image data is stored in the order of glDrawPixels:  row-major order
 * with the lower-left image pixel stored in the first array position
 * (ie. bottom-to-top).
 *
 * Since the only type initially supported is GL_UNSIGNED_BYTE, if the
 * context is in RGBA mode, each pixel will be stored as a 4-byte RGBA
 * value.  If the context is in color indexed mode, each pixel will be
 * stored as a 1-byte value.
 *
 * If the context's viewport hasn't been initialized yet, it will now be
 * initialized to (0,0,width,height).
 *
 * Input:  ctx - the rendering context
 *         buffer - the image buffer memory
 *         type - data type for pixel components, only GL_UNSIGNED_BYTE
 *                supported now
 *         width, height - size of image buffer in pixels, at least 1
 * Return:  GL_TRUE if success, GL_FALSE if error because of invalid ctx,
 *          invalid buffer address, type!=GL_UNSIGNED_BYTE, width<1, height<1,
 *          width>internal limit or height>internal limit.
 */
GLboolean OSMesaMakeCurrent( OSMesaContext ctx, void *buffer, GLenum type,
                             GLsizei width, GLsizei height )
{
   if (!ctx || !buffer || type!=GL_UNSIGNED_BYTE
       || width<1 || height<1 || width>MAX_WIDTH || height>MAX_HEIGHT) {
      return GL_FALSE;
   }

   gl_set_context( ctx->gl_ctx );

   ctx->buffer = buffer;
   ctx->width = width;
   ctx->height = height;

   osmesa_setup_DD_pointers();

   Current = ctx;

   /* init viewport */
   if (ctx->gl_ctx->Viewport.Width==0) {
      /* initialize viewport and scissor box to buffer size */
      gl_viewport( 0, 0, width, height );
      CC.Scissor.X = 0;
      CC.Scissor.Y = 0;
      CC.Scissor.Width = width;
      CC.Scissor.Height = height;
   }

   return GL_TRUE;
}




/**********************************************************************/
/*** Device Driver Functions                                        ***/
/**********************************************************************/


/*
 * Useful macros:
 */
#define PACK_RGBA(R,G,B,A)  (((R) << 24) | ((G) << 16) | ((B) << 8) | (A))
#define PACK_ABGR(R,G,B,A)  (((A) << 24) | ((B) << 16) | ((G) << 8) | (R))

#define PIXELADDR1(X,Y)  ((GLubyte *) Current->buffer + (Y)*Current->width + (X))
#define PIXELADDR4(X,Y)   ((GLuint *) Current->buffer + (Y)*Current->width + (X))
#define PIXELADDR4B(X,Y)  ((GLubyte *) Current->buffer + ((Y)*Current->width + (X))*4)




static GLboolean set_buffer( GLenum mode )
{
   if (mode==GL_FRONT) {
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}


static void clear_index( GLuint index )
{
   Current->clearpixel = index;
}



static void clear_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   /* This trick facilitates big & little endian */
   GLubyte *clr = (GLubyte *) &Current->clearpixel;
   clr[0] = r;
   clr[1] = g;
   clr[2] = b;
   clr[3] = a;
}



static void clear( GLboolean all, GLint x, GLint y, GLint width, GLint height )
{
   if (Current->format==GL_RGBA) {
      if (all) {
         /* Clear whole RGBA buffer */
         GLuint i, n, *ptr4;
         n = Current->width * Current->height;
         ptr4 = (GLuint *) Current->buffer;
         for (i=0;i<n;i++) {
            *ptr4++ = Current->clearpixel;
         }
      }
      else {
         /* Clear part of RGBA buffer */
         GLuint i, j;
         for (i=0;i<height;i++) {
            GLuint *ptr4 = PIXELADDR4( x, (y+i) );
            for (j=0;j<width;j++) {
               *ptr4++ = Current->clearpixel;
            }
         }
      }
   }
   else {
      if (all) {
         /* Clear whole CI buffer */
         MEMSET(Current->buffer, Current->clearpixel, Current->width*Current->height);
      }
      else {
         /* Clear part of CI buffer */
         GLuint i, j;
         for (i=0;i<height;i++) {
            GLubyte *ptr1 = PIXELADDR1( x, (y+i) );
            for (j=0;j<width;j++) {
               *ptr1++ = Current->clearpixel;
            }
         }
      }
   }
}



static void set_index( GLuint index )
{
   Current->pixel = index;
}



static void set_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   /* This trick facilitates big & little endian */
   GLubyte *clr = (GLubyte *) &Current->pixel;
   clr[0] = r;
   clr[1] = g;
   clr[2] = b;
   clr[3] = a;
}



static void buffer_size( GLuint *width, GLuint *height, GLuint *depth )
{
   *width = Current->width;
   *height = Current->height;
   *depth = 8;
}



static void write_RGBA_span( GLuint n, GLint x, GLint y,
                             const GLubyte red[], const GLubyte green[],
			     const GLubyte blue[], const GLubyte alpha[],
			     const GLubyte mask[] )
{
   GLuint *ptr4 = PIXELADDR4( x, y );
   GLuint i;
   if (mask) {
      for (i=0;i<n;i++,ptr4++) {
         if (mask[i]) {
            *ptr4 = PACK_RGBA( red[i], green[i], blue[i], alpha[i] );
         }
      }
   }
   else {
      for (i=0;i<n;i++,ptr4++) {
         *ptr4 = PACK_RGBA( red[i], green[i], blue[i], alpha[i] );
      }
   }
}



static void write_ABGR_span( GLuint n, GLint x, GLint y,
			     const GLubyte red[], const GLubyte green[],
                             const GLubyte blue[], const GLubyte alpha[],
			     const GLubyte mask[] )
{
   GLuint *ptr4 = PIXELADDR4(x,y);
   GLuint i;
   if (mask) {
      for (i=0;i<n;i++,ptr4++) {
         if (mask[i]) {
            *ptr4 = PACK_ABGR( red[i], green[i], blue[i], alpha[i] );
         }
      }
   }
   else {
      for (i=0;i<n;i++,ptr4++) {
         *ptr4 = PACK_ABGR( red[i], green[i], blue[i], alpha[i] );
      }
   }
}



static void write_monocolor_span( GLuint n, GLint x, GLint y,
				  const GLubyte mask[] )
{
   GLuint *ptr4 = PIXELADDR4(x,y);
   GLuint i;
   for (i=0;i<n;i++,ptr4++) {
      if (mask[i]) {
         *ptr4 = Current->pixel;
      }
   }
}



static void write_RGBA_pixels( GLuint n, const GLint x[], const GLint y[],
                               const GLubyte red[], const GLubyte green[],
			       const GLubyte blue[], const GLubyte alpha[],
			       const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLuint *ptr4 = PIXELADDR4(x[i],y[i]);
         *ptr4 = PACK_RGBA( red[i], green[i], blue[i], alpha[i] );
      }
   }
}



static void write_ABGR_pixels( GLuint n, const GLint x[], const GLint y[],
                               const GLubyte red[], const GLubyte green[],
			       const GLubyte blue[], const GLubyte alpha[],
			       const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLuint *ptr4 = PIXELADDR4(x[i],y[i]);
         *ptr4 = PACK_ABGR( red[i], green[i], blue[i], alpha[i] );
      }
   }
}



static void write_monocolor_pixels( GLuint n, const GLint x[], const GLint y[],
				    const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLuint *ptr4 = PIXELADDR4(x[i],y[i]);
         *ptr4 = Current->pixel;
      }
   }
}



static void write_index_span( GLuint n, GLint x, GLint y, const GLuint index[],
			      const GLubyte mask[] )
{
   GLubyte *ptr1 = PIXELADDR1(x,y);
   GLuint i;
   for (i=0;i<n;i++,ptr1++) {
      if (mask[i]) {
         *ptr1 = (GLubyte) index[i];
      }
   }
}



static void write_monoindex_span( GLuint n, GLint x, GLint y,
				  const GLubyte mask[] )
{
   GLubyte *ptr1 = PIXELADDR1(x,y);
   GLuint i;
   for (i=0;i<n;i++,ptr1++) {
      if (mask[i]) {
         *ptr1 = (GLubyte) Current->pixel;
      }
   }
}



static void write_index_pixels( GLuint n, const GLint x[], const GLint y[],
			        const GLuint index[], const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr1 = PIXELADDR1(x[i],y[i]);
         *ptr1 = (GLubyte) index[i];
      }
   }
}



static void write_monoindex_pixels( GLuint n, const GLint x[], const GLint y[],
				    const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr1 = PIXELADDR1(x[i],y[i]);
         *ptr1 = (GLubyte) Current->pixel;
      }
   }
}



static void read_index_span( GLuint n, GLint x, GLint y, GLuint index[] )
{
   GLuint i;
   GLubyte *ptr1 = PIXELADDR1(x,y);
   for (i=0;i<n;i++,ptr1++) {
      index[i] = (GLuint) *ptr1;
   }
}


static void read_color_span( GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
			     GLubyte blue[], GLubyte alpha[] )
{
   GLuint i;
   GLubyte *ptr1 = PIXELADDR4B(x,y);
   for (i=0;i<n;i++) {
      red[i]   = *ptr1++;
      green[i] = *ptr1++;
      blue[i]  = *ptr1++;
      alpha[i] = *ptr1++;
   }
}


static void read_index_pixels( GLuint n, const GLint x[], const GLint y[],
			       GLuint index[], const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i] ) {
         GLubyte *ptr1 = PIXELADDR1(x[i],y[i]);
         index[i] = (GLuint) *ptr1;
      }
   }
}


static void read_color_pixels( GLuint n, const GLint x[], const GLint y[],
			       GLubyte red[], GLubyte green[],
			       GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr1 = PIXELADDR4B(x[i],y[i]);
         red[i]   = *ptr1++;
         green[i] = *ptr1++;
         blue[i]  = *ptr1++;
         alpha[i] = *ptr1;
      }
   }
}


static points_func choose_points_function( void )
{
   /* No accelerated points functions */
   return NULL;
}

static line_func choose_line_function( void )
{
   /* No accelerated line functions */
   return NULL;
}



/**********************************************************************/
/*****                  Optimized polygon rendering               *****/
/**********************************************************************/

static GLint lx[MAX_HEIGHT], rx[MAX_HEIGHT];	/* X bounds */
static GLfixed lz[MAX_HEIGHT], rz[MAX_HEIGHT];	/* Z values */
static GLfixed lr[MAX_HEIGHT], rr[MAX_HEIGHT];	/* Red */
static GLfixed lg[MAX_HEIGHT], rg[MAX_HEIGHT];	/* Green */
static GLfixed lb[MAX_HEIGHT], rb[MAX_HEIGHT];	/* Blue */
static GLfixed la[MAX_HEIGHT], ra[MAX_HEIGHT];	/* Alpha */



/*
 * Smooth-shaded, z-less polygon, RGBA byte order.
 */
static void smooth_RGBA_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_ALPHA
#define INTERP_Z

#define INNER_CODE						\
   GLint i;							\
   GLuint *img = PIXELADDR4(xmin,y);   				\
   for (i=0;i<len;i++) {					\
      if (FixedToUns(fz) < *zptr) {				\
         *img = PACK_RGBA( FixedToInt(fr), FixedToInt(fg),	\
		           FixedToInt(fb), FixedToInt(fa) );	\
         *zptr = FixedToUns(fz);				\
      }								\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;  fa += fdadx;	\
      fz += fdzdx;  zptr++;  img++;				\
   }

#include "polytemp.h"
}


/*
 * Smooth-shaded, z-less polygon, ABGR byte order.
 */
static void smooth_ABGR_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_ALPHA
#define INTERP_Z

#define INNER_CODE						\
   GLint i;							\
   GLuint *img = PIXELADDR4(xmin,y);   				\
   for (i=0;i<len;i++) {					\
      if (FixedToUns(fz) < *zptr) {				\
         *img = PACK_ABGR( FixedToInt(fr), FixedToInt(fg),	\
		           FixedToInt(fb), FixedToInt(fa) );	\
         *zptr = FixedToUns(fz);				\
      }								\
      fr += fdrdx;  fg += fdgdx;  fb += fdbdx;  fa += fdadx;	\
      fz += fdzdx;  zptr++;  img++;				\
   }

#include "polytemp.h"
}



/*
 * Smooth-shaded, z-less polygon.
 */
static void flat_RGBA_z_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
#define INTERP_COLOR
#define INTERP_ALPHA
#define INTERP_Z

#define SETUP_CODE			\
   GLubyte r = VB.Color[pv][0];		\
   GLubyte g = VB.Color[pv][1];		\
   GLubyte b = VB.Color[pv][2];		\
   GLubyte a = VB.Color[pv][3];		\
   GLuint pixel = PACK_RGBA(r,g,b,a );

#define INNER_CODE			\
   GLint i;				\
   GLuint *img = PIXELADDR4(xmin,y);   	\
   for (i=0;i<len;i++) {		\
      if (FixedToUns(fz) < *zptr) {	\
	 *img = pixel;			\
         *zptr = FixedToUns(fz);	\
      }					\
      fz += fdzdx;  zptr++;  img++;	\
   }

#include "polytemp.h"
}



/*
 * Return pointer to an accelerated polygon function if possible.
 */
static polygon_func choose_polygon_function( void )
{
   GLuint i4 = 1;
   GLubyte *i1 = (GLubyte *) &i4;
   GLint little_endian = *i1;

   if (CC.Polygon.SmoothFlag)     return NULL;
   if (CC.Polygon.StippleFlag)    return NULL;
   if (CC.Texture.Enabled)        return NULL;

   if (CC.RasterMask==DEPTH_BIT
       && CC.Depth.Func==GL_LESS
       && CC.Depth.Mask==GL_TRUE
       && Current->format==GL_RGBA) {
      if (CC.Light.ShadeModel==GL_SMOOTH) {
         if (little_endian) {
            return smooth_ABGR_z_polygon;
         }
         else {
            return smooth_RGBA_z_polygon;
         }
      }
      else {
         return flat_RGBA_z_polygon;
      }
   }
   return NULL;
}



static void osmesa_setup_DD_pointers( void )
{
   GLuint i4 = 1;
   GLubyte *i1 = (GLubyte *) &i4;
   GLint little_endian = *i1;

   /* one-time setup for polygon drawing */
   static int first_time=1;
   if (first_time) {
      int i;
      for (i=0;i<MAX_HEIGHT;i++) {
         lx[i] = MAX_WIDTH;
         rx[i] = -1;
      }
      first_time=0;
   }

   DD.update_state = osmesa_setup_DD_pointers;

   DD.set_buffer = set_buffer;
   DD.color = set_color;
   DD.index = set_index;
   DD.clear_index = clear_index;
   DD.clear_color = clear_color;
   DD.clear = clear;

   DD.buffer_size = buffer_size;

   DD.get_points_func = choose_points_function;
   DD.get_line_func = choose_line_function;
   DD.get_polygon_func = choose_polygon_function;

   if (little_endian) {
      DD.write_color_span = write_ABGR_span;
      DD.write_color_pixels = write_ABGR_pixels;
   }
   else {
      DD.write_color_span = write_RGBA_span;
      DD.write_color_pixels = write_RGBA_pixels;
   }
   DD.write_index_span = write_index_span;
   DD.write_monocolor_span = write_monocolor_span;
   DD.write_monoindex_span = write_monoindex_span;
   DD.write_index_pixels = write_index_pixels;
   DD.write_monocolor_pixels = write_monocolor_pixels;
   DD.write_monoindex_pixels = write_monoindex_pixels;

   DD.read_color_span = read_color_span;
   DD.read_index_span = read_index_span;
   DD.read_color_pixels = read_color_pixels;
   DD.read_index_pixels = read_index_pixels;
}
