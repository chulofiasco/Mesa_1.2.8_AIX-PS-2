/* eval.h */

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
$Id: eval.h,v 1.3 1995/05/22 20:59:34 brianp Exp $

$Log: eval.h,v $
 * Revision 1.3  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.2  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:22:00  brianp
 * Initial revision
 *
 */


#ifndef EVAL_H
#define EVAL_H

#include "GL/gl.h"


extern void gl_init_eval( void );


extern void gl_free_control_points( GLenum target, GLfloat *data );


extern void gl_map1( GLenum target, GLfloat u1, GLfloat u2, GLint stride,
		     GLint order, const GLfloat *points );

extern void gl_map2( GLenum target,
		     GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
		     GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
		     const GLfloat *points );


extern void gl_evalcoord1( GLfloat u );

extern void gl_evalcoord2( GLfloat u, GLfloat v );


extern void gl_mapgrid1( GLint un, GLfloat u1, GLfloat u2 );

extern void gl_mapgrid2( GLint un, GLfloat u1, GLfloat u2,
			 GLint vn, GLfloat v1, GLfloat v2 );

#endif
