/* $Id: osmesa.h,v 1.1 1995/10/12 16:44:12 brianp Exp $ */

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
$Log: osmesa.h,v $
 * Revision 1.1  1995/10/12  16:44:12  brianp
 * Initial revision
 *
 */


/*
 * Mesa Off-Screen rendering interface.
 *
 * This is an operating system and window system independent interface to
 * Mesa which allows one to render images into a client-supplied buffer in
 * main memory.  Such images may manipulated or saved in whatever way the
 * client wants.
 *
 * There are only three API functions:
 *   1. OSMesaCreateContext - create a new Off-Screen Mesa rendering context
 *   2. OSMesaMakeCurrent - bind an OSMesaContext to a client's image buffer
 *                          and make the specified context the current one.
 *   3. OSMesaDestroyContext - destroy an OSMesaContext
 *
 *
 * The limits on the width and height of an image buffer are MAX_WIDTH and
 * MAX_HEIGHT as defined in Mesa/src/config.h.  Defaults are 1280 and 1024.
 * You can increase them as needed but beware that many temporary arrays in
 * Mesa are dimensioned by MAX_WIDTH or MAX_WIDTH.
 */



#ifndef OSMESA_H
#define OSMESA_H



#ifdef __cplusplus
extern "C" {
#endif


#include "GL/gl.h"



typedef struct osmesa_context *OSMesaContext;



/*
 * Create an Off-Screen Mesa rendering context.  The only attribute needed is
 * an RGBA vs Color-Index mode flag.
 *
 * Input:  format - either GL_RGBA or GL_COLOR_INDEX
 *         sharelist - specifies another OSMesaContext with which to share
 *                     display lists.  NULL indicates no sharing.
 * Return:  an OSMesaContext or 0 if error
 */
extern OSMesaContext OSMesaCreateContext( GLenum format,
                                          OSMesaContext sharelist );




/*
 * Destroy an Off-Screen Mesa rendering context.
 *
 * Input:  ctx - the context to destroy
 */
extern void OSMesaDestroyContext( OSMesaContext ctx );



/*
 * Bind an OSMesaContext to an image buffer.  The image buffer is just a
 * block of memory which the client provides.  Its size must be at least
 * as large as width*height*sizeof(type).  Its address should be a multiple
 * of 4 if using RGBA mode.
 *
 * Image data is stored in the order of glDrawPixels:  row-major order
 * with the lower-left image pixel stored in the first array position
 * (ie. bottom-to-top).
 *
 * Since the only type initially supported is GL_UNSIGNED_BYTE, if the
 * context is in RGBA mode, each pixel will be stored as a 4-byte RGBA
 * value.  If the context is in color indexed mode, each pixel will be
 * stored as a 1-byte value.
 *
 * If the context's viewport hasn't been initialized yet, it will now be
 * initialized to (0,0,width,height).
 *
 * Input:  ctx - the rendering context
 *         buffer - the image buffer memory
 *         type - data type for pixel components, only GL_UNSIGNED_BYTE
 *                supported now
 *         width, height - size of image buffer in pixels, at least 1
 * Return:  GL_TRUE if success, GL_FALSE if error because of invalid ctx,
 *          invalid buffer address, type!=GL_UNSIGNED_BYTE, width<1, height<1,
 *          width>internal limit or height>internal limit.
 */
extern GLboolean OSMesaMakeCurrent( OSMesaContext ctx,
                                    void *buffer, GLenum type,
                                    GLsizei width, GLsizei height );




#ifdef __cplusplus
}
#endif


#endif
