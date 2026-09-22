/* $Id: xmesa2.c,v 1.22 1996/04/25 21:09:53 brianp Exp $ */

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
$Log: xmesa2.c,v $
 * Revision 1.22  1996/04/25  21:09:53  brianp
 * removed references to amult, added DD.update_state assignment
 *
 * Revision 1.21  1996/03/22  20:55:40  brianp
 * nicer monochrome visual output
 *
 * Revision 1.20  1996/03/02  21:41:05  brianp
 * added PF_8R8G8B pixel format
 *
 * Revision 1.19  1996/03/01  20:06:42  brianp
 * added byte-swapping code to clear_16/32_bit_ximage() functions
 *
 * Revision 1.18  1996/02/26  16:23:09  brianp
 * added PF_5R6G5B pixel format from Joerg Hessdoerfer
 *
 * Revision 1.17  1996/02/23  22:19:23  brianp
 * optimized pixel reading for PF_HPCR per Jean-Luc Daems
 *
 * Revision 1.16  1996/02/06  03:25:43  brianp
 * added new gamma correction code
 *
 * Revision 1.15  1996/01/19  18:09:13  brianp
 * added XGetImage error trapping
 *
 * Revision 1.14  1996/01/16  15:07:53  brianp
 * used wrong pixel array function for PF_LOOKUP pixmap mode
 *
 * Revision 1.13  1995/12/30  01:05:11  brianp
 * use renamed DITHER macro
 * fixed XImage clearing problem on Crays
 *
 * Revision 1.11  1995/12/14  19:44:08  brianp
 * fixed a syntax error
 *
 * Revision 1.10  1995/12/13  23:30:07  brianp
 * replaced OFFSET macros with PIXELADDR macros
 *
 * Revision 1.9  1995/11/30  00:52:30  brianp
 * fixed a few type warnings for Sun compiler
 *
 * Revision 1.8  1995/11/30  00:21:23  brianp
 * added new PF_GRAYSCALE mode
 *
 * Revision 1.7  1995/11/03  17:41:48  brianp
 * removed unused vars, fixed code for C++ compilation
 *
 * Revision 1.6  1995/11/01  23:19:21  brianp
 * fixed bugs in write_span_DITHER_pixmap and write_span_LOOKUP_pixmap
 *
 * Revision 1.5  1995/10/30  15:33:49  brianp
 * added mask argument to read_[color|index]_pixels functions
 *
 * Revision 1.4  1995/10/30  15:13:53  brianp
 * replaced Current variable with XMesa
 * moved accelerated point, line, polygon functions to xmesa3.c
 *
 * Revision 1.3  1995/10/22  20:28:34  brianp
 * removed some dead code
 *
 * Revision 1.2  1995/10/19  15:54:06  brianp
 * changed clear_color and set_color arguments to GLubytes
 *
 * Revision 1.1  1995/10/17  21:37:09  brianp
 * Initial revision
 *
 */



/*
 * Mesa/X11 interface, part 2.
 *
 * This file contains the implementations of all the DD.* functions.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include "dd.h"
#include "macros.h"
#include "xmesaP.h"
#include "GL/xmesa.h"




/*
 * Abort with an error message.
 */
static void die( char *s )
{
   fprintf( stderr, "%s\n", s );
   abort();
}



/*
 * The following functions are used to trap XGetImage() calls which
 * generate BadMatch errors if the drawable isn't mapped.
 */

static int caught_xgetimage_error = 0;
static int (*old_xerror_handler)( Display *dpy, XErrorEvent *ev );
static unsigned long xgetimage_serial;

/*
 * This is the error handler which will be called if XGetImage fails.
 */
static int xgetimage_error_handler( Display *dpy, XErrorEvent *ev )
{
   if (ev->serial==xgetimage_serial && ev->error_code==BadMatch) {
      /* caught the expected error */
      caught_xgetimage_error = 0;
   }
   else {
      /* call the original X error handler, if any.  otherwise ignore */
      if (old_xerror_handler) {
         (*old_xerror_handler)( dpy, ev );
      }
   }
   return 0;
}


/*
 * Call this right before XGetImage to setup error trap.
 */
static void catch_xgetimage_errors( Display *dpy )
{
   xgetimage_serial = NextRequest( dpy );
   old_xerror_handler = XSetErrorHandler( xgetimage_error_handler );
   caught_xgetimage_error = 0;
}


/*
 * Call this right after XGetImage to check if an error occured.
 */
static int check_xgetimage_errors( void )
{
   /* restore old handler */
   (void) XSetErrorHandler( old_xerror_handler );
   /* return 0=no error, 1=error caught */
   return caught_xgetimage_error;
}


/*
 * Read a pixel from an X drawable.
 */
static unsigned long read_pixel( Display *dpy, Drawable d, int x, int y )
{
   XImage *pixel;
   unsigned long p;
   int error;
   catch_xgetimage_errors( dpy );
   pixel = XGetImage( dpy, d, x, y, 1, 1, AllPlanes, ZPixmap );
   error = check_xgetimage_errors();
   if (pixel && !error) {
      p = XGetPixel( pixel, 0, 0 );
   }
   else {
      p = 0;
   }
   if (pixel) {
      XDestroyImage( pixel );
   }
   return p;
}




/*
 * Return the size (width,height,depth) of the current color buffer.
 * This function should be called by the glViewport function because
 * glViewport is often called when the window gets resized.  We need to
 * update some X/Mesa stuff when that happens.
 * Output:  width - width of buffer in pixels.
 *          height - height of buffer in pixels.
 *          depth - In Color Index mode, return bits/pixel
 *                - In RGBA mode, return bits/component
 */
static void buffer_size( GLuint *width, GLuint *height, GLuint *depth )
{
   Window root;
   int winx, winy;
   unsigned int winwidth, winheight;
   unsigned int bw, d;

   winwidth = XMesa->width > 0 ? XMesa->width : 300;
   winheight = XMesa->height > 0 ? XMesa->height : 300;

   if (XMesa->display && XMesa->frontbuffer) {
      if (!XGetGeometry( XMesa->display, XMesa->frontbuffer, &root,
		 &winx, &winy, &winwidth, &winheight, &bw, &d )) {
         winwidth = XMesa->width > 0 ? XMesa->width : 300;
         winheight = XMesa->height > 0 ? XMesa->height : 300;
      }
   }

   if (winwidth > MAX_WIDTH) winwidth = MAX_WIDTH;
   if (winheight > MAX_HEIGHT) winheight = MAX_HEIGHT;

   *width = winwidth;
   *height = winheight;
   *depth = XMesa->depth;

   if (winwidth!=XMesa->width || winheight!=XMesa->height) {
      XMesa->width = winwidth;
      XMesa->height = winheight;
      xmesa_alloc_back_buffer( XMesa );

   }

   /* Needed by FLIP macro */
   XMesa->bottom = (int) winheight - 1;

   if (XMesa->backimage) {
      /* Needed by PIXELADDR1 macro */
      XMesa->ximage_width1 = XMesa->backimage->bytes_per_line;
      XMesa->ximage_origin1 = (GLubyte *) XMesa->backimage->data
                            + XMesa->ximage_width1 * (winheight-1);

      /* Needed by PIXELADDR2 macro */
      XMesa->ximage_width2 = XMesa->backimage->bytes_per_line / 2;
      XMesa->ximage_origin2 = (GLushort *) XMesa->backimage->data
                            + XMesa->ximage_width2 * (winheight-1);

      /* Needed by PIXELADDR4 macro */
      XMesa->ximage_width4 = XMesa->backimage->width;
      XMesa->ximage_origin4 = (GLuint *) XMesa->backimage->data
                            + XMesa->ximage_width4 * (winheight-1);
   }
}


static void finish( void )
{
   if (XMesa) {
      XSync( XMesa->display, False );
   }
}


static void flush( void )
{
   if (XMesa) {
      XFlush( XMesa->display );
   }
}



static GLboolean set_buffer( GLenum mode )
{
   if (mode==GL_FRONT) {
      /* read/write front buffer */
      XMesa->buffer = XMesa->frontbuffer;
      xmesa_setup_DD_pointers();
      return GL_TRUE;
   }
   else if (mode==GL_BACK && XMesa->db_state) {
      /* read/write back buffer */
      if (XMesa->backpixmap) {
         XMesa->buffer = XMesa->backpixmap;
      }
      else if (XMesa->backimage) {
         XMesa->buffer = None;
      }
      else {
         /* just in case, probably a serious error? */
         XMesa->buffer = XMesa->frontbuffer;
      }
      xmesa_setup_DD_pointers();
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}



static void clear_index( GLuint index )
{
   XMesa->clearpixel = (unsigned long) index;
   XSetForeground( XMesa->display, XMesa->cleargc, (unsigned long) index );
}


static void clear_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   unsigned long p = 0;
   switch (XMesa->pixelformat) {
      case PF_INDEX:
         return;
      case PF_TRUECOLOR:
	 p = PACK_RGB( r, g, b );
	 break;
      case PF_8A8B8G8R:
         p = PACK_8A8B8G8R( r, g, b, a );
	 break;
      case PF_8R8G8B:
         p = PACK_8R8G8B( r, g, b );
	 break;
      case PF_5R6G5B:
         p = PACK_5R6G5B( r, g, b );
	 break;
      case PF_DITHER:
	 p = DITHER( 0, 0, r, g, b );
	 break;
      case PF_1BIT:
	 p = (r+g+b) > 382U;   /* 382 = (3*255)/2 */
	 break;
      case PF_HPCR:
         p = DITHER_HPCR(1, 1, r, g, b);
         break;
      case PF_LOOKUP:
         p = LOOKUP( r, g, b );
         break;
      case PF_GRAYSCALE:
         p = GRAY_RGB( r, g, b );
         break;
      default:
	 abort();
   }
   XMesa->clearpixel = p;
   XSetForeground( XMesa->display, XMesa->cleargc, p );
}


/* Set current color index */
static void set_index( GLuint index )
{
   unsigned long p = (unsigned long) index;
   XMesa->pixel = p;
   XSetForeground( XMesa->display, XMesa->gc1, p );
}


/* Set current drawing color */
static void set_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   register unsigned long p = 0;
   switch (XMesa->pixelformat) {
      case PF_INDEX:
         return;
      case PF_TRUECOLOR:
	 p = PACK_RGB( r, g, b );
	 break;
      case PF_8A8B8G8R:
         p = PACK_8A8B8G8R( r, g, b, a );
	 break;
      case PF_8R8G8B:
         p = PACK_8R8G8B( r, g, b );
	 break;
      case PF_5R6G5B:
         p = PACK_5R6G5B( r, g, b );
	 break;
      case PF_DITHER:
         XMesa->red = r;
         XMesa->green = g;
         XMesa->blue = b;
	 p = DITHER( 0, 0, r, g, b );
	 break;
      case PF_1BIT:
         XMesa->red = r;
         XMesa->green = g;
         XMesa->blue = b;
	 p = (r+g+b) > 382U;   /* 382 = (3*255)/2 */
	 break;
      case PF_HPCR:
         XMesa->red = r;
         XMesa->green = g;
         XMesa->blue = b;
         p = DITHER_HPCR(1, 1, r, g, b);
         break;
      case PF_LOOKUP:
         p = LOOKUP( r, g, b );
         break;
      case PF_GRAYSCALE:
         p = GRAY_RGB( r, g, b );
         break;
      default:
	 abort();
   }
   XMesa->pixel = p;
   XSetForeground( XMesa->display, XMesa->gc1, p );
}



/* Set index mask ala glIndexMask */
static GLboolean index_mask( GLuint mask )
{
   if (XMesa->buffer==XIMAGE) {
      return GL_FALSE;
   }
   else {
      unsigned long m;
      if (mask==0xffffffff) {
         m = AllPlanes;
      }
      else {
         m = (unsigned long) mask;
      }
      XSetPlaneMask( XMesa->display, XMesa->gc1, m );
      XSetPlaneMask( XMesa->display, XMesa->gc2, m );
      XSetPlaneMask( XMesa->display, XMesa->cleargc, m );
      return GL_TRUE;
   }
}


/* Implements glColorMask() */
static GLboolean color_mask( GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask )
{
#if defined(__cplusplus) || defined(c_plusplus)
   int xclass = XMesa->visual->c_class;
#else
   int xclass = XMesa->visual->class;
#endif

   if (XMesa->buffer!=XIMAGE && (xclass==TrueColor || xclass==DirectColor)) {
      unsigned long m;
      if (rmask && gmask && bmask) {
         m = AllPlanes;
      }
      else {
         m = 0;
         if (rmask)   m |= XMesa->visual->red_mask;
         if (gmask)   m |= XMesa->visual->green_mask;
         if (bmask)   m |= XMesa->visual->blue_mask;
      }
      XSetPlaneMask( XMesa->display, XMesa->gc1, m );
      XSetPlaneMask( XMesa->display, XMesa->gc2, m );
      XSetPlaneMask( XMesa->display, XMesa->cleargc, m );
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}


/*
 * Set the pixel logic operation.  Return GL_TRUE if the device driver
 * can perform the operation, otherwise return GL_FALSE.  GL_COPY _must_
 * be operational, obviously.
 */
static GLboolean logicop( GLenum op )
{
   int func;
   if (!XMesa)  return GL_FALSE;
   if ((XMesa->buffer==XIMAGE) && op!=GL_COPY) {
      /* X can't do logic ops in Ximages, except for GL_COPY */
      return GL_FALSE;
   }
   switch (op) {
      case GL_CLEAR:		func = GXclear;		break;
      case GL_SET:		func = GXset;		break;
      case GL_COPY:		func = GXcopy;		break;
      case GL_COPY_INVERTED:	func = GXcopyInverted;	break;
      case GL_NOOP:		func = GXnoop;		break;
      case GL_INVERT:		func = GXinvert;	break;
      case GL_AND:		func = GXand;		break;
      case GL_NAND:		func = GXnand;		break;
      case GL_OR:		func = GXor;		break;
      case GL_NOR:		func = GXnor;		break;
      case GL_XOR:		func = GXxor;		break;
      case GL_EQUIV:		func = GXequiv;		break;
      case GL_AND_REVERSE:	func = GXandReverse;	break;
      case GL_AND_INVERTED:	func = GXandInverted;	break;
      case GL_OR_REVERSE:	func = GXorReverse;	break;
      case GL_OR_INVERTED:	func = GXorInverted;	break;
      default:  return GL_FALSE;
   }
   XSetFunction( XMesa->display, XMesa->gc1, func );
   XSetFunction( XMesa->display, XMesa->gc2, func );
   return GL_TRUE;
}


/*
 * Enable/disable dithering
 */
static void dither( GLboolean enable )
{
   if (enable) {
      XMesa->pixelformat = XMesa->dithered_pf;
   }
   else {
      XMesa->pixelformat = XMesa->undithered_pf;
   }
   xmesa_setup_DD_pointers();
}



/**********************************************************************/
/*** glClear implementations                                        ***/
/**********************************************************************/

static void clear_pixmap( GLboolean all,
                          GLint x, GLint y, GLint width, GLint height )
{
   if (all) {
      XFillRectangle( XMesa->display, XMesa->buffer,
                      XMesa->cleargc,
                      0, 0, XMesa->width+1, XMesa->height+1 );
   }
   else {
      XFillRectangle( XMesa->display, XMesa->buffer,
                      XMesa->cleargc,
                      x, XMesa->height - y - height, width, height );
   }
}


static void clear_8bit_ximage( GLboolean all,
                               GLint x, GLint y, GLint width, GLint height )
{
   if (all) {
      size_t n = XMesa->backimage->bytes_per_line*XMesa->backimage->height;
      MEMSET( XMesa->backimage->data, XMesa->clearpixel, n );
   }
   else {
      GLint i;
      for (i=0;i<height;i++) {
         GLubyte *ptr = PIXELADDR1( x, y+i );
         MEMSET( ptr, XMesa->clearpixel, width );
      }
   }
}


static void clear_16bit_ximage( GLboolean all,
                                GLint x, GLint y, GLint width, GLint height )
{
   if (all) {
      register GLuint n;
      register GLushort *ptr2 =(GLushort *) XMesa->backimage->data;
      register GLushort pixel = (GLushort) XMesa->clearpixel;
      if (XMesa->swapbytes) {
         pixel = ((pixel >> 8) & 0x00ff)
               | ((pixel << 8) & 0xff00);
      }
      if ((pixel & 0xff) == (pixel >> 8)) {
         /* low and high bytes are equal so use memset() */
         n = XMesa->backimage->bytes_per_line * XMesa->height;
         MEMSET( ptr2, pixel & 0xff, n );
      }
      else {
         n = XMesa->backimage->bytes_per_line/2 * XMesa->height;
	 do {
	    *ptr2++ = pixel;
	    n--;
	 } while (n!=0);
      }
   }
   else {
      register int i, j;
      register GLushort pixel = (GLushort) XMesa->clearpixel;
      for (j=0;j<height;j++) {
	 register GLushort *ptr2 = PIXELADDR2( x, y+j );
         for (i=0;i<width;i++) {
            *ptr2++ = pixel;
         }
      }
   }
}


static void clear_32bit_ximage( GLboolean all,
                                GLint x, GLint y, GLint width, GLint height )
{
   if (all) {
      register GLint n = XMesa->width * XMesa->height;
      register GLuint *ptr = (GLuint *) XMesa->backimage->data;
      register GLuint pixel = (GLuint) XMesa->clearpixel;
      if (XMesa->swapbytes) {
         pixel = ((pixel >> 24) & 0x000000ff)
               | ((pixel >> 8)  & 0x0000ff00)
               | ((pixel << 8)  & 0x00ff0000)
               | ((pixel << 24) & 0xff000000);
      }
      if (pixel==0) {
         MEMSET( ptr, pixel, 4*n );
      }
      else {
         do {
            *ptr++ = pixel;
            n--;
         } while (n!=0);
      }
   }
   else {
      register int i, j;
      register GLuint pixel = (GLuint) XMesa->clearpixel;
      for (j=0;j<height;j++) {
         register GLuint *ptr4 = PIXELADDR4( x, y+j );
         for (i=0;i<width;i++) {
            *ptr4++ = pixel;
         }
      }
   }
}


static void clear_nbit_ximage( GLboolean all,
                               GLint x, GLint y, GLint width, GLint height )
{
   if (all) {
      register int i, j;
      for (j=0;j<XMesa->height;j++) {
         for (i=0;i<XMesa->width;i++) {
            XPutPixel( XMesa->backimage, i, j, XMesa->clearpixel );
         }
      }
   }
   else {
      /* TODO: optimize this */
      register int i, j;
      y = FLIP(y);
      for (j=0;j<height;j++) {
         for (i=0;i<width;i++) {
            XPutPixel( XMesa->backimage, x+i, y-j, XMesa->clearpixel );
         }
      }
   }
}




/*
 * The Mesa library needs to be able to draw pixels in a number of ways:
 *   1. RGB vs Color Index
 *   2. as horizontal spans (polygons, images) vs random locations (points,
 *      lines)
 *   3. different color per-pixel or same color for all pixels
 *
 * Furthermore, the X driver needs to support rendering to 3 possible
 * "buffers", usually one, but sometimes two at a time:
 *   1. The front buffer as an X window
 *   2. The back buffer as a Pixmap
 *   3. The back buffer as an XImage
 *
 * Finally, if the back buffer is an XImage, we can avoid using XPutPixel and
 * optimize common cases such as 24-bit and 8-bit modes.
 *
 * By multiplication, there's at least 48 possible combinations of the above.
 *
 * Below are implementations of the most commonly used combinations.  They are
 * accessed through function pointers which get initialized here and are used
 * directly from the Mesa library.  The 8 function pointers directly correspond
 * to the first 3 cases listed above.
 *
 *
 * The function naming convention is:
 *
 *   write_[span|pixels]_[mono]_[format]_[pixmap|ximage]
 *
 * New functions optimized for specific cases can be added without too much
 * trouble.  An example might be the 24-bit TrueColor mode 8A8R8G8B which is
 * found on IBM RS/6000 X servers.
 */




/**********************************************************************/
/*** Write COLOR SPAN functions                                     ***/
/**********************************************************************/


#define COLOR_SPAN_ARGS	GLuint n, GLint x, GLint y,			\
			const GLubyte red[], const GLubyte green[],	\
			const GLubyte blue[], const GLubyte alpha[],	\
			const GLubyte mask[]

/* NOTE: if mask==NULL, draw all pixels */


/*
 * Write a span of PF_TRUECOLOR pixels to a pixmap.
 */
static void write_span_TRUECOLOR_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p = PACK_RGB( red[i], green[i], blue[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                       (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         XPutPixel( XMesa->rowimage, i, 0,
                    PACK_RGB( red[i], green[i], blue[i] ) );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_8A8B8G8R pixels to a pixmap.
 */
static void write_span_8A8B8G8R_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                        (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      register GLuint *ptr4 = (GLuint *) XMesa->rowimage->data;
      for (i=0;i<n;i++) {
         *ptr4++ = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_8R8G8B pixels to a pixmap.
 */
static void write_span_8R8G8B_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = PACK_8R8G8B( red[i], green[i], blue[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                        (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      register GLuint *ptr4 = (GLuint *) XMesa->rowimage->data;
      for (i=0;i<n;i++) {
         *ptr4++ = PACK_8R8G8B( red[i], green[i], blue[i] );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_5R6G5B pixels to a pixmap.
 */
static void write_span_5R6G5B_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned short p;
            p = PACK_5R6G5B( red[i], green[i], blue[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                        (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      register GLushort *ptr2 = (GLushort *) XMesa->rowimage->data;
      for (i=0;i<n;i++) {
         *ptr2++ = PACK_5R6G5B( red[i], green[i], blue[i] );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_DITHER pixels to a pixmap.
 */
static void write_span_DITHER_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = DITHER( x, y, red[i], green[i], blue[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                        (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         XPutPixel( XMesa->rowimage, i, 0,
                    DITHER( i, y, red[i], green[i], blue[i] ) );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_1BIT pixels to a pixmap.
 */
static void write_span_1BIT_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = DITHER_1BIT( x, y, red[i], green[i], blue[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                        (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         XPutPixel( XMesa->rowimage, i, 0,
                    DITHER_1BIT( x+i, y, red[i], green[i], blue[i] ) );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_HPCR pixels to a pixmap.
 */
static void write_span_HPCR_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = DITHER_HPCR( x, y, red[i], green[i], blue[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                       (int) x, (int) y );
         }
      }
   }
   else {
      register GLubyte *ptr = (GLubyte *) XMesa->rowimage->data;
      for (i=0;i<n;i++) {
         *ptr++ = DITHER_HPCR( (x+i), y, red[i], green[i], blue[i] );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_LOOKUP pixels to a pixmap.
 */
static void write_span_LOOKUP_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = LOOKUP( red[i], green[i], blue[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                       (int) x, (int) y );
         }
      }
   }
   else {
      for (i=0;i<n;i++) {
         XPutPixel( XMesa->rowimage, i, 0, LOOKUP(red[i],green[i],blue[i]) );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_GRAYSCALE pixels to a pixmap.
 */
static void write_span_GRAYSCALE_pixmap( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = GRAY_RGB( red[i], green[i], blue[i] );
            XSetForeground( XMesa->display, XMesa->gc2, p );
            XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                       (int) x, (int) y );
         }
      }
   }
   else {
      for (i=0;i<n;i++) {
         XPutPixel( XMesa->rowimage, i, 0, GRAY_RGB(red[i],green[i],blue[i]) );
      }
      XPutImage( XMesa->display, XMesa->buffer, XMesa->gc2,
                 XMesa->rowimage, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_TRUECOLOR pixels to an XImage.
 */
static void write_span_TRUECOLOR_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p = PACK_RGB( red[i], green[i], blue[i] );
            XPutPixel( XMesa->backimage, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         register unsigned long p = PACK_RGB( red[i], green[i], blue[i] );
         XPutPixel( XMesa->backimage, x, y, p );
      }
   }
}


/*
 * Write a span of PF_8A8B8G8R-format pixels to an ximage.
 */
static void write_span_8A8B8G8R_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   register GLuint *ptr = PIXELADDR4( x, y );
   if (mask) {
      for (i=0;i<n;i++,ptr++) {
         if (mask[i]) {
            *ptr = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,ptr++) {
         *ptr = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
      }
   }
}


/*
 * Write a span of PF_8R8G8B-format pixels to an ximage.
 */
static void write_span_8R8G8B_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   register GLuint *ptr = PIXELADDR4( x, y );
   if (mask) {
      for (i=0;i<n;i++,ptr++) {
         if (mask[i]) {
            *ptr = PACK_8R8G8B( red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,ptr++) {
         *ptr = PACK_8R8G8B( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write a span of PF_5R6G5B-format pixels to an ximage.
 */
static void write_span_5R6G5B_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   register GLushort *ptr = PIXELADDR2( x, y );
   if (mask) {
      for (i=0;i<n;i++,ptr++) {
         if (mask[i]) {
            *ptr = PACK_5R6G5B( red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,ptr++) {
         *ptr = PACK_5R6G5B( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write a span of PF_DITHER pixels to an XImage.
 */
static void write_span_DITHER_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = DITHER( x, y, red[i], green[i], blue[i] );
            XPutPixel( XMesa->backimage, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         register unsigned long p;
         p = DITHER( x, y, red[i], green[i], blue[i] );
         XPutPixel( XMesa->backimage, x, y, p );
      }
   }
}



/*
 * Write a span of 8-bit PF_DITHER pixels to an XImage.
 */
static void write_span_DITHER8_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( x, y );
   if (mask) {
      for (i=0;i<n;i++,x++,ptr++) {
         if (mask[i]) {
            *ptr = DITHER( x, y, red[i], green[i], blue[i] );
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++,ptr++) {
         *ptr = DITHER( x, y, red[i], green[i], blue[i] );
      }
   }
}



/*
 * Write a span of PF_1BIT pixels to an XImage.
 */
static void write_span_1BIT_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = DITHER_1BIT( x, y, red[i], green[i], blue[i] );
            XPutPixel( XMesa->backimage, x, y, p );
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++) {
         register unsigned long p;
         p = DITHER_1BIT( x, y, red[i], green[i], blue[i] );
         XPutPixel( XMesa->backimage, x, y, p );
      }
   }
}


/*
 * Write a span of PF_HPCR pixels to an XImage.
 */
static void write_span_HPCR_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( x, y );
   if (mask) {
      for (i=0;i<n;i++,x++,ptr++) {
         if (mask[i]) {
            *ptr = DITHER_HPCR( x, y, red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++,ptr++) {
         *ptr = DITHER_HPCR( x, y, red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write a span of PF_LOOKUP pixels to an XImage.
 */
static void write_span_LOOKUP_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p;
            p = LOOKUP( red[i], green[i], blue[i] );
            XPutPixel( XMesa->backimage, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         register unsigned long p;
         p = LOOKUP( red[i], green[i], blue[i] );
         XPutPixel( XMesa->backimage, x, y, p );
      }
   }
}


/*
 * Write a span of PF_GRAYSCALE pixels to an XImage.
 */
static void write_span_GRAYSCALE_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            register unsigned long p = GRAY_RGB( red[i], green[i], blue[i] );
            XPutPixel( XMesa->backimage, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         register unsigned long p = GRAY_RGB( red[i], green[i], blue[i] );
         XPutPixel( XMesa->backimage, x, y, p );
      }
   }
}


/*
 * Write a span of 8-bit PF_GRAYSCALE pixels to an XImage.
 */
static void write_span_GRAYSCALE8_ximage( COLOR_SPAN_ARGS )
{
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( x, y );
   if (mask) {
      for (i=0;i<n;i++,ptr++) {
         if (mask[i]) {
            *ptr = GRAY_RGB( red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,ptr++) {
         *ptr = GRAY_RGB( red[i], green[i], blue[i] );
      }
   }
}




/**********************************************************************/
/*** Write COLOR PIXEL functions                                    ***/
/**********************************************************************/


#define COLOR_PIXEL_ARGS   GLuint n, const GLint x[], const GLint y[],	\
			   const GLubyte red[], const GLubyte green[],	\
			   const GLubyte blue[], const GLubyte alpha[],	\
			   const GLubyte mask[]


/*
 * Write an array of PF_TRUECOLOR pixels to a pixmap.
 */
static void write_pixels_TRUECOLOR_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
	 p = PACK_RGB( red[i], green[i], blue[i] );
	 XSetForeground( XMesa->display, XMesa->gc2, p );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
		    (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_8A8B8G8R pixels to a pixmap.
 */
static void write_pixels_8A8B8G8R_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
         p = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
	 XSetForeground( XMesa->display, XMesa->gc2, p );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
	             (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_8R8G8B pixels to a pixmap.
 */
static void write_pixels_8R8G8B_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
         p = PACK_8R8G8B( red[i], green[i], blue[i] );
	 XSetForeground( XMesa->display, XMesa->gc2, p );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
	             (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_5R6G5B pixels to a pixmap.
 */
static void write_pixels_5R6G5B_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
         p = PACK_5R6G5B( red[i], green[i], blue[i] );
	 XSetForeground( XMesa->display, XMesa->gc2, p );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
	             (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to a pixmap.
 */
static void write_pixels_DITHER_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
	 p = DITHER( x[i], y[i], red[i], green[i], blue[i] );
	 XSetForeground( XMesa->display, XMesa->gc2, p );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
	             (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to a pixmap.
 */
static void write_pixels_1BIT_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
	 p = DITHER_1BIT( x[i], y[i], red[i], green[i], blue[i] );
	 XSetForeground( XMesa->display, XMesa->gc2, p );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
	             (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_HPCR pixels to a pixmap.
 */
static void write_pixels_HPCR_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         register unsigned long p;
         p = DITHER_HPCR( x[i], y[i], red[i], green[i], blue[i] );
         XSetForeground( XMesa->display, XMesa->gc2, p );
         XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                     (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_LOOKUP pixels to a pixmap.
 */
static void write_pixels_LOOKUP_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         register unsigned long p;
         p = LOOKUP( red[i], green[i], blue[i] );
         XSetForeground( XMesa->display, XMesa->gc2, p );
         XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                     (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_GRAYSCALE pixels to a pixmap.
 */
static void write_pixels_GRAYSCALE_pixmap( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         register unsigned long p;
         p = GRAY_RGB( red[i], green[i], blue[i] );
         XSetForeground( XMesa->display, XMesa->gc2, p );
         XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                     (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_TRUECOLOR pixels to an ximage.
 */
static void write_pixels_TRUECOLOR_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
	 p = PACK_RGB( red[i], green[i], blue[i] );
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of PF_8A8B8G8R pixels to an ximage.
 */
static void write_pixels_8A8B8G8R_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( x[i], y[i] );
         *ptr = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
      }
   }
}


/*
 * Write an array of PF_8R8G8B pixels to an ximage.
 */
static void write_pixels_8R8G8B_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( x[i], y[i] );
         *ptr = PACK_8R8G8B( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_5R6G5B pixels to an ximage.
 */
static void write_pixels_5R6G5B_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLushort *ptr = PIXELADDR2( x[i], y[i] );
         *ptr = PACK_5R6G5B( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to an XImage.
 */
static void write_pixels_DITHER_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
	 p = DITHER( x[i], y[i], red[i], green[i], blue[i] );
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of 8-bit PF_DITHER pixels to an XImage.
 */
static void write_pixels_DITHER8_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1(x[i],y[i]);
	 *ptr = DITHER( x[i], y[i], red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to an XImage.
 */
static void write_pixels_1BIT_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p;
	 p = DITHER_1BIT( x[i], y[i], red[i], green[i], blue[i] );
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of PF_HPCR pixels to an XImage.
 */
static void write_pixels_HPCR_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr = PIXELADDR1(x[i],y[i]);
         *ptr = DITHER_HPCR( x[i], y[i], red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_LOOKUP pixels to an XImage.
 */
static void write_pixels_LOOKUP_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         register unsigned long p = LOOKUP( red[i], green[i], blue[i] );
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of PF_GRAYSCALE pixels to an XImage.
 */
static void write_pixels_GRAYSCALE_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 register unsigned long p = GRAY_RGB( red[i], green[i], blue[i] );
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of 8-bit PF_GRAYSCALE pixels to an XImage.
 */
static void write_pixels_GRAYSCALE8_ximage( COLOR_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1( x[i], y[i] );
	 *ptr = GRAY_RGB( red[i], green[i], blue[i] );
      }
   }
}




/**********************************************************************/
/*** Write MONO COLOR SPAN functions                                ***/
/**********************************************************************/

#define MONO_SPAN_ARGS  GLuint n, GLint x, GLint y, const GLubyte mask[]


/*
 * Write a span of identical pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_pixmap( MONO_SPAN_ARGS )
{
   register GLuint i;
   register GLboolean write_all;
   y = FLIP( y );
   write_all = GL_TRUE;
   for (i=0;i<n;i++) {
      if (!mask[i]) {
	 write_all = GL_FALSE;
	 break;
      }
   }
   if (write_all) {
      XFillRectangle( XMesa->display, XMesa->buffer, XMesa->gc1,
		      (int) x, (int) y, n, 1 );
   }
   else {
      for (i=0;i<n;i++,x++) {
	 if (mask[i]) {
	    XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc1,
		        (int) x, (int) y );
	 }
      }
   }
}


/*
 * Write a span of PF_DITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_DITHER_pixmap( MONO_SPAN_ARGS )
{
   register GLuint i;
   register unsigned long p = XMesa->pixel;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   y = FLIP( y );
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         p = DITHER( x, y, r, g, b );
         XSetForeground( XMesa->display, XMesa->gc2, p );
         XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                    (int) x, (int) y );
      }
   }
}


/*
 * Write a span of PF_1BIT pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_1BIT_pixmap( MONO_SPAN_ARGS )
{
   register GLuint i;
   register unsigned long p = XMesa->pixel;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   y = FLIP( y );
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         p = DITHER_1BIT( x, y, r, g, b );
         XSetForeground( XMesa->display, XMesa->gc2, p );
         XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
                    (int) x, (int) y );
      }
   }
}


/*
 * Write a span of identical pixels to an XImage.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_ximage( MONO_SPAN_ARGS )
{
   register GLuint i;
   register unsigned long p = XMesa->pixel;
   y = FLIP( y );
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XPutPixel( XMesa->backimage, x, y, p );
      }
   }
}


/*
 * Write a span of identical 8A8B8G8R pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_8A8B8G8R_ximage( MONO_SPAN_ARGS )
{
   GLuint i, p, *ptr;
   p = (GLuint) XMesa->pixel;
   ptr = PIXELADDR4( x, y );
   for (i=0;i<n;i++,ptr++) {
      if (mask[i]) {
	 *ptr = p;
      }
   }
}


/*
 * Write a span of identical 8R8G8B pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_8R8G8B_ximage( MONO_SPAN_ARGS )
{
   GLuint i, p, *ptr;
   p = (GLuint) XMesa->pixel;
   ptr = PIXELADDR4( x, y );
   for (i=0;i<n;i++,ptr++) {
      if (mask[i]) {
	 *ptr = p;
      }
   }
}


/*
 * Write a span of identical DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_DITHER_ximage( MONO_SPAN_ARGS )
{
   register GLuint i;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 unsigned long p = DITHER( x, y, r, g, b );
	 XPutPixel( XMesa->backimage, x, y, p );
      }
   }
}


/*
 * Write a span of identical 8-bit DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_DITHER8_ximage( MONO_SPAN_ARGS )
{
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1(x,y);
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   for (i=0;i<n;i++,ptr++,x++) {
      if (mask[i]) {
	 *ptr = DITHER( x, y, r, g, b );
      }
   }
}


/*
 * Write a span of identical PF_1BIT pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_1BIT_ximage( MONO_SPAN_ARGS )
{
   register GLuint i;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 unsigned long p = DITHER_1BIT( x, y, r, g, b );
	 XPutPixel( XMesa->backimage, x, y, p );
      }
   }
}


/*
 * Write a span of identical HPCR pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_HPCR_ximage( MONO_SPAN_ARGS )
{
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1(x,y);
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   for (i=0;i<n;i++,ptr++,x++) {
      if (mask[i]) {
         *ptr = DITHER_HPCR( x, y, r, g, b );
      }
   }
}



/*
 * Write a span of identical 8-bit GRAYSCALE pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_GRAYSCALE8_ximage( MONO_SPAN_ARGS )
{
   GLuint i;
   unsigned long p = XMesa->pixel;
   GLubyte *ptr = PIXELADDR1(x,y);
   for (i=0;i<n;i++,ptr++) {
      if (mask[i]) {
	 *ptr = p;
      }
   }
}




/**********************************************************************/
/*** Write MONO COLOR PIXELS functions                              ***/
/**********************************************************************/

#define MONO_PIXEL_ARGS		GLuint n, const GLint x[], const GLint y[], \
				const GLubyte mask[]

/*
 * Write an array of identical pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_pixmap( MONO_PIXEL_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc1,
		    (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_DITHER_pixmap( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p = DITHER( x[i], y[i], r, g, b );
         XSetForeground( XMesa->display, XMesa->gc2, p );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
		    (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_1BIT_pixmap( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p = DITHER_1BIT( x[i], y[i], r, g, b );
         XSetForeground( XMesa->display, XMesa->gc2, p );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2,
		    (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of identical pixels to an XImage.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_ximage( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register unsigned long p = XMesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), p );
      }
   }
}



/*
 * Write an array of identical 8A8B8G8R pixels to an XImage.  The pixel value
 * is the one set by DD.color().
 */
static void write_pixels_mono_8A8B8G8R_ximage( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register GLuint p = (GLuint) XMesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( x[i], y[i] );
	 *ptr = p;
      }
   }
}


/*
 * Write an array of identical 8R8G8B pixels to an XImage.  The pixel value
 * is the one set by DD.color().
 */
static void write_pixels_mono_8R8G8B_ximage( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register GLuint p = (GLuint) XMesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( x[i], y[i] );
	 *ptr = p;
      }
   }
}


/*
 * Write an array of identical PF_DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_pixels_mono_DITHER_ximage( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 unsigned long p = DITHER( x[i], y[i], r, g, b );
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of identical 8-bit PF_DITHER pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_DITHER8_ximage( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1(x[i],y[i]);
	 *ptr = DITHER( x[i], y[i], r, g, b );
      }
   }
}


/*
 * Write an array of identical PF_1BIT pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_pixels_mono_1BIT_ximage( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 unsigned long p = DITHER_1BIT( x[i], y[i], r, g, b );
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of identical PF_HPCR pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_HPCR_ximage( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register GLubyte r, g, b;
   r = XMesa->red;
   g = XMesa->green;
   b = XMesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr = PIXELADDR1(x[i],y[i]);
         *ptr = DITHER_HPCR( x[i], y[i], r, g, b );
      }
   }
}


/*
 * Write an array of identical 8-bit PF_GRAYSCALE pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_GRAYSCALE8_ximage( MONO_PIXEL_ARGS )
{
   register GLuint i;
   register unsigned long p = XMesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1(x[i],y[i]);
	 *ptr = p;
      }
   }
}




/**********************************************************************/
/*** Write INDEX SPAN functions                                     ***/
/**********************************************************************/

#define INDEX_SPAN_ARGS   GLuint n, GLint x, GLint y, const GLuint index[], \
			  const GLubyte mask[]


/*
 * Write a span of CI pixels to a Pixmap.
 */
static void write_span_index_pixmap( INDEX_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XSetForeground( XMesa->display, XMesa->gc2,
			 (unsigned long) index[i] );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2, (int) x, (int) y );
      }
   }
}


/*
 * Write a span of CI pixels to an XImage.
 */
static void write_span_index_ximage( INDEX_SPAN_ARGS )
{
   register GLuint i;
   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XPutPixel( XMesa->backimage, x, y, (unsigned long) index[i] );
      }
   }
}



/**********************************************************************/
/*** Write INDEX PIXELS functions                                   ***/
/**********************************************************************/

#define INDEX_PIXELS_ARGS	GLuint n, const GLint x[], const GLint y[], \
				const GLuint index[], const GLubyte mask[]


/*
 * Write an array of CI pixels to a Pixmap.
 */
static void write_pixels_index_pixmap( INDEX_PIXELS_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XSetForeground( XMesa->display, XMesa->gc2,
			 (unsigned long) index[i] );
	 XDrawPoint( XMesa->display, XMesa->buffer, XMesa->gc2, 
		     (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of CI pixels to an XImage.
 */
static void write_pixels_index_ximage( INDEX_PIXELS_ARGS )
{
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( XMesa->backimage, x[i], FLIP(y[i]), (unsigned long) index[i] );
      }
   }
}




/**********************************************************************/
/*****                      Pixel reading                         *****/
/**********************************************************************/



/*
 * Read a horizontal span of color-index pixels.
 */
static void read_index_span( GLuint n, GLint x, GLint y, GLuint index[] )
{
   int i;

   y = FLIP(y);

   if (XMesa->buffer) {
      XImage *span;
      int error;
      catch_xgetimage_errors( XMesa->display );
      span = XGetImage( XMesa->display, XMesa->buffer,
		        x, y, n, 1, AllPlanes, ZPixmap );
      error = check_xgetimage_errors();
      if (span && !error) {
	 for (i=0;i<n;i++) {
	    index[i] = (GLuint) XGetPixel( span, i, 0 );
	 }
      }
      else {
	 /* return 0 pixels */
	 for (i=0;i<n;i++) {
	    index[i] = 0;
	 }
      }
      if (span) {
	 XDestroyImage( span );
      }
   }
   else if (XMesa->backimage) {
      for (i=0;i<n;i++,x++) {
	 index[i] = (GLuint) XGetPixel( XMesa->backimage, x, y );
      }
   }
}



/*
 * Read a horizontal span of color pixels.
 */
static void read_color_span( GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
   register GLuint i;

   if (XMesa->buffer) {
      XImage *span;
      int error;
      catch_xgetimage_errors( XMesa->display );
      span = XGetImage( XMesa->display, XMesa->buffer,
		        x, FLIP(y), n, 1, AllPlanes, ZPixmap );
      error = check_xgetimage_errors();
      if (span && !error) {
	 switch (XMesa->pixelformat) {
	    case PF_TRUECOLOR:
               {
                  GLint rshift = XMesa->rshift, rmult = XMesa->rmult;
                  GLint gshift = XMesa->gshift, gmult = XMesa->gmult;
                  GLint bshift = XMesa->bshift, bmult = XMesa->bmult;
                  for (i=0;i<n;i++) {
                     unsigned long p = XGetPixel( span, i, 0 );
                     red[i]   = (GLubyte) ((p >> rshift) & rmult);
                     green[i] = (GLubyte) ((p >> gshift) & gmult);
                     blue[i]  = (GLubyte) ((p >> bshift) & bmult);
                     alpha[i] = 255;
                  }
               }
	       break;
	    case PF_8A8B8G8R:
               {
                  GLuint *ptr4 = (GLuint *) span->data;
                  for (i=0;i<n;i++) {
                     GLuint p4 = *ptr4++;
                     red[i]   = (GLubyte) ( p4        & 0xff);
                     green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                     blue[i]  = (GLubyte) ((p4 >> 16) & 0xff);
                     alpha[i] = (GLubyte) ((p4 >> 24) & 0xff);
                  }
	       }
	       break;
            case PF_8R8G8B:
               {
                  GLuint *ptr4 = (GLuint *) span->data;
                  for (i=0;i<n;i++) {
                     GLuint p4 = *ptr4++;
                     red[i]   = (GLubyte) ((p4 >> 16) & 0xff);
                     green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                     blue[i]  = (GLubyte) ( p4        & 0xff);
                     alpha[i] = 255;
                  }
	       }
	       break;
            case PF_5R6G5B:
               {
                  GLushort *ptr2 = (GLushort *) span->data;
                  for (i=0;i<n;i++) {
                     GLushort p2 = *ptr2++;
                     red[i]   = (GLubyte) ((p2 >> 11) & 0x1f);
                     green[i] = (GLubyte) ((p2 >> 5)  & 0x3f);
                     blue[i]  = (GLubyte) ( p2        & 0x1f);
                     alpha[i] = 255;
                  }
	       }
               break;
            case PF_HPCR:
               {
                  GLubyte *ptr1 = (GLubyte *) span->data;
                  for (i=0;i<n;i++) {
                     GLubyte p = *ptr1++;
                     red[i]   =  p & 0xE0;
                     green[i] = (p & 0x1C) << 3;
                     blue[i]  = (p & 0x03) << 6;
                     alpha[i] = 255;
                  }
               }
               break;
	    case PF_DITHER:
	    case PF_LOOKUP:
	    case PF_GRAYSCALE:
               {
                  GLubyte *red_table   = XMesa->pixel_to_r;
                  GLubyte *green_table = XMesa->pixel_to_g;
                  GLubyte *blue_table  = XMesa->pixel_to_b;
                  if (XMesa->depth==8) {
                     GLubyte *ptr1 = (GLubyte *) span->data;
                     for (i=0;i<n;i++) {
                        unsigned long p = *ptr1++;
                        red[i]   = red_table[p];
                        green[i] = green_table[p];
                        blue[i]  = blue_table[p];
                        alpha[i] = 255;
                     }
                  }
                  else {
                     for (i=0;i<n;i++) {
                        unsigned long p = XGetPixel( span, i, 0 );
                        red[i]   = red_table[p];
                        green[i] = green_table[p];
                        blue[i]  = blue_table[p];
                        alpha[i] = 255;
                     }
                  }
               }
	       break;
	    case PF_1BIT:
	       for (i=0;i<n;i++) {
		  unsigned long p = XGetPixel( span, i, 0 );
		  red[i]   = (GLubyte) (p * 255);
		  green[i] = (GLubyte) (p * 255);
		  blue[i]  = (GLubyte) (p * 255);
		  alpha[i] = 255;
	       }
	       break;
	    default:
	       die("Problem in DD.read_color_span (1)");
	 }
      }
      else {
	 /* return black pixels */
	 for (i=0;i<n;i++) {
	    red[i] = green[i] = blue[i] = alpha[i] = 0;
	 }
      }
      if (span) {
	 XDestroyImage( span );
      }
   }
   else if (XMesa->backimage) {
      switch (XMesa->pixelformat) {
	 case PF_TRUECOLOR:
            {
               GLint rshift = XMesa->rshift, rmult = XMesa->rmult;
               GLint gshift = XMesa->gshift, gmult = XMesa->gmult;
               GLint bshift = XMesa->bshift, bmult = XMesa->bmult;
               y = FLIP(y);
               for (i=0;i<n;i++,x++) {
                  unsigned long p = XGetPixel( XMesa->backimage, x, y );
                  red[i]   = (GLubyte) ((p >> rshift) & rmult);
                  green[i] = (GLubyte) ((p >> gshift) & gmult);
                  blue[i]  = (GLubyte) ((p >> bshift) & bmult);
                  alpha[i] = 255;
               }
            }
	    break;
	 case PF_8A8B8G8R:
            {
               GLuint *ptr4 = PIXELADDR4( x, y );
               for (i=0;i<n;i++) {
                  GLuint p4 = *ptr4++;
                  red[i]   = (GLubyte) ( p4        & 0xff);
                  green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ((p4 >> 16) & 0xff);
                  alpha[i] = (GLint)   ((p4 >> 24) & 0xff);
               }
            }
	    break;
	 case PF_8R8G8B:
            {
               GLuint *ptr4 = PIXELADDR4( x, y );
               for (i=0;i<n;i++) {
                  GLuint p4 = *ptr4++;
                  red[i]   = (GLubyte) ((p4 >> 16) & 0xff);
                  green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ( p4        & 0xff);
                  alpha[i] = 255;
               }
            }
	    break;
         case PF_5R6G5B:
            {
               GLushort *ptr2 = PIXELADDR2( x, y );
               for (i=0;i<n;i++) {
                  GLushort p2 = *ptr2++;
                  red[i]   = (GLubyte) ((p2 >> 11) & 0x1f);
                  green[i] = (GLubyte) ((p2 >> 5)  & 0x3f);
                  blue[i]  = (GLubyte) ( p2        & 0x1f);
                  alpha[i] = 255;
               }
            }
            break;
         case PF_HPCR:
            {
               GLubyte *ptr1 = PIXELADDR1( x, y );
               for (i=0;i<n;i++) {
                  GLubyte p = *ptr1++;
                  red[i]   =  p & 0xE0;
                  green[i] = (p & 0x1C) << 3;
                  blue[i]  = (p & 0x03) << 6;
                  alpha[i] = 255;
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               GLubyte *red_table   = XMesa->pixel_to_r;
               GLubyte *green_table = XMesa->pixel_to_g;
               GLubyte *blue_table  = XMesa->pixel_to_b;
               if (XMesa->depth==8) {
                  GLubyte *ptr1 = PIXELADDR1(x,y);
                  for (i=0;i<n;i++) {
                     unsigned long p = *ptr1++;
                     red[i]   = red_table[p];
                     green[i] = green_table[p];
                     blue[i]  = blue_table[p];
                     alpha[i] = 255;
                  }
               }
               else {
                  y = FLIP(y);
                  for (i=0;i<n;i++,x++) {
                     unsigned long p = XGetPixel( XMesa->backimage, x, y );
                     red[i]   = red_table[p];
                     green[i] = green_table[p];
                     blue[i]  = blue_table[p];
                     alpha[i] = 255;
                  }
               }
            }
	    break;
	 case PF_1BIT:
            y = FLIP(y);
	    for (i=0;i<n;i++,x++) {
	       unsigned long p = XGetPixel( XMesa->backimage, x, y );
	       red[i]   = (GLubyte) (p * 255);
	       green[i] = (GLubyte) (p * 255);
	       blue[i]  = (GLubyte) (p * 255);
	       alpha[i] = 255;
	    }
	    break;
	 default:
	    die("Problem in DD.read_color_span (2)");
      }
   }
}



/*
 * Read an array of color index pixels.
 */
static void read_index_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLuint indx[], const GLubyte mask[] )
{
   register GLuint i;
   if (XMesa->buffer) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            indx[i] = (GLuint) read_pixel( XMesa->display, XMesa->buffer,
                                           x[i], FLIP(y[i]) );
         }
      }
   }
   else if (XMesa->backimage) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            indx[i] = (GLuint) XGetPixel( XMesa->backimage, x[i], FLIP(y[i]) );
         }
      }
   }
}



static void read_color_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLubyte red[], GLubyte green[],
                               GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   register GLuint i;

   if (XMesa->buffer) {
      switch (XMesa->pixelformat) {
	 case PF_TRUECOLOR:
            {
               GLint rshift = XMesa->rshift, rmult = XMesa->rmult;
               GLint gshift = XMesa->gshift, gmult = XMesa->gmult;
               GLint bshift = XMesa->bshift, bmult = XMesa->bmult;
               for (i=0;i<n;i++) {
                  if (mask[i] ) {
                     unsigned long p = read_pixel( XMesa->display,
                                          XMesa->buffer, x[i], FLIP(y[i]) );
                     red[i]   = (GLubyte) ((p >> rshift) & rmult);
                     green[i] = (GLubyte) ((p >> gshift) & gmult);
                     blue[i]  = (GLubyte) ((p >> bshift) & bmult);
                     alpha[i] = 255;
                  }
               }
	    }
	    break;
	 case PF_8A8B8G8R:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( XMesa->display, XMesa->buffer,
                                                x[i], FLIP(y[i]) );
                  red[i]   = (GLubyte) ( p        & 0xff);
                  green[i] = (GLubyte) ((p >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ((p >> 16) & 0xff);
                  alpha[i] = (GLubyte) ((p >> 24) & 0xff);
               }
	    }
	    break;
	 case PF_8R8G8B:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( XMesa->display, XMesa->buffer,
                                                x[i], FLIP(y[i]) );
                  red[i]   = (GLubyte) ((p >> 16) & 0xff);
                  green[i] = (GLubyte) ((p >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ( p        & 0xff);
                  alpha[i] = 255;
               }
	    }
	    break;
         case PF_5R6G5B:
            for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( XMesa->display, XMesa->buffer,
                                                x[i], FLIP(y[i]) );
                  red[i]   = (GLubyte) ((p >> 11) & 0x1f);
                  green[i] = (GLubyte) ((p >> 5)  & 0x3f);
                  blue[i]  = (GLubyte) ( p        & 0x1f);
                  alpha[i] = 255;
               }
            }
            break;
         case PF_HPCR:
            {
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( XMesa->display,
                                          XMesa->buffer, x[i], FLIP(y[i]) );
                     red[i]   =  p & 0xE0;
                     green[i] = (p & 0x1C) << 3;
                     blue[i]  = (p & 0x03) << 6;
                     alpha[i] = 255;
                  }
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               GLubyte *red_table   = XMesa->pixel_to_r;
               GLubyte *green_table = XMesa->pixel_to_g;
               GLubyte *blue_table  = XMesa->pixel_to_b;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( XMesa->display,
                                         XMesa->buffer, x[i], FLIP(y[i]) );
                     red[i]   = red_table[p];
                     green[i] = green_table[p];
                     blue[i]  = blue_table[p];
                     alpha[i] = 255;
                  }
               }
	    }
	    break;
	 case PF_1BIT:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( XMesa->display, XMesa->buffer,
                                                x[i], FLIP(y[i]) );
                  red[i]   = (GLubyte) (p * 255);
                  green[i] = (GLubyte) (p * 255);
                  blue[i]  = (GLubyte) (p * 255);
                  alpha[i] = 255;
               }
	    }
	    break;
	 default:
	    die("Problem in DD.read_color_pixels (1)");
      }
   }
   else if (XMesa->backimage) {
      switch (XMesa->pixelformat) {
	 case PF_TRUECOLOR:
            {
               GLint rshift = XMesa->rshift, rmult = XMesa->rmult;
               GLint gshift = XMesa->gshift, gmult = XMesa->gmult;
               GLint bshift = XMesa->bshift, bmult = XMesa->bmult;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p;
                     p = XGetPixel( XMesa->backimage, x[i], FLIP(y[i]) );
                     red[i]   = (GLubyte) ((p >> rshift) & rmult);
                     green[i] = (GLubyte) ((p >> gshift) & gmult);
                     blue[i]  = (GLubyte) ((p >> bshift) & bmult);
                     alpha[i] = 255;
                  }
               }
	    }
	    break;
	 case PF_8A8B8G8R:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLuint *ptr4 = PIXELADDR4( x[i], y[i] );
                  GLuint p4 = *ptr4;
                  red[i]   = (GLubyte) ( p4        & 0xff);
                  green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ((p4 >> 16) & 0xff);
                  alpha[i] = (GLubyte) ((p4 >> 24) & 0xff);
               }
	    }
	    break;
	 case PF_8R8G8B:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLuint *ptr4 = PIXELADDR4( x[i], y[i] );
                  GLuint p4 = *ptr4;
                  red[i]   = (GLubyte) ((p4 >> 16) & 0xff);
                  green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ( p4        & 0xff);
                  alpha[i] = 255;
               }
	    }
	    break;
         case PF_5R6G5B:
            for (i=0;i<n;i++) {
               if (mask[i]) {
                  GLushort *ptr2 = PIXELADDR2( x[i], y[i] );
                  GLushort p2 = *ptr2;
                  red[i]   = (GLubyte) ((p2 >> 11) & 0x1f);
                  green[i] = (GLubyte) ((p2 >> 5)  & 0x3f);
                  blue[i]  = (GLubyte) ( p2        & 0x1f);
                  alpha[i] = 255;
               }
            }
            break;
         case PF_HPCR:
            for (i=0;i<n;i++) {
               if (mask[i]) {
                  GLubyte *ptr1 = PIXELADDR1( x[i], y[i] );
                  GLubyte p = *ptr1;
                  red[i]   =  p & 0xE0;
                  green[i] = (p & 0x1C) << 3;
                  blue[i]  = (p & 0x03) << 6;
                  alpha[i] = 255;
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               GLubyte *red_table   = XMesa->pixel_to_r;
               GLubyte *green_table = XMesa->pixel_to_g;
               GLubyte *blue_table  = XMesa->pixel_to_b;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p;
                     p = XGetPixel( XMesa->backimage, x[i], FLIP(y[i]) );
                     red[i]   = red_table[p];
                     green[i] = green_table[p];
                     blue[i]  = blue_table[p];
                     alpha[i] = 255;
                  }
               }
	    }
	    break;
	 case PF_1BIT:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p;
                  p = XGetPixel( XMesa->backimage, x[i], FLIP(y[i]) );
                  red[i]   = (GLubyte) (p * 255);
                  green[i] = (GLubyte) (p * 255);
                  blue[i]  = (GLubyte) (p * 255);
                  alpha[i] = 255;
               }
	    }
	    break;
	 default:
	    die("Problem in DD.read_color_pixels (1)");
      }
   }
}




/*
 * Initialize all the DD.* function pointers depeding on the current
 * color buffer configuration.  This is mainly called by XMesaMakeCurrent.
 */
void xmesa_setup_DD_pointers( void )
{

   /*
    * Always the same:
    */
   DD.update_state = xmesa_setup_DD_pointers;
   DD.buffer_size = buffer_size;
   DD.flush = flush;
   DD.finish = finish;

   DD.set_buffer = set_buffer;

   DD.index = set_index;
   DD.color = set_color;
   DD.clear_index = clear_index;
   DD.clear_color = clear_color;
   DD.index_mask = index_mask;
   DD.color_mask = color_mask;
   DD.logicop = logicop;
   DD.dither = dither;
   DD.get_points_func = xmesa_get_points_func;
   DD.get_line_func = xmesa_get_line_func;
   DD.get_polygon_func = xmesa_get_polygon_func;

   /*
    * These drawing functions depend on current color buffer config:
    */
   if (XMesa->buffer!=XIMAGE) {
      /* Writing to window or back pixmap */
      DD.clear = clear_pixmap;
      switch (XMesa->pixelformat) {
	 case PF_INDEX:
	    DD.write_index_span       = write_span_index_pixmap;
	    DD.write_monoindex_span   = write_span_mono_pixmap;
	    DD.write_index_pixels     = write_pixels_index_pixmap;
	    DD.write_monoindex_pixels = write_pixels_mono_pixmap;
	    break;
	 case PF_TRUECOLOR:
	    DD.write_color_span       = write_span_TRUECOLOR_pixmap;
	    DD.write_monocolor_span   = write_span_mono_pixmap;
	    DD.write_color_pixels     = write_pixels_TRUECOLOR_pixmap;
	    DD.write_monocolor_pixels = write_pixels_mono_pixmap;
	    break;
	 case PF_8A8B8G8R:
	    DD.write_color_span       = write_span_8A8B8G8R_pixmap;
	    DD.write_monocolor_span   = write_span_mono_pixmap;
	    DD.write_color_pixels     = write_pixels_8A8B8G8R_pixmap;
	    DD.write_monocolor_pixels = write_pixels_mono_pixmap;
	    break;
	 case PF_8R8G8B:
	    DD.write_color_span       = write_span_8R8G8B_pixmap;
	    DD.write_monocolor_span   = write_span_mono_pixmap;
	    DD.write_color_pixels     = write_pixels_8R8G8B_pixmap;
	    DD.write_monocolor_pixels = write_pixels_mono_pixmap;
	    break;
	 case PF_5R6G5B:
	    DD.write_color_span       = write_span_5R6G5B_pixmap;
	    DD.write_monocolor_span   = write_span_mono_pixmap;
	    DD.write_color_pixels     = write_pixels_5R6G5B_pixmap;
	    DD.write_monocolor_pixels = write_pixels_mono_pixmap;
	    break;
	 case PF_DITHER:
	    DD.write_color_span       = write_span_DITHER_pixmap;
	    DD.write_monocolor_span   = write_span_mono_DITHER_pixmap;
	    DD.write_color_pixels     = write_pixels_DITHER_pixmap;
	    DD.write_monocolor_pixels = write_pixels_mono_DITHER_pixmap;
	    break;
	 case PF_1BIT:
	    DD.write_color_span       = write_span_1BIT_pixmap;
	    DD.write_monocolor_span   = write_span_mono_1BIT_pixmap;
	    DD.write_color_pixels     = write_pixels_1BIT_pixmap;
	    DD.write_monocolor_pixels = write_pixels_mono_1BIT_pixmap;
	    break;
         case PF_HPCR:
            DD.write_color_span       = write_span_HPCR_pixmap;
            DD.write_monocolor_span   = write_span_mono_pixmap;
            DD.write_color_pixels     = write_pixels_HPCR_pixmap;
            DD.write_monocolor_pixels = write_pixels_mono_pixmap;
            break;
         case PF_LOOKUP:
            DD.write_color_span       = write_span_LOOKUP_pixmap;
            DD.write_monocolor_span   = write_span_mono_pixmap;
            DD.write_color_pixels     = write_pixels_LOOKUP_pixmap;
            DD.write_monocolor_pixels = write_pixels_mono_pixmap;
            break;
         case PF_GRAYSCALE:
            DD.write_color_span       = write_span_GRAYSCALE_pixmap;
            DD.write_monocolor_span   = write_span_mono_pixmap;
            DD.write_color_pixels     = write_pixels_GRAYSCALE_pixmap;
            DD.write_monocolor_pixels = write_pixels_mono_pixmap;
            break;
	 default:
	    die("Bad pixel format in xmesa_setup_DD_pointers (1)");
      }
   }
   else if (XMesa->buffer==XIMAGE) {
      /* Writing to back XImage */
      if (sizeof(GLushort)!=2 || sizeof(GLuint)!=4) {
         /* Do this on Crays */
         DD.clear = clear_nbit_ximage;
      }
      else {
         /* Do this on most machines */
         switch (XMesa->backimage->bits_per_pixel) {
            case 8:
               DD.clear = clear_8bit_ximage;
               break;
            case 16:
               DD.clear = clear_16bit_ximage;
               break;
            case 32:
               DD.clear = clear_32bit_ximage;
               break;
            default:
               DD.clear = clear_nbit_ximage;
               break;
         }
      }
      switch (XMesa->pixelformat) {
	 case PF_INDEX:
	    DD.write_index_span       = write_span_index_ximage;
	    DD.write_monoindex_span   = write_span_mono_ximage;
	    DD.write_index_pixels     = write_pixels_index_ximage;
	    DD.write_monoindex_pixels = write_pixels_mono_ximage;
	    break;
	 case PF_TRUECOLOR:
	    /* Generic RGB */
	    DD.write_color_span       = write_span_TRUECOLOR_ximage;
	    DD.write_monocolor_span   = write_span_mono_ximage;
	    DD.write_color_pixels     = write_pixels_TRUECOLOR_ximage;
	    DD.write_monocolor_pixels = write_pixels_mono_ximage;
	    break;
	 case PF_8A8B8G8R:
	    DD.write_color_span       = write_span_8A8B8G8R_ximage;
	    DD.write_monocolor_span   = write_span_mono_8A8B8G8R_ximage;
	    DD.write_color_pixels     = write_pixels_8A8B8G8R_ximage;
	    DD.write_monocolor_pixels = write_pixels_mono_8A8B8G8R_ximage;
	    break;
	 case PF_8R8G8B:
	    DD.write_color_span       = write_span_8R8G8B_ximage;
	    DD.write_monocolor_span   = write_span_mono_8R8G8B_ximage;
	    DD.write_color_pixels     = write_pixels_8R8G8B_ximage;
	    DD.write_monocolor_pixels = write_pixels_mono_8R8G8B_ximage;
	    break;
	 case PF_5R6G5B:
	    DD.write_color_span       = write_span_5R6G5B_ximage;
	    DD.write_monocolor_span   = write_span_mono_ximage;
	    DD.write_color_pixels     = write_pixels_5R6G5B_ximage;
	    DD.write_monocolor_pixels = write_pixels_mono_ximage;
	    break;
	 case PF_DITHER:
	    if (XMesa->depth==8) {
	       DD.write_color_span       = write_span_DITHER8_ximage;
	       DD.write_monocolor_span   = write_span_mono_DITHER8_ximage;
	       DD.write_color_pixels     = write_pixels_DITHER8_ximage;
	       DD.write_monocolor_pixels = write_pixels_mono_DITHER8_ximage;
	    }
	    else {
	       DD.write_color_span       = write_span_DITHER_ximage;
	       DD.write_monocolor_span   = write_span_mono_DITHER_ximage;
	       DD.write_color_pixels     = write_pixels_DITHER_ximage;
	       DD.write_monocolor_pixels = write_pixels_mono_DITHER_ximage;
	    }
	    break;
	 case PF_1BIT:
	    DD.write_color_span       = write_span_1BIT_ximage;
	    DD.write_monocolor_span   = write_span_mono_1BIT_ximage;
	    DD.write_color_pixels     = write_pixels_1BIT_ximage;
	    DD.write_monocolor_pixels = write_pixels_mono_1BIT_ximage;
	    break;
         case PF_HPCR:
            DD.write_color_span       = write_span_HPCR_ximage;
            DD.write_monocolor_span   = write_span_mono_HPCR_ximage;
            DD.write_color_pixels     = write_pixels_HPCR_ximage;
            DD.write_monocolor_pixels = write_pixels_mono_HPCR_ximage;
            break;
         case PF_LOOKUP:
            DD.write_color_span       = write_span_LOOKUP_ximage;
            DD.write_monocolor_span   = write_span_mono_ximage;
            DD.write_color_pixels     = write_pixels_LOOKUP_ximage;
            DD.write_monocolor_pixels = write_pixels_mono_ximage;
            break;
         case PF_GRAYSCALE:
	    if (XMesa->depth==8) {
	       DD.write_color_span       = write_span_GRAYSCALE8_ximage;
	       DD.write_monocolor_span   = write_span_mono_GRAYSCALE8_ximage;
	       DD.write_color_pixels     = write_pixels_GRAYSCALE8_ximage;
	       DD.write_monocolor_pixels = write_pixels_mono_ximage;
	    }
	    else {
	       DD.write_color_span       = write_span_GRAYSCALE_ximage;
	       DD.write_monocolor_span   = write_span_mono_ximage;
	       DD.write_color_pixels     = write_pixels_GRAYSCALE_ximage;
	       DD.write_monocolor_pixels = write_pixels_mono_ximage;
	    }
	    break;
	 default:
	    die("Bad pixel format in xmesa_setup_DD_pointers (2)");
      }
   }

   /* Pixel/span reading functions: */
   DD.read_index_span = read_index_span;
   DD.read_color_span = read_color_span;
   DD.read_index_pixels = read_index_pixels;
   DD.read_color_pixels = read_color_pixels;
}
