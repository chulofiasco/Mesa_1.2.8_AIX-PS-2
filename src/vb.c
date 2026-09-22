/* vb.c */

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
$Id: vb.c,v 1.5 1995/09/28 19:38:57 brianp Exp $

$Log: vb.c,v $
 * Revision 1.5  1995/09/28  19:38:57  brianp
 * replaced ClipFlag[] with Unclipped[]
 *
 * Revision 1.4  1995/09/25  19:24:54  brianp
 * added code to initialize VB.MaterialMask and VB.MaterialChanges
 *
 * Revision 1.3  1995/09/07  14:17:08  brianp
 * new gl_init_vb() function
 *
 * Revision 1.2  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.1  1995/03/24  15:31:44  brianp
 * Initial revision
 *
 */


#include "vb.h"



/*
 * Vertex buffer:  this data structure holds all the per-vertex data
 * need to construct and render points, lines, and polygons.
 */


struct vertex_buffer VB;



/*
 * Initialize the vertex buffer.  Only needs to be done once.
 */
void gl_init_vb( void )
{
   GLuint i;

   for (i=0;i<VB_SIZE;i++) {
      VB.Unclipped[i] = 1;
      VB.MaterialMask[i] = 0;
   }
   VB.MaterialChanges = GL_FALSE;
}

