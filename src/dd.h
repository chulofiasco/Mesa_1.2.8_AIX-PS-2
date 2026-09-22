/* $Id: dd.h,v 1.22 1996/05/01 15:47:07 brianp Exp $ */

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
$Log: dd.h,v $
 * Revision 1.22  1996/05/01  15:47:07  brianp
 * added read_depth_span_float() and read_depth_span_int() functions
 *
 * Revision 1.21  1996/04/25  20:54:57  brianp
 * redesigned DD: optional function pointers, new depth buffer functions
 *
 * Revision 1.20  1995/11/10  20:42:57  brianp
 * removed old comments
 *
 * Revision 1.19  1995/10/30  15:30:48  brianp
 * added mask[] argument to read_[color|index]_pixels
 *
 * Revision 1.18  1995/10/19  15:46:31  brianp
 * changed clear_color and color arguments to GLubytes
 *
 * Revision 1.17  1995/10/17  21:43:22  brianp
 * finished changes for new device driver interface
 *
 * Revision 1.16  1995/09/21  14:07:55  brianp
 * more new DD prototyping
 *
 * Revision 1.15  1995/09/20  18:19:58  brianp
 * prototype device driver changes described
 *
 * Revision 1.14  1995/09/15  18:46:39  brianp
 * new functions pointers for all device driver functions
 *
 * Revision 1.13  1995/07/24  18:55:20  brianp
 * added dd_finish()
 *
 * Revision 1.12  1995/06/12  15:38:23  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.11  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.10  1995/04/12  15:37:05  brianp
 * removed dd_draw_pixel(), dd_draw_line(), and dd_draw_polygon()
 *
 * Revision 1.9  1995/04/11  14:04:57  brianp
 * introduced DD struct of function pointers
 *
 * Revision 1.8  1995/03/30  21:07:49  brianp
 * updated to use pointers to CC.write_* functions
 *
 * Revision 1.7  1995/03/22  21:36:53  brianp
 * removed mode from dd_buffer_info()
 *
 * Revision 1.6  1995/03/08  15:10:02  brianp
 * added support for dd_logicop
 * added dd_clear_index and dd_clear_color
 *
 * Revision 1.5  1995/03/07  19:01:39  brianp
 * added dd_read_index_pixels() and dd_read_color_pixels()
 *
 * Revision 1.4  1995/03/07  14:20:41  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.3  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/27  22:48:36  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  14:20:25  brianp
 * Initial revision
 *
 */


#ifndef DD_INCLUDED
#define DD_INCLUDED


#include "context.h"



/*
 *                      Device Driver (DD) interface
 *
 *
 * All device driver functions are accessed via pointers in the global
 * DD struct.  The reason we use function pointers is to:
 *   1. allow switching between a number of different device drivers at
 *      runtime.
 *   2. use optimized functions dependant on frame buffer configuration
 *
 * The function pointers in the DD struct are divided into two groups:
 * mandatory and optional.  Mandatory functions have to be implemented by
 * every device driver.  Optional functions may or may not be implemented
 * by the device driver.  Optional functions provide ways to take advantage
 * of special hardware or optimized algorithms.
 *
 * When should the device driver set the DD pointers?
 *   1. When a "MakeCurrent" function is called such as GLXMakeCurrent(),
 *      XMesaMakeCurrent(), WMesaMakeCurrent(), etc.  In this case, _all_
 *      the mandatory DD pointers must be updated.  But do this after
 *      the call to gl_set_context().
 *   2. Whenever the DD.update_state() function is called.  In this case
 *      only some DD pointers may need to be updated.
 *
 * In either case, the device driver should re-examine the GL context state
 * and update the DD pointers as necessary.
 *
 *
 * Here's a quick description of most device driver function's purpose:
 *
 * Mandatory functions:
 * --------------------
 * update_state - called by Mesa whenever it thinks the device driver
 *                should update its DD pointers.
 * clear_index - implements glClearIndex()
 * clear_color - implements glClearColor()
 * clear_color_buffer - implements glClear(), with some special arguments
 * index - implements glIndex()
 * color - implements glColor()
 *
 * set_buffer - selects the front or back buffer for reading and writing.
 *              The default value is the buffer selected for writing pixels.
 *              When pixels have to be read from the color buffer, the core
 *              library will  call this function to temporarily select the
 *              "read" buffer, then restore it to the "draw" buffer.
 *
 * buffer_size - return width, height, depth of image buffer
 *
 * write_color_span - write a horizontal run of RGBA pixels
 * write_monocolor_span - write a horizontal run of mono-RGBA pixels
 * write_color_pixels - write a random array of RGBA pixels
 * write_monocolor_pixels - write a random array of mono-RGBA pixels
 *
 * write_index_span - write a horizontal run of CI pixels
 * write_monoindex_span - write a horizontal run of mono-CI pixels
 * write_index_pixels - write a random array of CI pixels
 * write_monoindex_pixels - write a random array of mono-CI pixels
 *
 * read_index_span - read a horizontal run of color index pixels
 * read_color_span - read a horizontal run of RGBA pixels
 * read_index_pixels - read a random array of CI pixels
 * read_color_pixels - read a random array of RGBA pixels
 *
 *
 * Optional functions:
 * -------------------
 * finish - implements glFinish()
 * flush - implements glFlush()
 *
 * index_mask - implements glIndexMask() if possible, else return GL_FALSE
 * color_mask - implements glColorMask() if possible, else return GL_FALSE
 * logicop - implements glLogicOp() if possible, else return GL_FALSE
 * dither - enable/disable dithering
 *
 *
 * Depth (Z) buffer functions may be implemented by drivers for systems
 * with hardware Z buffers.  The functions are:
 *
 * alloc_depth_buffer - called when the depth buffer must be allocated or
 *     possibly resized.
 * clear_depth_buffer - clear the depth buffer to depth specified by
 *     CC.Depth.Clear value.
 * depth_test_span/pixels - apply the depth buffer test to an span/array of
 *     pixels and return an updated pixel mask.  This function is not used
 *     when accelerated point, line, polygon functions are used.
 * read_depth_span_float - return depth values in [0,1] for glReadPixels
 * read_depth_span_int - return depth values as integers for glReadPixels
 *
 *
 * Accelerated point, line, polygon drawing:
 *
 * get_points_func - returns a pointer to an accelerated point drawing function
 * get_line_func - returns a pointer to an accelerated line drawing function
 * get_polygon_func - returns a pointer to an accelerated polygon function
 *
 *
 * Miscellaneous
 *
 * draw_pixels - implements glDrawPixels or returns GL_FALSE if it can't do
 *     the job.
 * begin, end - called by glBegin/glEnd so the device driver can do whatever
 *     it may need to do (window system synchronization, for example)
 *
 *
 * Notes:
 * ------
 *   RGBA = red/green/blue/alpha
 *   CI = color index (color mapped mode)
 *   mono = all pixels have the same color or index
 *
 *   The write_ functions all take an array of mask flags which indicate
 *   whether or not the pixel should be written.  One special case exists
 *   in the write_color_span function: if the mask array is NULL, then
 *   draw all pixels.  This is an optimization used for glDrawPixels().
 *
 * IN ALL CASES:
 *      X coordinates start at 0 at the left and increase to the right
 *      Y coordinates start at 0 at the bottom and increase upward
 */



struct dd_function_table {

   /***
    *** Mandatory functions:  these functions must be implemented by
    *** every device driver.
    ***/

   /*
    * This is called by Mesa whenever it thinks the device driver should
    * re-examine the GL state and update its DD pointers.
    */
   void (*update_state)( void );

   /* Implements glClearIndex() */
   void (*clear_index)( GLuint index );

   /* Implements glClearColor() */
   void (*clear_color)( GLubyte red, GLubyte green,
                        GLubyte glue, GLubyte alpha );

   /* Implements glClear(GL_COLOR_BUFFER_BIT) */
   void (*clear)( GLboolean all, GLint x, GLint y, GLint width, GLint height );

   /* Implements glIndex() */
   void (*index)( GLuint index );

   /* Implements glColor() */
   void (*color)( GLubyte red, GLubyte green, GLubyte glue, GLubyte alpha );

   /* Select front/back buffer as the read source and write destination */
   GLboolean (*set_buffer)( GLenum mode );

   /* Returns dimensions of the color buffer */
   void (*buffer_size)( GLuint *width, GLuint *height, GLuint *depth );

   /*
    * Return pointers to accelerated point, line, polygon rendering functions:
    * May return NULL.
    */
   points_func (*get_points_func)( void );
   line_func (*get_line_func)( void );
   polygon_func (*get_polygon_func)( void );

   /*
    * Functions for writing pixels to the frame buffer:
    */
   void (*write_color_span)( GLuint n, GLint x, GLint y,
			     const GLubyte red[], const GLubyte green[],
			     const GLubyte blue[], const GLubyte alpha[],
			     const GLubyte mask[] );
   void (*write_monocolor_span)( GLuint n, GLint x, GLint y,
				 const GLubyte mask[] );
   void (*write_color_pixels)( GLuint n, const GLint x[], const GLint y[],
			       const GLubyte red[], const GLubyte green[],
			       const GLubyte blue[], const GLubyte alpha[],
			       const GLubyte mask[] );
   void (*write_monocolor_pixels)( GLuint n, const GLint x[], const GLint y[],
				   const GLubyte mask[] );
   void (*write_index_span)( GLuint n, GLint x, GLint y, const GLuint index[],
			     const GLubyte mask[] );
   void (*write_monoindex_span)( GLuint n, GLint x, GLint y,
				 const GLubyte mask[] );
   void (*write_index_pixels)( GLuint n, const GLint x[], const GLint y[],
			       const GLuint index[], const GLubyte mask[] );
   void (*write_monoindex_pixels)( GLuint n, const GLint x[], const GLint y[],
				   const GLubyte mask[] );

   /*
    * Functions to read pixels from frame buffer:
    */
   void (*read_index_span)( GLuint n, GLint x, GLint y, GLuint index[] );
   void (*read_color_span)( GLuint n, GLint x, GLint y,
			    GLubyte red[], GLubyte green[],
			    GLubyte blue[], GLubyte alpha[] );
   void (*read_index_pixels)( GLuint n, const GLint x[], const GLint y[],
			      GLuint indx[], const GLubyte mask[] );
   void (*read_color_pixels)( GLuint n, const GLint x[], const GLint y[],
			      GLubyte red[], GLubyte green[],
			      GLubyte blue[], GLubyte alpha[],
                              const GLubyte mask[] );



   /***
    *** Optional functions:  these functions may or may not be implemented
    *** by the device driver.  If the device driver doesn't implement them
    *** it should never touch these pointers since Mesa will either set them
    *** to NULL or point them at a fall-back function.
    ***/

   /* Implementes glFinish() */
   void (*finish)( void );

   /* Implements glFlush() */
   void (*flush)( void );

   /* Implements glIndexMask(), returns GL_FALSE if can't be implemented */
   GLboolean (*index_mask)( GLuint mask );

   /* Implements glColorMask(), returns GL_FALSE if can't be implemented */
   GLboolean (*color_mask)( GLboolean rmask, GLboolean gmask,
                            GLboolean bmask, GLboolean amask );

   /* Implements glLogicOp(), returns GL_FALSE if can't be implemented */
   GLboolean (*logicop)( GLenum op );

   /* Implements glEn/Disable( GL_DITHER ) */
   void (*dither)( GLboolean enable );


   /*
    * For supporting hardware Z buffers:
    */
   void (*alloc_depth_buffer)( void );
   void (*clear_depth_buffer)( void );

   GLuint (*depth_test_span)( GLuint n, GLint x, GLint y, const GLdepth z[],
                              GLubyte mask[] );
   void (*depth_test_pixels)( GLuint n, const GLint x[], const GLint y[],
                              const GLdepth z[], GLubyte mask[] );
   void (*read_depth_span_float)( GLuint n, GLint x, GLint y, GLfloat depth[]);
   void (*read_depth_span_int)( GLuint n, GLint x, GLint y, GLdepth depth[] );

   /*
    * Implements glDrawPixels, returns GL_FALSE if can't be done.
    * 'packed' indicates whether or not the pixels are in packed or unpacked
    * format.
    */
   GLboolean (*draw_pixels)( GLint x, GLint y, GLsizei width, GLsizei height,
                             GLenum format, GLenum type, GLboolean packed,
                             const GLvoid *pixels );

   /*
    * glBegin/glEnd functions: these are called whenever glBegin or glEnd
    * are executed in case the device driver needs to do something special.
    */
   void (*begin)( GLenum mode );
   void (*end)( void );

};



extern struct dd_function_table DD;


extern void gl_init_dd_function_table( void );


#endif

