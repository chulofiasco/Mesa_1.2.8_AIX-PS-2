/* xform.h */

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
$Id: xform.h,v 1.15 1996/02/15 16:52:29 brianp Exp $

$Log: xform.h,v $
 * Revision 1.15  1996/02/15  16:52:29  brianp
 * renamed a few transformation functions and added gl_xform_points_3fv()
 *
 * Revision 1.14  1995/12/30  17:19:58  brianp
 * new, faster glTranslate and glScale functions
 *
 * Revision 1.13  1995/12/19  22:17:36  brianp
 * added CC.RasterOffsetX/Y to MAP_X() and MAP_Y()
 *
 * Revision 1.12  1995/09/27  18:30:44  brianp
 * added gl_transform_normals
 *
 * Revision 1.11  1995/09/26  16:05:08  brianp
 * removed gl_transform_point, gl_transform_normal
 * added gl_transform_points
 *
 * Revision 1.10  1995/09/13  14:46:18  brianp
 * changed TRANSFORM_POINT and TRANSFORM_NORMAL macros
 * removed TRANSFORM_XYZW macro
 *
 * Revision 1.9  1995/07/25  16:42:34  brianp
 * added TRANSFORM_XYZW macro
 *
 * Revision 1.8  1995/06/02  18:52:50  brianp
 * replaced 0.00001 with 0.00001F (avoid a double/float conversion)
 *
 * Revision 1.7  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.6  1995/03/24  17:01:15  brianp
 * removed assert() call
 *
 * Revision 1.5  1995/03/10  15:20:06  brianp
 * added divide by zero check to TRANSFORM_NORMAL
 *
 * Revision 1.4  1995/03/09  21:42:45  brianp
 * new ModelViewInv matrix logic
 *
 * Revision 1.3  1995/03/09  20:08:39  brianp
 * introduced TRANSFORM_POINT and TRANSFORM_NORMAL macros
 *
 * Revision 1.2  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:28:31  brianp
 * Initial revision
 *
 */


#ifndef XFORM_H
#define XFORM_H


#include "context.h"


/* Map NDC coords to window coords: */
#define MAP_X(X)   ( (X) * CC.Viewport.Sx + CC.Viewport.Tx + CC.RasterOffsetX )
#define MAP_Y(Y)   ( (Y) * CC.Viewport.Sy + CC.Viewport.Ty + CC.RasterOffsetY )
#define MAP_Z(Z)   ( (Z) * CC.Viewport.Sz + CC.Viewport.Tz )



/*
 * Transform a point (column vector) by a matrix:   Q = M * P
 */
#define TRANSFORM_POINT( Q, M, P )					\
   Q[0] = M[0] * P[0] + M[4] * P[1] + M[8] *  P[2] + M[12] * P[3];	\
   Q[1] = M[1] * P[0] + M[5] * P[1] + M[9] *  P[2] + M[13] * P[3];	\
   Q[2] = M[2] * P[0] + M[6] * P[1] + M[10] * P[2] + M[14] * P[3];	\
   Q[3] = M[3] * P[0] + M[7] * P[1] + M[11] * P[2] + M[15] * P[3];


/*
 * Transform a normal (row vector) by a matrix:  [NX NY NZ] = N * MAT
 */
#define TRANSFORM_NORMAL( NX, NY, NZ, N, MAT )		\
   NX = N[0] * MAT[0] + N[1] * MAT[1] + N[2] * MAT[2];	\
   NY = N[0] * MAT[4] + N[1] * MAT[5] + N[2] * MAT[6];	\
   NZ = N[0] * MAT[8] + N[1] * MAT[9] + N[2] * MAT[10];	\



extern void gl_xform_points_4fv( GLuint n, GLfloat q[][4], const GLfloat m[16],
                                 GLfloat p[][4] );


extern void gl_xform_points_3fv( GLuint n, GLfloat q[][4], const GLfloat m[16],
                                 GLfloat p[][3] );


extern void gl_xform_normals_3fv( GLuint n, GLfloat v[][3],
                                  const GLfloat m[16],
                                  GLfloat u[][3], GLboolean normalize );


extern void gl_transform_vector( GLfloat u[4],
				 const GLfloat v[4], const GLfloat m[16] );


extern void gl_compute_modelview_inverse( void );


extern void gl_load_matrix( const GLfloat *m );

extern void gl_mult_matrix( const GLfloat *m );

extern void gl_viewport( GLint x, GLint y, GLsizei width, GLsizei height );

extern void gl_compute_viewport( struct gl_context *c,
				 GLint x, GLint y,
				 GLsizei width, GLsizei height );


extern void gl_scale( GLfloat x, GLfloat y, GLfloat z );

extern void gl_translate( GLfloat x, GLfloat y, GLfloat z );


#endif
