/* $Id: depth.c,v 1.21 1996/05/15 18:20:01 brianp Exp $ */

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
$Log: depth.c,v $
 * Revision 1.21  1996/05/15  18:20:01  brianp
 * added branch-less Z testing
 *
 * Revision 1.20  1996/05/01  15:47:48  brianp
 * added gl_read_depth_span_int()
 *
 * Revision 1.19  1996/04/25  20:39:21  brianp
 * added comments about DD.<depth function> calling conventions
 *
 * Revision 1.18  1996/03/21  22:47:20  brianp
 * removed obsolete comments
 *
 * Revision 1.17  1996/01/22  15:26:29  brianp
 * set CC.NewState in glDepthFunc() and glDepthMask()
 *
 * Revision 1.16  1996/01/12  22:30:52  brianp
 * use the Z_ADDRESS() macro
 *
 * Revision 1.15  1995/12/19  16:41:48  brianp
 * optimized 16-bit clear for more clear values
 *
 * Revision 1.14  1995/12/18  17:25:05  brianp
 * use new GLdepth datatype
 *
 * Revision 1.13  1995/11/30  00:18:54  brianp
 * manually unrolled loop in gl_clear_depth_buffer()
 *
 * Revision 1.12  1995/09/22  21:53:35  brianp
 * optimized gl_clear_depth_buffer for loop unrolling
 *
 * Revision 1.11  1995/07/11  15:10:38  brianp
 * fixed bug in gl_depth_test_span when func=GL_ALWAYS and DepthMask=GL_FALSE
 *
 * Revision 1.10  1995/06/21  15:09:17  brianp
 * added a comment to gl_clear_depth_buffer
 *
 * Revision 1.9  1995/06/15  21:08:05  brianp
 * fixed bug in gl_read_depth_span() per Asif Khan
 *
 * Revision 1.8  1995/06/09  20:20:50  brianp
 * renamed gl_depth_test() as gl_depth_test_span() and return 'passed' count
 *
 * Revision 1.7  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.6  1995/05/12  19:24:13  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.5  1995/04/19  13:48:18  brianp
 * renamed occurances of near and far for SCO x86 Unix
 *
 * Revision 1.4  1995/03/09  21:34:01  brianp
 * removed #include stdio.h
 *
 * Revision 1.3  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/27  22:48:39  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  14:20:28  brianp
 * Initial revision
 *
 */


/*
 * Depth buffer functions
 */



#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "depth.h"
#include "list.h"
#include "macros.h"




void glClearDepth( GLclampd depth )
{
   if (CC.CompileFlag) {
      gl_save_cleardepth( (GLfloat) depth );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glClearDepth" );
      }
      CC.Depth.Clear = (GLfloat) CLAMP( depth, 0.0, 1.0 );
   }
}



void glDepthFunc( GLenum func )
{
   if (CC.CompileFlag) {
      gl_save_depthfunc( func );
   }

   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glDepthFunc" );
      }

      switch (func) {
	 case GL_NEVER:
	 case GL_LESS:    /* (default) pass if incoming z < stored z */
	 case GL_GEQUAL:
	 case GL_LEQUAL:
	 case GL_GREATER:
	 case GL_NOTEQUAL:
	 case GL_EQUAL:
	 case GL_ALWAYS:
	    CC.Depth.Func = func;
            CC.NewState = GL_TRUE;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glDepth.Func" );
      }
   }
}



void glDepthMask( GLboolean flag )
{
   if (CC.CompileFlag) {
      gl_save_depthmask( flag );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glDepthMask" );
      }

      /*
       * GL_TRUE indicates depth buffer writing is enabled (default)
       * GL_FALSE indicates depth buffer writing is disabled
       */
      CC.Depth.Mask = flag;
      CC.NewState = GL_TRUE;
   }
}



void glDepthRange( GLclampd nearval, GLclampd farval )
{
   /*
    * nearval - specifies mapping of the near clipping plane to window
    *   coordinates, default is 0
    * farval - specifies mapping of the far clipping plane to window
    *   coordinates, default is 1
    *
    * After clipping and div by w, z coords are in -1.0 to 1.0,
    * corresponding to near and far clipping planes.  glDepthRange
    * specifies a linear mapping of the normalized z coords in
    * this range to window z coords.
    */

   if (CC.CompileFlag) {
      gl_save_depthrange( nearval, farval );
   }
   if (CC.ExecuteFlag) {
      GLfloat n, f;

      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glDepthRange" );
      }

      n = (GLfloat) CLAMP( nearval, 0.0, 1.0 );
      f = (GLfloat) CLAMP( farval, 0.0, 1.0 );

      CC.Viewport.Near = n;
      CC.Viewport.Far = f;
      CC.Viewport.Sz = (f - n) / 2.0;
      CC.Viewport.Tz = (f - n) / 2.0 + n;
   }
}



/*
 * Apply depth buffer test to a horizontal span of pixels:
 *
 * FOREACH pixel[i] in the span DO
 *    IF mask[i]!=0 THEN
 *       IF depth test passes THEN
 *          update z value
 *       ELSE
 *          mask[i] = 0;
 *       ENDIF
 *    ENDIF
 * ENDDO
 *
 * This function is only called through DD.depth_test_span.
 *
 * Input:  n - number of pixels in the span
 *         x, y - location of leftmost pixel in span in window coords
 *         z - array [n] of integer depth values
 * In/Out:  mask - array [n] of flags (1=draw pixel, 0=don't draw) 
 * Return:  number of pixels which passed depth test
 */
GLuint gl_depth_test_span( GLuint n, GLint x, GLint y, const GLdepth z[],
			   GLubyte mask[] )
{
   GLdepth *zptr = Z_ADDRESS( x, y );
   GLubyte *m = mask;
   GLuint i;
   GLuint passed = 0;

   /* switch cases ordered from most frequent to less frequent */
   switch (CC.Depth.Func) {
      case GL_LESS:
         if (CC.Depth.Mask) {
	    /* Update Z buffer */
#ifdef SAFE
	    for (i=0; i<n; i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] < *zptr) {
		     /* pass */
		     *zptr = z[i];
		     passed++;
		  }
		  else {
		     /* fail */
		     *m = 0;
		  }
	       }
	    }
#else
            /* OPTIMIZED: no branches */
	    for (i=0; i<n; i++) {
               GLint newz = z[i];
               GLint oldz = zptr[i];
               GLint p = mask[i] & (newz < oldz);        /* 0 or 1 */
               mask[i] = p;
               passed += p;
               p = 0 - p;                                /* 0 or ~0 */
               zptr[i] = (oldz & ~p) | (newz & p);
	    }
#endif
	 }
	 else {
	    /* Don't update Z buffer */
#ifdef SAFE
	    for (i=0; i<n; i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] < *zptr) {
		     /* pass */
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
#else
            /* OPTIMIZED: no branches */
	    for (i=0; i<n; i++) {
               GLint p = mask[i] & (z[i] < zptr[i]);
               mask[i] = p;
               passed += p;
	    }
#endif
	 }
	 break;
      case GL_LEQUAL:
	 if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] <= *zptr) {
		     *zptr = z[i];
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] <= *zptr) {
		     /* pass */
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_GEQUAL:
	 if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] >= *zptr) {
		     *zptr = z[i];
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] >= *zptr) {
		     /* pass */
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_GREATER:
	 if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] > *zptr) {
		     *zptr = z[i];
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] > *zptr) {
		     /* pass */
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_NOTEQUAL:
	 if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] != *zptr) {
		     *zptr = z[i];
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] != *zptr) {
		     /* pass */
		     passed++;
		  }
		  else {
		     *m = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_EQUAL:
	 if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] == *zptr) {
		     *zptr = z[i];
		     passed++;
		  }
		  else {
		     *m =0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  if (z[i] == *zptr) {
		     /* pass */
		     passed++;
		  }
		  else {
		     *m =0;
		  }
	       }
	    }
	 }
	 break;
      case GL_ALWAYS:
	 if (CC.Depth.Mask) {
	    /* Update Z buffer */
#ifdef SAFE
	    for (i=0;i<n;i++,zptr++,m++) {
	       if (*m) {
		  *zptr = z[i];
		  passed++;
	       }
	    }
#else
            /* OPTIMIZED: no branches */
	    for (i=0;i<n;i++) {
               GLint p = 0 - mask[i];   /* 0 or ~0 */
               zptr[i] = (zptr[i] & ~p) | (z[i] & p);
               passed += mask[i];
	    }
#endif
	 }
	 else {
	    /* Don't update Z buffer or mask */
	    passed = n;
	 }
	 break;
      case GL_NEVER:
	 for (i=0;i<n;i++) {
	    mask[i] = 0;
	 }
	 break;
   } /*switch*/

   return passed;
}




#define ZADDR_SETUP   GLdepth *depthbuffer = CC.DepthBuffer; \
                      GLint width = CC.BufferWidth;

#define ZADDR( X, Y )   (depthbuffer + (Y) * width + (X) )



/*
 * Perform depth testing on an array of pixels.
 * This function is only called through DD.depth_test_pixels.
 */
void gl_depth_test_pixels( GLuint n, const GLint x[], const GLint y[],
			   const GLdepth z[], GLubyte mask[] )
{
   register GLdepth *zptr;
   register GLuint i;

   /* switch cases ordered from most frequent to less frequent */
   switch (CC.Depth.Func) {
      case GL_LESS:
         if (CC.Depth.Mask) {
	    /* Update Z buffer */
#ifdef SAFE
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] < *zptr) {
		     /* pass */
		     *zptr = z[i];
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
#else
            /* OPTIMIZED: no branches */
            ZADDR_SETUP;
            for (i=0; i<n; i++) {
               GLdepth *zptr;
               GLint p;
               zptr = ZADDR(x[i],y[i]);
               p = (z[i] < *zptr) & mask[i];   /* 0 or 1 */
               p = 0 - p;                      /* 0 or ~0 */
               *zptr = (*zptr & (~p)) | (z[i] & p);
               mask[i] = p;
            }
#endif
	 }
	 else {
	    /* Don't update Z buffer */
#ifdef SAFE
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] < *zptr) {
		     /* pass */
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
#else
            /* OPTIMIZED: no branches */
            ZADDR_SETUP;
	    for (i=0; i<n; i++) {
               GLdepth *zptr = ZADDR(x[i],y[i]);
               mask[i] &= (z[i] < *zptr);
            }
#endif
	 }
	 break;
      case GL_LEQUAL:
         if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] <= *zptr) {
		     /* pass */
		     *zptr = z[i];
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] <= *zptr) {
		     /* pass */
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_GEQUAL:
         if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] >= *zptr) {
		     /* pass */
		     *zptr = z[i];
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] >= *zptr) {
		     /* pass */
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_GREATER:
         if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] > *zptr) {
		     /* pass */
		     *zptr = z[i];
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] > *zptr) {
		     /* pass */
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_NOTEQUAL:
         if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] != *zptr) {
		     /* pass */
		     *zptr = z[i];
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] != *zptr) {
		     /* pass */
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_EQUAL:
         if (CC.Depth.Mask) {
	    /* Update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] == *zptr) {
		     /* pass */
		     *zptr = z[i];
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 else {
	    /* Don't update Z buffer */
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  if (z[i] == *zptr) {
		     /* pass */
		  }
		  else {
		     /* fail */
		     mask[i] = 0;
		  }
	       }
	    }
	 }
	 break;
      case GL_ALWAYS:
	 if (CC.Depth.Mask) {
	    /* Update Z buffer */
#ifdef SAFE
	    for (i=0; i<n; i++) {
	       if (mask[i]) {
		  zptr = Z_ADDRESS(x[i],y[i]);
		  *zptr = z[i];
	       }
	    }
#else
            /* OPTIMIZED: no branches */
            ZADDR_SETUP;
	    for (i=0; i<n; i++) {
               GLdepth *zptr = ZADDR(x[i],y[i]);
               GLint p = 0 - mask[i];   /* 0 or ~0 */
               *zptr = (*zptr & ~p) | (z[i] & p);
	    }
#endif
	 }
	 else {
	    /* Don't update Z buffer or mask */
	 }
	 break;
      case GL_NEVER:
	 /* depth test never passes */
	 for (i=0;i<n;i++) {
	    mask[i] = 0;
	 }
	 break;
   } /*switch*/
}




/*
 * Return a span of depth values from the depth buffer as floats in [0,1].
 * This function is only called through DD.read_depth_span_float()
 * Input:  n - how many pixels
 *         x,y - location of first pixel
 * Output:  depth - the array of depth values
 */
void gl_read_depth_span_float( GLuint n, GLint x, GLint y, GLfloat depth[] )
{
   GLdepth *zptr;
   GLfloat scale;
   GLuint i;

   scale = 1.0F / DEPTH_SCALE;

   if (CC.DepthBuffer) {
      zptr = Z_ADDRESS( x, y );
      for (i=0;i<n;i++) {
	 depth[i] = (GLfloat) zptr[i] * scale;
      }
   }
   else {
      for (i=0;i<n;i++) {
	 depth[i] = 0.0F;
      }
   }
}


/*
 * Return a span of depth values from the depth buffer as integers in
 * [0,MAX_DEPTH].
 * This function is only called through DD.read_depth_span_int()
 * Input:  n - how many pixels
 *         x,y - location of first pixel
 * Output:  depth - the array of depth values
 */
void gl_read_depth_span_int( GLuint n, GLint x, GLint y, GLdepth depth[] )
{
   if (CC.DepthBuffer) {
      GLdepth *zptr = Z_ADDRESS( x, y );
      MEMCPY( depth, zptr, n * sizeof(GLdepth) );
   }
   else {
      GLuint i;
      for (i=0;i<n;i++) {
	 depth[i] = 0.0;
      }
   }
}



/*
 * Allocate a new depth buffer.  If there's already a depth buffer allocated
 * it will be free()'d.  The new depth buffer will be uniniitalized.
 * This function is only called through DD.alloc_depth_buffer.
 */
void gl_alloc_depth_buffer( void )
{
   GLint w, h;

   /* deallocate current depth buffer if present */
   if (CC.DepthBuffer) {
      free(CC.DepthBuffer);
      CC.DepthBuffer = NULL;
   }

   w = CC.BufferWidth;
   h = CC.BufferHeight;
   if (w <= 0 || h <= 0) return;
   if (w > MAX_WIDTH) w = MAX_WIDTH;
   if (h > MAX_HEIGHT) h = MAX_HEIGHT;

   /* allocate new depth buffer */
   CC.DepthBuffer = (GLdepth *) malloc( w * h * sizeof(GLdepth) );
   if (!CC.DepthBuffer) {
      /* out of memory */
      CC.Depth.Test = GL_FALSE;
      gl_error( GL_OUT_OF_MEMORY, "Couldn't allocate depth buffer" );
   }
}




/*
 * Clear the depth buffer.  If the depth buffer doesn't exist yet we'll
 * allocate it now.
 * This function is only called through DD.clear_depth_buffer.
 */
void gl_clear_depth_buffer( void )
{
   /* The loops in this function have been written so the IRIX 5.3
    * C compiler can unroll them.  Hopefully other compilers can too!
    */

   if (!CC.DepthBuffer) {
      gl_alloc_depth_buffer();
   }

   if (CC.DepthBuffer) {
      GLdepth clear_value = (GLdepth) (CC.Depth.Clear * DEPTH_SCALE);

      if (CC.Scissor.Enabled) {
	 /* only clear scissor region */
	 GLint y;
	 for (y=CC.Scissor.Ymin; y<=CC.Scissor.Ymax; y++) {
            GLdepth *d = Z_ADDRESS( CC.Scissor.Xmin, y );
            GLint n = CC.Scissor.Xmax - CC.Scissor.Xmin + 1;
            do {
               *d++ = clear_value;
               n--;
            } while (n);
	 }
      }
      else {
	 /* clear whole buffer */
	 if (sizeof(GLdepth)==2 && (clear_value&0xff)==(clear_value>>8)) {
            /* lower and upper bytes of clear_value are same, use MEMSET */
	    MEMSET( CC.DepthBuffer, clear_value&0xff,
                    2*CC.BufferWidth*CC.BufferHeight);
	 }
	 else {
	    GLdepth *d = CC.DepthBuffer;
	    GLint n = CC.BufferWidth * CC.BufferHeight;
	    while (n>=16) {
	       d[0] = clear_value;    d[1] = clear_value;
	       d[2] = clear_value;    d[3] = clear_value;
	       d[4] = clear_value;    d[5] = clear_value;
	       d[6] = clear_value;    d[7] = clear_value;
	       d[8] = clear_value;    d[9] = clear_value;
	       d[10] = clear_value;   d[11] = clear_value;
	       d[12] = clear_value;   d[13] = clear_value;
	       d[14] = clear_value;   d[15] = clear_value;
	       d += 16;
	       n -= 16;
	    }
	    while (n>0) {
	       *d++ = clear_value;
	       n--;
	    }
	 }
      }
   }
}



