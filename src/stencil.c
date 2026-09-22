/* $Id: stencil.c,v 1.16 1996/04/25 20:38:38 brianp Exp $ */

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
$Log: stencil.c,v $
 * Revision 1.16  1996/04/25  20:38:38  brianp
 * implement gl_depth_stencil_span/pixels in terms of DD.depth_test_span/pixels
 *
 * Revision 1.15  1996/04/15  17:29:09  brianp
 * fixed several stencil bugs
 *
 * Revision 1.14  1996/01/11  22:53:04  brianp
 * use new GLstencil datatype and STENCIL_ADDRESS() macro
 *
 * Revision 1.13  1995/12/18  17:27:48  brianp
 * use new GLdepth datatype
 * replaced a few loops with MEMSET()
 *
 * Revision 1.12  1995/11/01  15:13:11  brianp
 * renamed variable operator as oper per Steven Spitz
 *
 * Revision 1.11  1995/07/24  20:35:20  brianp
 * replaced memset() with MEMSET() and memcpy() with MEMCPY()
 *
 * Revision 1.10  1995/06/22  14:26:50  brianp
 * glStencilFunc, glStencilOp, glStencilMask now work in display lists
 *
 * Revision 1.9  1995/06/21  15:10:07  brianp
 * allocate stencil buffer in gl_clear_stencil_buffer() if it doesn't exist
 *
 * Revision 1.8  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.7  1995/05/12  16:57:22  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.6  1995/03/10  16:23:11  brianp
 * added include pb.h
 *
 * Revision 1.5  1995/03/10  15:52:47  brianp
 * change MAX_WIDTH to PB_SIZE in stencil_pixels
 *
 * Revision 1.4  1995/03/09  21:32:28  brianp
 * removed #include stdio.h
 *
 * Revision 1.3  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/03/01  17:44:36  brianp
 * added stenciling for PB
 *
 * Revision 1.1  1995/02/24  14:28:31  brianp
 * Initial revision
 *
 */


#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "dd.h"
#include "list.h"
#include "macros.h"
#include "pb.h"
#include "stencil.h"



/*
 * Return the address of a stencil buffer value given the window coords:
 */
#define STENCIL_ADDRESS(X,Y)  (CC.StencilBuffer + CC.BufferWidth * (Y) + (X))


void glClearStencil( GLint s )
{
   if (CC.CompileFlag) {
      gl_save_clearstencil( s );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glClearStencil" );
	 return;
      }

      CC.Stencil.Clear = (GLstencil) s;
   }
}



void glStencilFunc( GLenum func, GLint ref, GLuint mask )
{
   if (CC.CompileFlag) {
      gl_save_stencilfunc( func, ref, mask );
   }
   if (CC.ExecuteFlag) {
      GLint maxref;

      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glStencilFunc" );
	 return;
      }

      switch (func) {
	 case GL_NEVER:
	 case GL_LESS:
	 case GL_LEQUAL:
	 case GL_GREATER:
	 case GL_GEQUAL:
	 case GL_EQUAL:
	 case GL_NOTEQUAL:
	 case GL_ALWAYS:
	    CC.Stencil.Function = func;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glStencilFunc" );
	    return;
      }

      maxref = (1 << STENCIL_BITS) - 1;
      CC.Stencil.Ref = CLAMP( ref, 0, maxref );

      CC.Stencil.ValueMask = mask;
   }
}



void glStencilMask( GLuint mask )
{
   if (CC.CompileFlag) {
      gl_save_stencilmask( mask );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glStencilMask" );
	 return;
      }
      CC.Stencil.WriteMask = (GLstencil) mask;
   }
}



void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
   if (CC.CompileFlag) {
      gl_save_stencilop( fail, zfail, zpass );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glStencilOp" );
	 return;
      }
      switch (fail) {
	 case GL_KEEP:
	 case GL_ZERO:
	 case GL_REPLACE:
	 case GL_INCR:
	 case GL_DECR:
	 case GL_INVERT:
	    CC.Stencil.FailFunc = fail;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glStencilOp" );
	    return;
      }
      switch (zfail) {
	 case GL_KEEP:
	 case GL_ZERO:
	 case GL_REPLACE:
	 case GL_INCR:
	 case GL_DECR:
	 case GL_INVERT:
	    CC.Stencil.ZFailFunc = zfail;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glStencilOp" );
	    return;
      }
      switch (zpass) {
	 case GL_KEEP:
	 case GL_ZERO:
	 case GL_REPLACE:
	 case GL_INCR:
	 case GL_DECR:
	 case GL_INVERT:
	    CC.Stencil.ZPassFunc = zpass;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glStencilOp" );
	    return;
      }
   }
}



/* Stencil Logic:

IF stencil test fails THEN
   Don't write the pixel (RGBA,Z)
   Execute FailOp
ELSE
   Write the pixel
ENDIF

Perform Depth Test

IF depth test passes OR no depth buffer THEN
   Execute ZPass
   Write the pixel
ELSE
   Execute ZFail
ENDIF

*/




/*
 * Apply the given stencil operator for each pixel in the span whose
 * mask flag is set.
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in the span
 *         oper - the stencil buffer operator
 *         mask - array [n] of flag:  1=apply operator, 0=don't apply operator
 */
static void apply_stencil_op_to_span( GLuint n, GLint x, GLint y,
				      GLenum oper, GLubyte mask[] )
{
   GLint i;
   GLstencil s, ref;
   GLstencil wrtmask, invmask;
   GLstencil *stencil;

   wrtmask = CC.Stencil.WriteMask;
   invmask = ~CC.Stencil.WriteMask;
   ref = CC.Stencil.Ref;
   stencil = STENCIL_ADDRESS( x, y );

   switch (oper) {
      case GL_KEEP:
         /* do nothing */
         break;
      case GL_ZERO:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  stencil[i] = 0;
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  stencil[i] = stencil[i] & invmask;
	       }
	    }
	 }
	 break;
      case GL_REPLACE:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  stencil[i] = ref;
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  s = stencil[i];
		  stencil[i] = (invmask & s ) | (wrtmask & ref);
	       }
	    }
	 }
	 break;
      case GL_INCR:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  s = stencil[i];
		  if (s<0xff) {
		     stencil[i] = s+1;
		  }
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  /* VERIFY logic of adding 1 to a write-masked value */
		  s = stencil[i];
		  if (s<0xff) {
		     stencil[i] = (invmask & s) | (wrtmask & (s+1));
		  }
	       }
	    }
	 }
	 break;
      case GL_DECR:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  s = stencil[i];
		  if (s>0) {
		     stencil[i] = s-1;
		  }
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  /* VERIFY logic of subtracting 1 to a write-masked value */
		  s = stencil[i];
		  if (s>0) {
		     stencil[i] = (invmask & s) | (wrtmask & (s-1));
		  }
	       }
	    }
	 }
	 break;
      case GL_INVERT:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  s = stencil[i];
		  stencil[i] = ~s;
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
		  s = stencil[i];
		  stencil[i] = (invmask & s) | (wrtmask & ~s);
	       }
	    }
	 }
	 break;
      default:
         abort();
   }
}




/*
 * Apply stencil test to a span of pixels before depth buffering.
 * Input:  n - number of pixels in the span
 *         x, y - coordinate of left-most pixel in the span
 *         mask - array [n] of flag:  0=skip the pixel, 1=stencil the pixel
 * Output:  mask - pixels which fail the stencil test will have their
 *                 mask flag set to 0.
 * Return:  0 = all pixels failed, 1 = zero or more pixels passed.
 */
GLint gl_stencil_span( GLuint n, GLint x, GLint y, GLubyte mask[] )
{
   GLubyte fail[MAX_WIDTH];
   GLint allfail = 0;
   GLuint i;
   GLstencil r, s;
   GLstencil *stencil;

   stencil = STENCIL_ADDRESS( x, y );

   /*
    * Perform stencil test.  The results of this operation are stored
    * in the fail[] array:
    *   IF fail[i] is non-zero THEN
    *       the stencil fail operator is to be applied
    *   ELSE
    *       the stencil fail operator is not to be applied
    *   ENDIF
    */
   switch (CC.Stencil.Function) {
      case GL_NEVER:
         /* always fail */
         for (i=0;i<n;i++) {
	    if (mask[i]) {
	       mask[i] = 0;
	       fail[i] = 1;
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 allfail = 1;
	 break;
      case GL_LESS:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       s = stencil[i] & CC.Stencil.ValueMask;
	       if (r < s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_LEQUAL:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       s = stencil[i] & CC.Stencil.ValueMask;
	       if (r <= s) {
		  /* pass */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_GREATER:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       s = stencil[i] & CC.Stencil.ValueMask;
	       if (r > s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_GEQUAL:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       s = stencil[i] & CC.Stencil.ValueMask;
	       if (r >= s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_EQUAL:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       s = stencil[i] & CC.Stencil.ValueMask;
	       if (r == s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_NOTEQUAL:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
	       s = stencil[i] & CC.Stencil.ValueMask;
	       if (r != s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_ALWAYS:
	 /* always pass */
	 for (i=0;i<n;i++) {
	    fail[i] = 0;
	 }
	 break;
      default:
         abort();
   }

   apply_stencil_op_to_span( n, x, y, CC.Stencil.FailFunc, fail );

   return (allfail) ? 0 : 1;
}




/*
 * Apply the combination depth-buffer/stencil operator to a span of pixels.
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in span
 *         z - array [n] of z values
 * Input:  mask - array [n] of flags  (1=test this pixel, 0=skip the pixel)
 * Output:  mask - array [n] of flags (1=depth test passed, 0=failed) 
 */
void gl_depth_stencil_span( GLuint n, GLint x, GLint y, const GLdepth z[],
			    GLubyte mask[] )
{
   if (CC.Depth.Test==GL_FALSE) {
      /*
       * No depth buffer, just apply zpass stencil function to active pixels.
       */
      apply_stencil_op_to_span( n, x, y, CC.Stencil.ZPassFunc, mask );
   }
   else {
      /*
       * Perform depth buffering, then apply zpass or zfail stencil function.
       */
      GLubyte passmask[MAX_WIDTH], failmask[MAX_WIDTH], oldmask[MAX_WIDTH];
      GLuint i;

      /* init pass and fail masks to zero, copy mask[] to oldmask[] */
      for (i=0;i<n;i++) {
	 passmask[i] = failmask[i] = 0;
         oldmask[i] = mask[i];
      }

      /* apply the depth test */
      (*DD.depth_test_span)( n, x, y, z, mask );

      /* set the stencil pass/fail flags according to result of depth test */
      for (i=0;i<n;i++) {
         if (oldmask[i]) {
            if (mask[i]) {
               passmask[i] = 1;
            }
            else {
               failmask[i] = 1;
            }
         }
      }

      /* apply the pass and fail operations */
      apply_stencil_op_to_span( n, x, y, CC.Stencil.ZFailFunc, failmask );
      apply_stencil_op_to_span( n, x, y, CC.Stencil.ZPassFunc, passmask );
   }
}




/*
 * Apply the given stencil operator for each pixel in the array whose
 * mask flag is set.
 * Input:  n - number of pixels in the span
 *         x, y - array of [n] pixels
 *         operator - the stencil buffer operator
 *         mask - array [n] of flag:  1=apply operator, 0=don't apply operator
 */
static void apply_stencil_op_to_pixels( GLuint n, const GLint x[],
				        const GLint y[],
				        GLenum oper, GLubyte mask[] )
{
   GLint i;
   GLstencil ref;
   GLstencil wrtmask, invmask;

   wrtmask = CC.Stencil.WriteMask;
   invmask = ~CC.Stencil.WriteMask;

   ref = CC.Stencil.Ref;

   switch (oper) {
      case GL_KEEP:
         /* do nothing */
         break;
      case GL_ZERO:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
                  *sptr = 0;
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
		  *sptr = invmask & *sptr;
	       }
	    }
	 }
	 break;
      case GL_REPLACE:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
                  *sptr = ref;
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
		  *sptr = (invmask & *sptr ) | (wrtmask & ref);
	       }
	    }
	 }
	 break;
      case GL_INCR:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
		  if (*sptr < 0xff) {
		     *sptr = *sptr + 1;
		  }
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
		  if (*sptr<0xff) {
		     *sptr = (invmask & *sptr) | (wrtmask & (*sptr+1));
		  }
	       }
	    }
	 }
	 break;
      case GL_DECR:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
		  if (*sptr>0) {
		     *sptr = *sptr - 1;
		  }
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
		  if (*sptr>0) {
		     *sptr = (invmask & *sptr) | (wrtmask & (*sptr-1));
		  }
	       }
	    }
	 }
	 break;
      case GL_INVERT:
	 if (invmask==0) {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
                  *sptr = ~*sptr;
	       }
	    }
	 }
	 else {
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLstencil *sptr = STENCIL_ADDRESS( x[i], y[i] );
                  *sptr = (invmask & *sptr) | (wrtmask & ~*sptr);
	       }
	    }
	 }
	 break;
      default:
         abort();
	 return;
   }
}



/*
 * Apply stencil test to an array of pixels before depth buffering.
 * Input:  n - number of pixels in the span
 *         x, y - array of [n] pixels to stencil
 *         mask - array [n] of flag:  0=skip the pixel, 1=stencil the pixel
 * Output:  mask - pixels which fail the stencil test will have their
 *                 mask flag set to 0.
 * Return:  0 = all pixels failed, 1 = zero or more pixels passed.
 */
GLint gl_stencil_pixels( GLuint n, const GLint x[], const GLint y[],
			 GLubyte mask[] )
{
   GLubyte fail[PB_SIZE];
   GLstencil r, s;
   GLuint i;
   GLint allfail = 0;

   /*
    * Perform stencil test.  The results of this operation are stored
    * in the fail[] array:
    *   IF fail[i] is non-zero THEN
    *       the stencil fail operator is to be applied
    *   ELSE
    *       the stencil fail operator is not to be applied
    *   ENDIF
    */

   switch (CC.Stencil.Function) {
      case GL_NEVER:
         /* always fail */
         for (i=0;i<n;i++) {
	    if (mask[i]) {
	       mask[i] = 0;
	       fail[i] = 1;
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 allfail = 1;
	 break;
      case GL_LESS:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
               GLstencil *sptr = STENCIL_ADDRESS(x[i],y[i]);
	       s = *sptr & CC.Stencil.ValueMask;
	       if (r < s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_LEQUAL:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
               GLstencil *sptr = STENCIL_ADDRESS(x[i],y[i]);
	       s = *sptr & CC.Stencil.ValueMask;
	       if (r <= s) {
		  /* pass */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_GREATER:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
               GLstencil *sptr = STENCIL_ADDRESS(x[i],y[i]);
	       s = *sptr & CC.Stencil.ValueMask;
	       if (r > s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_GEQUAL:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
               GLstencil *sptr = STENCIL_ADDRESS(x[i],y[i]);
	       s = *sptr & CC.Stencil.ValueMask;
	       if (r >= s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_EQUAL:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
               GLstencil *sptr = STENCIL_ADDRESS(x[i],y[i]);
	       s = *sptr & CC.Stencil.ValueMask;
	       if (r == s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_NOTEQUAL:
	 r = CC.Stencil.Ref & CC.Stencil.ValueMask;
	 for (i=0;i<n;i++) {
	    if (mask[i]) {
               GLstencil *sptr = STENCIL_ADDRESS(x[i],y[i]);
	       s = *sptr & CC.Stencil.ValueMask;
	       if (r != s) {
		  /* passed */
		  fail[i] = 0;
	       }
	       else {
		  fail[i] = 1;
		  mask[i] = 0;
	       }
	    }
	    else {
	       fail[i] = 0;
	    }
	 }
	 break;
      case GL_ALWAYS:
	 /* always pass */
	 for (i=0;i<n;i++) {
	    fail[i] = 0;
	 }
	 break;
      default:
         abort();
   }

   apply_stencil_op_to_pixels( n, x, y, CC.Stencil.FailFunc, fail );

   return (allfail) ? 0 : 1;
}




/*
 * Apply the combination depth-buffer/stencil operator to a span of pixels.
 * Input:  n - number of pixels in the span
 *         x, y - array of [n] pixels to stencil
 *         z - array [n] of z values
 * Input:  mask - array [n] of flags  (1=test this pixel, 0=skip the pixel)
 * Output:  mask - array [n] of flags (1=depth test passed, 0=failed) 
 */
void gl_depth_stencil_pixels( GLuint n, const GLint x[], const GLint y[],
			      const GLdepth z[], GLubyte mask[] )
{
   if (CC.Depth.Test==GL_FALSE) {
      /*
       * No depth buffer, just apply zpass stencil function to active pixels.
       */
      apply_stencil_op_to_pixels( n, x, y, CC.Stencil.ZPassFunc, mask );
   }
   else {
      /*
       * Perform depth buffering, then apply zpass or zfail stencil function.
       */
      GLubyte passmask[PB_SIZE], failmask[PB_SIZE], oldmask[PB_SIZE];
      GLuint i;

      /* init pass and fail masks to zero */
      for (i=0;i<n;i++) {
	 passmask[i] = failmask[i] = 0;
         oldmask[i] = mask[i];
      }

      /* apply the depth test */
      (*DD.depth_test_pixels)( n, x, y, z, mask );

      /* set the stencil pass/fail flags according to result of depth test */
      for (i=0;i<n;i++) {
         if (oldmask[i]) {
            if (mask[i]) {
               passmask[i] = 1;
            }
            else {
               failmask[i] = 1;
            }
         }
      }

      /* apply the pass and fail operations */
      apply_stencil_op_to_pixels( n, x, y, CC.Stencil.ZFailFunc, failmask );
      apply_stencil_op_to_pixels( n, x, y, CC.Stencil.ZPassFunc, passmask );
   }

}



/*
 * Return a span of stencil values from the stencil buffer.
 * Input:  n - how many pixels
 *         x,y - location of first pixel
 * Output:  stencil - the array of stencil values
 */
void gl_read_stencil_span( GLuint n, GLint x, GLint y, GLubyte stencil[] )
{
   GLstencil *s;

   if (CC.StencilBuffer) {
      s = STENCIL_ADDRESS( x, y );
      MEMCPY( stencil, s, n * sizeof(GLubyte) );
   }
}



/*
 * Write a span of stencil values to the stencil buffer.
 * Input:  n - how many pixels
 *         x,y - location of first pixel
 *         stencil - the array of stencil values
 */
void gl_write_stencil_span( GLuint n, GLint x, GLint y,
			    const GLubyte stencil[] )
{
   GLstencil *s;

   if (CC.StencilBuffer) {
      s = STENCIL_ADDRESS( x, y );
      MEMCPY( s, stencil, n * sizeof(GLubyte) );
   }
}



/*
 * Allocate a new stencil buffer.  If there's an old one it will be
 * deallocated first.  The new stencil buffer will be uninitialized.
 */
void gl_alloc_stencil_buffer( void )
{
   GLuint buffersize;

   buffersize = CC.BufferWidth * CC.BufferHeight;

   /* deallocate current stencil buffer if present */
   if (CC.StencilBuffer) {
      free(CC.StencilBuffer);
      CC.StencilBuffer = NULL;
   }

   /* allocate new stencil buffer */
   CC.StencilBuffer = (GLstencil *) malloc( buffersize * sizeof(GLstencil) );
   if (!CC.StencilBuffer) {
      /* out of memory */
      CC.Stencil.Enabled = GL_FALSE;
      gl_error( GL_OUT_OF_MEMORY, "gl_alloc_stencil_buffer" );
   }
}




/*
 * Clear the stencil buffer.  If the stencil buffer doesn't exist yet we'll
 * allocate it now.
 */
void gl_clear_stencil_buffer( void )
{
   if (!CC.StencilBuffer) {
      gl_alloc_stencil_buffer();
   }

   if (CC.StencilBuffer) {
      if (CC.Scissor.Enabled) {
         /* clear scissor region only */
	 GLint y, width = CC.Scissor.Xmax-CC.Scissor.Xmin+1;
 	 for (y=CC.Scissor.Ymin; y<=CC.Scissor.Ymax; y++) {
            GLstencil *ptr = STENCIL_ADDRESS( CC.Scissor.Xmin, y );
            MEMSET( ptr, CC.Stencil.Clear, width * sizeof(GLubyte) );
	 }
      }
      else {
         /* clear whole stencil buffer */
         MEMSET( CC.StencilBuffer, CC.Stencil.Clear,
		 CC.BufferWidth * CC.BufferHeight * sizeof(GLubyte) );
      }
   }
}
