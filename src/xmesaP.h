/* $Id: xmesaP.h,v 1.28 1996/04/25 20:56:58 brianp Exp $ */

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
$Log: xmesaP.h,v $
 * Revision 1.28  1996/04/25  20:56:58  brianp
 * added depth/stencil/accum_size and scrnum fields
 *
 * Revision 1.27  1996/03/02  21:41:05  brianp
 * added PF_8R8G8B pixel format
 *
 * Revision 1.26  1996/03/01  20:04:36  brianp
 * put the swapbytes field back in!
 *
 * Revision 1.25  1996/02/26  16:23:09  brianp
 * added PF_5R6G5B pixel format from Joerg Hessdoerfer
 *
 * Revision 1.24  1996/02/23  17:13:23  brianp
 * removed swapbytes field
 *
 * Revision 1.23  1996/02/06  03:25:43  brianp
 * added new gamma correction code
 *
 * Revision 1.22  1995/12/30  01:04:24  brianp
 * added PACK_8A8B8G8R macro
 * renamed DITHER_8BIT macro as just DITHER
 *
 * Revision 1.21  1995/12/21  17:57:21  brianp
 * added PIXELADDR2 macro, cleaned up the file a bit
 *
 * Revision 1.20  1995/12/13  23:29:48  brianp
 * replaced OFFSET macros with PIXELADDR macros
 *
 * Revision 1.19  1995/11/30  00:51:40  brianp
 * added casts to DITHER_HPCR macro to silence Sun compiler warnings
 *
 * Revision 1.18  1995/11/30  00:20:39  brianp
 * added new PF_GRAYSCALE mode
 *
 * Revision 1.17  1995/10/30  15:14:21  brianp
 * replaced Current variable with XMesa
 *
 * Revision 1.16  1995/10/19  15:50:38  brianp
 * added rgb_flag and share_list fields
 *
 * Revision 1.15  1995/10/17  21:38:17  brianp
 * many changes for new X/Mesa device driver
 *
 * Revision 1.14  1995/09/15  18:49:25  brianp
 * added rowimage span buffer
 *
 * Revision 1.13  1995/08/24  22:07:40  brianp
 * added swapbytes field
 *
 * Revision 1.12  1995/07/24  19:02:30  brianp
 * new comments for PF_DITHER and its tables
 *
 * Revision 1.11  1995/05/31  14:59:55  brianp
 * shm field not dependent on SHM symbol
 *
 * Revision 1.10  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.9  1995/04/17  13:57:14  brianp
 * changed to support GLXPixmaps
 *
 * Revision 1.8  1995/03/30  21:10:12  brianp
 * replaced colorpacking with pixelformat
 *
 * Revision 1.7  1995/03/22  21:38:17  brianp
 * *** empty log message ***
 *
 * Revision 1.6  1995/03/13  20:56:23  brianp
 * removed drawbuffer and readbuffer fields
 *
 * Revision 1.5  1995/03/08  15:10:02  brianp
 * added support for dd_logicop
 * added dd_clear_index and dd_clear_color
 *
 * Revision 1.4  1995/03/07  14:21:36  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.3  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/03/02  19:31:42  brianp
 * added defines for FRONT_PIXMAP, BACK_PIXMAP, and BACK_XIMAGE
 *
 * Revision 1.1  1995/02/28  21:23:31  brianp
 * Initial revision
 *
 */


#ifndef XMESAP_H
#define XMESAP_H


#ifdef SHM
#  include <X11/extensions/XShm.h>
#endif
#include "GL/xmesa.h"
#include "context.h"


/*
 * This struct is used by xmesa[12].c and glx.c only!
 */



struct xmesa_context {
        struct gl_context *gl_ctx;	/* the core library context */

	GLboolean initialized;	/* Is this context initialized yet? */

        GLboolean rgb_flag;	/* In RGBA mode? */
        GLboolean alpha_flag;	/* Need alpha buffer? */
	GLint depth_size;
	GLint stencil_size;
	GLint accum_size;

        struct xmesa_context *share_list;

	Display *display;	/* the X11 display */
	Visual *visual;		/* the X11 visual */
        int scrnum;		/* screen number */

	GC gc1;			/* GC for infrequent color changes */
	GC gc2;			/* GC for frequent color changes */
	GC cleargc;		/* GC for clearing the color buffer */

        GLboolean swapbytes;	/* Host byte order != display byte order? */

	GLuint db_state;	/* 0 = single buffered */
				/* BACK_PIXMAP = use Pixmap for back buffer */
				/* BACK_XIMAGE = use XImage for back buffer */

	Drawable frontbuffer;	/* either a window or pixmap */
	Pixmap backpixmap;	/* back buffer Pixmap */
	XImage *backimage;	/* back buffer XImage */

        Drawable buffer;	/* the current buffer, either equal to */
				/* frontbuffer, backpixmap or XIMAGE (None) */

	GLuint shm;		/* X Shared Memory extension status:	*/
				/*    0 = not available			*/
				/*    1 = XImage support available	*/
				/*    2 = Pixmap support available too	*/
#ifdef SHM
	XShmSegmentInfo shminfo;
#endif

        XImage *rowimage;	/* Used for optimized span writing */

	GLuint width, height;	/* size of buffer */
	GLuint depth;		/* bits per pixel (1, 8, 24, etc) */

        GLint bottom;              /* used for FLIP macro below */
        GLubyte *ximage_origin1;   /* used for PIXELADDR1 macro */
        GLint ximage_width1;
        GLushort *ximage_origin2;  /* used for PIXELADDR2 macro */
        GLint ximage_width2;
        GLuint *ximage_origin4;    /* used for PIXELADDR4 macro */
        GLint ximage_width4;

        GLfloat RedGamma;	/* Gamma values, 1.0 is default */
        GLfloat GreenGamma;
        GLfloat BlueGamma;

        GLubyte red, green, blue; /* current drawing color */
	unsigned long pixel;	  /* current color index or RGBA pixel value */

	unsigned long clearpixel; /* pixel for clearing the color buffers */

	GLuint pixelformat;	/* Current pixel format */
        GLuint dithered_pf;	/* Pixel format when dithering */
        GLuint undithered_pf;	/* Pixel format when not dithering */

	/* For PF_TRUECOLOR */
        GLint rmult, gmult, bmult;	/* Multiplier */
	GLint rshift, gshift, bshift;	/* Bit shifts */
	unsigned long r_to_pixel[256];	/* Converts red to pixel bits */
	unsigned long g_to_pixel[256];	/* Converts green to pixel bits */
	unsigned long b_to_pixel[256];	/* Converts blue to pixel bits */

	/* For PF_DITHER, PF_LOOKUP, PF_GRAYSCALE */
	unsigned long color_table[256];		/* RGB -> pixel value */

	/* For PF_DITHER, PF_LOOKUP, PF_GRAYSCALE */
	GLubyte pixel_to_r[65536];		/* pixel value -> red */
	GLubyte pixel_to_g[65536];		/* pixel value -> green */
	GLubyte pixel_to_b[65536];		/* pixel value -> blue */
};



/* Values for XMesa->dest: */
#define FRONT_PIXMAP	1
#define BACK_PIXMAP	2
#define BACK_XIMAGE	4


/* Values for XMesa->pixelformat: */
#define PF_INDEX	1	/* Color Index mode */
#define PF_TRUECOLOR	2	/* TrueColor or DirectColor, any depth */
#define PF_8A8B8G8R	3	/* 32-bit TrueColor:  8-A, 8-B, 8-G, 8-R */
#define PF_8R8G8B	4	/* 32-bit TrueColor:  8-R, 8-G, 8-B */
#define PF_5R6G5B	5	/* 16-bit TrueColor:  5-R, 6-G, 5-B bits */
#define PF_DITHER	6	/* Dithered RGB using a lookup table */
#define PF_LOOKUP	7	/* Undithered RGB using a lookup table */
#define PF_HPCR		8	/* HP Color Recovery (ad@lms.be 30/08/95) */
#define PF_1BIT		9	/* monochrome dithering of RGB */
#define PF_GRAYSCALE	10	/* Grayscale or StaticGray */



/*
 * If pixelformat==PF_TRUECOLOR:
 */
#define PACK_RGB( R, G, B )		\
	(XMesa->r_to_pixel[R] | XMesa->g_to_pixel[G] | XMesa->b_to_pixel[B])


/*
 * If pixelformat==PF_8A8B8G8R:
 */
#define PACK_8A8B8G8R( R, G, B, A )	\
	( ((A) << 24) | ((B) << 16) | ((G) << 8) | (R) )


/*
 * If pixelformat==PF_8R8G8B:
 */
#define PACK_8R8G8B( R, G, B)	 ( ((R) << 16) | ((G) << 8) | (B) )


/*
 * If pixelformat==PF_5R6G5B:
 */
#define PACK_5R6G5B( R, G, B)	 ( ((R) << 11) | ((G) << 5) | (B) )

 

/*
 * If pixelformat==PF_DITHER:
 *
 * Improved 8-bit RGB dithering code contributed by Bob Mercier
 * (mercier@hollywood.cinenet.net).  Thanks Bob!
 */
#define _R	5
#define _G	9
#define _B	5
#define _DX	4
#define _DY	4
#define _D	(_DX*_DY)
#define _MIX(r,g,b)	(((r)*_G+(g))*_B+(b))
/*#define _DITH(C,c,d)	(((unsigned)((_D*(C-1)+1)*c+d))/(_D*256))*/
#define _DITH(C,c,d)	(((unsigned)((_D*(C-1)+1)*c+d)) >> 12)

#define MAXC	256
static int kernel8[_DY][_DX] = {
   { 0 * MAXC,  8 * MAXC,  2 * MAXC, 10 * MAXC},
   {12 * MAXC,  4 * MAXC, 14 * MAXC,  6 * MAXC},
   { 3 * MAXC, 11 * MAXC,  1 * MAXC,  9 * MAXC},
   {15 * MAXC,  7 * MAXC, 13 * MAXC,  5 * MAXC},
};
static int __d;
#define DITHER( X, Y, R, G, B )				\
	(__d = kernel8[(Y)&(_DY-1)][(X)&(_DX-1)],	\
	 XMesa->color_table[_MIX(_DITH(_R, (R), __d),	\
				 _DITH(_G, (G), __d),	\
				 _DITH(_B, (B), __d))])



/*
 * If pixelformat==PF_LOOKUP:
 */
#define _DITH0(C,c)	(((unsigned)((_D*(C-1)+1)*c)) >> 12)
#define LOOKUP( R, G, B )				\
	 XMesa->color_table[_MIX(_DITH0(_R, (R)),	\
				 _DITH0(_G, (G)),	\
				 _DITH0(_B, (B)))]



/*
 * If pixelformat==PF_HPCR:
 *
 *      HP Color Recovery dithering               (ad@lms.be 30/08/95)
 *      HP has on it's 8-bit 700-series computers, a feature called
 *      'Color Recovery'.  This allows near 24-bit output (so they say).
 *      It is enabled by selecting the 8-bit  TrueColor  visual AND
 *      corresponding  colormap (see tkInitWindow) AND doing some special
 *      dither.
 */
static short HPCR_DR[2][16] = {
    { 16, -4,  1,-11, 14, -6,  3, -9, 15, -5,  2,-10, 13, -7,  4, -8},
    {-15,  5,  0, 12,-13,  7, -2, 10,-14,  6, -1, 11,-12,  8, -3,  9} };
static short HPCR_DG[2][16] = {
    {-11, 15, -7,  3, -8, 14, -4,  2,-10, 16, -6,  4, -9, 13, -5,  1},
    { 12,-14,  8, -2,  9,-13,  5, -1, 11,-15,  7, -3, 10,-12,  6,  0} };
static short HPCR_DB[2][16] = {
    {  6,-18, 26,-14,  2,-22, 30,-10,  8,-16, 28,-12,  4,-20, 32, -8},
    { -4, 20,-24, 16,  0, 24,-28, 12, -6, 18,-26, 14, -2, 22,-30, 10} };

extern short hpcr_rTbl[256], hpcr_gTbl[256], hpcr_bTbl[256];
static int _hpcr_x, _hpcr_y;
#define DITHER_HPCR( X, Y, R, G, B )				\
  (_hpcr_x=((X)%16), _hpcr_y=((Y)%2),				\
   ( ((hpcr_rTbl[R] + HPCR_DR[_hpcr_y][_hpcr_x]) & 0xE0)	\
   |(((hpcr_gTbl[G] + HPCR_DG[_hpcr_y][_hpcr_x]) & 0xE0)>>3)	\
   | ((hpcr_bTbl[B] + HPCR_DB[_hpcr_y][_hpcr_x])>>6)		\
   )								\
  )



/*
 * If pixelformat==PF_1BIT:
 */
static int kernel1[4][4] = {  0*47,  9*47,  4*47, 12*47,
			      6*47,  2*47, 14*47,  8*47,
			     10*47,  1*47,  5*47, 11*47,
			      7*47, 13*47,  3*47, 15*47 };
#define DITHER_1BIT( X, Y, R, G, B )	\
	( ((int)(R)+(int)(G)+(int)(B)) > kernel1[(X)&3][(Y)&3] )



/*
 * If pixelformat==PF_GRAYSCALE:
 */
#define GRAY_RGB( R, G, B )   XMesa->color_table[(R) + (G) + (B)]



#define XIMAGE None


/*
 * Converts a GL window Y coord to an X window Y coord:
 */
#define FLIP(Y)  (XMesa->bottom-(Y))


/*
 * Return the address of a 1, 2 or 4-byte pixel in the back XImage:
 * X==0 is left, Y==0 is bottom.
 */
#define PIXELADDR1( X, Y )  \
           ( XMesa->ximage_origin1 - (Y) * XMesa->ximage_width1 + (X) )

#define PIXELADDR2( X, Y )  \
           ( XMesa->ximage_origin2 - (Y) * XMesa->ximage_width2 + (X) )

#define PIXELADDR4( X, Y )  \
           ( XMesa->ximage_origin4 - (Y) * XMesa->ximage_width4 + (X) )



/*
 * External variables:
 */
extern XMesaContext XMesa;


/*
 * External functions:
 */
extern void xmesa_alloc_back_buffer( XMesaContext c );
extern void xmesa_setup_DD_pointers( void );
extern points_func xmesa_get_points_func( void );
extern line_func xmesa_get_line_func( void );
extern polygon_func xmesa_get_polygon_func( void );


#endif
