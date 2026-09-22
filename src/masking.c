/* $Id: masking.c,v 1.3 1996/02/19 21:50:00 brianp Exp $ */

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
$Log: masking.c,v $
 * Revision 1.3  1996/02/19  21:50:00  brianp
 * added support for software alpha buffering
 *
 * Revision 1.2  1995/10/30  15:33:24  brianp
 * added mask argument to gl_mask_[color|index]_pixels functions
 *
 * Revision 1.1  1995/10/13  22:40:00  brianp
 * Initial revision
 *
 */


/*
 * Implement the effect of glColorMask and glIndexMask in software.
 */



#include <string.h>
#include "alphabuf.h"
#include "dd.h"
#include "context.h"
#include "macros.h"
#include "masking.h"
#include "pb.h"




/*
 * Implement glColorMask for a span of RGBA pixels.
 */
void gl_mask_color_span( GLuint n, GLint x, GLint y,
                         GLubyte red[], GLubyte green[],
                         GLubyte blue[], GLubyte alpha[] )
{
   GLubyte r[MAX_WIDTH], g[MAX_WIDTH], b[MAX_WIDTH], a[MAX_WIDTH];

   (*DD.read_color_span)( n, x, y, r, g, b, a );
   if (CC.RasterMask & ALPHABUF_BIT) {
      gl_read_alpha_span( n, x, y, a );
   }

   if ((CC.Color.ColorMask & 8) == 0) {
      /* replace source reds with frame buffer reds */
      MEMCPY( red, r, n );
   }
   if ((CC.Color.ColorMask & 4) == 0) {
      /* replace source greens with frame buffer greens */
      MEMCPY( green, g, n );
   }
   if ((CC.Color.ColorMask & 2) == 0) {
      /* replace source blues with frame buffer blues */
      MEMCPY( blue, b, n );
   }
   if ((CC.Color.ColorMask & 1) == 0) {
      /* replace source alphas with frame buffer alphas */
      MEMCPY( alpha, a, n );
   }
}



/*
 * Implement glColorMask for an array of RGBA pixels.
 */
void gl_mask_color_pixels( GLuint n, const GLint x[], const GLint y[],
                           GLubyte red[], GLubyte green[],
                           GLubyte blue[], GLubyte alpha[],
                           const GLubyte mask[] )
{
   GLubyte r[PB_SIZE], g[PB_SIZE], b[PB_SIZE], a[PB_SIZE];

   (*DD.read_color_pixels)( n, x, y, r, g, b, a, mask );
   if (CC.RasterMask & ALPHABUF_BIT) {
      gl_read_alpha_pixels( n, x, y, a, mask );
   }

   if ((CC.Color.ColorMask & 8) == 0) {
      /* replace source reds with frame buffer reds */
      MEMCPY( red, r, n );
   }
   if ((CC.Color.ColorMask & 4) == 0) {
      /* replace source greens with frame buffer greens */
      MEMCPY( green, g, n );
   }
   if ((CC.Color.ColorMask & 2) == 0) {
      /* replace source blues with frame buffer blues */
      MEMCPY( blue, b, n );
   }
   if ((CC.Color.ColorMask & 1) == 0) {
      /* replace source alphas with frame buffer alphas */
      MEMCPY( alpha, a, n );
   }
}



/*
 * Implement glIndexMask for a span of CI pixels.
 */
void gl_mask_index_span( GLuint n, GLint x, GLint y, GLuint index[] )
{
   GLuint i;
   GLuint fbindexes[MAX_WIDTH];
   GLuint msrc, mdest;

   (*DD.read_index_span)( n, x, y, fbindexes );

   msrc = CC.Color.IndexMask;
   mdest = ~msrc;

   for (i=0;i<n;i++) {
      index[i] = (index[i] & msrc) | (fbindexes[i] & mdest);
   }
}



/*
 * Implement glIndexMask for an array of CI pixels.
 */
void gl_mask_index_pixels( GLuint n, const GLint x[], const GLint y[],
                           GLuint index[], const GLubyte mask[] )
{
   GLuint i;
   GLuint fbindexes[MAX_WIDTH];
   GLuint msrc, mdest;

   (*DD.read_index_pixels)( n, x, y, fbindexes, mask );

   msrc = CC.Color.IndexMask;
   mdest = ~msrc;

   for (i=0;i<n;i++) {
      index[i] = (index[i] & msrc) | (fbindexes[i] & mdest);
   }
}

