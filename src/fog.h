/* fog.h */

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
$Id: fog.h,v 1.9 1995/12/30 17:16:17 brianp Exp $

$Log: fog.h,v $
 * Revision 1.9  1995/12/30  17:16:17  brianp
 * gl_fog_color_vertices now takes integer colors instead of floats
 *
 * Revision 1.8  1995/12/20  15:26:39  brianp
 * changed color index array arguments to GLuint
 *
 * Revision 1.7  1995/12/18  17:25:35  brianp
 * use new GLdepth datatype, replace MAX_DEPTH with DEPTH_SCALE
 *
 * Revision 1.6  1995/11/03  22:35:35  brianp
 * replaced gl_fog_color_vertex with gl_fog_color_vertices
 * replaced gl_fog_index_vertex with gl_fog_index_vertices
 *
 * Revision 1.5  1995/06/12  15:33:31  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.4  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.3  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/27  22:48:52  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  14:23:03  brianp
 * Initial revision
 *
 */


#ifndef FOG_H
#define FOG_H


#include "GL/gl.h"


extern void gl_fog( GLenum pname, const GLfloat *params );


extern void
gl_fog_color_vertices( GLuint n, GLfloat v[][4], GLfixed color[][4] );

extern void
gl_fog_index_vertices( GLuint n, GLfloat v[][4], GLuint indx[] );



extern void gl_fog_color_pixels( GLuint n, const GLdepth z[],
				 GLubyte red[], GLubyte green[],
				 GLubyte blue[], GLubyte alpha[] );

extern void gl_fog_index_pixels( GLuint n, const GLdepth z[], GLuint indx[] );


#endif
