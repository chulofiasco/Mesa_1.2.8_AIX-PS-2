/* $Id: dd.c,v 1.5 1996/05/01 15:47:28 brianp Exp $ */

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
$Log: dd.c,v $
 * Revision 1.5  1996/05/01  15:47:28  brianp
 * added read_depth_span_float() and read_depth_span_int() functions
 *
 * Revision 1.4  1996/04/25  20:55:39  brianp
 * added gl_init_dd_function_table()
 *
 * Revision 1.3  1996/03/26  19:08:44  brianp
 * added gl_dummy_function()
 *
 * Revision 1.2  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.1  1995/04/11  14:05:51  brianp
 * Initial revision
 *
 */


#include <string.h>
#include "dd.h"
#include "depth.h"
#include "macros.h"


struct dd_function_table DD;



/*
 * This function initializes all the entries in the device driver table.
 * This function is only called by gl_set_context().
 * The actual device driver should then make it's own updates too.
 */
void gl_init_dd_function_table( void )
{
   /* Initialize to all NULL pointers to start */
   MEMSET( &DD, 0, sizeof(struct dd_function_table) );

   /*
    * Set up default depth buffer functions.
    * If the device driver has its own depth (Z) buffer then it will
    * override these pointers.
    */
   DD.alloc_depth_buffer = gl_alloc_depth_buffer;
   DD.clear_depth_buffer = gl_clear_depth_buffer;
   DD.depth_test_span = gl_depth_test_span;
   DD.depth_test_pixels = gl_depth_test_pixels;
   DD.read_depth_span_float = gl_read_depth_span_float;
   DD.read_depth_span_int = gl_read_depth_span_int;
}
