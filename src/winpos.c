/* $Id: winpos.c,v 1.2 1996/05/08 16:47:42 brianp Exp $ */

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
$Log: winpos.c,v $
 * Revision 1.2  1996/05/08  16:47:42  brianp
 * added glWindowPos[234]f[v]() functions
 *
 * Revision 1.1  1996/05/01  20:46:26  brianp
 * Initial revision
 *
 */


/*
 * GL_MESA_window_pos extension
 *
 * This extension offers a set of functions named glWindowPos*MESA() which
 * directly set the current raster position to the given window coordinate.
 * glWindowPos*MESA() are similar to glRasterPos*() but bypass the
 * modelview, projection and viewport transformations.
 *
 * These functions should be very handy in conjunction with glDrawPixels()
 * and glCopyPixels().
 *
 * If your application uses glWindowPos*MESA() and may be compiled with
 * a real OpenGL instead of Mesa you can simply copy the winpos.[ch] files
 * into your source tree and compile them with the rest of your code
 * since glWindowPos*MESA() can, and is, implemented in terms of standard
 * OpenGL commands when not using Mesa.  In your source files which use
 * glWindowPos*MESA() just #include "winpos.h".
 */



#include <GL/gl.h>


#ifdef GL_MESA_window_pos


#include "context.h"
#include "draw.h"
#include "list.h"
#include "macros.h"


/*
 * Mesa implementation of glWindowPos*MESA()
 */
void glWindowPos4fMESA( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   if (CC.CompileFlag) {
      gl_save_windowpos( x, y, z, w );
   }
   if (CC.ExecuteFlag) {
      gl_windowpos( x, y, z, w );
   }
}


#else


/*
 * OpenGL implementation of glWindowPos*MESA()
 */
void glWindowPos4fMESA( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GLfloat fx, fy;

   /* Push current matrix mode and viewport attributes */
   glPushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );

   /* Setup projection parameters */
   glMatrixMode( GL_PROJECTION );
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode( GL_MODELVIEW );
   glPushMatrix();
   glLoadIdentity();

   glDepthRange( z, z );
   glViewport( (int) x - 1, (int) y - 1, 2, 2 );

   /* set the raster (window) position */
   fx = x - (int) x;
   fy = y - (int) y;
   glRasterPos4f( fx, fy, 0.0, w );

   /* restore matrices, viewport and matrix mode */
   glPopMatrix();
   glMatrixMode( GL_PROJECTION );
   glPopMatrix();

   glPopAttrib();
}


#endif



/*
 * Remaining glWindowPos*MESA() variations
 */

void glWindowPos2iMESA( GLint x, GLint y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}

void glWindowPos2sMESA( GLshort x, GLshort y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}

void glWindowPos2fMESA( GLfloat x, GLfloat y )
{
   glWindowPos4fMESA( x, y, 0.0F, 1.0F );
}

void glWindowPos2dMESA( GLdouble x, GLdouble y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


void glWindowPos2ivMESA( const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}

void glWindowPos2svMESA( const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}

void glWindowPos2fvMESA( const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], 0.0F, 1.0F );
}

void glWindowPos2dvMESA( const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}


void glWindowPos3iMESA( GLint x, GLint y, GLint z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}

void glWindowPos3sMESA( GLshort x, GLshort y, GLshort z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}

void glWindowPos3fMESA( GLfloat x, GLfloat y, GLfloat z )
{
   glWindowPos4fMESA( x, y, z, 1.0F );
}

void glWindowPos3dMESA( GLdouble x, GLdouble y, GLdouble z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


void glWindowPos3ivMESA( const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}

void glWindowPos3svMESA( const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}

void glWindowPos3fvMESA( const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], p[2], 1.0F );
}

void glWindowPos3dvMESA( const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}


void glWindowPos4iMESA( GLint x, GLint y, GLint z, GLint w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}

void glWindowPos4sMESA( GLshort x, GLshort y, GLshort z, GLshort w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}

void glWindowPos4dMESA( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


void glWindowPos4ivMESA( const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}

void glWindowPos4svMESA( const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}

void glWindowPos4fvMESA( const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], p[2], p[3] );
}

void glWindowPos4dvMESA( const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}

