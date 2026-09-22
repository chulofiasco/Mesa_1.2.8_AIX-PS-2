/* stencil.h */

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
$Id: stencil.h,v 1.5 1995/12/18 17:28:08 brianp Exp $

$Log: stencil.h,v $
 * Revision 1.5  1995/12/18  17:28:08  brianp
 * use new GLdepth datatype
 *
 * Revision 1.4  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.3  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/03/01  17:44:38  brianp
 * added stenciling for PB
 *
 * Revision 1.1  1995/02/24  14:28:31  brianp
 * Initial revision
 *
 */


#ifndef STENCIL_H
#define STENCIL_H


#include "GL/gl.h"


extern GLint gl_stencil_span( GLuint n, GLint x, GLint y, GLubyte mask[] );


extern void gl_depth_stencil_span( GLuint n, GLint x, GLint y,
				   const GLdepth z[], GLubyte mask[] );


extern GLint gl_stencil_pixels( GLuint n, const GLint x[], const GLint y[],
			        GLubyte mask[] );


extern void gl_depth_stencil_pixels( GLuint n, const GLint x[],
				     const GLint y[], const GLdepth z[],
				     GLubyte mask[] );


extern void gl_read_stencil_span( GLuint n, GLint x, GLint y,
				  GLubyte stencil[] );


extern void gl_write_stencil_span( GLuint n, GLint x, GLint y,
				   const GLubyte stencil[] );


extern void gl_alloc_stencil_buffer( void );


extern void gl_clear_stencil_buffer( void );


#endif
