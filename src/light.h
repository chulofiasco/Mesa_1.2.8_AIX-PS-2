/* $Id: light.h,v 1.11 1996/02/26 15:20:01 brianp Exp $ */

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
$Log: light.h,v $
 * Revision 1.11  1996/02/26  15:20:01  brianp
 * added gl_compute_spot_exp_table() and gl_compute_material_shine_table()
 *
 * Revision 1.10  1996/02/14  16:56:23  brianp
 * replaced gl_index_shade() with gl_index_shade_vertices()
 *
 * Revision 1.9  1996/02/13  17:52:03  brianp
 * added gl_update_lighting() and gl_color_shade_vertices_fast()
 *
 * Revision 1.8  1996/01/07  22:49:21  brianp
 * removed gl_color_shade(), added gl_init_lighting()
 *
 * Revision 1.7  1995/12/30  00:54:26  brianp
 * return integer colors instead of floats in shading functions
 *
 * Revision 1.6  1995/12/20  15:27:03  brianp
 * gl_index_shade changed to return GLuint color indexes instead of GLfloat
 *
 * Revision 1.5  1995/11/01  21:43:16  brianp
 * added gl_color_shade_vertices()
 *
 * Revision 1.4  1995/07/25  13:27:51  brianp
 * gl_index_shade() returns GLfloats instead of GLuints
 *
 * Revision 1.3  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.2  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:23:06  brianp
 * Initial revision
 *
 */


#ifndef LIGHT_H
#define LIGHT_H

#include "context.h"


extern void gl_light( GLenum light, GLenum pname, const GLfloat *params );


extern void gl_material( GLenum face, GLenum pname, const GLfloat *params );


extern void gl_lightmodel( GLenum pname, const GLfloat *params );


extern void gl_compute_spot_exp_table( struct gl_light *l );

extern void gl_compute_material_shine_table( struct gl_material *m );

extern void gl_update_lighting( void );



extern void gl_color_shade_vertices( GLuint n,
                                     GLfloat vertex[][4],
                                     GLfloat normal[][3],
                                     GLuint twoside,
                                     GLfixed frontcolor[][4],
                                     GLfixed backcolor[][4] );


extern void gl_color_shade_vertices_fast( GLuint n,
                                          GLfloat vertex[][4],
                                          GLfloat normal[][3],
                                          GLuint twoside,
                                          GLfixed frontcolor[][4],
                                          GLfixed backcolor[][4] );


extern void gl_index_shade_vertices( GLuint n,
                                     GLfloat vertex[][4],
                                     GLfloat normal[][3],
                                     GLuint twoside,
                                     GLuint frontindex[],
                                     GLuint backindex[] );

#endif

