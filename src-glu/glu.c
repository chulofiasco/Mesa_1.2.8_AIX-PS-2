/* $Id: glu.c,v 1.21 1996/05/15 18:36:22 brianp Exp $ */

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
$Log: glu.c,v $
 * Revision 1.21  1996/05/15  18:36:22  brianp
 * changed version string to 1.2.8
 *
 * Revision 1.20  1996/02/13  22:22:07  brianp
 * changed version string to 1.2.7
 *
 * Revision 1.19  1996/01/22  16:14:05  brianp
 * changed version string to 1.2.6
 *
 * Revision 1.18  1995/11/29  18:02:08  brianp
 * changed version string to 1.2.5
 *
 * Revision 1.17  1995/10/19  16:34:40  brianp
 * updated version string to 1.2.4
 *
 * Revision 1.16  1995/09/19  13:33:06  brianp
 * change PI to M_PI
 * incorporated Bogdan Sikorski's Sep 19 updates
 *
 * Revision 1.15  1995/07/28  21:35:49  brianp
 * changed all GLUenum to GLenum
 *
 * Revision 1.14  1995/07/18  20:23:05  brianp
 * much more complete gluErrorString() implementation
 * updated gluGetString(GLU_VERSION) for 1.2.2
 *
 * Revision 1.13  1995/06/09  21:48:05  brianp
 * changed version string to 1.2.1
 *
 * Revision 1.12  1995/05/24  12:55:02  brianp
 * changed gluGetString version to 1.2
 *
 * Revision 1.11  1995/05/22  16:56:20  brianp
 * Release 1.2
 *
 * Revision 1.10  1995/05/16  19:17:21  brianp
 * minor changes to allow compilation with real OpenGL headers
 *
 * Revision 1.9  1995/05/15  13:38:02  brianp
 * fixed gluLookAt() bug per Michael Pichler
 *
 * Revision 1.8  1995/04/28  16:21:29  brianp
 * added tesselation errors to gluErrorString()
 *
 * Revision 1.7  1995/04/18  15:51:21  brianp
 * fixed warnings on Suns
 *
 * Revision 1.6  1995/04/17  13:45:03  brianp
 * added gluGetString for GLU 1.1
 *
 * Revision 1.5  1995/03/16  20:37:44  brianp
 * fixed gluPickMatrix
 *
 * Revision 1.4  1995/03/06  17:34:45  brianp
 * fixed gluOrtho2D bug
 * removed unused gluProject and gluUnproject functions
 *
 * Revision 1.3  1995/03/04  19:39:18  brianp
 * version 1.1 beta
 *
 * Revision 1.2  1995/02/24  15:54:21  brianp
 * ifdef'd out gluProject, gluUnproject stubs
 *
 * Revision 1.1  1995/02/24  15:45:01  brianp
 * Initial revision
 *
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "gluP.h"



/*
 * Miscellaneous utility functions
 */


#ifndef M_PI
#define M_PI 3.1415926536
#endif
#define EPS 0.00001




void gluLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez,
		GLdouble centerx, GLdouble centery, GLdouble centerz,
		GLdouble upx, GLdouble upy, GLdouble upz )
{
   GLdouble m[16];
   GLdouble x[3], y[3], z[3];
   GLdouble mag;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = eyex - centerx;
   z[1] = eyey - centery;
   z[2] = eyez - centerz;
   mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
   if (mag) {  /* mpichler, 19950515 */
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
   }

   /* Y vector */
   y[0] = upx;
   y[1] = upy;
   y[2] = upz;

   /* X vector = Y cross Z */
   x[0] =  y[1]*z[2] - y[2]*z[1];
   x[1] = -y[0]*z[2] + y[2]*z[0];
   x[2] =  y[0]*z[1] - y[1]*z[0];

   /* Recompute Y = Z cross X */
   y[0] =  z[1]*x[2] - z[2]*x[1];
   y[1] = -z[0]*x[2] + z[2]*x[0];
   y[2] =  z[0]*x[1] - z[1]*x[0];

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

   mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
   if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
   }

#define M(row,col)  m[col*4+row]
   M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
   M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
   M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
   M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;
#undef M
   glMultMatrixd( m );

   /* Translate Eye to Origin */
   glTranslated( -eyex, -eyey, -eyez );

}



void gluOrtho2D( GLdouble left, GLdouble right,
		 GLdouble bottom, GLdouble top )
{
   GLdouble m[16];
   GLdouble dx = right - left;
   GLdouble dy = top - bottom;
   if (dx == 0.0 || dy == 0.0) return;
   
   m[0] = 2.0 / dx; m[1] = 0.0; m[2] = 0.0; m[3] = 0.0;
   m[4] = 0.0; m[5] = 2.0 / dy; m[6] = 0.0; m[7] = 0.0;
   m[8] = 0.0; m[9] = 0.0; m[10] = -1.0; m[11] = 0.0;
   m[12] = -(right + left) / dx;
   m[13] = -(top + bottom) / dy;
   m[14] = 0.0;
   m[15] = 1.0;

   glMultMatrixd(m);
}



void gluPerspective( GLdouble fovy, GLdouble aspect,
		     GLdouble zNear, GLdouble zFar )
{
   GLdouble xmin, xmax, ymin, ymax;

   ymax = zNear * tan( fovy * M_PI / 360.0 );
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}



void gluPickMatrix( GLdouble x, GLdouble y,
		    GLdouble width, GLdouble height,
		    GLint viewport[4] )
{
   GLfloat m[16];
   GLfloat sx, sy;
   GLfloat tx, ty;

   sx = viewport[2] / width;
   sy = viewport[3] / height;
   tx = (viewport[2] + 2.0 * (viewport[0] - x)) / width;
   ty = (viewport[3] + 2.0 * (viewport[1] - y)) / height;

#define M(row,col)  m[col*4+row]
   M(0,0) = sx;   M(0,1) = 0.0;  M(0,2) = 0.0;  M(0,3) = tx;
   M(1,0) = 0.0;  M(1,1) = sy;   M(1,2) = 0.0;  M(1,3) = ty;
   M(2,0) = 0.0;  M(2,1) = 0.0;  M(2,2) = 1.0;  M(2,3) = 0.0;
   M(3,0) = 0.0;  M(3,1) = 0.0;  M(3,2) = 0.0;  M(3,3) = 1.0;
#undef M

   glMultMatrixf( m );
}



const GLubyte* gluErrorString( GLenum errorCode )
{
   if (errorCode < 65536 && errorCode > 1000) errorCode |= 0x10000;
   static char *tess_error[] = {
      "missing gluEndPolygon",
      "missing gluBeginPolygon",
      "misoriented contour",
      "vertex/edge intersection",
      "misoriented or self-intersecting loops",
      "coincident vertices",
      "colinear vertices",
      "intersecting edges",
      "not coplanar contours"
   };
   static char *nurbs_error[] = {
      "spline order un-supported",
      "too few knots",
      "valid knot range is empty",
      "decreasing knot sequence knot",
      "knot multiplicity greater than order of spline",
      "endcurve() must follow bgncurve()",
      "bgncurve() must precede endcurve()",
      "missing or extra geometric data",
      "can't draw pwlcurves",
      "missing bgncurve()",
      "missing bgnsurface()",
      "endtrim() must precede endsurface()",
      "bgnsurface() must precede endsurface()",
      "curve of improper type passed as trim curve",
      "bgnsurface() must precede bgntrim()",
      "endtrim() must follow bgntrim()",
      "bgntrim() must precede endtrim()",
      "invalid or missing trim curve",
      "bgntrim() must precede pwlcurve()",
      "pwlcurve referenced twice",
      "pwlcurve and nurbscurve mixed",
      "improper usage of trim data type",
      "nurbscurve referenced twice",
      "nurbscurve and pwlcurve mixed",
      "nurbssurface referenced twice",
      "invalid property",
      "endsurface() must follow bgnsurface()",
      "misoriented trim curves",
      "intersecting trim curves",
      "UNUSED",
      "unconnected trim curves",
      "unknown knot error",
      "negative vertex count encountered",
      "negative byte-stride encounteed",
      "unknown type descriptor",
      "null control array or knot vector",
      "duplicate point on pwlcurve"
   };

   /* GL Errors */
   if (errorCode==GL_NO_ERROR) {
      return (GLubyte *) "no error";
   }
   else if (errorCode==GL_INVALID_VALUE) {
      return (GLubyte *) "invalid value";
   }
   else if (errorCode==GL_INVALID_ENUM) {
      return (GLubyte *) "invalid enum";
   }
   else if (errorCode==GL_INVALID_OPERATION) {
      return (GLubyte *) "invalid operation";
   }
   else if (errorCode==GL_STACK_OVERFLOW) {
      return (GLubyte *) "stack overflow";
   }
   else if (errorCode==GL_STACK_UNDERFLOW) {
      return (GLubyte *) "stack underflow";
   }
   else if (errorCode==GL_OUT_OF_MEMORY) {
      return (GLubyte *) "out of memory";
   }
   /* GLU Errors */
   else if (errorCode==GLU_NO_ERROR) {
      return (GLubyte *) "no error";
   }
   else if (errorCode==GLU_INVALID_ENUM) {
      return (GLubyte *) "invalid enum";
   }
   else if (errorCode==GLU_INVALID_VALUE) {
      return (GLubyte *) "invalid value";
   }
   else if (errorCode==GLU_OUT_OF_MEMORY) {
      return (GLubyte *) "out of memory";
   }
   else if (errorCode==GLU_INCOMPATIBLE_GL_VERSION) {
      return (GLubyte *) "incompatible GL version";
   }
   else if (errorCode>=GLU_TESS_ERROR1 && errorCode<=GLU_TESS_ERROR9) {
      return (GLubyte *) tess_error[errorCode-GLU_TESS_ERROR1];
   }
   else if (errorCode>=GLU_NURBS_ERROR1 && errorCode<=GLU_NURBS_ERROR37) {
      return (GLubyte *) nurbs_error[errorCode-GLU_NURBS_ERROR1];
   }
   else {
      return NULL;
   }
}



/*
 * New in GLU 1.1
 */

const GLubyte* gluGetString( GLenum name )
{
   static char *extensions = "";
   static char *version = "1.2.8 Mesa";

   if (name < 65536 && name > 0) name |= 0x10000;

   if (name == GLU_EXTENSIONS || (name & 0xffff) == (GLU_EXTENSIONS & 0xffff)) {
      return (GLubyte *) extensions;
   }
   if (name == GLU_VERSION || (name & 0xffff) == (GLU_VERSION & 0xffff)) {
      return (GLubyte *) version;
   }
   return NULL;
}
