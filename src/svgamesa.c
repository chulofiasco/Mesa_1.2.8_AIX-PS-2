/* $Id: svgamesa.c,v 1.3 1996/04/25 20:49:54 brianp Exp $ */


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
$Log: svgamesa.c,v $
 * Revision 1.3  1996/04/25  20:49:54  brianp
 * removed optional, unimplemented DD functions
 *
 * Revision 1.2  1996/01/16  17:06:56  brianp
 * changed "svgamesa.h" to "GL/svgamesa.h"
 *
 * Revision 1.1  1996/01/16  15:16:41  brianp
 * Initial revision
 *
 */


/*
 * Linux SVGA/Mesa interface.
 *
 * This interface is not finished!  Still have to implement pixel
 * reading functions and double buffering.  Then, look into accelerated
 * line and polygon rendering.
 */


#ifdef SVGA


#include <stdio.h>
#include <vga.h>
#include "GL/svgamesa.h"
#include "context.h"
#include "dd.h"
#include "xform.h"



struct svgamesa_context {
   struct gl_context *glctx;	/* the core Mesa context */
   GLuint index;		/* current color index */
   GLint red, green, blue;	/* current rgb color */
   GLint width, height;		/* size of color buffer */
   GLint depth;			/* bits per pixel (8,16,24 or 32) */
};


static SVGAMesaContext SVGAMesa = NULL;    /* the current context */



/*
 * Convert Mesa window Y coordinate to VGA screen Y coordinate:
 */
#define FLIP(Y)  (SVGAMesa->height-(Y)-1)



/**********************************************************************/
/*****                 Miscellaneous functions                    *****/
/**********************************************************************/


static void buffer_size( GLuint *width, GLuint *height, GLuint *depth )
{
   int colors;
   *width = SVGAMesa->width = vga_getxdim();
   *height = SVGAMesa->height = vga_getydim();
   colors = vga_getcolors();
   if (colors==256)  *depth = 8;
   else if (colors=32768)  *depth = 16;
   else  *depth = 24;
}


/* Set current color index */
static void set_index( GLuint index )
{
   SVGAMesa->index = index;
   vga_setcolor( index );
}


/* Set current drawing color */
static void set_color( GLubyte red, GLubyte green,
                       GLubyte blue, GLubyte alpha )
{
   SVGAMesa->red = red;
   SVGAMesa->green = green;
   SVGAMesa->blue = blue;
   vga_setrgbcolor( red, green, blue );
}


static void clear_index( GLuint index )
{
   /* TODO: Implements glClearIndex() */
}


static void clear_color( GLubyte red, GLubyte green,
                         GLubyte blue, GLubyte alpha )
{
   /* TODO: Implements glClearColor() */
}


static void clear( GLboolean all,
                   GLint x, GLint y, GLint width, GLint height )
{
   vga_clear();
}


static GLboolean set_buffer( GLenum mode )
{
   /* TODO: implement double buffering and use this function to select */
   /* between front and back buffers. */
   return GL_TRUE;
}


static points_func choose_points_function( void )
{
   /* No accelerated point drawing functions */
   return NULL;
}


static line_func choose_line_function( void )
{
   /* No accelerated line drawing functions */
   return NULL;
}


static polygon_func choose_polygon_function( void )
{
   /* No accelerated polygon drawing functions */
   return NULL;
}



/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/


static void write_index_span( GLuint n, GLint x, GLint y,
                              const GLuint index[],
                              const GLubyte mask[] )
{
   int i;
   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         vga_setcolor( index[i] );
         vga_drawpixel( x, y );
      }
   }
}



static void write_monoindex_span(GLuint n,GLint x,GLint y,const GLubyte mask[])
{
   int i;
   y = FLIP(y);
   /* use current color index */
   vga_setcolor( SVGAMesa->index );
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         vga_drawpixel( x, y );
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
            vga_setrgbcolor( red[i], green[i], blue[i] );
            vga_drawpixel( x, y );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0; i<n; i++, x++) {
         vga_setrgbcolor( red[i], green[i], blue[i] );
         vga_drawpixel( x, y );
      }
   }
}



static void write_monocolor_span( GLuint n, GLint x, GLint y,
                                  const GLubyte mask[])
{
   int i;
   y=FLIP(y);
   /* use current rgb color */
   vga_setrgbcolor( SVGAMesa->red, SVGAMesa->green, SVGAMesa->blue );
   for (i=0; i<n; i++, x++) {
      if (mask[i]) {
         vga_drawpixel( x, y );
      }
   }
}



/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/


static void read_index_span( GLuint n, GLint x, GLint y, GLuint index[])
{
   int i;
   y = FLIP(y);
   for (i=0; i<n; i++,x++) {
      index[i] = vga_getpixel( x, y );
   }
}



static void read_color_span( GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
   int i;
   for (i=0; i<n; i++, x++) {
      /* TODO */
   }
}



/**********************************************************************/
/*****                  Write arrays of pixels                    *****/
/**********************************************************************/


static void write_index_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLuint index[], const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         vga_setcolor( index[i] );
         vga_drawpixel( x[i], FLIP(y[i]) );
      }
   }
}



static void write_monoindex_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
   /* use current color index */
   vga_setcolor( SVGAMesa->index );
   for (i=0; i<n; i++) {
      if (mask[i]) {
         vga_drawpixel( x[i], FLIP(y[i]) );
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
         vga_setrgbcolor( r[i], g[i], b[i] );
         vga_drawpixel( x[i], FLIP(y[i]) );
      }
   }
}



static void write_monocolor_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
   /* use current rgb color */
   vga_setrgbcolor( SVGAMesa->red, SVGAMesa->green, SVGAMesa->blue );
   for (i=0; i<n; i++) {
      if (mask[i]) {
         vga_drawpixel( x[i], FLIP(y[i]) );
      }
   }
}




/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

/* Read an array of color index pixels. */
static void read_index_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLuint index[], const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++,x++) {
      index[i] = vga_getpixel( x[i], FLIP(y[i]) );
   }
}



static void read_color_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLubyte red[], GLubyte green[],
                               GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   /* TODO */
}



static void svgamesa_setup_DD_pointers( void )
{
   /* Initialize all the pointers in the DD struct.  Do this whenever */
   /* a new context is made current or we change buffers via set_buffer! */

   DD.update_state = svgamesa_setup_DD_pointers;

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
   /* TODO: use different funcs for 8, 16, 32-bit depths */
   DD.write_color_span       = write_color_span;
   DD.write_monocolor_span   = write_monocolor_span;
   DD.write_color_pixels     = write_color_pixels;
   DD.write_monocolor_pixels = write_monocolor_pixels;
   DD.write_index_span       = write_index_span;
   DD.write_monoindex_span   = write_monoindex_span;
   DD.write_index_pixels     = write_index_pixels;
   DD.write_monoindex_pixels = write_monoindex_pixels;

   /* Pixel/span reading functions: */
   /* TODO: use different funcs for 8, 16, 32-bit depths */
   DD.read_index_span = read_index_span;
   DD.read_color_span = read_color_span;
   DD.read_index_pixels = read_index_pixels;
   DD.read_color_pixels = read_color_pixels;
}



/*
 * Create a new VGA/Mesa context and return a handle to it.
 */
SVGAMesaContext SVGAMesaCreateContext( void )
{
   SVGAMesaContext ctx;
   GLboolean rgb_flag;
   GLfloat redscale, greenscale, bluescale, alphascale;
   GLboolean db_flag;
   int colors;

   /* determine if we're in RGB or color index mode */
   colors = vga_getcolors();
   if (colors==32768) {
      rgb_flag = GL_TRUE;
      redscale = greenscale = bluescale = alphascale = 255.0;
   }
   else {
      rgb_flag = GL_FALSE;
      redscale = greenscale = bluescale = alphascale = 0.0;
   }
   db_flag = GL_FALSE;

   ctx = (SVGAMesaContext) malloc( sizeof(struct svgamesa_context) );

   ctx->glctx = gl_new_context( rgb_flag,
                                redscale, greenscale, bluescale, alphascale,
                                db_flag, NULL );

   ctx->index = 1;
   ctx->red = ctx->green = ctx->blue = 255;

   ctx->width = ctx->height = 0;  /* temporary until first "make-current" */

   return ctx;
}




/*
 * Destroy the given VGA/Mesa context.
 */
void SVGAMesaDestroyContext( SVGAMesaContext ctx )
{
   if (ctx) {
      gl_destroy_context( ctx->glctx );
      free( ctx );
      if (ctx==SVGAMesa) {
         SVGAMesa = NULL;
      }
   }
}



/*
 * Make the specified VGA/Mesa context the current one.
 */
void SVGAMesaMakeCurrent( SVGAMesaContext ctx )
{
   SVGAMesa = ctx;
   gl_set_context( ctx->glctx );
   svgamesa_setup_DD_pointers();

   if (ctx->width==0 || ctx->height==0) {
      /* setup initial viewport */
      ctx->width = vga_getxdim();
      ctx->height = vga_getydim();
      gl_viewport( 0, 0, ctx->width, ctx->height );
   }
}



/*
 * Return a handle to the current VGA/Mesa context.
 */
SVGAMesaContext SVGAMesaGetCurrentContext( void )
{
   return SVGAMesa;
}


#else

/*
 * Need this to provide at least one external definition.
 */

int gl_svga_dummy_function(void)
{
   return 0;
}

#endif  /*SVGA*/

