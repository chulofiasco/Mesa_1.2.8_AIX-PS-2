/* $Id: alphabuf.h,v 1.1 1996/02/19 21:53:51 brianp Exp $ */

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
$Log: alphabuf.h,v $
 * Revision 1.1  1996/02/19  21:53:51  brianp
 * Initial revision
 *
 */



#ifndef ALPHABUF_H
#define ALPHABUF_H


#include "context.h"


extern void gl_alloc_alpha_buffers( void );


extern void gl_clear_alpha_buffers( void );


extern void gl_write_alpha_span( GLuint n, GLint x, GLint y,
                                 GLubyte alpha[], GLubyte mask[] );


extern void gl_write_mono_alpha_span( GLuint n, GLint x, GLint y,
                                      GLubyte alpha, GLubyte mask[] );



extern void gl_write_alpha_pixels( GLuint n, const GLint x[], const GLint y[],
                                   const GLubyte alpha[],
                                   const GLubyte mask[] );


extern void gl_write_mono_alpha_pixels( GLuint n, const GLint x[],
                                        const GLint y[], GLubyte alpha,
                                        const GLubyte mask[] );


extern void  gl_read_alpha_span( GLuint n, GLint x, GLint y, GLubyte alpha[] );


extern void gl_read_alpha_pixels( GLuint n, const GLint x[], const GLint y[],
                                  GLubyte alpha[], const GLubyte mask[] );


#endif

