/* $Id: xform.c,v 1.32 1996/04/25 20:42:13 brianp Exp $ */

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
$Log: xform.c,v $
 * Revision 1.32  1996/04/25  20:42:13  brianp
 * replaced gl_alloc_depth_buffer() call with DD.alloc_depth_buffer()
 *
 * Revision 1.31  1996/04/18  14:55:31  brianp
 * fixed a few erroneous error messages
 *
 * Revision 1.30  1996/04/11  20:03:16  brianp
 * fixed CC.IdentityTexMat bug, cleaned up glTranslate() and glScale() code
 *
 * Revision 1.29  1996/03/22  20:54:08  brianp
 * removed CC.ClipSpans stuff
 *
 * Revision 1.28  1996/02/23  17:10:40  brianp
 * optimized matmul() and gl_transform_vector() per Jean-Luc Daems
 *
 * Revision 1.27  1996/02/19  21:50:00  brianp
 * added support for software alpha buffering
 *
 * Revision 1.26  1996/02/15  16:52:29  brianp
 * renamed a few transformation functions and added gl_xform_points_3fv()
 *
 * Revision 1.25  1995/12/30  17:19:58  brianp
 * new, faster glTranslate and glScale functions
 *
 * Revision 1.24  1995/12/19  17:07:49  brianp
 * new implementation of glMatrixMode
 *
 * Revision 1.23  1995/12/12  21:47:15  brianp
 * optimized gl_transform_vertices() by checking for common matrix values
 *
 * Revision 1.22  1995/10/31  19:40:35  brianp
 * removed INLINE macro, more trouble than it was worth
 *
 * Revision 1.21  1995/10/16  15:26:56  brianp
 * replaced dd_buffer_info with DD.buffer_size call
 *
 * Revision 1.20  1995/09/30  15:35:13  brianp
 * added test for identity texture matrix in gl_mult_matrix
 *
 * Revision 1.19  1995/09/27  18:31:40  brianp
 * added gl_transform_normals
 *
 * Revision 1.18  1995/09/26  16:05:26  brianp
 * removed gl_transform_point, gl_transform_normal
 * added gl_transform_points
 *
 * Revision 1.17  1995/09/13  14:42:23  brianp
 * added logic to test for identity texture matrix
 *
 * Revision 1.16  1995/07/24  20:35:20  brianp
 * replaced memset() with MEMSET() and memcpy() with MEMCPY()
 *
 * Revision 1.15  1995/07/24  18:58:28  brianp
 * correct gl_viewport() semantics, introduced CC.ClipSpans flag
 *
 * Revision 1.14  1995/06/21  15:11:08  brianp
 * better allocation policy for depth, stencil buffer in gl_viewport
 *
 * Revision 1.13  1995/06/20  16:26:32  brianp
 * removed fudge value from viewport transformation scale and translation
 *
 * Revision 1.12  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.11  1995/05/12  16:57:22  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.10  1995/04/19  13:48:18  brianp
 * renamed occurances of near and far for SCO x86 Unix
 *
 * Revision 1.9  1995/04/01  16:17:47  brianp
 * fixed mult_matrix bug in glTranslatef and glScalef
 *
 * Revision 1.8  1995/03/28  16:10:33  brianp
 * removed singular matrix error message
 *
 * Revision 1.7  1995/03/22  21:37:47  brianp
 * removed mode from dd_buffer_info()
 *
 * Revision 1.6  1995/03/10  17:13:20  brianp
 * new matmul and invert_matrix functions from Thomas Malik
 *
 * Revision 1.5  1995/03/10  15:19:43  brianp
 * added divide by zero check to gl_transform_normal
 *
 * Revision 1.4  1995/03/09  21:42:30  brianp
 * new ModelViewInv matrix logic
 *
 * Revision 1.3  1995/03/09  20:08:48  brianp
 * changed order of arguments to gl_transform_ functions to be more logical
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:28:31  brianp
 * Initial revision
 *
 */


/*
 * Geometry Transformation.
 *
 *
 * NOTES:
 * 1. 4x4 transformation matrices are stored in memory in column major order.
 * 2. Points/vertices are to be thought of as column vectors.
 * 3. Transformation of a point p by a matrix M is: p' = M * p
 *
 */



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alphabuf.h"
#include "context.h"
#include "depth.h"
#include "dd.h"
#include "list.h"
#include "macros.h"
#include "stencil.h"



#ifndef M_PI
#  define M_PI (3.1415926)
#endif



static GLfloat Identity[16] = {
   1.0, 0.0, 0.0, 0.0,
   0.0, 1.0, 0.0, 0.0,
   0.0, 0.0, 1.0, 0.0,
   0.0, 0.0, 0.0, 1.0
};




#ifdef DEBUG
static void print_matrix( const GLfloat m[16] )
{
   int i;

   for (i=0;i<4;i++) {
      printf("%f %f %f %f\n", m[i], m[4+i], m[8+i], m[12+i] );
   }
}
#endif



/*
 * Perform a 4x4 matrix multiplication  (product = a x b).
 * Input:  a, b - matrices to multiply
 * Output:  product - product of a and b
 */
static void
matmul( GLfloat *product, const GLfloat *a, const GLfloat *b )
{
   /* This matmul was contributed by Thomas Malik */
   GLfloat temp[16];
   GLint i;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define T(row,col)  temp[(col<<2)+row]

   /* i-te Zeile */
   for (i = 0; i < 4; i++) {
      GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      T(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      T(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      T(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      T(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }

#undef A
#undef B
#undef T
   MEMCPY( product, temp, 16*sizeof(GLfloat) );
}



/*
 * Find the inverse of the 4 by 4 matrix b using gausian elimination
 * and return it in a.
 *
 * This function was contributed by Thomas Malik (malik@rhrk.uni-kl.de).
 * Thanks Thomas!
 */
static void invert_matrix(const GLfloat *b,GLfloat * a)
{
#define MAT(m,r,c) ((m)[(c)*4+(r)])

  GLfloat val, val2;
  GLint   i, j, k, ind;
  GLfloat tmp[16];

  MEMCPY(a,Identity,sizeof(float)*16);
  MEMCPY(tmp, b,sizeof(float)*16);

  for (i = 0; i != 4; i++) {

    val = MAT(tmp,i,i);			/* find pivot */
    ind = i;
    for (j = i + 1; j != 4; j++) {
      if (fabs(MAT(tmp,j,i)) > fabs(val)) {
	ind = j;
	val = MAT(tmp,j,i);
      }
    }

    if (ind != i) {			/* swap columns */
      for (j = 0; j != 4; j++) {
	val2 = MAT(a,i,j);
	MAT(a,i,j) = MAT(a,ind,j);
	MAT(a,ind,j) = val2;
	val2 = MAT(tmp,i,j);
	MAT(tmp,i,j) = MAT(tmp,ind,j);
	MAT(tmp,ind,j) = val2;
      }
    }

    if (val == 0.0F) {	
       /* The matrix is singular (has no inverse).  This isn't really
	* an error since singular matrices can be used for projecting
	* shadows, etc.  We let the inverse be the identity matrix.
	*/
       /*fprintf(stderr,"Singular matrix, no inverse!\n");*/
       MEMCPY( a, Identity, 16*sizeof(GLfloat) );
       return;
    }

    for (j = 0; j != 4; j++) {
      MAT(tmp,i,j) /= val;
      MAT(a,i,j) /= val;
    }

    for (j = 0; j != 4; j++) {		/* eliminate column */
      if (j == i)
	continue;
      val = MAT(tmp,j,i);
      for (k = 0; k != 4; k++) {
	MAT(tmp,j,k) -= MAT(tmp,i,k) * val;
	MAT(a,j,k) -= MAT(a,i,k) * val;
      }
    }
  }
#undef MAT
}




/*
 * Compute the inverse of the current ModelViewMatrix.
 */
void gl_compute_modelview_inverse( void )
{
   invert_matrix( CC.ModelViewMatrix, CC.ModelViewInv );
   CC.ModelViewInvValid = GL_TRUE;
}



/*
 * Apply a transformation matrix to an array of [X Y Z W] coordinates:
 *   for i in 0 to n-1 do   q[i] = m * p[i]
 * where p[i] and q[i] are 4-element column vectors and m is a 16-element
 * transformation matrix.
 */
void gl_xform_points_4fv( GLuint n, GLfloat q[][4], const GLfloat m[16],
                          GLfloat p[][4] )
{
   /* This function has been carefully crafted to maximize register usage
    * and use loop unrolling with IRIX 5.3's cc.  Hopefully other compilers
    * will like this code too.
    */
   {
      GLuint i;
      GLfloat m0 = m[0],  m4 = m[4],  m8 = m[8],  m12 = m[12];
      GLfloat m1 = m[1],  m5 = m[5],  m9 = m[9],  m13 = m[13];
      if (m12==0.0F && m13==0.0F) {
         /* common case */
         for (i=0;i<n;i++) {
            GLfloat p0 = p[i][0], p1 = p[i][1], p2 = p[i][2];
            q[i][0] = m0 * p0 + m4  * p1 + m8 * p2;
            q[i][1] = m1 * p0 + m5  * p1 + m9 * p2;
         }
      }
      else {
         /* general case */
         for (i=0;i<n;i++) {
            GLfloat p0 = p[i][0], p1 = p[i][1], p2 = p[i][2], p3 = p[i][3];
            q[i][0] = m0 * p0 + m4  * p1 + m8 * p2 + m12 * p3;
            q[i][1] = m1 * p0 + m5  * p1 + m9 * p2 + m13 * p3;
         }
      }
   }
   {
      GLuint i;
      GLfloat m2 = m[2],  m6 = m[6],  m10 = m[10],  m14 = m[14];
      GLfloat m3 = m[3],  m7 = m[7],  m11 = m[11],  m15 = m[15];
      if (m3==0.0F && m7==0.0F && m11==0.0F && m15==1.0F) {
         /* common case */
         for (i=0;i<n;i++) {
            GLfloat p0 = p[i][0], p1 = p[i][1], p2 = p[i][2], p3 = p[i][3];
            q[i][2] = m2 * p0 + m6 * p1 + m10 * p2 + m14 * p3;
            q[i][3] = p3;
         }
      }
      else {
         /* general case */
         for (i=0;i<n;i++) {
            GLfloat p0 = p[i][0], p1 = p[i][1], p2 = p[i][2], p3 = p[i][3];
            q[i][2] = m2 * p0 + m6 * p1 + m10 * p2 + m14 * p3;
            q[i][3] = m3 * p0 + m7 * p1 + m11 * p2 + m15 * p3;
         }
      }
   }
}



/*
 * Apply a transformation matrix to an array of [X Y Z] coordinates:
 *   for i in 0 to n-1 do   q[i] = m * p[i]
 */
void gl_xform_points_3fv( GLuint n, GLfloat q[][4], const GLfloat m[16],
                          GLfloat p[][3] )
{
   /* This function has been carefully crafted to maximize register usage
    * and use loop unrolling with IRIX 5.3's cc.  Hopefully other compilers
    * will like this code too.
    */
   {
      GLuint i;
      GLfloat m0 = m[0],  m4 = m[4],  m8 = m[8],  m12 = m[12];
      GLfloat m1 = m[1],  m5 = m[5],  m9 = m[9],  m13 = m[13];
      for (i=0;i<n;i++) {
         GLfloat p0 = p[i][0], p1 = p[i][1], p2 = p[i][2];
         q[i][0] = m0 * p0 + m4  * p1 + m8 * p2 + m12;
         q[i][1] = m1 * p0 + m5  * p1 + m9 * p2 + m13;
      }
   }
   {
      GLuint i;
      GLfloat m2 = m[2],  m6 = m[6],  m10 = m[10],  m14 = m[14];
      GLfloat m3 = m[3],  m7 = m[7],  m11 = m[11],  m15 = m[15];
      if (m3==0.0F && m7==0.0F && m11==0.0F && m15==1.0F) {
         /* common case */
         for (i=0;i<n;i++) {
            GLfloat p0 = p[i][0], p1 = p[i][1], p2 = p[i][2];
            q[i][2] = m2 * p0 + m6 * p1 + m10 * p2 + m14;
            q[i][3] = 1.0F;
         }
      }
      else {
         /* general case */
         for (i=0;i<n;i++) {
            GLfloat p0 = p[i][0], p1 = p[i][1], p2 = p[i][2];
            q[i][2] = m2 * p0 + m6 * p1 + m10 * p2 + m14;
            q[i][3] = m3 * p0 + m7 * p1 + m11 * p2 + m15;
         }
      }
   }
}



/*
 * Apply a transformation matrix to an array of normal vectors:
 *   for i in 0 to n-1 do  v[i] = u[i] * m
 * where u[i] and v[i] are 3-element row vectors and m is a 16-element
 * transformation matrix.
 * If the normalize flag is true the normals will be scaled to length 1.
 */
void gl_xform_normals_3fv( GLuint n, GLfloat v[][3], const GLfloat m[16],
                           GLfloat u[][3], GLboolean normalize )
{
   if (normalize) {
      /* Transform normals and scale to unit length */
      GLuint i;
      GLfloat m0 = m[0],  m4 = m[4],  m8 = m[8];
      GLfloat m1 = m[1],  m5 = m[5],  m9 = m[9];
      GLfloat m2 = m[2],  m6 = m[6],  m10 = m[10];
      for (i=0;i<n;i++) {
         GLdouble tx, ty, tz;
         {
            GLfloat ux = u[i][0],  uy = u[i][1],  uz = u[i][2];
            tx = ux * m0 + uy * m1 + uz * m2;
            ty = ux * m4 + uy * m5 + uz * m6;
            tz = ux * m8 + uy * m9 + uz * m10;
         }
         {
            GLdouble len, scale;
            len = sqrt( tx*tx + ty*ty + tz*tz );
            scale = (len>0.00001) ? (1.0 / len) : 1.0;
            v[i][0] = tx * scale;
            v[i][1] = ty * scale;
            v[i][2] = tz * scale;
         }
      }
   }
   else {
      /* Just transform normals, don't scale */
      GLuint i;
      GLfloat m0 = m[0],  m4 = m[4],  m8 = m[8];
      GLfloat m1 = m[1],  m5 = m[5],  m9 = m[9];
      GLfloat m2 = m[2],  m6 = m[6],  m10 = m[10];
      for (i=0;i<n;i++) {
         GLfloat ux = u[i][0],  uy = u[i][1],  uz = u[i][2];
         v[i][0] = ux * m0 + uy * m1 + uz * m2;
         v[i][1] = ux * m4 + uy * m5 + uz * m6;
         v[i][2] = ux * m8 + uy * m9 + uz * m10;
      }
   }
}



/*
 * Transform a 4-element row vector (1x4 matrix) by a 4x4 matrix.  This
 * function is used for transforming clipping plane equations and spotlight
 * directions.
 * Mathematically,  u = v * m.
 * Input:  v - input vector
 *         m - transformation matrix
 * Output:  u - transformed vector
 */
void gl_transform_vector( GLfloat u[4], const GLfloat v[4], const GLfloat m[16] )
{
   GLfloat v0=v[0], v1=v[1], v2=v[2], v3=v[3];
#define M(row,col)  m[col*4+row]
   u[0] = v0 * M(0,0) + v1 * M(1,0) + v2 * M(2,0) + v3 * M(3,0);
   u[1] = v0 * M(0,1) + v1 * M(1,1) + v2 * M(2,1) + v3 * M(3,1);
   u[2] = v0 * M(0,2) + v1 * M(1,2) + v2 * M(2,2) + v3 * M(3,2);
   u[3] = v0 * M(0,3) + v1 * M(1,3) + v2 * M(2,3) + v3 * M(3,3);
#undef M
}



void glMatrixMode( GLenum mode )
{
   if (CC.CompileFlag) {
      gl_save_matrixmode( mode );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
         gl_error( GL_INVALID_OPERATION, "glMatrixMode" );
         return;
      }
      switch (mode) {
         case GL_MODELVIEW:
         case GL_PROJECTION:
         case GL_TEXTURE:
            CC.Transform.MatrixMode = mode;
            break;
         default:
            gl_error( GL_INVALID_ENUM, "glMatrixMode" );
      }
   }
}



void glOrtho( GLdouble left, GLdouble right,
	      GLdouble bottom, GLdouble top,
	      GLdouble nearval, GLdouble farval )
{
   GLfloat x, y, z;
   GLfloat tx, ty, tz;
   GLfloat m[16];

   x = 2.0 / (right-left);
   y = 2.0 / (top-bottom);
   z = -2.0 / (farval-nearval);
   tx = -(right+left) / (right-left);
   ty = -(top+bottom) / (top-bottom);
   tz = -(farval+nearval) / (farval-nearval);

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = 0.0F;  M(0,3) = tx;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = 0.0F;  M(1,3) = ty;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = z;     M(2,3) = tz;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = 0.0F;  M(3,3) = 1.0F;
#undef M

   glMultMatrixf( m );
}



void glFrustum( GLdouble left, GLdouble right,
		GLdouble bottom, GLdouble top,
		GLdouble nearval, GLdouble farval )
{
   GLfloat x, y, a, b, c, d;
   GLfloat m[16];

   if (nearval<=0.0 || farval<=0.0) {
      gl_error( GL_INVALID_VALUE, "glFrustum(near or far)" );
   }

   x = (2.0*nearval) / (right-left);
   y = (2.0*nearval) / (top-bottom);
   a = (right+left) / (right-left);
   b = (top+bottom) / (top-bottom);
   c = -(farval+nearval) / ( farval-nearval);
   d = -(2.0*farval*nearval) / (farval-nearval);  /* error? */

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M

   glMultMatrixf( m );
}



/*
 * Define a new viewport and reallocate auxillary buffers if the size of
 * the window (color buffer) has changed.
 */
void gl_viewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
   GLint newsize;
   GLuint buf_width, buf_height, buf_depth;

   if (width<0 || height<0) {
      gl_error( GL_INVALID_VALUE, "glViewport" );
      return;
   }
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glViewport" );
      return;
   }

#ifdef LEAVEOUT
   /* clamp x, y, width, and height to implementation dependent range */
   x = CLAMP( x, 0, MAX_WIDTH-1 );
   y = CLAMP( y, 0, MAX_HEIGHT-1 );
   if (width<1) {
      width = 1;
   }
   if (height<1) {
      height = 1;
   }
   if (x+width>MAX_WIDTH) {
      width = MAX_WIDTH - x;
   }
   if (y+height>MAX_HEIGHT) {
      height = MAX_HEIGHT - y;
   }
#endif

   /* ask device driver for size of output buffer */
   (*DD.buffer_size)( &buf_width, &buf_height, &buf_depth );

   /* see if size of device driver buffer has changed */
   newsize = CC.BufferWidth!=buf_width || CC.BufferHeight!=buf_height;

   /* save buffer size */
   CC.BufferWidth = buf_width;
   CC.BufferHeight = buf_height;

   /* Save viewport */
   CC.Viewport.X = x;
   CC.Viewport.Width = width;
   CC.Viewport.Y = y;
   CC.Viewport.Height = height;

   /* compute scale and bias values */
   CC.Viewport.Sx = (GLfloat) width / 2.0F;
   CC.Viewport.Tx = CC.Viewport.Sx + x;
   CC.Viewport.Sy = (GLfloat) height / 2.0F;
   CC.Viewport.Ty = CC.Viewport.Sy + y;

   /* Reallocate other buffers if needed. */
   if (newsize && CC.DepthBuffer) {
      /* reallocate depth buffer, if there is one */
      (*DD.alloc_depth_buffer)();
   }
   if (newsize && CC.StencilBuffer) {
      /* reallocate stencil buffer, if there is one */
      gl_alloc_stencil_buffer();
   }
   if (newsize && CC.AccumBuffer) {
      /* Deallocate the accumulation buffer.  A new one will be allocated */
      /* if/when glAccum() is called again. */
      free( CC.AccumBuffer );
      CC.AccumBuffer = NULL;
   }
   if (newsize && (CC.FrontAlphaEnabled || CC.BackAlphaEnabled)) {
      gl_alloc_alpha_buffers();
   }

   CC.NewState = GL_TRUE;
}



void glViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
   if (CC.ExecuteFlag) {
      gl_viewport( x, y, width, height );
   }
   if (CC.CompileFlag) {
      gl_save_viewport( x, y, width, height );
   }
}




/*
 * Determine if the given matrix is the identity matrix.
 */
static GLboolean is_identity( const GLfloat m[16] )
{
   if (   m[0]==1.0F && m[4]==0.0F && m[ 8]==0.0F && m[12]==0.0F
       && m[1]==0.0F && m[5]==1.0F && m[ 9]==0.0F && m[13]==0.0F
       && m[2]==0.0F && m[6]==0.0F && m[10]==1.0F && m[14]==0.0F
       && m[3]==0.0F && m[7]==0.0F && m[11]==0.0F && m[15]==1.0F) {
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}




void glPushMatrix( void )
{
   if (CC.CompileFlag) {
      gl_save_pushmatrix();
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glPushMatrix" );
	 return;
      }
      switch (CC.Transform.MatrixMode) {
	 case GL_MODELVIEW:
	    if (CC.ModelViewStackDepth>=MAX_MODELVIEW_STACK_DEPTH-1) {
	       gl_error( GL_STACK_OVERFLOW, "glPushMatrix");
	       return;
	    }
	    MEMCPY( CC.ModelViewStack[CC.ModelViewStackDepth],
		    CC.ModelViewMatrix,
		    16*sizeof(GLfloat) );
	    CC.ModelViewStackDepth++;
	    break;
	 case GL_PROJECTION:
	    if (CC.ProjectionStackDepth>=MAX_PROJECTION_STACK_DEPTH) {
	       gl_error( GL_STACK_OVERFLOW, "glPushMatrix");
	       return;
	    }
	    MEMCPY( CC.ProjectionStack[CC.ProjectionStackDepth],
		    CC.ProjectionMatrix,
		    16*sizeof(GLfloat) );
	    CC.ProjectionStackDepth++;
	    break;
	 case GL_TEXTURE:
	    if (CC.TextureStackDepth>=MAX_TEXTURE_STACK_DEPTH) {
	       gl_error( GL_STACK_OVERFLOW, "glPushMatrix");
	       return;
	    }
	    MEMCPY( CC.TextureStack[CC.TextureStackDepth],
		    CC.TextureMatrix,
		    16*sizeof(GLfloat) );
	    CC.TextureStackDepth++;
	    break;
      }
   }
}



void glPopMatrix( void )
{
   if (CC.CompileFlag) {
      gl_save_popmatrix();
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glPopMatrix" );
	 return;
      }
      switch (CC.Transform.MatrixMode) {
	 case GL_MODELVIEW:
	    if (CC.ModelViewStackDepth==0) {
	       gl_error( GL_STACK_UNDERFLOW, "glPopMatrix");
	       return;
	    }
	    CC.ModelViewStackDepth--;
	    MEMCPY( CC.ModelViewMatrix,
		    CC.ModelViewStack[CC.ModelViewStackDepth],
		    16*sizeof(GLfloat) );
	    CC.ModelViewInvValid = GL_FALSE;
	    break;
	 case GL_PROJECTION:
	    if (CC.ProjectionStackDepth==0) {
	       gl_error( GL_STACK_UNDERFLOW, "glPopMatrix");
	       return;
	    }
	    CC.ProjectionStackDepth--;
	    MEMCPY( CC.ProjectionMatrix,
		    CC.ProjectionStack[CC.ProjectionStackDepth],
		    16*sizeof(GLfloat) );
	    break;
	 case GL_TEXTURE:
	    if (CC.TextureStackDepth==0) {
	       gl_error( GL_STACK_UNDERFLOW, "glPopMatrix");
	       return;
	    }
	    CC.TextureStackDepth--;
	    MEMCPY( CC.TextureMatrix,
		    CC.TextureStack[CC.TextureStackDepth],
		    16*sizeof(GLfloat) );
            CC.IdentityTexMat = is_identity( CC.TextureMatrix );
	    break;
      }
   }
}



void gl_load_matrix( const GLfloat *m )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glLoadMatrix" );
      return;
   }
   switch (CC.Transform.MatrixMode) {
      case GL_MODELVIEW:
         MEMCPY( CC.ModelViewMatrix, m, 16*sizeof(GLfloat) );
	 CC.ModelViewInvValid = GL_FALSE;
	 break;
      case GL_PROJECTION:
	 MEMCPY( CC.ProjectionMatrix, m, 16*sizeof(GLfloat) );
	 break;
      case GL_TEXTURE:
	 MEMCPY( CC.TextureMatrix, m, 16*sizeof(GLfloat) );
         CC.IdentityTexMat = is_identity( CC.TextureMatrix );
	 break;
   }
}



void glLoadMatrixd( const GLdouble *m )
{
   GLfloat fm[16];
   GLuint i;

   for (i=0;i<16;i++) {
      fm[i] = (GLfloat) m[i];
   }

   glLoadMatrixf( fm );
}



void glLoadMatrixf( const GLfloat *m )
{
   if (CC.CompileFlag) {
      gl_save_loadmatrix( m );
   }
   if (CC.ExecuteFlag) {
      gl_load_matrix( m );
   }
}



void glLoadIdentity( void )
{
   if (CC.CompileFlag) {
      gl_save_loadmatrix( Identity );
   }
   if (CC.ExecuteFlag) {
      gl_load_matrix( Identity );
   }
}



void gl_mult_matrix( const GLfloat *m )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glMultMatrix" );
      return;
   }
   switch (CC.Transform.MatrixMode) {
      case GL_MODELVIEW:
         matmul( CC.ModelViewMatrix, CC.ModelViewMatrix, m );
	 CC.ModelViewInvValid = GL_FALSE;
	 break;
      case GL_PROJECTION:
	 matmul( CC.ProjectionMatrix, CC.ProjectionMatrix, m );
	 break;
      case GL_TEXTURE:
	 matmul( CC.TextureMatrix, CC.TextureMatrix, m );
         CC.IdentityTexMat = is_identity( CC.TextureMatrix );
	 break;
   }
}



void glMultMatrixd( const GLdouble *m )
{
   GLfloat fm[16];
   GLuint i;

   for (i=0;i<16;i++) {
      fm[i] = (GLfloat) m[i];
   }

   glMultMatrixf( fm );
}



void glMultMatrixf( const GLfloat *m )
{
   if (CC.CompileFlag) {
      gl_save_multmatrix( m );
   }
   if (CC.ExecuteFlag) {
      gl_mult_matrix( m );
   }
}



void glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
   glRotatef( (GLfloat) angle, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}



void glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
   /* This function contributed by Erich Boleyn (erich@uruk.org) */

   GLfloat m[16];
   GLfloat mag, s, c;
   GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;

   s = sin( angle * (M_PI / 180.0) );
   c = cos( angle * (M_PI / 180.0) );

   mag = sqrt( x*x + y*y + z*z );

   if (mag == 0.0)
     return;

   x /= mag;
   y /= mag;
   z /= mag;

#define M(row,col)  m[col*4+row]

   /*
    *     Arbitrary axis rotation matrix.
    *
    *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
    *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
    *  (which is about the X-axis), and the two composite transforms
    *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
    *  from the arbitrary axis to the X-axis then back.  They are
    *  all elementary rotations.
    *
    *  Rz' is a rotation about the Z-axis, to bring the axis vector
    *  into the x-z plane.  Then Ry' is applied, rotating about the
    *  Y-axis to bring the axis vector parallel with the X-axis.  The
    *  rotation about the X-axis is then performed.  Ry and Rz are
    *  simply the respective inverse transforms to bring the arbitrary
    *  axis back to it's original orientation.  The first transforms
    *  Rz' and Ry' are considered inverses, since the data from the
    *  arbitrary axis gives you info on how to get to it, not how
    *  to get away from it, and an inverse must be applied.
    *
    *  The basic calculation used is to recognize that the arbitrary
    *  axis vector (x, y, z), since it is of unit length, actually
    *  represents the sines and cosines of the angles to rotate the
    *  X-axis to the same orientation, with theta being the angle about
    *  Z and phi the angle about Y (in the order described above)
    *  as follows:
    *
    *  cos ( theta ) = x / sqrt ( 1 - z^2 )
    *  sin ( theta ) = y / sqrt ( 1 - z^2 )
    *
    *  cos ( phi ) = sqrt ( 1 - z^2 )
    *  sin ( phi ) = z
    *
    *  Note that cos ( phi ) can further be inserted to the above
    *  formulas:
    *
    *  cos ( theta ) = x / cos ( phi )
    *  sin ( theta ) = y / sin ( phi )
    *
    *  ...etc.  Because of those relations and the standard trigonometric
    *  relations, it is pssible to reduce the transforms down to what
    *  is used below.  It may be that any primary axis chosen will give the
    *  same results (modulo a sign convention) using thie method.
    *
    *  Particularly nice is to notice that all divisions that might
    *  have caused trouble when parallel to certain planes or
    *  axis go away with care paid to reducing the expressions.
    *  After checking, it does perform correctly under all cases, since
    *  in all the cases of division where the denominator would have
    *  been zero, the numerator would have been zero as well, giving
    *  the expected result.
    */

   xx = x * x;
   yy = y * y;
   zz = z * z;
   xy = x * y;
   yz = y * z;
   zx = z * x;
   xs = x * s;
   ys = y * s;
   zs = z * s;
   one_c = 1.0F - c;

   M(0,0) = (one_c * xx) + c;
   M(0,1) = (one_c * xy) - zs;
   M(0,2) = (one_c * zx) + ys;
   M(0,3) = 0.0F;

   M(1,0) = (one_c * xy) + zs;
   M(1,1) = (one_c * yy) + c;
   M(1,2) = (one_c * yz) - xs;
   M(1,3) = 0.0F;

   M(2,0) = (one_c * zx) - ys;
   M(2,1) = (one_c * yz) + xs;
   M(2,2) = (one_c * zz) + c;
   M(2,3) = 0.0F;

   M(3,0) = 0.0F;
   M(3,1) = 0.0F;
   M(3,2) = 0.0F;
   M(3,3) = 1.0F;

#undef M

   if (CC.CompileFlag) {
      gl_save_multmatrix( m );
   }
   if (CC.ExecuteFlag) {
      gl_mult_matrix( m );
   }
}



/*
 * Execute a glScale call
 */
void gl_scale( GLfloat x, GLfloat y, GLfloat z )
{
   GLfloat *m;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glScale" );
      return;
   }
   switch (CC.Transform.MatrixMode) {
      case GL_MODELVIEW:
         m = CC.ModelViewMatrix;
	 CC.ModelViewInvValid = GL_FALSE;
	 break;
      case GL_PROJECTION:
         m = CC.ProjectionMatrix;
	 break;
      case GL_TEXTURE:
         m = CC.TextureMatrix;
	 break;
      default:
         return;
   }
   m[0] *= x;   m[4] *= y;   m[8]  *= z;
   m[1] *= x;   m[5] *= y;   m[9]  *= z;
   m[2] *= x;   m[6] *= y;   m[10] *= z;
   m[3] *= x;   m[7] *= y;   m[11] *= z;

   if (CC.Transform.MatrixMode==GL_TEXTURE) {
      CC.IdentityTexMat = is_identity( CC.TextureMatrix );
   }
}



void glScaled( GLdouble x, GLdouble y, GLdouble z )
{
   if (CC.CompileFlag) {
      gl_save_scale( (GLfloat) x, (GLfloat) y, (GLfloat) z );
   }
   if (CC.ExecuteFlag) {
      gl_scale( (GLfloat) x, (GLfloat) y, (GLfloat) z );
   }
}



void glScalef( GLfloat x, GLfloat y, GLfloat z )
{
   if (CC.CompileFlag) {
      gl_save_scale( x, y, z );
   }
   if (CC.ExecuteFlag) {
      gl_scale( x, y, z );
   }
}



/*
 * Execute a glTranslate call
 */
void gl_translate( GLfloat x, GLfloat y, GLfloat z )
{
   GLfloat *m;
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glTranslate" );
      return;
   }
   switch (CC.Transform.MatrixMode) {
      case GL_MODELVIEW:
         m = CC.ModelViewMatrix;
	 CC.ModelViewInvValid = GL_FALSE;
	 break;
      case GL_PROJECTION:
         m = CC.ProjectionMatrix;
	 break;
      case GL_TEXTURE:
         m = CC.TextureMatrix;
	 break;
      default:
         return;
   }

   m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
   m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
   m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
   m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];

   if (CC.Transform.MatrixMode==GL_TEXTURE) {
      CC.IdentityTexMat = is_identity( CC.TextureMatrix );
   }
}



void glTranslated( GLdouble x, GLdouble y, GLdouble z )
{
   if (CC.CompileFlag) {
      gl_save_translate( (GLfloat) x, (GLfloat) y, (GLfloat) z );
   }
   if (CC.ExecuteFlag) {
      gl_translate( (GLfloat) x, (GLfloat) y, (GLfloat) z );
   }
}



void glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
   if (CC.CompileFlag) {
      gl_save_translate( x, y, z );
   }
   if (CC.ExecuteFlag) {
      gl_translate( x, y, z );
   }
}
