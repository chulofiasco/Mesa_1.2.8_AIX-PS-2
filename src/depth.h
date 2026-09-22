/* depth.h */

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
$Id: depth.h,v 1.8 1996/05/01 15:47:48 brianp Exp $

$Log: depth.h,v $
 * Revision 1.8  1996/05/01  15:47:48  brianp
 * added gl_read_depth_span_int()
 *
 * Revision 1.7  1996/01/12  22:30:38  brianp
 * added Z_ADDRESS() macro
 *
 * Revision 1.6  1995/12/18  17:25:20  brianp
 * use new GLdepth datatype
 *
 * Revision 1.5  1995/06/09  20:20:50  brianp
 * renamed gl_depth_test() as gl_depth_test_span() and return 'passed' count
 *
 * Revision 1.4  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.3  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/27  22:48:41  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  14:20:31  brianp
 * Initial revision
 *
 */


#ifndef DEPTH_H
#define DEPTH_H



#include "GL/gl.h"


/*
 * Return the address of the Z-buffer value for window coordinate (x,y):
 */
#define Z_ADDRESS( X, Y )  (CC.DepthBuffer + CC.BufferWidth * (Y) + (X))




extern GLuint gl_depth_test_span( GLuint n, GLint x, GLint y,
				  const GLdepth z[], GLubyte mask[] );


extern void gl_depth_test_pixels( GLuint n, const GLint x[], const GLint y[],
                                  const GLdepth z[], GLubyte mask[] );


extern void gl_read_depth_span_float( GLuint n, GLint x, GLint y,
                                      GLfloat depth[] );


extern void gl_read_depth_span_int( GLuint n, GLint x, GLint y,
                                    GLdepth depth[] );


extern void gl_alloc_depth_buffer( void );


extern void gl_clear_depth_buffer( void );


#endif
