/* $Id: glx.h,v 1.13 1996/05/22 14:02:08 brianp Exp $ */

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
$Log: glx.h,v $
 * Revision 1.13  1996/05/22  14:02:08  brianp
 * added GLX_EXT_visual_info extension
 *
 * Revision 1.12  1996/04/26  16:58:52  brianp
 * added the GLX_MESA_pixmap_colormap extension
 *
 * Revision 1.11  1996/04/25  19:30:06  brianp
 * GLX typedef's dependent on whether or not MESA is defined in gl.h
 *
 * Revision 1.10  1995/10/12  16:58:37  brianp
 * updated notes to reflect latest code changes
 *
 * Revision 1.9  1995/08/03  19:34:37  brianp
 * explictly assigned values to GLX_ symbols to match OpenGL
 *
 * Revision 1.8  1995/07/28  15:01:05  brianp
 * added a few notes on using GLX
 *
 * Revision 1.7  1995/05/22  17:03:21  brianp
 * Release 1.2
 *
 * Revision 1.6  1995/04/28  20:04:36  brianp
 * rename GLX_BAD_ATTRIB to GLX_BAD_ATTRIBUTE
 *
 * Revision 1.5  1995/04/17  14:42:00  brianp
 * added GLX version 1.1 functions and symbols
 *
 * Revision 1.4  1995/04/05  18:29:41  brianp
 * added GLX error constants
 *
 * Revision 1.3  1995/03/04  19:45:47  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/28  21:22:39  brianp
 * fixed a typo
 *
 * Revision 1.1  1995/02/28  21:21:03  brianp
 * Initial revision
 *
 */


#ifndef GLX_H
#define GLX_H


/*
 * A pseudo-GLX implementation to allow GLX-based OpenGL programs to
 * work with Mesa.
 *
 * Notes:
 *   1. If the visual passed to glXGetConfig was not one returned by
 *      glXChooseVisual then the GLX_RGBA and GLX_DOUBLEBUFFER queries
 *      will always return True and the GLX_DEPTH_SIZE query will always
 *      return non-zero.
 *   2. The glXIsDirect() function always returns True.
 */



#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "GL/gl.h"
#ifdef MESA
#include "GL/xmesa.h"
#endif



#ifdef __cplusplus
extern "C" {
#endif


#define GLX_VERSION_1_1		1


/*
 * Tokens for glXChooseVisual and glXGetConfig:
 */
enum _GLX_CONFIGS {
	GLX_USE_GL		= 1,
	GLX_BUFFER_SIZE		= 2,
	GLX_LEVEL		= 3,
	GLX_RGBA		= 4,
	GLX_DOUBLEBUFFER	= 5, 
	GLX_STEREO		= 6,
	GLX_AUX_BUFFERS		= 7,
	GLX_RED_SIZE		= 8,
	GLX_GREEN_SIZE		= 9,
	GLX_BLUE_SIZE		= 10,
	GLX_ALPHA_SIZE		= 11,
	GLX_DEPTH_SIZE		= 12,
	GLX_STENCIL_SIZE	= 13,
	GLX_ACCUM_RED_SIZE	= 14,
	GLX_ACCUM_GREEN_SIZE	= 15,
	GLX_ACCUM_BLUE_SIZE	= 16,
	GLX_ACCUM_ALPHA_SIZE	= 17,

	/* GLX_EXT_visual_info extension */
	GLX_X_VISUAL_TYPE_EXT		= 0x22,
	GLX_TRANSPARENT_TYPE_EXT	= 0x23,
	GLX_TRANSPARENT_INDEX_VALUE_EXT	= 0x24,
	GLX_TRANSPARENT_RED_VALUE_EXT	= 0x25,
	GLX_TRANSPARENT_GREEN_VALUE_EXT	= 0x26,
	GLX_TRANSPARENT_BLUE_VALUE_EXT	= 0x27,
	GLX_TRANSPARENT_ALPHA_VALUE_EXT	= 0x28
};


/*
 * Error codes returned by glXGetConfig:
 */
#define GLX_BAD_SCREEN		1
#define GLX_BAD_ATTRIBUTE	2
#define GLX_NO_EXTENSION	3
#define GLX_BAD_VISUAL		4
#define GLX_BAD_CONTEXT		5
#define GLX_BAD_VALUE       	6
#define GLX_BAD_ENUM		7


/*
 * GLX 1.1 and later:
 */
#define GLX_VENDOR		1
#define GLX_VERSION		2
#define GLX_EXTENSIONS 		3


/*
 * GLX_visual_info extension
 */
#define GLX_TRUE_COLOR_EXT		0x8002
#define GLX_DIRECT_COLOR_EXT		0x8003
#define GLX_PSEUDO_COLOR_EXT		0x8004
#define GLX_STATIC_COLOR_EXT		0x8005
#define GLX_GRAY_SCALE_EXT		0x8006
#define GLX_STATIC_GRAY_EXT		0x8007
#define GLX_NONE_EXT			0x8000
#define GLX_TRANSPARENT_RGB_EXT		0x8008
#define GLX_TRANSPARENT_INDEX_EXT	0x8009


/*
 * Compile-time extension tests
 */
#ifdef MESA
#define GLX_EXT_visual_info	1
#define GLX_MESA_pixmap_colormap 1
#endif



#ifdef MESA
   typedef XMesaContext GLXContext;
   typedef Pixmap GLXPixmap;
   typedef Drawable GLXDrawable;
#else
   typedef void * GLXContext;
   typedef XID GLXPixmap;
   typedef XID GLXDrawable;
#endif
typedef XID GLXContextID;



extern XVisualInfo* glXChooseVisual( Display *dpy, int screen,
				     int *attribList );

extern GLXContext glXCreateContext( Display *dpy, XVisualInfo *vis,
				    GLXContext shareList, Bool direct );

extern void glXDestroyContext( Display *dpy, GLXContext ctx );

extern Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable,
			    GLXContext ctx);

extern void glXCopyContext( Display *dpy, GLXContext src, GLXContext dst,
			    GLuint mask );

extern void glXSwapBuffers( Display *dpy, GLXDrawable drawable );

extern GLXPixmap glXCreateGLXPixmap( Display *dpy, XVisualInfo *visual,
				     Pixmap pixmap );

extern void glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap );

extern Bool glXQueryExtension( Display *dpy, int *errorb, int *event );

extern Bool glXQueryVersion( Display *dpy, int *maj, int *min );

extern Bool glXIsDirect( Display *dpy, GLXContext ctx );

extern int glXGetConfig( Display *dpy, XVisualInfo *visual,
			 int attrib, int *value );

extern GLXContext glXGetCurrentContext( void );

extern GLXDrawable glXGetCurrentDrawable( void );

extern void glXWaitGL( void );

extern void glXWaitX( void );

extern void glXUseXFont( Font font, int first, int count, int list );



/* GLX 1.1 and later */
extern const char *glXQueryExtensionsString( Display *dpy, int screen );

extern const char *glXQueryServerString( Display *dpy, int screen, int name );

extern const char *glXGetClientString( Display *dpy, int name );



/*
 * Mesa GLX Extensions
 */

#ifdef GLX_MESA_pixmap_colormap
extern GLXPixmap glXCreateGLXPixmapMESA( Display *dpy, XVisualInfo *visual,
                                         Pixmap pixmap, Colormap cmap );
#endif




#ifdef __cplusplus
}
#endif

#endif
