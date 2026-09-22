/* pixel.h */

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
$Id: pixel.h,v 1.7 1995/12/19 16:42:44 brianp Exp $

$Log: pixel.h,v $
 * Revision 1.7  1995/12/19  16:42:44  brianp
 * added gl_pixel_transfer()
 *
 * Revision 1.6  1995/12/18  17:26:35  brianp
 * use new GLdepth datatype
 *
 * Revision 1.5  1995/10/16  15:26:34  brianp
 * added gl_write_zoomed_stencil_span
 *
 * Revision 1.4  1995/10/14  16:28:56  brianp
 * added glPixelZoom support
 *
 * Revision 1.3  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.2  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:25:08  brianp
 * Initial revision
 *
 */


#ifndef PIXEL_H
#define PIXEL_H


#include "GL/gl.h"


extern void gl_pixel_map( GLenum map, GLint mapsize, const GLfloat *values );

extern void gl_pixel_transfer( GLenum pname, GLfloat param );


extern void gl_flip_bytes( GLubyte *p, GLuint n );


extern GLvoid *gl_unpack( GLsizei width, GLsizei height, GLenum format,
			  GLenum type, const GLvoid *pixels );


extern void
gl_write_zoomed_color_span( GLuint n, GLint x, GLint y, const GLdepth z[],
                            const GLubyte red[], const GLubyte green[],
                            const GLubyte blue[], const GLubyte alpha[],
                            GLint y0 );


extern void
gl_write_zoomed_index_span( GLuint n, GLint x, GLint y, const GLdepth z[],
                            const GLuint indexes[], GLint y0 );


extern void
gl_write_zoomed_stencil_span( GLuint n, GLint x, GLint y,
                              const GLubyte stencil[], GLint y0 );


#endif

