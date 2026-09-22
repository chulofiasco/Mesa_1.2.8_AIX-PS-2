/* $Id: xmesa1.c,v 1.24 1996/05/08 20:23:33 brianp Exp $ */

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
$Log: xmesa1.c,v $
 * Revision 1.24  1996/05/08  20:23:33  brianp
 * added MESA_XSYNC environment variable support
 *
 * Revision 1.23  1996/04/26  16:59:50  brianp
 * new colormap handling, added colormap argument to XMesaBindPixmap()
 *
 * Revision 1.22  1996/04/25  21:09:20  brianp
 * better XAllocColor error handling, prototyped new gl_new_context() func
 *
 * Revision 1.21  1996/03/02  21:41:05  brianp
 * added PF_8R8G8B pixel format
 *
 * Revision 1.20  1996/03/01  20:04:55  brianp
 * test for byte swapping and added a discussion of the subject
 *
 * Revision 1.19  1996/02/26  16:23:09  brianp
 * added PF_5R6G5B pixel format from Joerg Hessdoerfer
 *
 * Revision 1.18  1996/02/23  17:13:37  brianp
 * removed erroneous XImage byte swapping code
 *
 * Revision 1.17  1996/02/19  21:50:48  brianp
 * better XImage->data allocation
 *
 * Revision 1.16  1996/02/09  22:22:39  brianp
 * fixed default gamma value bug
 *
 * Revision 1.15  1996/02/06  03:25:43  brianp
 * added new gamma correction code
 *
 * Revision 1.14  1996/01/22  21:05:04  brianp
 * changed HPCR undithered pixelformat to PF_HPCR, per Alex De Bruyn
 *
 * Revision 1.13  1996/01/19  18:09:27  brianp
 * fixed potential X error handler bug in alloc_shm_back_buffer()
 *
 * Revision 1.12  1996/01/07  22:51:16  brianp
 * re-enable XSync() in glXSwapBuffers()
 *
 * Revision 1.11  1996/01/05  01:19:01  brianp
 * added profiling to XMesaSwapBuffers()
 *
 * Revision 1.10  1995/12/21  17:58:13  brianp
 * fixed the call to get_drawable_size in case of 8-byte integers
 *
 * Revision 1.9  1995/12/12  21:44:04  brianp
 * only print color warning message if MESA_DEBUG is defined
 *
 * Revision 1.8  1995/11/30  00:52:30  brianp
 * fixed a few type warnings for Sun compiler
 *
 * Revision 1.7  1995/11/30  00:20:56  brianp
 * added new PF_GRAYSCALE mode
 *
 * Revision 1.6  1995/11/08  22:08:50  brianp
 * added 8-bit TrueColor dithering
 *
 * Revision 1.5  1995/11/03  17:41:48  brianp
 * removed unused vars, fixed code for C++ compilation
 *
 * Revision 1.4  1995/11/01  15:14:03  brianp
 * renamed all class variables per Steven Spitz
 *
 * Revision 1.3  1995/10/30  15:13:01  brianp
 * replaced Current variable with XMesa
 *
 * Revision 1.2  1995/10/19  15:53:20  brianp
 * new arguments to gl_new_context()
 *
 * Revision 1.1  1995/10/17  21:36:55  brianp
 * Initial revision
 *
 */



/*
 * Mesa/X11 interface, part 1.
 *
 * This file contains the implementations of all the XMesa* functions.
 *
 *
 * NOTES:
 *
 * The window coordinate system origin (0,0) is in the lower-left corner
 * of the window.  X11's window coordinate origin is in the upper-left
 * corner of the window.  Therefore, most drawing functions in this
 * file have to flip Y coordinates.
 *
 * Define SHM in the Makefile with -DSHM if you want to compile in support
 * for the MIT Shared Memory extension.  If enabled, when you use an Ximage
 * for the back buffer in double buffered mode, the "swap" operation will
 * be faster.  You must also link with -lXext.
 *
 * Byte swapping:  If the Mesa host and the X display use a different
 * byte order then there's some trickiness to be aware of when using
 * XImages.  The byte ordering used for the XImage is that of the X
 * display, not the Mesa host.
 * The color-to-pixel encoding for True/DirectColor must be done
 * according to the display's visual red_mask, green_mask, and blue_mask.
 * If XPutPixel is used to put a pixel into an XImage then XPutPixel will
 * do byte swapping if needed.  If one wants to directly "poke" the pixel
 * into the XImage's buffer then the pixel must be byte swapped first.  In
 * Mesa, when byte swapping is needed we use the PF_TRUECOLOR pixel format
 * and use XPutPixel everywhere except in the implementation of
 * glClear(GL_COLOR_BUFFER_BIT).  We want this function to be fast so
 * instead of using XPutPixel we "poke" our values after byte-swapping
 * the clear pixel value if needed.
 *
 */


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef SHM
#  include <sys/ipc.h>
#  include <sys/shm.h>
#  include <X11/extensions/XShm.h>
#endif
#include "GL/xmesa.h"
#include "xmesaP.h"
#include "context.h"
/*#include "dd.h"*/
#include "macros.h"
#include "xform.h"



XMesaContext XMesa = NULL;


/*
 * Lookup tables for HPCR pixel format:
 */
short hpcr_rTbl[256] = {
 16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  23,
 24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31,
 32,  32,  33,  33,  34,  34,  35,  35,  36,  36,  37,  37,  38,  38,  39,  39,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239
};

short hpcr_gTbl[256] = {
 16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  23,
 24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31,
 32,  32,  33,  33,  34,  34,  35,  35,  36,  36,  37,  37,  38,  38,  39,  39,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239
};

short hpcr_bTbl[256] = {
 32,  32,  33,  33,  34,  34,  35,  35,  36,  36,  37,  37,  38,  38,  39,  39,
 40,  40,  41,  41,  42,  42,  43,  43,  44,  44,  45,  45,  46,  46,  47,  47,
 48,  48,  49,  49,  50,  50,  51,  51,  52,  52,  53,  53,  54,  54,  55,  55,
 56,  56,  57,  57,  58,  58,  59,  59,  60,  60,  61,  61,  62,  62,  63,  63,
 64,  64,  65,  65,  66,  66,  67,  67,  68,  68,  69,  69,  70,  70,  71,  71,
 72,  72,  73,  73,  74,  74,  75,  75,  76,  76,  77,  77,  78,  78,  79,  79,
 80,  80,  81,  81,  82,  82,  83,  83,  84,  84,  85,  85,  86,  86,  87,  87,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223
};



/**********************************************************************/
/*****                      Private Functions                     *****/
/**********************************************************************/


/*
 * X/Mesa Error reporting function:
 */
static void error( const char *msg )
{
   fprintf( stderr, "X/Mesa error: %s\n", msg );
}


/*
 * Return the host's byte order as LSBFirst or MSBFirst ala X.
 */
static int host_byte_order( void )
{
   int i = 1;
   char *cptr = (char *) &i;
   return (*cptr==1) ? LSBFirst : MSBFirst;
}



/*
 * Error handling.
 */
static int mesaXErrorFlag = 0;

static int mesaHandleXError( Display *dpy, XErrorEvent *event )
{
    mesaXErrorFlag = 1;
    return 0;
}


/*
 * Check if the X Shared Memory extension is available.
 * Return:  0 = not available
 *          1 = shared XImage support available
 *          2 = shared Pixmap support available also
 */
static int check_for_xshm( Display *display )
{
#ifdef SHM
   int major, minor, ignore;
   Bool pixmaps;

   if (XQueryExtension( display, "MIT-SHM", &ignore, &ignore, &ignore )) {
      if (XShmQueryVersion( display, &major, &minor, &pixmaps )==True) {
	 return (pixmaps==True) ? 2 : 1;
      }
      else {
	 return 0;
      }
   }
   else {
      return 0;
   }
#else
   /* Can't compile XSHM support */
   return 0;
#endif
}


/*
 * Allocate a shared memory XImage back buffer for the given context.
 * Return:  GL_TRUE if success, GL_FALSE if error
 */
static GLboolean alloc_shm_back_buffer( XMesaContext c )
{
#ifdef SHM
   /*
    * We have to do a _lot_ of error checking here to be sure we can
    * really use the XSHM extension.  It seems different servers trigger
    * errors at different points if the extension won't work.  Therefore
    * we have to be very careful...
    */
   GC gc;
   int (*old_handler)( Display *, XErrorEvent * );

   c->backimage = XShmCreateImage( c->display, c->visual, c->depth,
				   ZPixmap, NULL, &c->shminfo,
				   c->width, c->height );
   if (c->backimage == NULL) {
      error( "alloc_back_buffer: Shared memory error (XShmCreateImage), disabling." );
      c->shm = 0;
      return GL_FALSE;
   }

   c->shminfo.shmid = shmget( IPC_PRIVATE, c->backimage->bytes_per_line
			     * c->backimage->height, IPC_CREAT|0777 );
   if (c->shminfo.shmid < 0) {
      perror("alloc_back_buffer");
      XDestroyImage( c->backimage );
      c->backimage = NULL;
      error( "alloc_back_buffer: Shared memory error (shmget), disabling." );
      c->shm = 0;
      return GL_FALSE;
   }

   c->shminfo.shmaddr = c->backimage->data
                      = (char*)shmat( c->shminfo.shmid, 0, 0 );
   if (c->shminfo.shmaddr == (char *) -1) {
      perror("alloc_back_buffer");
      XDestroyImage( c->backimage );
      c->backimage = NULL;
      error("alloc_back_buffer: Shared memory error (shmat), disabling.");
      c->shm = 0;
      return GL_FALSE;
   }

   c->shminfo.readOnly = False;
   mesaXErrorFlag = 0;
   old_handler = XSetErrorHandler( mesaHandleXError );
   /* This may trigger the X protocol error we're ready to catch: */
   XShmAttach( c->display, &c->shminfo );
   XSync( c->display, False );

   if (mesaXErrorFlag) {
      /* we are on a remote display, this error is normal, don't print it */
      XFlush( c->display );
      mesaXErrorFlag = 0;
      XDestroyImage( c->backimage );
      shmdt( c->shminfo.shmaddr );
      shmctl( c->shminfo.shmid, IPC_RMID, 0 );
      c->backimage = NULL;
      c->shm = 0;
      (void) XSetErrorHandler( old_handler );
      return GL_FALSE;
   }

   shmctl( c->shminfo.shmid, IPC_RMID, 0 ); /* nobody else needs it */

   /* Finally, try an XShmPutImage to be really sure the extension works */
   gc = XCreateGC( c->display, c->frontbuffer, 0, NULL );
   XShmPutImage( c->display, c->frontbuffer, gc,
		 c->backimage, 0, 0, 0, 0, 1, 1 /*one pixel*/, False );
   XSync( c->display, False );
   XFreeGC( c->display, gc );
   (void) XSetErrorHandler( old_handler );
   if (mesaXErrorFlag) {
      XFlush( c->display );
      mesaXErrorFlag = 0;
      XDestroyImage( c->backimage );
      shmdt( c->shminfo.shmaddr );
      shmctl( c->shminfo.shmid, IPC_RMID, 0 );
      c->backimage = NULL;
      c->shm = 0;
      return GL_FALSE;
   }

   return GL_TRUE;
#else
   /* Can't compile XSHM support */
   return GL_FALSE;
#endif
}



/*
 * Setup an off-screen pixmap or Ximage to use as the back buffer.
 * Input:  c - the X/Mesa context.
 */
void xmesa_alloc_back_buffer( XMesaContext c )
{
   if (c->db_state==BACK_XIMAGE) {
      /* Deallocate the old backimage, if any */
      if (c->backimage) {
#ifdef SHM
	 if (c->shm) {
	    XShmDetach( c->display, &c->shminfo );
	    XDestroyImage( c->backimage );
	    shmdt( c->shminfo.shmaddr );
	 }
	 else
#endif
	   XDestroyImage( c->backimage );
	 c->backimage = NULL;
      }

      /* Allocate new back buffer */
      if (c->shm==0 || alloc_shm_back_buffer(c)==GL_FALSE) {
	 /* Allocate a regular XImage for the back buffer. */
	 c->backimage = XCreateImage( c->display, c->visual, c->depth,
				      ZPixmap, 0,   /* format, offset */
				      NULL, c->width, c->height,
				      8, 0 );  /* pad, bytes_per_line */
	 if (!c->backimage) {
	    error("alloc_back_buffer: XCreateImage failed.");
	 }
         c->backimage->data = malloc( c->backimage->height
                                      * c->backimage->bytes_per_line );
         if (!c->backimage->data) {
            error("alloc_back_buffer: malloc failed.");
            XDestroyImage( c->backimage );
            c->backimage = NULL;
         }
      }
      c->backpixmap = None;
   }
   else if (c->db_state==BACK_PIXMAP) {
      Pixmap old_pixmap = c->backpixmap;
      /* Free the old back pixmap */
      if (c->backpixmap) {
	 XFreePixmap( c->display, c->backpixmap );
      }
      /* Allocate new back pixmap */
      c->backpixmap = XCreatePixmap( c->display, c->frontbuffer,
				     c->width, c->height, c->depth );
      c->backimage = NULL;
      /* update other references to backpixmap */
      if (c->buffer==old_pixmap) {
	 c->buffer = c->backpixmap;
      }
   }
}



/*
 * Return the width and height of the given drawable.
 */
static void get_drawable_size( Display *dpy, Drawable d,
			       unsigned int *width, unsigned int *height )
{
   Window root;
   int x, y;
   unsigned int bw, depth;

   XGetGeometry( dpy, d, &root, &x, &y, width, height, &bw, &depth );
}



/*
 * Apply gamma correction to an intensity value in [0..max].  Return the
 * new intensity value.
 */
static GLint gamma_adjust( GLfloat gamma, GLint value, GLint max )
{
   double x = (double) value / (double) max;
   return (GLint) ((GLfloat) max * pow( x, 1.0F/gamma ) );
}



/*
 * A replacement for XAllocColor.  This function should never
 * fail to allocate a color.  When XAllocColor fails, we return
 * the nearest matching color.  If we have to allocate many colors
 * this function isn't too efficient; the XQueryColors() could be
 * done just once.
 * Written by Michael Pichler, Brian Paul, Mark Kilgard
 * Input:  dpy - X display
 *         cmap - X colormap
 *         cmapSize - size of colormap
 * In/Out: color - the XColor struct
 * Output:  exact - 1=exact color match, 0=closest match
 */
static void
noFaultXAllocColor( Display *dpy, Colormap cmap, int cmapSize,
                    XColor *color, int *exact )
{
  XColor *ctable, subColor;
  int i, bestmatch;
  double mindist;       /* 3*2^16^2 exceeds long int precision. 
                         */

  for (;;) {
    /* First try just using XAllocColor. */
    if (XAllocColor(dpy, cmap, color)) {
       *exact = 1;
       return;
    }

    /* Retrieve color table entries. */
    /* XXX alloca candidate. */
    ctable = (XColor *) malloc(cmapSize * sizeof(XColor));
    for (i = 0; i < cmapSize; i++)
      ctable[i].pixel = i;
    XQueryColors(dpy, cmap, ctable, cmapSize);

    /* Find best match. */
    bestmatch = -1;
    mindist = 0.0;
    for (i = 0; i < cmapSize; i++) {
      double dr = (double) color->red - (double) ctable[i].red;
      double dg = (double) color->green - (double) ctable[i].green;
      double db = (double) color->blue - (double) ctable[i].blue;
      double dist = dr * dr + dg * dg + db * db;
      if (bestmatch < 0 || dist < mindist) {
        bestmatch = i;
        mindist = dist;
      }
    }

    /* Return result. */
    subColor.red = ctable[bestmatch].red;
    subColor.green = ctable[bestmatch].green;
    subColor.blue = ctable[bestmatch].blue;
    free(ctable);
    if (XAllocColor(dpy, cmap, &subColor)) {
      *color = subColor;
      *exact = 0;
      return;
    }
    /* Extremely unlikely, but possibly color was deallocated
       and reallocated by someone else before we could
       XAllocColor the color cell we located.  If so, loop
       again... */
  }
}



/*
 * Do setup for PF_GRAYSCALE pixel format.
 */
static int setup_grayscale( XMesaContext c, Window window, Colormap cmap )
{
   int gray, i;
   int colorsfailed = 0;
   XColor xcol;
   XColor *ctable = NULL;

   if (c->depth<4 || c->depth>16 || !cmap) {
      return 0;
   }

   /* Allocate 256 shades of gray */
   for (gray=0;gray<256;gray++) {
      GLint r = gamma_adjust( c->RedGamma,   gray, 255 );
      GLint g = gamma_adjust( c->GreenGamma, gray, 255 );
      GLint b = gamma_adjust( c->BlueGamma,  gray, 255 );
      int exact;

      xcol.red   = (r << 8) | r;
      xcol.green = (g << 8) | g;
      xcol.blue  = (b << 8) | b;

      noFaultXAllocColor( c->display, cmap, c->visual->map_entries,
                          &xcol, &exact );
      if (!exact) {
         colorsfailed++;
      }

      c->color_table[gray] = xcol.pixel;
      c->pixel_to_r[xcol.pixel] = gray * 30 / 100;
      c->pixel_to_g[xcol.pixel] = gray * 59 / 100;
      c->pixel_to_b[xcol.pixel] = gray * 11 / 100;
   }

   if (colorsfailed && getenv("MESA_DEBUG")) {
      fprintf( stderr,
               "Note: %d out of 256 needed colors do not match exactly.\n",
	       colorsfailed, 256 );
   }

#define WEIGHT
#ifdef WEIGHT
   c->rmult = 30 * 255 / 100;
   c->gmult = 59 * 255 / 100;
   c->bmult = 11 * 255 / 100;
#else
   c->rmult = 255/3;
   c->gmult = 255/3;
   c->bmult = 255/3;
#endif
   c->dithered_pf = PF_GRAYSCALE;
   c->undithered_pf = PF_GRAYSCALE;
   c->pixel = c->color_table[255];      /* white */
   c->clearpixel = c->color_table[0];   /* black */

   return 1;
}



/*
 * Setup RGB rendering for a window with a PseudoColor, StaticColor,
 * or 8-bit TrueColor visual visual.  We try to allocate a palette of 225
 * colors (5 red, 9 green, 5 blue) and dither to approximate a 24-bit RGB
 * color.  While this function was originally designed just for 8-bit
 * visuals, it has also proven to work from 4-bit up to 16-bit visuals.
 * Dithering code contributed by Bob Mercier.
 */
static int setup_dithered_color( XMesaContext c, Window window, Colormap cmap )
{
   int r, g, b, i;
   int colorsfailed = 0;
   XColor xcol;
   XColor *allcolors = NULL, *acptr; 

   if (c->depth<4 || c->depth>16 || !cmap) {
      return 0;
   }

   /* Allocate X colors and initialize color_table[], red_table[], etc */
   for (r = 0; r < _R; r++) {
      for (g = 0; g < _G; g++) {
	 for (b = 0; b < _B; b++) {
            int exact;

	    xcol.red   = gamma_adjust( c->RedGamma,   r*65535/(_R-1), 65535 );
	    xcol.green = gamma_adjust( c->GreenGamma, g*65535/(_G-1), 65535 );
	    xcol.blue  = gamma_adjust( c->BlueGamma,  b*65535/(_B-1), 65535 );

            noFaultXAllocColor( c->display, cmap, c->visual->map_entries,
                                &xcol, &exact );

            if (!exact) {
               colorsfailed++;
            }

	    i = _MIX( r, g, b );
	    c->color_table[i] = xcol.pixel;
	    c->pixel_to_r[xcol.pixel] = r * 255 / (_R-1);
	    c->pixel_to_g[xcol.pixel] = g * 255 / (_G-1);
	    c->pixel_to_b[xcol.pixel] = b * 255 / (_B-1);
	 }
      }
   }

   if (colorsfailed && getenv("MESA_DEBUG")) {
      fprintf( stderr,
               "Note: %d out of %d needed colors do not match exactly.\n",
	       colorsfailed, _R*_G*_B );
   }

   c->rmult = 255;
   c->gmult = 255;
   c->bmult = 255;
   c->dithered_pf = PF_DITHER;
   c->undithered_pf = PF_LOOKUP;
   c->pixel = c->color_table[_MIX(_R-1,_G-1,_B-1)];  /* white */
   c->clearpixel = c->color_table[_MIX(0,0,0)];      /* black */

   return 1;
}



/*
 * Setup RGB rendering for a window with a True/DirectColor visual.
 */
static int setup_truecolor( XMesaContext c, Window window, Colormap cmap )
{
   unsigned long rmask, gmask, bmask;

   /* Red */
   c->rshift = 0;
   rmask = c->visual->red_mask;
   while ((rmask & 1)==0) {
      c->rshift++;
      rmask = rmask >> 1;
   }
   c->rmult = (GLint) rmask;

   /* Green */
   c->gshift = 0;
   gmask = c->visual->green_mask;
   while ((gmask & 1)==0) {
      c->gshift++;
      gmask = gmask >> 1;
   }
   c->gmult = (GLint) gmask;

   /* Blue */
   c->bshift = 0;
   bmask = c->visual->blue_mask;
   while ((bmask & 1)==0) {
      c->bshift++;
      bmask = bmask >> 1;
   }
   c->bmult = (GLint) bmask;

   if (   c->visual->red_mask  ==0x0000ff
       && c->visual->green_mask==0x00ff00
       && c->visual->blue_mask ==0xff0000
       && host_byte_order()==ImageByteOrder(c->display) && sizeof(GLuint)==4
       && c->RedGamma==1.0 && c->GreenGamma==1.0 && c->BlueGamma==1.0) {
      /* common 24-bit config used on SGI, Sun */
      c->undithered_pf = c->dithered_pf = PF_8A8B8G8R;
      c->pixel = 0xffffffff;  /* white */
      c->clearpixel = 0x0;    /* black */
   }
   else if (c->visual->red_mask  ==0xff0000
       &&   c->visual->green_mask==0x00ff00
       &&   c->visual->blue_mask ==0x0000ff
       && host_byte_order()==ImageByteOrder(c->display) && sizeof(GLuint)==4
       && c->RedGamma==1.0 && c->GreenGamma==1.0 && c->BlueGamma==1.0) {
      /* common 24-bit config used on Linux, HP, IBM */
      c->undithered_pf = c->dithered_pf = PF_8R8G8B;
      c->pixel = 0xffffff;    /* white */
      c->clearpixel = 0x0;    /* black */
   }
   else if (c->visual->red_mask  ==0xf800
       &&   c->visual->green_mask==0x07e0
       &&   c->visual->blue_mask ==0x001f
       && host_byte_order()==ImageByteOrder(c->display) && sizeof(GLushort)==2
       && c->RedGamma==1.0 && c->GreenGamma==1.0 && c->BlueGamma==1.0) {
      /* 5-6-5 color weight on common PC VGA boards */
      c->undithered_pf = c->dithered_pf = PF_5R6G5B;
      c->pixel = 0xffff;      /* white */
      c->clearpixel = 0x0;    /* black */
   }
   else if (c->visual->red_mask  ==0xe0
       &&   c->visual->green_mask==0x1c
       &&   c->visual->blue_mask ==0x03
       && XInternAtom(c->display, "_HP_RGB_SMOOTH_MAP_LIST", True)) {
      /* HP Color Recovery
       * Contributed by:  Alex De Bruyn (ad@lms.be)
       * To work properly, the atom _HP_RGB_SMOOTH_MAP_LIST must be defined
       * on the root window AND the colormap obtainable by XGetRGBColormaps
       * for that atom must be set on the window.  (see also tkInitWindow)
       * If that colormap is not set, the output will look stripy.
       */

      /* Setup color tables with gamma correction */
      int i;
      double g = 1.0 / c->RedGamma; 
      for (i=0; i<256; i++) { 
         hpcr_rTbl[i] = 255.0 * pow( hpcr_rTbl[i]/255.0, g ) + 0.5;
         if (hpcr_rTbl[i] < 16)
            hpcr_rTbl[i] = 16;
         else if (hpcr_rTbl[i] >= 240)
            hpcr_rTbl[i] = 239;
      }
      g = 1.0 / c->GreenGamma;
      for (i=0; i<256; i++) {
         hpcr_gTbl[i] = 255.0 * pow( hpcr_gTbl[i]/255.0, g ) + 0.5;
         if (hpcr_gTbl[i] < 16)
            hpcr_gTbl[i] = 16;
         else if (hpcr_gTbl[i] >= 240)
            hpcr_gTbl[i] = 239;
      }
      g = 1.0 / c->BlueGamma;
      for (i=0; i<256; i++) {
         hpcr_bTbl[i] = 255.0 * pow( hpcr_bTbl[i]/255.0, g ) + 0.5;
         if (hpcr_bTbl[i] < 32)
            hpcr_bTbl[i] = 32;
         else if (hpcr_bTbl[i] >= 224)
            hpcr_bTbl[i] = 223;
      }
      c->undithered_pf = PF_HPCR;  /* can't really disable dithering for now */
      c->dithered_pf = PF_HPCR;
      c->pixel = c->visual->red_mask | c->visual->green_mask
               | c->visual->blue_mask;  /* white */
      c->clearpixel = 0x0;    /* black */
      c->rmult = 255;
      c->gmult = 255;
      c->bmult = 255;
   }
   else if (c->depth==8) {
      /* dither if 8-bit */
      return setup_dithered_color( c, window, cmap );
   }
   else {
      /* general case (12-bit TrueColor for example) */
      GLint i;
      /* setup r,g,btable[] arrays with gamma correction */
      for (i=0;i<=c->rmult;i++) {
         c->r_to_pixel[i] = gamma_adjust(c->RedGamma,   i, c->rmult) << c->rshift;
      }
      for (i=0;i<=c->gmult;i++) {
         c->g_to_pixel[i] = gamma_adjust(c->GreenGamma, i, c->gmult) << c->gshift;
      }
      for (i=0;i<=c->bmult;i++) {
         c->b_to_pixel[i] = gamma_adjust(c->BlueGamma,  i, c->bmult) << c->bshift;
      }
      c->undithered_pf = c->dithered_pf = PF_TRUECOLOR;
      c->pixel = c->visual->red_mask | c->visual->green_mask
	         | c->visual->blue_mask;  /* white */
      c->clearpixel = 0x0;    /* black */
   }

   return 1;
}



/*
 * Setup RGB rendering for a window with a monochrome visual.
 */
static int setup_monochrome( XMesaContext c )
{
   /* Simulate RGB in monochrome */
   c->rmult = 255;
   c->gmult = 255;
   c->bmult = 255;
   c->dithered_pf = c->undithered_pf = PF_1BIT;
   c->pixel      = WhitePixel( c->display, c->scrnum );  /* white */
   c->clearpixel = BlackPixel( c->display, c->scrnum );  /* black */
   return 1;
}




/*
 * When a context is "made current" for the first time, we can finally
 * finish initializing the context.
 * Input:  c - the XMesaContext to initialize
 *         window - the window/pixmap we're rendering into
 *         cmap - the colormap associated with the window/pixmap
 * Return:  GL_TRUE=success, GL_FALSE=failure
 */
static GLboolean initialize_context( XMesaContext c, Window window,
                                     Colormap cmap )
{
   XGCValues gcvalues;
   char *gamma;

   assert( c->initialized==GL_FALSE );

   /* check for MESA_GAMMA environment variable */
   gamma = getenv("MESA_GAMMA");
   if (gamma) {
      c->RedGamma = c->GreenGamma = c->BlueGamma = 0.0;
      sscanf( gamma, "%f %f %f", &c->RedGamma, &c->GreenGamma, &c->BlueGamma );
      if (c->RedGamma<=0.0)    c->RedGamma = 1.0;
      if (c->GreenGamma<=0.0)  c->GreenGamma = c->RedGamma;
      if (c->BlueGamma<=0.0)   c->BlueGamma = c->RedGamma;
   }
   else {
      c->RedGamma = c->GreenGamma = c->BlueGamma = 1.0;
   }

   if (c->rgb_flag==GL_FALSE) {
      /* COLOR-INDEXED WINDOW:
       * Even if the visual is TrueColor or DirectColor we treat it as
       * being color indexed.  This is weird but might be useful to someone.
       */
      c->dithered_pf = c->undithered_pf = PF_INDEX;
      c->pixel = 1;
      c->clearpixel = 0;
      c->rmult = c->gmult = c->bmult = 0;
   }
   else {
      /* RGB WINDOW:
       * If the visual is TrueColor or DirectColor we're all set.  Other-
       * wise, we simulate RGB mode using a color-mapped window.
       */
      int xclass;
#if defined(__cplusplus) || defined(c_plusplus)
      xclass = c->visual->c_class;
#else
      xclass = c->visual->class;
#endif
      if (xclass==TrueColor || xclass==DirectColor) {
	 if (!setup_truecolor( c, window, cmap )) {
            return GL_FALSE;
         }
      }
      else if (xclass==StaticGray && c->depth==1) {
	 if (!setup_monochrome( c )) {
            return GL_FALSE;
         }
      }
      else if (xclass==GrayScale || xclass==StaticGray) {
         if (!setup_grayscale( c, window, cmap )) {
            return GL_FALSE;
         }
      }
      else if ((xclass==PseudoColor || xclass==StaticColor)
               && c->depth>=4 && c->depth<=16) {
	 if (!setup_dithered_color( c, window, cmap )) {
            return GL_FALSE;
         }
      }
      else {
	 error("XMesaCreateContext: can't simulate RGB mode with given visual.");
	 return GL_FALSE;
      }
   }

   /* Now allocate the GL context */
   c->gl_ctx = gl_new_context( c->rgb_flag,
                               (GLfloat) c->rmult,
                               (GLfloat) c->gmult,
                               (GLfloat) c->bmult,
                               255.0, /*alpha_scale*/
                               c->db_state ? GL_TRUE : GL_FALSE,
                               c->share_list ? c->share_list->gl_ctx : NULL );
#ifdef FUTURE
   c->gl_ctx = gl_new_context( c->rgb_flag,
                               c->alpha_flag,
                               c->db_state ? GL_TRUE : GL_FALSE,
                               c->depth_size,
                               c->stencil_size,
                               c->accum_size,
                               (GLfloat) c->rmult,
                               (GLfloat) c->gmult,
                               (GLfloat) c->bmult,
                               (GLfloat) 255.0,
                               c->share_list ? c->share_list->gl_ctx : NULL );
#endif

   /* Dithering is enabled by default */
   c->pixelformat = c->dithered_pf;

   {
      unsigned int w, h;
      get_drawable_size( c->display, window, &w, &h );
      c->width = w;
      c->height = h;
   }

   if (c->db_state) {
      /* Double buffered */
      xmesa_alloc_back_buffer( c );
      if (c->db_state==BACK_PIXMAP) {
         c->buffer = c->backpixmap;
      }
      else {
         c->buffer = XIMAGE;
      }
   }
   else {
      /* Single Buffered */
      c->buffer = c->frontbuffer;
   }

   /* X11 graphics context */
   c->gc1 = XCreateGC( c->display, window, 0, NULL );
   XSetForeground( c->display, c->gc1, c->pixel );
   XSetBackground( c->display, c->gc1, c->pixel );
   XSetFunction( c->display, c->gc1, GXcopy );
   c->gc2 = XCreateGC( c->display, window, 0, NULL );
   XSetForeground( c->display, c->gc2, c->pixel );
   XSetBackground( c->display, c->gc2, c->pixel );
   XSetFunction( c->display, c->gc2, GXcopy );
   /*
    * Don't generate Graphics Expose/NoExpose events in swapbuffers().
    * Patch contributed by Michael Pichler May 15, 1995.
    */
   gcvalues.graphics_exposures = False;
   c->cleargc = XCreateGC( c->display, window, GCGraphicsExposures, &gcvalues);
   XSetForeground( c->display, c->cleargc, c->clearpixel );
   XSetBackground( c->display, c->cleargc, c->clearpixel );
   XSetFunction( c->display, c->cleargc, GXcopy );

   /* Initialize the row buffer XImage for use in write_color_span() */
   c->rowimage = XCreateImage( c->display, c->visual, c->depth,
                               ZPixmap, 0,                  /*format, offset*/
                               (char*) malloc(MAX_WIDTH*4), /*data*/
                               MAX_WIDTH, 1,                /*width, height*/
                               32,                          /*bitmap_pad*/
                               0                         /*bytes_per_line*/ );


   c->initialized = GL_TRUE;

   return GL_TRUE;
}




/**********************************************************************/
/*****                       Public Functions                     *****/
/**********************************************************************/


/*
 * Create a new XMesaContext for rendering into an X11 window.
 *
 * Input:  display - X11 display
 *         visinfo - X11 VisualInfo describing the colorbuffer we'll use
 *         rgb_flag - GL_TRUE = RGB mode,
 *                    GL_FALSE = color index mode
 *         db_flag - GL_TRUE = double-buffered,
 *                   GL_FALSE = single buffered
 *         ximage_flag - GL_TRUE = use an XImage for back buffer,
 *                       GL_FALSE = use an off-screen pixmap for back buffer
 *         share_list - another XMesaContext with which to share display
 *                      lists or NULL if no sharing is wanted.
 * Return:  an XMesaContext or NULL if error.
 */
XMesaContext XMesaCreateContext( Display *display,
				 XVisualInfo *visinfo,
				 GLboolean rgb_flag,
                                 GLboolean alpha_flag,
				 GLboolean db_flag,
                                 GLint depth_size,
                                 GLint stencil_size,
                                 GLint accum_size,
				 GLboolean ximage_flag,
				 XMesaContext share_list )
{
   XMesaContext c;

   /* allocate xmesa_context struct initialized to zeros */
   c = (struct xmesa_context *) calloc( 1, sizeof(struct xmesa_context) );
   if (!c) {
      return NULL;
   }

   c->rgb_flag = rgb_flag;
   c->share_list = share_list;

   if (host_byte_order()==ImageByteOrder(display)) {
      c->swapbytes = GL_FALSE;
   }
   else {
      c->swapbytes = GL_TRUE;
   }

   /* X stuff */
   c->display = display;
   c->visual = visinfo->visual;
   c->scrnum = visinfo->screen;
   c->depth = visinfo->depth;
   c->shm = check_for_xshm( display );

   /* Double buffering configuration */
   if (db_flag) {
      if (ximage_flag) {
	 c->db_state = BACK_XIMAGE;
      }
      else {
	 c->db_state = BACK_PIXMAP;
      }
   }
   else {
      c->db_state = 0;
   }

   if (getenv("MESA_XSYNC")) {
      XSynchronize( display, 1 );    /* This makes debugging X easier */
   }

   return c;
}




void XMesaDestroyContext( XMesaContext c )
{
   if (c->gl_ctx)  gl_destroy_context( c->gl_ctx );

   if (c->gc1)  XFreeGC( c->display, c->gc1 );
   if (c->gc2)  XFreeGC( c->display, c->gc2 );
   if (c->cleargc)  XFreeGC( c->display, c->cleargc );

   if (c->backimage) {
#ifdef SHM
       if (c->shm) {
	   XShmDetach( c->display, &c->shminfo );
	   XDestroyImage( c->backimage );
	   shmdt( c->shminfo.shmaddr );
       }
       else
#endif
	   XDestroyImage( c->backimage );
   }
   if (c->backpixmap) {
      XFreePixmap( c->display, c->backpixmap );
   }
   if (c->rowimage) {
      free( c->rowimage->data );
      c->rowimage->data = NULL;
      XDestroyImage( c->rowimage );
   }
   free( c );
}



/*
 * Bind an X/Mesa context to an X window.
 * Input:  c - the XMesaContext
 *         W - the window
 * Return:  GL_TRUE=success, GL_FALSE=failure
 */
GLboolean XMesaBindWindow( XMesaContext c, Window w )
{
   if (c->buffer==c->frontbuffer) {
      c->buffer = w;
   }
   c->frontbuffer = w;
   if (!c->initialized) {
      XWindowAttributes attr;
      XGetWindowAttributes( c->display, w, &attr );
      if (!initialize_context( c, w, attr.colormap )) {
	 return GL_FALSE;
      }
   }
   return GL_TRUE;
}



/*
 * Bind an X/Mesa context to an X pixmap.
 * Input:  c - the XMesaContext
 *         p - the pixmap
 *         cmap - the colormap, may be 0 if using a TrueColor or DirectColor
 *                visual for the pixmap
 * Return:  GL_TRUE=success, GL_FALSE=failure
 */
GLboolean XMesaBindPixmap( XMesaContext c, Pixmap p, Colormap cmap )
{
   if (c->buffer==c->frontbuffer) {
      c->buffer = p;
   }
   c->frontbuffer = p;

   if (!c->initialized) {
      if (!initialize_context( c, p, cmap )) {
	 return GL_FALSE;
      }
   }
   return GL_TRUE;
}



GLboolean XMesaMakeCurrent( XMesaContext c )
{
   if (c) {
      if (!c->initialized) {
         return GL_FALSE;
      }
      gl_set_context( c->gl_ctx );
      XMesa = c;

      xmesa_setup_DD_pointers();

      if (XMesa->gl_ctx->Viewport.Width==0) {
	 /* initialize viewport to window size */
	 gl_viewport( 0, 0, XMesa->width, XMesa->height );
	 CC.Scissor.Width = XMesa->width;
	 CC.Scissor.Height = XMesa->height;
      }
   }
   else {
      /* Detach */
      XMesa = NULL;
   }
   return GL_TRUE;
}



XMesaContext XMesaGetCurrentContext( void )
{
   return XMesa;
}



/*
 * Copy the back buffer to the front buffer.  If there's no back buffer
 * this is a no-op.
 */
void XMesaSwapBuffers( void )
{
#ifdef PROFILE
   GLdouble t0 = gl_time();
#endif
   if (XMesa->db_state) {
      if (XMesa->backimage) {
	 /* Copy Ximage from host's memory to server's window */
#ifdef SHM
	 if (XMesa->shm) {
	    XShmPutImage( XMesa->display, XMesa->frontbuffer,
			  XMesa->cleargc,
			  XMesa->backimage, 0, 0,
			  0, 0, XMesa->width, XMesa->height, False );
	    /* wait for finished event??? */
	 }
	 else
#endif
         {
            XPutImage( XMesa->display, XMesa->frontbuffer,
                       XMesa->cleargc,
                       XMesa->backimage, 0, 0,
                       0, 0, XMesa->width, XMesa->height );
         }
      }
      else {
	 /* Copy pixmap to window on server */
	 XCopyArea( XMesa->display,
		    XMesa->backpixmap,   /* source drawable */
		    XMesa->frontbuffer,  /* dest. drawable */
		    XMesa->cleargc,
		    0, 0, XMesa->width, XMesa->height,  /* source region */
		    0, 0                 /* dest region */
		   );
      }
   }
   XSync( XMesa->display, False );
#ifdef PROFILE
   CC.SwapCount++;
   CC.SwapTime += gl_time() - t0;
#endif
}



/*
 * Return a pointer to the XMesa backbuffer Pixmap or XImage.  This function
 * is a way to get "under the hood" of X/Mesa so one can manipulate the
 * back buffer directly.
 * Output:  pixmap - pointer to back buffer's Pixmap, or 0
 *          ximage - pointer to back buffer's XImage, or NULL
 * Return:  GL_TRUE = context is double buffered
 *          GL_FALSE = context is single buffered
 */
GLboolean XMesaGetBackBuffer( Pixmap *pixmap, XImage **ximage )
{
   if (XMesa->db_state) {
      if (pixmap)  *pixmap = XMesa->backpixmap;
      if (ximage)  *ximage = XMesa->backimage;
      return GL_TRUE;
   }
   else {
      *pixmap = 0;
      *ximage = NULL;
      return GL_FALSE;
   }
}
