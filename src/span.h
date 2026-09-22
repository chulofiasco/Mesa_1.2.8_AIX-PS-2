/* span.h */

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
$Id: span.h,v 1.8 1995/12/30 01:01:05 brianp Exp $

$Log: span.h,v $
 * Revision 1.8  1995/12/30  01:01:05  brianp
 * gl_write_monocolor_span now takes integer color, not floating point
 *
 * Revision 1.7  1995/12/18  17:27:22  brianp
 * use new GLdepth datatype
 *
 * Revision 1.6  1995/06/12  15:42:53  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.5  1995/05/31  14:57:36  brianp
 * added gl_read_index_span() and gl_read_color_span()
 *
 * Revision 1.4  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.3  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/27  22:49:11  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  14:28:31  brianp
 * Initial revision
 *
 */


#ifndef SPAN_H
#define SPAN_H


#include "GL/gl.h"


extern void gl_write_index_span( GLuint n, GLint x, GLint y, GLdepth z[],
				 GLuint index[], GLenum primitive );


extern void gl_write_monoindex_span( GLuint n, GLint x, GLint y, GLdepth z[],
				     GLuint index, GLenum primitive );


extern void gl_write_color_span( GLuint n, GLint x, GLint y, GLdepth z[],
				 GLubyte red[], GLubyte green[],
				 GLubyte blue[], GLubyte alpha[],
				 GLenum primitive );


extern void gl_write_monocolor_span( GLuint n, GLint x, GLint y, GLdepth z[],
				     GLint r, GLint g, GLint b, GLint a,
                                     GLenum primitive );


extern void gl_write_texture_span( GLuint n, GLint x, GLint y, GLdepth z[],
				   GLfloat s[], GLfloat t[],
				   GLubyte red[], GLubyte green[],
				   GLubyte blue[], GLubyte alpha[],
				   GLenum primitive );


extern void gl_read_color_span( GLuint n, GLint x, GLint y,
			        GLubyte red[], GLubyte green[],
			        GLubyte blue[], GLubyte alpha[] );


extern void gl_read_index_span( GLuint n, GLint x, GLint y, GLuint indx[] );


#endif
