/* clip.h */

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
 * Line and polygon clipping.
 */


/*
$Id: clip.h,v 1.5 1995/10/14 17:41:29 brianp Exp $

$Log: clip.h,v $
 * Revision 1.5  1995/10/14  17:41:29  brianp
 * made glClipPlane display list-able
 *
 * Revision 1.4  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.3  1995/03/23  17:04:31  brianp
 * added GL_VIEWCLIP_POINT macro
 *
 * Revision 1.2  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:18:12  brianp
 * Initial revision
 *
 */


#ifndef CLIP_H
#define CLIP_H


#include "GL/gl.h"
#include "config.h"



#ifdef DEBUG
#  define GL_VIEWCLIP_POINT( V )   gl_viewclip_point( V )
#else
#  define GL_VIEWCLIP_POINT( V )			\
     (    (V)[0] <= (V)[3] && (V)[0] >= -(V)[3]		\
       && (V)[1] <= (V)[3] && (V)[1] >= -(V)[3]		\
       && (V)[2] <= (V)[3] && (V)[2] >= -(V)[3] )
#endif



extern void gl_clipplane( GLenum plane, const float *equation );



extern GLuint gl_viewclip_point( const GLfloat v[] );

extern GLuint gl_viewclip_line( GLuint *i, GLuint *j );

extern GLuint gl_viewclip_polygon( GLuint n, GLuint vlist[] );



extern GLuint gl_userclip_point( const GLfloat v[] );

extern GLuint gl_userclip_line( GLuint *i, GLuint *j );

extern GLuint gl_userclip_polygon( GLuint n, GLuint vlist[] );


#endif

