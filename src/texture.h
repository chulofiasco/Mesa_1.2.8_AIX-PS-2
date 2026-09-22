/* texture.h */

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
$Id: texture.h,v 1.7 1995/10/14 17:42:45 brianp Exp $

$Log: texture.h,v $
 * Revision 1.7  1995/10/14  17:42:45  brianp
 * compile glTexEnv, glTexParameter, and glTexGen into display lists
 *
 * Revision 1.6  1995/06/12  15:43:06  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.5  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.4  1995/05/15  16:08:07  brianp
 * renamed texturing functions, added new ones
 *
 * Revision 1.3  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/03/02  19:10:55  brianp
 * changed gl_texgen arguments
 *
 * Revision 1.1  1995/02/24  14:28:31  brianp
 * Initial revision
 *
 */


#ifndef TEXTURE_H
#define TEXTURE_H



extern void
gl_texenv( GLenum target, GLenum pname, const GLfloat *param );


extern void
gl_texparameter( GLenum target, GLenum pname, const GLfloat *params );


extern void
gl_texgen( GLenum coord, GLenum pname, const GLfloat *params );


extern void gl_do_texgen( const GLfloat obj[4], const GLfloat eye[4],
                          const GLfloat normal[3], GLfloat texcoord[4] );


extern void gl_teximage1d( GLint level, GLint components,
			   GLsizei width, GLint border,
			   const GLubyte *pixels );


extern void gl_teximage2d( GLint level, GLint components,
			   GLsizei width, GLsizei height, GLint border,
			   const GLubyte *pixels );


extern void gl_texture_pixels_1d( GLuint n, GLfloat s[],
				  GLubyte red[], GLubyte green[],
				  GLubyte blue[], GLubyte alpha[] );


extern void gl_texture_pixels_2d( GLuint n, GLfloat s[], GLfloat t[],
				  GLubyte red[], GLubyte green[],
				  GLubyte blue[], GLubyte alpha[] );


#endif

