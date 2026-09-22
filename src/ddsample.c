/* $Id: ddsample.c,v 1.5 1996/04/25 20:44:43 brianp Exp $ */

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
 *
 */

/*
$Log: ddsample.c,v $
 * Revision 1.5  1996/04/25  20:44:43  brianp
 * updated for new optional DD function pointers
 *
 * Revision 1.4  1995/12/19  22:18:22  brianp
 * removed 0.5 window offsets thanks to CC.RasterOffsetX/Y
 *
 * Revision 1.3  1995/10/30  15:28:55  brianp
 * pass mask array to read_[index|color]_pixels()
 *
 * Revision 1.2  1995/10/19  15:51:39  brianp
 * updated set_color, clear_color and gl_new_context arguments
 *
 * Revision 1.1  1995/10/17  21:24:20  brianp
 * Initial revision
 *
 */



/*
 * This is a sample template for writing new Mesa device drivers.
 * Let's say you're interfacing Mesa to a window/operating system
 * called FOO.  Replace all occurances of FOO with the real name
 * you select for your interface  (i.e. XMesa, WMesa, AMesa).
 *
 * You'll have to design an API for clients to use, defined in a
 * header called Mesa/include/GL/FOOmesa.h
 *
 * Note that you'll usually have to flip Y coordinates since Mesa's
 * window coordinates start at the bottom and increase upward.  Most
 * window system's Y-axis increases downward
 *
 * See dd.h for more device driver info.
 * See the other device driver implementations for ideas.
 *
 * Functions marked OPTIONAL may be completely omitted by the driver.
 *
 * The Makefile should compile this module along with the rest of
 * the core Mesa library.
 */


#include <stdlib.h>
#include "gl/FOOMesa.h"
#include "context.h"
#include "dd.h"
#include "xform.h"
#include "vb.h"




/*
 * This is the FOO/Mesa context structure.  This usually contains
 * info about what window/buffer we're rendering too, the current
 * drawing color, etc.
 */
struct FOOmesa_context
{
   struct gl_context *gl_ctx;	/* the core library context */
   GLboolean db_flag;	/* double buffered? */
   GLboolean rgb_flag;	/* RGB mode? */
   GLuint depth;		/* bits per pixel (1, 8, 24, etc) */
   unsigned long pixel;	/* current color index or RGBA pixel value */
   unsigned long clearpixel; /* pixel for clearing the color buffers */

   /* etc... */
};




/**********************************************************************/
/*****              Miscellaneous device driver funcs             *****/
/**********************************************************************/


static void finish( void )
{
   /* OPTIONAL FUNCTION: implements glFinish if possible */
}



static void flush( void )
{
   /* OPTIONAL FUNCTION: implements glFlush if possible */
}



static void clear_index( GLuint index )
{
   /* implement glClearIndex */
   /* usually just save the value in the context struct */
}



static void clear_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   /* implement glClearColor */
   /* color components are floats in [0,1] */
   /* usually just save the value in the context struct */
}



static void clear( GLboolean all, GLint x, GLint y, GLint width, GLint height )
{
/*
 * Clear the specified region of the color buffer using the clear color
 * or index as specified by one of the two functions above.
 * If all==GL_TRUE, clear whole buffer */
 */
}



static void set_index( GLuint index )
{
   /* Set the current color index. */
}



static void set_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   /* Set the current RGBA color. */
   /* r is in 0..CC.RedScale */
   /* g is in 0..CC.GreenScale */
   /* b is in 0..CC.BlueScale */
   /* a is in 0..CC.AlphaScale */
}



static GLboolean index_mask( GLuint mask )
{
   /* OPTIONAL FUNCTION: implement glIndexMask if possible, else
    * return GL_FALSE
    */
}



static GLboolean color_mask( GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask)
{
   /* OPTIONAL FUNCTION: implement glColorMask if possible, else
    * return GL_FALSE
    */
}



static GLboolean logicop( GLenum op )
{
   /*
    * OPTIONAL FUNCTION: 
    * Implements glLogicOp if possible.  Return GL_TRUE if the device driver
    * can perform the operation, otherwise return GL_FALSE.  If GL_FALSE
    * is returned, the logic op will be done in software by Mesa.
    */
}


static void dither( GLboolean enable )
{
   /* OPTIONAL FUNCTION: enable/disable dithering if applicable */
}



static GLboolean set_buffer( GLenum mode )
{
   /* set the current drawing/reading buffer, return GL_TRUE or GL_FALSE */
   /* for success/failure */
   setup_DD_pointers();
}



static void buffer_size( GLuint *width, GLuint *height, GLuint *depth )
{
   /* return the width, height and depth (bits/pixel) of the current buffer */
   /* if anything special has to been done when the buffer/window is */
   /* resized, do it now */
}



/**********************************************************************/
/*****           Accelerated point, line, polygon rendering       *****/
/**********************************************************************/


static void fast_points_function( GLuint first, GLuint last )
{
   /* Render a number of points by some hardware/OS accerated method */
   if (VB.MonoColor) {
      /* draw all points using the current color (set_color) */
      for (i=first;i<=last;i++) {
         if (VB.Unclipped[i]) {
            /* compute window coordinate */
            int x, y;
            x =       (GLint) VB.Win[i][0];
            y = FLIP( (GLint) VB.Win[i][1] );
            PLOT_PIXEL( x, y, currentcolor );
         }
      }
   }
   else {
      /* each point is a different color */
      for (i=first;i<=last;i++) {
         if (VB.Unclipped[i]) {
            int x, y;
            x =       (GLint) VB.Win[i][0];
            y = FLIP( (GLint) VB.Win[i][1] );
            PLOT_PIXEL( x, y, VB.Color[i] );
         }
      }
   }
}



static points_func choose_points_function( void )
{
   /* Examine the current rendering state and return a pointer to a */
   /* fast point-rendering function if possible. */
   if (CC.Point.Size==1.0 && !CC.Point.SmoothFlag && CC.RasterMask==0
       && !CC.Texture.Enabled && /* ETC, ETC */) {
      return fast_points_function;
   }
   else {
      return NULL;
   }
}



static void fast_line_function( GLuint v0, GLuint v1, GLuint pv )
{
   /* Render a line by some hardware/OS accerated method */
   int x0, y0, x1, y1;
   unsigned long pixel;

   if (VB.MonoColor) {
      pixel = current color;
   }
   else {
      pixel = VB.Color[pv];
   }

   x0 =       (int) VB.Win[v0][0];
   y0 = FLIP( (int) VB.Win[v0][1] );
   x1 =       (int) VB.Win[v1][0];
   y1 = FLIP( (int) VB.Win[v1][1] );

   DRAW_LINE( x0,y0, x1,y1, pixel );
}



static line_func choose_line_function( void )
{
   /* Examine the current rendering state and return a pointer to a */
   /* fast line-rendering function if possible. */
   if (CC.Line.Width==1.0 && !CC.Line.SmoothFlag && !CC.Line.StippleFlag
       && CC.Light.ShadeModel==GL_FLAT && CC.RasterMask==0
       && !CC.Texture.Enabled && /* ETC, ETC */ ) {
      return fast_line_function;
   }
   else {
      return NULL;
   }
}


static void fast_polygon_function( GLuint n, GLuint vlist[], GLuint pv )
{
   /* Render a line by some hardware/OS accerated method */
   int i;

   if (VB.MonoColor) {
      pixel = current color or index;
   }
   else {
      pixel = VB.Color[pv] or VB.Index[pv];
   }

   BEGIN_POLYGON;
   for (i=0; i<n; i++) {
      int x, y;
      j = vlist[i];
      x =       (int) VB.Win[j][0];
      y = FLIP( (int) VB.Win[j][1] );
      VERTEX( x, y );
   }
   END_POLYGON;
}



static polygon_func choose_polygon_function( void )
{
   /* Examine the current rendering state and return a pointer to a */
   /* fast polygon-rendering function if possible. */
   if (!CC.Polygon.SmoothFlag && !CC.Polygon.StippleFlag
       && CC.Light.ShadeModel==GL_FLAT && CC.RasterMask==0
       && !CC.Texture.Enabled && /* ETC, ETC */ ) {
      return fast_polygon_function;
   }
   else {
      return NULL;
   }
}



/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/


static void write_index_span( GLuint n, GLint x, GLint y,
                              const GLuint index[],
                              const GLubyte mask[] )
{
   int i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         /* draw pixel (x[i],y[i]) using index[i] */
      }
   }
}



static void write_monoindex_span(GLuint n,GLint x,GLint y,const GLubyte mask[])
{
   int i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         /* draw pixel (x[i],y[i]) using current color index */
      }
   }
}



static void write_color_span( GLuint n, GLint x, GLint y,
                              const GLubyte red[], const GLubyte green[],
                              const GLubyte blue[], const GLubyte alpha[],
                              const GLubyte mask[] )
{
   int i;
   y=FLIP(y);
   if (mask) {
      /* draw some pixels */
      for (i=0; i<n; i++, x++) {
         if (mask[i]) {
            /* draw pixel x,y using color red[i]/green[i]/blue[i]/alpha[i] */
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0; i<n; i++, x++) {
         /* draw pixel x,y using color red[i]/green[i]/blue[i]/alpha[i] */
      }
   }
}



static void write_monocolor_span( GLuint n, GLint x, GLint y,
                                  const GLubyte mask[])
{
   int i;
   y=FLIP(y);
   for (i=0; i<n; i++, x++) {
      if (mask[i]) {
         plot pixel (x,y) using current color
      }
   }
}



/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/


static void read_index_span( GLuint n, GLint x, GLint y, GLuint index[])
{
   int i;
   for (i=0; i<n; i++) {
      index[i] = read_pixel(x[i],y[i]);
   }
}



static void read_color_span( GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
   int i;
   for (i=0; i<n; i++, x++) {
      red[i] = read_red( x, y );
      green[i] = read_green( x, y );
      /* etc */
   }
}



/**********************************************************************/
/*****              Write arrays of pixels                        *****/
/**********************************************************************/


static void write_index_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLuint index[], const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         plot pixel x[i], y[i] using index[i]
      }
   }
}



static void write_monoindex_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         write pixel x[i], y[i] using current index
      }
   }
}



static void write_color_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLubyte r[], const GLubyte g[],
                                const GLubyte b[], const GLubyte a[],
                                const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         write pixel x[i], y[i] using red[i],green[i],blue[i],alpha[i]
      }
   }
}



static void write_monocolor_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         write pixel x[i], y[i] using current color
      }
   }
}




/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

/* Read an array of color index pixels. */
static void read_index_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLuint indx[], const GLubyte mask[] )
{
  int i;
  for (i=0; i<n; i++) {
     if (mask[i]) {
        index[i] = read_pixel x[i], y[i]
     }
  }
}



static void read_color_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLubyte red[], GLubyte green[],
                               GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         red[i] = read_red( x[i], y[i] );
         green[i] = read_green( x[i], y[i] );
         /* etc */
      }
   }
}




/**********************************************************************/
/**********************************************************************/


static void setup_DD_pointers( void )
{
   /* Initialize all the pointers in the DD struct.  Do this whenever */
   /* a new context is made current or we change buffers via set_buffer! */

   DD.update_state = setup_DD_pointers;

   DD.clear_index = clear_index;
   DD.clear_color = clear_color;
   DD.clear = clear;

   DD.index = set_index;
   DD.color = set_color;

   DD.set_buffer = set_buffer;
   DD.buffer_size = buffer_size;

   DD.get_points_func = choose_points_function;
   DD.get_line_func = choose_line_function;
   DD.get_polygon_func = choose_polygon_function;

   /* Pixel/span writing functions: */
   DD.write_color_span       = write_color_span;
   DD.write_monocolor_span   = write_monocolor_span;
   DD.write_color_pixels     = write_color_pixels;
   DD.write_monocolor_pixels = write_monocolor_pixels;
   DD.write_index_span       = write_index_span;
   DD.write_monoindex_span   = write_monoindex_span;
   DD.write_index_pixels     = write_index_pixels;
   DD.write_monoindex_pixels = write_monoindex_pixels;

   /* Pixel/span reading functions: */
   DD.read_index_span = read_index_span;
   DD.read_color_span = read_color_span;
   DD.read_index_pixels = read_index_pixels;
   DD.read_color_pixels = read_color_pixels;


   /*
    * OPTIONAL FUNCTIONS:  these may be left uninitialized if the device
    * driver can't/needn't implement them.
    */
   DD.finish = finish;
   DD.flush = flush;
   DD.index_mask = index_mask;
   DD.color_mask = color_mask;
   DD.logicop = logicop;
   DD.dither = dither;
}



/**********************************************************************/
/*****               FOO/Mesa API Functions                       *****/
/**********************************************************************/


/*
 * Implement the client-visible FOO/Mesa interface functions defined
 * in Mesa/include/GL/FOOmesa.h
 */



FOOMesaContext FOOMesaCreateContext( /* some args */,
                                     GLboolean rgb_flag,
                                     GLboolean db_flag )
{
   /* Create a new FOO/Mesa context */
   /* Be sure to initialize the following in the core Mesa context: */
   /* RedScale, GreenScale, BlueScale, AlphaScale */
   /* DrawBuffer, ReadBuffer */
   /* RGBAflag, DBflag */
   FOOMesaContext c;

   c->gl_ctx = gl_new_context( rgb_flag,
                               redscale, greenscale, bluescale, alphascale,
                               db_flag, sharelist );


  return newcontext;
}



void FOOMesaDestroyContext( FOOMesaContext c )
{
   /* destroy a FOO/Mesa context */
   gl_destroy_context( c->gl_ctx );
   free( c );
}



void FOOMesaMakeCurrent( FOOMesaContext c )
{
   /* Make the specified context the current one */
   /* the order of operations here is very important! */
   gl_set_context( c->gl_ctx );
   Current-context = c;
   setup_DD_pointers();
   if (Current->gl_ctx->Viewport.Width==0) {
      /* initialize viewport to window size */
      gl_viewport( 0, 0, Current->width, Current->height );
   }
   /* whatever else... */
}



void FOOMesaSwapBuffers( void )
{
   /* copy/swap back buffer to front if applicable */
}



/* you may need other FOO/Mesa functions too.......... */

