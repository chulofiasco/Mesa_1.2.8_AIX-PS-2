/* $Id: masking.h,v 1.2 1995/10/30 15:33:24 brianp Exp $ */

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
$Log: masking.h,v $
 * Revision 1.2  1995/10/30  15:33:24  brianp
 * added mask argument to gl_mask_[color|index]_pixels functions
 *
 * Revision 1.1  1995/10/13  22:40:00  brianp
 * Initial revision
 *
 */


#ifndef MASKING_H
#define MASKING_H



#include "GL/gl.h"



/*
 * Implement glColorMask for a span of RGBA pixels.
 */
extern void gl_mask_color_span( GLuint n, GLint x, GLint y,
                                GLubyte red[], GLubyte green[],
                                GLubyte blue[], GLubyte alpha[] );



/*
 * Implement glColorMask for an array of RGBA pixels.
 */
extern void gl_mask_color_pixels( GLuint n, const GLint x[], const GLint y[],
                                  GLubyte red[], GLubyte green[],
                                  GLubyte blue[], GLubyte alpha[],
                                  const GLubyte mask[] );



/*
 * Implement glIndexMask for a span of CI pixels.
 */
extern void gl_mask_index_span( GLuint n, GLint x, GLint y, GLuint index[] );



/*
 * Implement glIndexMask for an array of CI pixels.
 */
extern void gl_mask_index_pixels( GLuint n, const GLint x[], const GLint y[],
                                  GLuint index[], const GLubyte mask[] );



#endif

