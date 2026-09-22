/* alpha.h */

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
 * Alpha operations.
 */


/*
$Id: alpha.h,v 1.4 1995/06/12 15:35:14 brianp Exp $

$Log: alpha.h,v $
 * Revision 1.4  1995/06/12  15:35:14  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.3  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.2  1995/03/04  19:25:08  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:15:28  brianp
 * Initial revision
 *
 */


#ifndef ALPHA_H
#define ALPHA_H


#include "GL/gl.h"


extern GLint gl_alpha_test( GLuint n, const GLubyte alpha[], GLubyte mask[] );


#endif
