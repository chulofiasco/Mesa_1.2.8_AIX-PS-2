/* $Id: config.h,v 1.14 1996/04/25 21:08:46 brianp Exp $ */

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
$Log: config.h,v $
 * Revision 1.14  1996/04/25  21:08:46  brianp
 * changed MAX_DISPLAYLISTS to 7000
 *
 * Revision 1.13  1996/02/02  23:21:39  brianp
 * increased MAX_TEXTURE_STACK_DEPTH, MAX_DISPLAYLISTS, and MAX_TEXTURE_LEVELS
 *
 * Revision 1.12  1996/01/11  20:09:25  brianp
 * rearranged a few lines
 *
 * Revision 1.11  1995/12/18  17:12:09  brianp
 * replaced ACC_TYPE with ACCUM_BITS
 * added new DEPTH_BITS config
 *
 * Revision 1.10  1995/12/12  21:47:47  brianp
 * changed depth buffer range to 24 bits
 *
 * Revision 1.9  1995/08/31  21:33:11  brianp
 * changed MAX_TEXTURE_LEVELS to  10
 *
 * Revision 1.8  1995/06/20  16:30:23  brianp
 * added SUB_PIX defines and DEPTH_SCALE
 *
 * Revision 1.7  1995/05/31  19:20:20  brianp
 * removed MAX_VERTICES
 *
 * Revision 1.6  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.5  1995/03/24  15:16:36  brianp
 * replaced ACCUM_BITS with ACC_TYPE
 *
 * Revision 1.4  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.3  1995/03/04  19:17:13  brianp
 * changed max line width to 10.0
 *
 * Revision 1.2  1995/02/24  15:13:56  brianp
 * *** empty log message ***
 *
 * Revision 1.1  1995/02/24  14:13:18  brianp
 * Initial revision
 *
 */



/*
 * Tunable configuration parameters.
 */



#ifndef CONFIG_H
#define CONFIG_H



/* Maximum modelview matrix stack depth: */
#define MAX_MODELVIEW_STACK_DEPTH 32

/* Maximum projection matrix stack depth: */
#define MAX_PROJECTION_STACK_DEPTH 32

/* Maximum texture matrix stack depth: */
#define MAX_TEXTURE_STACK_DEPTH 10

/* Maximum attribute stack depth: */
#define MAX_ATTRIB_STACK_DEPTH 16

/* Maximum number of display lists: */
#define MAX_DISPLAYLISTS 7000

/* Maximum recursion depth of display list calls: */
#define MAX_LIST_NESTING 64

/* Maximum number of lights: */
#define MAX_LIGHTS 8

/* Maximum user-defined clipping planes: */
#define MAX_CLIP_PLANES 6

/* Number of texture levels */
#define MAX_TEXTURE_LEVELS 11

/* Max texture size */
#define MAX_TEXTURE_SIZE   (1 << (MAX_TEXTURE_LEVELS-1))

/* Maximum pixel map lookup table size: */
#define MAX_PIXEL_MAP_TABLE 256

/* Number of bits of red, green, blue and alpha in color buffer: */
/* TODO: remove, this is a temporary kludge */
#define COLOR_BITS 8

/* Number of auxillary color buffers: */
#define NUM_AUX_BUFFERS 0

/* Maximum order (degree) of curves: */
#ifdef AMIGA
#   define MAX_EVAL_ORDER 12
#else
#   define MAX_EVAL_ORDER 30
#endif

/* Maximum Name stack depth */
#define MAX_NAME_STACK_DEPTH 64

/* Min and Max point sizes and granularity */
#define MIN_POINT_SIZE 1.0
#define MAX_POINT_SIZE 10.0
#define POINT_SIZE_GRANULARITY 1.0

/* Min and Max line widths and granularity */
#define MIN_LINE_WIDTH 1.0
#define MAX_LINE_WIDTH 10.0
#define LINE_WIDTH_GRANULARITY 1.0


/* Maximum viewport size: */
#ifdef AMIGA
#  define MAX_WIDTH 640
#  define MAX_HEIGHT 400
#else
#  define MAX_WIDTH 1280
#  define MAX_HEIGHT 1024
#endif



/*
 * Bits per accumulation buffer color component:  8 or 16
 */
#define ACCUM_BITS 16


/*
 * Bits per depth buffer value:  16 or 32
 */
#define DEPTH_BITS 16

#if DEPTH_BITS==16
#  define MAX_DEPTH 0xffff
#  define DEPTH_SCALE 65535.0F
#else
#  define MAX_DEPTH 0x00ffffff
#  define DEPTH_SCALE ((GLfloat) 0x00ffffff)
#endif


/*
 * Bits per stencil value:  8
 */
#define STENCIL_BITS 8



/* Sub-pixel window coordinate scaling: (not currently used) */
#define SUB_PIX_SCALE 256.0
#define SUB_PIX_SHIFT 8

#endif
