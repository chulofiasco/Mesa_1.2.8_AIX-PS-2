/* $Id: pb.c,v 1.31 1996/04/25 20:40:07 brianp Exp $ */

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
$Log: pb.c,v $
 * Revision 1.31  1996/04/25  20:40:07  brianp
 * replaced gl_depth_test_pixels() calls with DD.depth_test_pixels
 *
 * Revision 1.30  1996/03/29  15:27:30  brianp
 * RGBA masking and blending were done in the wrong order
 *
 * Revision 1.29  1996/02/19  21:50:00  brianp
 * added support for software alpha buffering
 *
 * Revision 1.28  1996/02/06  03:23:54  brianp
 * removed gamma correction code
 *
 * Revision 1.27  1996/01/22  15:31:49  brianp
 * replaced gl_init_pb() with PB_INIT macro
 * removed polygon stippling code (not needed)
 * use CC.MutablePixels instead of PB.mutable
 *
 * Revision 1.26  1995/12/30  00:57:46  brianp
 * use integer colors instead of floating point
 *
 * Revision 1.25  1995/11/09  16:57:05  brianp
 * added some missing PB.count=0 statements per Johan Nouvel
 *
 * Revision 1.24  1995/11/03  22:36:04  brianp
 * fixed fogging bug
 *
 * Revision 1.23  1995/10/30  15:31:14  brianp
 * added mask argument to gl_mask_[color|index]_pixels calls
 *
 * Revision 1.22  1995/10/19  15:47:51  brianp
 * added gamma support
 *
 * Revision 1.21  1995/10/13  22:41:43  brianp
 * removed dithering code, added color/index masking code
 *
 * Revision 1.20  1995/09/27  19:22:22  brianp
 * optimized several loops, replaced some with memset()
 *
 * Revision 1.19  1995/07/26  15:03:48  brianp
 * replaced some literals with variables for SunOS 4.x per Asif Khan
 *
 * Revision 1.18  1995/07/15  14:04:19  brianp
 * enabled texture mapping
 *
 * Revision 1.17  1995/06/20  16:18:53  brianp
 * removed clipflag logic, clip all pixels
 *
 * Revision 1.16  1995/06/12  15:42:14  brianp
 * changed some indentations
 *
 * Revision 1.15  1995/06/05  20:26:51  brianp
 * better PB.clipflag setup
 *
 * Revision 1.14  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.13  1995/05/18  14:43:31  brianp
 * implemented glIndexMask(0) and glColorMask(0,0,0,0)
 *
 * Revision 1.12  1995/05/17  13:17:22  brianp
 * changed default CC.Mode value to allow use of real OpenGL headers
 * removed need for CC.MajorMode variable
 *
 * Revision 1.11  1995/05/12  16:26:33  brianp
 * added pixel clipping
 *
 * Revision 1.10  1995/04/11  14:04:40  brianp
 * changed (*CC.write...) to (*DD.write...)
 *
 * Revision 1.9  1995/03/30  21:07:32  brianp
 * updated to use pointers to CC.write_* functions
 *
 * Revision 1.8  1995/03/27  20:32:17  brianp
 * new Texture.Enabled scheme
 *
 * Revision 1.7  1995/03/08  15:10:02  brianp
 * support for dd_logicop
 *
 * Revision 1.6  1995/03/07  19:02:08  brianp
 * added logicop, blending, and alpha test
 *
 * Revision 1.5  1995/03/07  14:21:05  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.4  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.3  1995/03/01  17:44:22  brianp
 * added stenciling for PB
 *
 * Revision 1.2  1995/02/27  22:48:56  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  17:51:55  brianp
 * Initial revision
 *
 */


/*
 * Pixel buffer:
 *
 * As fragments are produced (by point, line, and bitmap drawing) they
 * are accumlated in a buffer.  When the buffer is full or has to be
 * flushed (glEnd), we apply all enabled rasterization functions to the
 * pixels and write the results to the display buffer.  The goal is to
 * maximize the number of pixels processed inside loops and to minimize
 * the number of function calls.
 */



#include <string.h>
#include "alpha.h"
#include "alphabuf.h"
#include "blend.h"
#include "context.h"
#include "dd.h"
#include "depth.h"
#include "fog.h"
#include "logic.h"
#include "macros.h"
#include "masking.h"
#include "pb.h"
#include "scissor.h"
#include "stencil.h"
#include "texture.h"


struct pixel_buffer PB;



/*
 * When the pixel buffer is full, or needs to be flushed, call this
 * function.  All the pixels in the pixel buffer will be subjected
 * to texturing, scissoring, stippling, alpha testing, stenciling,
 * depth testing, blending, and finally written to the frame buffer.
 */
void gl_flush_pb( void )
{
   GLubyte mask[PB_SIZE+4];   /* add 4 for manually unrolled loop, below */

#ifdef DEBUG
   printf("Flush: %d\n", PB.count );
#endif

   if (PB.count==0)  return;

   /* initialize mask array and clip pixels simultaneously */
   {
      GLint w = CC.BufferWidth;
      GLint h = CC.BufferHeight;
      GLuint i = 0;
      /* manually unrolled loop, OK to go past PB.count */
      do {
         mask[i] = (PB.x[i]>=0) & (PB.x[i]<w) & (PB.y[i]>=0) & (PB.y[i]<h);
         i++;
         mask[i] = (PB.x[i]>=0) & (PB.x[i]<w) & (PB.y[i]>=0) & (PB.y[i]<h);
         i++;
         mask[i] = (PB.x[i]>=0) & (PB.x[i]<w) & (PB.y[i]>=0) & (PB.y[i]<h);
         i++;
         mask[i] = (PB.x[i]>=0) & (PB.x[i]<w) & (PB.y[i]>=0) & (PB.y[i]<h);
         i++;
      } while (i<PB.count);
   }

   if (CC.RGBAflag) {
      /* RGBA COLOR PIXELS */

      if (PB.mono && CC.MutablePixels) {
	 /* Copy flat color to all pixels */
         MEMSET( PB.r, PB.color[0], PB.count );
         MEMSET( PB.g, PB.color[1], PB.count );
         MEMSET( PB.b, PB.color[2], PB.count );
         MEMSET( PB.a, PB.color[3], PB.count );
      }

      /* If each pixel can be of a different color... */
      if (CC.MutablePixels || !PB.mono) {

	 if (CC.Texture.Enabled & 2) {
	    gl_texture_pixels_2d(PB.count, PB.s, PB.t, PB.r, PB.g, PB.b, PB.a);
	 }
	 else if (CC.Texture.Enabled & 1) {
	    gl_texture_pixels_1d( PB.count, PB.s, PB.r, PB.g, PB.b, PB.a );
	 }

	 if (CC.Fog.Enabled
             && (CC.Hint.Fog==GL_NICEST || PB.primitive==GL_BITMAP)) {
	    gl_fog_color_pixels( PB.count, PB.z,
				 PB.r, PB.g, PB.b, PB.a );
	 }

	 if (CC.Scissor.Enabled) {
	    if (gl_scissor_pixels( PB.count, PB.x, PB.y, mask )==0) {
	       PB.count = 0;
	       return;
	    }
	 }

	 if (CC.Color.AlphaEnabled) {
	    if (gl_alpha_test( PB.count, PB.a, mask )==0) {
	       PB.count = 0;
	       return;
	    }
	 }

	 if (CC.Stencil.Enabled) {
	    /* first stencil test */
	    if (gl_stencil_pixels( PB.count, PB.x, PB.y, mask )==0) {
	       PB.count = 0;
	       return;
	    }
	    /* depth buffering w/ stencil */
	    gl_depth_stencil_pixels( PB.count, PB.x, PB.y, PB.z, mask );
	 }
	 else if (CC.Depth.Test) {
	    /* regular depth testing */
	    (*DD.depth_test_pixels)( PB.count, PB.x, PB.y, PB.z, mask );
	 }

	 if (CC.Color.ColorMask) {
            if (CC.Color.BlendEnabled) {
               gl_blend_pixels( PB.count, PB.x, PB.y,
                                PB.r, PB.g, PB.b, PB.a, mask);
            }

            if (CC.Color.SWmasking) {
               gl_mask_color_pixels( PB.count, PB.x, PB.y,
                                     PB.r, PB.g, PB.b, PB.a, mask );
            }

            /* write pixels */
            (*DD.write_color_pixels)( PB.count, PB.x, PB.y,
                                      PB.r, PB.g, PB.b, PB.a, mask );
            if (CC.RasterMask & ALPHABUF_BIT) {
               gl_write_alpha_pixels( PB.count, PB.x, PB.y, PB.a, mask );
            }
         }

      }
      else {
	 /* Same color for all pixels */

	 if (CC.Scissor.Enabled) {
	    if (gl_scissor_pixels( PB.count, PB.x, PB.y, mask )==0) {
               PB.count = 0;
	       return;
	    }
	 }

	 if (CC.Color.AlphaEnabled) {
	    if (gl_alpha_test( PB.count, PB.a, mask )==0) {
               PB.count = 0;
	       return;
	    }
	 }

	 if (CC.Stencil.Enabled) {
	    /* first stencil test */
	    if (gl_stencil_pixels( PB.count, PB.x, PB.y, mask )==0) {
               PB.count = 0;
	       return;
	    }
	    /* depth buffering w/ stencil */
	    gl_depth_stencil_pixels( PB.count, PB.x, PB.y, PB.z, mask );
	 }
	 else if (CC.Depth.Test) {
	    /* regular depth testing */
	    (*DD.depth_test_pixels)( PB.count, PB.x, PB.y, PB.z, mask );
	 }

	 if (CC.Color.ColorMask) {
	    /* write pixels */
            GLubyte red, green, blue, alpha;
            red   = PB.color[0];
            green = PB.color[1];
            blue  = PB.color[2];
            alpha = PB.color[3];
	    (*DD.color)( red, green, blue, alpha );
	    (*DD.write_monocolor_pixels)( PB.count, PB.x, PB.y, mask );
            if (CC.RasterMask & ALPHABUF_BIT) {
               gl_write_mono_alpha_pixels( PB.count, PB.x, PB.y, alpha, mask );
            }
	 }
      }
   }
   else {
      /* COLOR INDEX PIXELS */

      /* If we may be writting pixels with different indexes... */
      if (PB.mono && CC.MutablePixels) {
	 /* copy index to all pixels */
         GLuint n = PB.count, indx = PB.index;
         GLuint *pbindex = PB.i;
         do {
	    *pbindex++ = indx;
            n--;
	 } while (n);
      }

      if (CC.MutablePixels || !PB.mono) {
	 /* Pixel color index may be modified */

	 if (CC.Fog.Enabled
             && (CC.Hint.Fog==GL_NICEST || PB.primitive==GL_BITMAP)) {
	    gl_fog_index_pixels( PB.count, PB.z, PB.i );
	 }

	 if (CC.Scissor.Enabled) {
	    if (gl_scissor_pixels( PB.count, PB.x, PB.y, mask )==0) {
               PB.count = 0;
	       return;
	    }
	 }

	 if (CC.Stencil.Enabled) {
	    /* first stencil test */
	    if (gl_stencil_pixels( PB.count, PB.x, PB.y, mask )==0) {
               PB.count = 0;
	       return;
	    }
	    /* depth buffering w/ stencil */
	    gl_depth_stencil_pixels( PB.count, PB.x, PB.y, PB.z, mask );
	 }
	 else if (CC.Depth.Test) {
	    /* regular depth testing */
	    (*DD.depth_test_pixels)( PB.count, PB.x, PB.y, PB.z, mask );
	 }

	 if (CC.Color.IndexMask) {
	    if (CC.Color.SWLogicOpEnabled) {
	       gl_logic_pixels( PB.count, PB.x, PB.y, PB.i, mask );
	    }

            if (CC.Color.SWmasking) {
               gl_mask_index_pixels( PB.count, PB.x, PB.y, PB.i, mask );
            }

	    /* write pixels */
	    (*DD.write_index_pixels)( PB.count, PB.x, PB.y, PB.i, mask );
	 }
      }
      else {
	 /* Same color index for all pixels */

	 if (CC.Scissor.Enabled) {
	    if (gl_scissor_pixels( PB.count, PB.x, PB.y, mask )==0) {
               PB.count = 0;
	       return;
	    }
	 }

	 if (CC.Stencil.Enabled) {
	    /* first stencil test */
	    if (gl_stencil_pixels( PB.count, PB.x, PB.y, mask )==0) {
               PB.count = 0;
	       return;
	    }
	    /* depth buffering w/ stencil */
	    gl_depth_stencil_pixels( PB.count, PB.x, PB.y, PB.z, mask );
	 }
	 else if (CC.Depth.Test) {
	    /* regular depth testing */
	    (*DD.depth_test_pixels)( PB.count, PB.x, PB.y, PB.z, mask );
	 }

	 if (CC.Color.IndexMask) {
	    /* write pixels */
	    (*DD.index)( PB.index );
	    (*DD.write_monoindex_pixels)( PB.count, PB.x, PB.y, mask );
	 }
      }
   }

   PB.count = 0;
}

