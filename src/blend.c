/* blend.c */

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
 * Pixel blending.
 */


/*
$Id: blend.c,v 1.16 1996/02/19 21:50:00 brianp Exp $

$Log: blend.c,v $
 * Revision 1.16  1996/02/19  21:50:00  brianp
 * added support for software alpha buffering
 *
 * Revision 1.15  1995/12/19  17:05:37  brianp
 * added gl_save_blendcolor() and gl_save_blendequation() calls
 *
 * Revision 1.14  1995/10/30  15:30:33  brianp
 * pass mask array to DD.read_color_pixels
 *
 * Revision 1.13  1995/08/31  21:30:26  brianp
 * renamed blend() as dd_blend()
 * use *DD.read_color_pixels instead of dd_read_color_pixels
 * introduced CC.NewState convention
 *
 * Revision 1.12  1995/06/12  15:34:31  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.11  1995/05/31  14:57:10  brianp
 * call gl_read_color_span() instead of dd_read_color_span()
 *
 * Revision 1.10  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.9  1995/05/12  19:20:19  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.8  1995/03/30  21:09:04  brianp
 * fixed bugs in blend scale factors
 *
 * Revision 1.7  1995/03/24  20:03:44  brianp
 * added missing cases to glBlendEquationEXT
 *
 * Revision 1.6  1995/03/24  16:59:17  brianp
 * fixed blending/logicop bug
 *
 * Revision 1.5  1995/03/13  20:55:02  brianp
 * introduced blend function
 * added rest of logic ops
 * optimized common cases
 *
 * Revision 1.4  1995/03/10  16:45:52  brianp
 * updated for new blending extensions
 *
 * Revision 1.3  1995/03/07  18:58:42  brianp
 * added gl_blend_pixels()
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:17:11  brianp
 * Initial revision
 *
 */


#ifdef DEBUG
#  include <assert.h>
#endif
#include "alphabuf.h"
#include "context.h"
#include "dd.h"
#include "list.h"
#include "macros.h"
#include "pb.h"
#include "span.h"



void glBlendFunc( GLenum sfactor, GLenum dfactor )
{
   if (CC.CompileFlag) {
      gl_save_blendfunc( sfactor, dfactor );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glBlendFunction" );
	 return;
      }

      switch (sfactor) {
	 case GL_ZERO:
	 case GL_ONE:
	 case GL_DST_COLOR:
	 case GL_ONE_MINUS_DST_COLOR:
	 case GL_SRC_ALPHA:
	 case GL_ONE_MINUS_SRC_ALPHA:
	 case GL_DST_ALPHA:
	 case GL_ONE_MINUS_DST_ALPHA:
	 case GL_SRC_ALPHA_SATURATE:
	 case GL_CONSTANT_COLOR_EXT:
	 case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
	 case GL_CONSTANT_ALPHA_EXT:
	 case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
	    CC.Color.BlendSrc = sfactor;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glBlendFunction" );
	    return;
      }

      switch (dfactor) {
	 case GL_ZERO:
	 case GL_ONE:
	 case GL_SRC_COLOR:
	 case GL_ONE_MINUS_SRC_COLOR:
	 case GL_SRC_ALPHA:
	 case GL_ONE_MINUS_SRC_ALPHA:
	 case GL_DST_ALPHA:
	 case GL_ONE_MINUS_DST_ALPHA:
	 case GL_CONSTANT_COLOR_EXT:
	 case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
	 case GL_CONSTANT_ALPHA_EXT:
	 case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
	    CC.Color.BlendDst = dfactor;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glBlendFunction" );
      }

      CC.NewState = GL_TRUE;
   }
}



/*
 * This is not a standard OpenGL function.  It is an extension!
 */
void glBlendEquationEXT( GLenum mode )
{
   if (CC.CompileFlag) {
      gl_save_blendequation( mode );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glBlendEquation" );
	 return;
      }

      switch (mode) {
	 case GL_MIN_EXT:
	 case GL_MAX_EXT:
	 case GL_LOGIC_OP:
	 case GL_FUNC_ADD_EXT:
	 case GL_FUNC_SUBTRACT_EXT:
	 case GL_FUNC_REVERSE_SUBTRACT_EXT:
	    CC.Color.BlendEquation = mode;
	    break;
	default:
	    gl_error( GL_INVALID_ENUM, "glBlendEquation" );
      }
      CC.NewState = GL_TRUE;
   }
}



/*
 * This is not a standard OpenGL function.  It is an extension!
 */
void glBlendColorEXT( GLclampf red, GLclampf green,
		      GLclampf blue, GLclampf alpha )
{
   if (CC.CompileFlag) {
      gl_save_blendcolor( red, green, blue, alpha );
   }
   if (CC.ExecuteFlag) {
      CC.Color.BlendColor[0] = CLAMP( red,   0.0, 1.0 );
      CC.Color.BlendColor[1] = CLAMP( green, 0.0, 1.0 );
      CC.Color.BlendColor[2] = CLAMP( blue,  0.0, 1.0 );
      CC.Color.BlendColor[3] = CLAMP( alpha, 0.0, 1.0 );
   }
}



/*
 * Do the real work of gl_blend_span() and gl_blend_pixels().
 * Input:  n - number of pixels
 *         mask - the usual write mask
 * In/Out:  red, green, blue, alpha - the incoming and modified pixels
 * Input:  rdest, gdest, bdest, adest - the pixels from the dest color buffer
 */
static void do_blend( GLuint n, const GLubyte mask[],
                      GLubyte red[], GLubyte green[],
                      GLubyte blue[], GLubyte alpha[],
                      const GLubyte rdest[], const GLubyte gdest[],
                      const GLubyte bdest[], const GLubyte adest[] )
{
   GLfloat rscale, gscale, bscale, ascale;
   GLuint i;


   rscale = 1.0F / CC.RedScale;
   gscale = 1.0F / CC.GreenScale;
   bscale = 1.0F / CC.BlueScale;
   ascale = 1.0F / CC.AlphaScale;

   /* Common cases: */

   if (CC.Color.BlendEquation==GL_FUNC_ADD_EXT
       && CC.Color.BlendSrc==GL_SRC_ALPHA
       && CC.Color.BlendDst==GL_ONE_MINUS_SRC_ALPHA) {
      /* Alpha blending */
      for (i=0;i<n;i++) {
	 if (mask[i]) {
	    GLfloat r, g, b, a;
	    GLfloat t = (GLfloat) alpha[i] * ascale;
	    r = red[i]   * t + rdest[i] * (1.0F-t);
	    g = green[i] * t + gdest[i] * (1.0F-t);
	    b = blue[i]  * t + bdest[i] * (1.0F-t);
	    a = alpha[i] * t + adest[i] * (1.0F-t);
	    red[i]   = (GLint) MIN2( r, CC.RedScale );
	    green[i] = (GLint) MIN2( g, CC.GreenScale );
	    blue[i]  = (GLint) MIN2( b, CC.BlueScale );
	    alpha[i] = (GLint) MIN2( a, CC.AlphaScale );
	 }
      }
   }
   else if (CC.Color.BlendEquation==GL_LOGIC_OP
	    && CC.Color.LogicOp==GL_XOR) {
      /* XOR drawing */
      for (i=0;i<n;i++) {
	 if (mask[i]) {
	    red[i]   ^= rdest[i];
	    green[i] ^= gdest[i];
	    blue[i]  ^= bdest[i];
	    alpha[i] ^= adest[i];
	 }
      }
   }

   /* General cases: */

   else if (CC.Color.BlendEquation==GL_FUNC_ADD_EXT
         || CC.Color.BlendEquation==GL_FUNC_SUBTRACT_EXT
         || CC.Color.BlendEquation==GL_FUNC_REVERSE_SUBTRACT_EXT) {
      for (i=0;i<n;i++) {
	 if (mask[i]) {
	    GLint Rs, Gs, Bs, As;  /* Source colors */
	    GLint Rd, Gd, Bd, Ad;  /* Dest colors */
	    GLfloat sR, sG, sB, sA;  /* Source scaling */
	    GLfloat dR, dG, dB, dA;  /* Dest scaling */
	    GLfloat r, g, b, a;

	    /* Source Color */
	    Rs = red[i];
	    Gs = green[i];
	    Bs = blue[i];
	    As = alpha[i];

	    /* Frame buffer color */
	    Rd = rdest[i];
	    Gd = gdest[i];
	    Bd = bdest[i];
	    Ad = adest[i];

	    /* Source scaling */
	    switch (CC.Color.BlendSrc) {
	       case GL_ZERO:
		  sR = sG = sB = sA = 0.0F;
		  break;
	       case GL_ONE:
		  sR = sG = sB = sA = 1.0F;
		  break;
	       case GL_DST_COLOR:
		  sR = (GLfloat) Rd * rscale;
		  sG = (GLfloat) Gd * gscale;
		  sB = (GLfloat) Bd * bscale;
		  sA = (GLfloat) Ad * ascale;
		  break;
	       case GL_ONE_MINUS_DST_COLOR:
		  sR = 1.0F - (GLfloat) Rd * rscale;
		  sG = 1.0F - (GLfloat) Gd * gscale;
		  sB = 1.0F - (GLfloat) Bd * bscale;
		  sA = 1.0F - (GLfloat) Ad * ascale;
		  break;
	       case GL_SRC_ALPHA:
		  sR = sG = sB = sA = (GLfloat) As * ascale;
		  break;
	       case GL_ONE_MINUS_SRC_ALPHA:
		  sR = sG = sB = sA = (GLfloat) 1.0F - (GLfloat) As * ascale;
		  break;
	       case GL_DST_ALPHA:
		  sR = sG = sB = sA =(GLfloat) Ad * ascale;
		  break;
	       case GL_ONE_MINUS_DST_ALPHA:
		  sR = sG = sB = sA = 1.0F - (GLfloat) Ad * ascale;
		  break;
	       case GL_SRC_ALPHA_SATURATE:
		  if (As < 1.0F - (GLfloat) Ad * ascale) {
		     sR = sG = sB = (GLfloat) As * ascale;
		  }
		  else {
		     sR = sG = sB = 1.0F - (GLfloat) Ad * ascale;
		  }
		  sA = 1.0;
		  break;
	       case GL_CONSTANT_COLOR_EXT:
		  sR = CC.Color.BlendColor[0];
		  sG = CC.Color.BlendColor[1];
		  sB = CC.Color.BlendColor[2];
		  sA = CC.Color.BlendColor[3];
		  break;
	       case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		  sR = 1.0F - CC.Color.BlendColor[0];
		  sG = 1.0F - CC.Color.BlendColor[1];
		  sB = 1.0F - CC.Color.BlendColor[2];
		  sA = 1.0F - CC.Color.BlendColor[3];
		  break;
	       case GL_CONSTANT_ALPHA_EXT:
		  sR = sG = sB = sA = CC.Color.BlendColor[3];
		  break;
	       case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
		  sR = sG = sB = sA = 1.0F - CC.Color.BlendColor[3];
		  break;
	       default:
		  gl_error( GL_INVALID_ENUM, "gl_blend source" );
	    }

	    /* Dest scaling */
	    switch (CC.Color.BlendDst) {
	       case GL_ZERO:
		  dR = dG = dB = dA = 0.0F;
		  break;
	       case GL_ONE:
		  dR = dG = dB = dA = 1.0F;
		  break;
	       case GL_SRC_COLOR:
		  dR = (GLfloat) Rs * rscale;
		  dG = (GLfloat) Gs * gscale;
		  dB = (GLfloat) Bs * bscale;
		  dA = (GLfloat) As * ascale;
		  break;
	       case GL_ONE_MINUS_SRC_COLOR:
		  dR = 1.0F - (GLfloat) Rs * rscale;
		  dG = 1.0F - (GLfloat) Gs * gscale;
		  dB = 1.0F - (GLfloat) Bs * bscale;
		  dA = 1.0F - (GLfloat) As * ascale;
		  break;
	       case GL_SRC_ALPHA:
		  dR = dG = dB = dA = (GLfloat) As * ascale;
		  break;
	       case GL_ONE_MINUS_SRC_ALPHA:
		  dR = dG = dB = dA = (GLfloat) 1.0F - (GLfloat) As * ascale;
		  break;
	       case GL_DST_ALPHA:
		  dR = dG = dB = dA = (GLfloat) Ad * ascale;
		  break;
	       case GL_ONE_MINUS_DST_ALPHA:
		  dR = dG = dB = dA = 1.0F - (GLfloat) Ad * ascale;
		  break;
	       case GL_CONSTANT_COLOR_EXT:
		  dR = CC.Color.BlendColor[0];
		  dG = CC.Color.BlendColor[1];
		  dB = CC.Color.BlendColor[2];
		  dA = CC.Color.BlendColor[3];
		  break;
	       case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		  dR = 1.0F - CC.Color.BlendColor[0];
		  dG = 1.0F - CC.Color.BlendColor[1];
		  dB = 1.0F - CC.Color.BlendColor[2];
		  dA = 1.0F - CC.Color.BlendColor[3];
		  break;
	       case GL_CONSTANT_ALPHA_EXT:
		  dR = dG = dB = dA = CC.Color.BlendColor[3];
		  break;
	       case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
		  dR = dG = dB = dA = 1.0F - CC.Color.BlendColor[3] * ascale;
		  break;
	       default:
		  gl_error( GL_INVALID_ENUM, "gl_blend dest" );
	    }

#ifdef DEBUG
	    assert( sR>= 0.0 && sR<=1.0 );
	    assert( sG>= 0.0 && sG<=1.0 );
	    assert( sB>= 0.0 && sB<=1.0 );
	    assert( sA>= 0.0 && sA<=1.0 );
	    assert( dR>= 0.0 && dR<=1.0 );
	    assert( dG>= 0.0 && dG<=1.0 );
	    assert( dB>= 0.0 && dB<=1.0 );
	    assert( dA>= 0.0 && dA<=1.0 );
#endif

	    /* compute blended color */
	    if (CC.Color.BlendEquation==GL_FUNC_ADD_EXT) {
	       r = Rs * sR + Rd * dR;
	       g = Gs * sG + Gd * dG;
	       b = Bs * sB + Bd * dB;
	       a = As * sA + Ad * dA;
	    }
	    else if (CC.Color.BlendEquation==GL_FUNC_SUBTRACT_EXT) {
	       r = Rs * sR - Rd * dR;
	       g = Gs * sG - Gd * dG;
	       b = Bs * sB - Bd * dB;
	       a = As * sA - Ad * dA;
	    }
	    else if (CC.Color.BlendEquation==GL_FUNC_REVERSE_SUBTRACT_EXT) {
	       r = Rd * dR - Rs * sR;
	       g = Gd * dG - Gs * sG;
	       b = Bd * dB - Bs * sB;
	       a = Ad * dA - As * sA;
	    }
	    red[i]   = (GLint) CLAMP( r, 0.0, CC.RedScale );
	    green[i] = (GLint) CLAMP( g, 0.0, CC.GreenScale );
	    blue[i]  = (GLint) CLAMP( b, 0.0, CC.BlueScale );
	    alpha[i] = (GLint) CLAMP( a, 0.0, CC.AlphaScale );
	 }
      }
   }

   else if (CC.Color.BlendEquation==GL_LOGIC_OP) {
      /* EXTENSION */
      switch (CC.Color.LogicOp) {
	 case GL_CLEAR:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i] = green[i] = blue[i] = alpha[i] = 0;
	       }
	    }
	    break;
	 case GL_SET:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = (GLint) CC.RedScale;
		  green[i] = (GLint) CC.GreenScale;
		  blue[i]  = (GLint) CC.BlueScale;
		  alpha[i] = (GLint) CC.AlphaScale;
	       }
	    }
	    break;
	 case GL_COPY:
	    /* do nothing */
	    break;
	 case GL_COPY_INVERTED:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = !red[i];
		  green[i] = !green[i];
		  blue[i]  = !blue[i];
		  alpha[i] = !alpha[i];
	       }
	    }
	    break;
	 case GL_NOOP:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = rdest[i];
		  green[i] = gdest[i];
		  blue[i]  = bdest[i];
		  alpha[i] = adest[i];
	       }
	    }
	    break;
	 case GL_INVERT:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = !rdest[i];
		  green[i] = !gdest[i];
		  blue[i]  = !bdest[i];
		  alpha[i] = !adest[i];
	       }
	    }
	    break;
	 case GL_AND:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   &= rdest[i];
		  green[i] &= gdest[i];
		  blue[i]  &= bdest[i];
		  alpha[i] &= adest[i];
	       }
	    }
	    break;
	 case GL_NAND:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = !(red[i]   & rdest[i]);
		  green[i] = !(green[i] & gdest[i]);
		  blue[i]  = !(blue[i]  & bdest[i]);
		  alpha[i] = !(alpha[i] & adest[i]);
	       }
	    }
	    break;
	 case GL_OR:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   |= rdest[i];
		  green[i] |= gdest[i];
		  blue[i]  |= bdest[i];
		  alpha[i] |= adest[i];
	       }
	    }
	    break;
	 case GL_NOR:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = !(red[i]   | rdest[i]);
		  green[i] = !(green[i] | gdest[i]);
		  blue[i]  = !(blue[i]  | bdest[i]);
		  alpha[i] = !(alpha[i] | adest[i]);
	       }
	    }
	    break;
	 case GL_XOR:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   ^= rdest[i];
		  green[i] ^= gdest[i];
		  blue[i]  ^= bdest[i];
		  alpha[i] ^= adest[i];
	       }
	    }
	    break;
	 case GL_EQUIV:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = !(red[i]   ^ rdest[i]);
		  green[i] = !(green[i] ^ gdest[i]);
		  blue[i]  = !(blue[i]  ^ bdest[i]);
		  alpha[i] = !(alpha[i] ^ adest[i]);
	       }
	    }
	    break;
	 case GL_AND_REVERSE:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = red[i]   & !rdest[i];
		  green[i] = green[i] & !gdest[i];
		  blue[i]  = blue[i]  & !bdest[i];
		  alpha[i] = alpha[i] & !adest[i];
	       }
	    }
	    break;
	 case GL_AND_INVERTED:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = !red[i]   & rdest[i];
		  green[i] = !green[i] & gdest[i];
		  blue[i]  = !blue[i]  & bdest[i];
		  alpha[i] = !alpha[i] & adest[i];
	       }
	    }
	    break;
	 case GL_OR_REVERSE:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = red[i]   | !rdest[i];
		  green[i] = green[i] | !gdest[i];
		  blue[i]  = blue[i]  | !bdest[i];
		  alpha[i] = alpha[i] | !adest[i];
	       }
	    }
	    break;
	 case GL_OR_INVERTED:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  red[i]   = !red[i]   | rdest[i];
		  green[i] = !green[i] | gdest[i];
		  blue[i]  = !blue[i]  | bdest[i];
		  alpha[i] = !alpha[i] | adest[i];
	       }
	    }
	    break;
	 default:
	    ;  /* nothing */
      }
   }
   else if (CC.Color.BlendEquation==GL_MIN_EXT) {
      /* EXTENSION */
      for (i=0;i<n;i++) {
	 if (mask[i]) {
	    red[i]   = MIN2( red[i],   rdest[i] );
	    green[i] = MIN2( green[i], gdest[i] );
	    blue[i]  = MIN2( blue[i],  bdest[i] );
	    alpha[i] = MIN2( alpha[i], adest[i] );
	 }
      }
   }
   else if (CC.Color.BlendEquation==GL_MAX_EXT) {
      /* EXTENSION */
      for (i=0;i<n;i++) {
	 if (mask[i]) {
	    red[i]   = MAX2( red[i],   rdest[i] );
	    green[i] = MAX2( green[i], gdest[i] );
	    blue[i]  = MAX2( blue[i],  bdest[i] );
	    alpha[i] = MAX2( alpha[i], adest[i] );
	 }
      }
   }

}





/*
 * Apply the blending operator to a span of pixels.
 * Input:  n - number of pixels in span
 *         x, y - location of leftmost pixel in span in window coords.
 *         mask - boolean mask indicating which pixels to blend.
 * In/Out:  red, green, blue, alpha - pixel values
 */
void gl_blend_span( GLuint n, GLint x, GLint y,
		    GLubyte red[], GLubyte green[],
		    GLubyte blue[], GLubyte alpha[],
		    GLubyte mask[] )
{
   GLubyte rdest[MAX_WIDTH], gdest[MAX_WIDTH];
   GLubyte bdest[MAX_WIDTH], adest[MAX_WIDTH];

   /* Check if device driver can do the work */
   if (CC.Color.BlendEquation==GL_LOGIC_OP && !CC.Color.SWLogicOpEnabled) {
      return;
   }

   /* Read span of current frame buffer pixels */
   gl_read_color_span( n, x, y, rdest, gdest, bdest, adest );

   do_blend( n, mask, red, green, blue, alpha, rdest, gdest, bdest, adest );
}





/*
 * Apply the blending operator to an array of pixels.
 * Input:  n - number of pixels in span
 *         x, y - array of pixel locations
 *         mask - boolean mask indicating which pixels to blend.
 * In/Out:  red, green, blue, alpha - array of pixel values
 */
void gl_blend_pixels( GLuint n, const GLint x[], const GLint y[],
		      GLubyte red[], GLubyte green[],
		      GLubyte blue[], GLubyte alpha[],
		      GLubyte mask[] )
{
   GLubyte rdest[PB_SIZE], gdest[PB_SIZE], bdest[PB_SIZE], adest[PB_SIZE];

   /* Check if device driver can do the work */
   if (CC.Color.BlendEquation==GL_LOGIC_OP && !CC.Color.SWLogicOpEnabled) {
      return;
   }

   /* Read pixels from current color buffer */
   (*DD.read_color_pixels)( n, x, y, rdest, gdest, bdest, adest, mask );
   if (CC.RasterMask & ALPHABUF_BIT) {
      gl_read_alpha_pixels( n, x, y, adest, mask );
   }

   do_blend( n, mask, red, green, blue, alpha, rdest, gdest, bdest, adest );
}
