/* $Id: xmesa.h,v 1.10 1996/04/26 16:58:31 brianp Exp $ */

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
$Log: xmesa.h,v $
 * Revision 1.10  1996/04/26  16:58:31  brianp
 * added Colormap argument to XMesaBindPixmap()
 *
 * Revision 1.9  1996/04/25  19:31:59  brianp
 * added new parameters to XMesaCreateContext()
 *
 * Revision 1.8  1996/02/15  16:00:31  brianp
 * added Amiga Amiwin support
 *
 * Revision 1.7  1995/10/12  16:58:55  brianp
 * fixed up include files
 *
 * Revision 1.6  1995/05/22  17:03:21  brianp
 * Release 1.2
 *
 * Revision 1.5  1995/05/15  16:11:41  brianp
 * added share_list argument to XMesaCreateContext()
 *
 * Revision 1.4  1995/04/19  13:47:56  brianp
 * added XMesaGetBackBuffer()
 *
 * Revision 1.3  1995/04/17  14:42:32  brianp
 * API changes: XMesaCreateContext, XMesaBindWindow, XMesaBindPixmap
 *
 * Revision 1.2  1995/03/04  19:45:47  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/28  21:21:03  brianp
 * Initial revision
 *
 */


/*
 * Mesa/X11 interface.  This header file serves as the documentation for
 * the Mesa/X11 interface functions.
 */


/* Sample Usage:

1. Make the X11 calls needed to open the display, select a visual, make a
   colormap, open a window, etc.

2. Call XMesaCreateContext() to create an X/Mesa rendering context.

3. Call XMesaBindWindow() to bind the context to your window.
   (or XMesaBindPixmap() to bind the context to an off-screen pixmap).

4. Call XMesaMakeCurrent() to make the context the active one.

5. Make gl* calls to render your graphics.

6. Use XMesaSwapBuffers() when double buffering to update the window.

7. When exiting, call XMesaDestroyContext().

*/




#ifndef XMESA_H
#define XMESA_H


#ifdef __cplusplus
extern "C" {
#endif


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "GL/gl.h"

#ifdef AMIWIN
#include <pragmas/xlib_pragmas.h>
extern struct Library *XLibBase;
#endif



/*
 * This is the XMesa context 'handle':
 */
typedef struct xmesa_context *XMesaContext;



/*
 * Create a new XMesaContext for rendering into an X11 window.
 *
 * Input:  display - X11 display
 *         window - X11 window to render into.
 *         rgb_flag - GL_TRUE = RGB mode,
 *                    GL_FALSE = color index mode
 *         alpha_flag - alpha buffer requested?
 *         db_flag - GL_TRUE = double-buffered,
 *                   GL_FALSE = single buffered
 *         depth_size - requested bits/depth values, or zero
 *         stencil_size - requested bits/stencil values, or zero
 *         accum_size - requested bits/component values, or zero
 *         ximage_flag - GL_TRUE = use an XImage for back buffer,
 *                       GL_FALSE = use an off-screen pixmap for back buffer
 *         share_list - another XMesaContext with which to share display
 *                      lists or NULL if no sharing is wanted.
 * Return:  an XMesaContext or NULL if error.
 */
extern XMesaContext XMesaCreateContext( Display *display,
                                        XVisualInfo *visinfo,
                                        GLboolean rgb_flag,
                                        GLboolean alpha_flag,
                                        GLboolean db_flag,
                                        GLint depth_size,
                                        GLint stencil_size,
                                        GLint accum_size,
                                        GLboolean ximage_flag,
                                        XMesaContext share_list );


/*
 * Destroy a rendering context as returned by XMesaCreateContext()
 */
extern void XMesaDestroyContext( XMesaContext ctx );


/*
 * Bind an X/Mesa context to an X window.
 */
extern GLboolean XMesaBindWindow( XMesaContext c, Window w );


/*
 * Bind an X/Mesa context to an X pixmap.
 */
extern GLboolean XMesaBindPixmap( XMesaContext c, Pixmap p, Colormap cmap );


/*
 * Make the specified context the current one.
 */
extern GLboolean XMesaMakeCurrent( XMesaContext ctx );


/*
 * Return a handle to the current context.
 */
extern XMesaContext XMesaGetCurrentContext( void );


/*
 * Swap the front and back buffers for the current context.  No action is
 * taken if the context is not double buffered.
 */
extern void XMesaSwapBuffers( void );


/*
 * Return a pointer to the XMesa backbuffer Pixmap or XImage.  This function
 * is a way to get "under the hood" of X/Mesa so one can manipulate the
 * back buffer directly.
 * Output:  pixmap - pointer to back buffer's Pixmap, or 0
 *          ximage - pointer to back buffer's XImage, or NULL
 * Return:  GL_TRUE = context is double buffered
 *          GL_FALSE = context is single buffered
 */
extern GLboolean XMesaGetBackBuffer( Pixmap *pixmap, XImage **ximage );



#ifdef __cplusplus
}
#endif


#endif
