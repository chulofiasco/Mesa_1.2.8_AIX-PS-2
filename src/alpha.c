/* alpha.c */

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
 * Alpha test
 */


/*
$Id: alpha.c,v 1.7 1995/07/20 15:34:37 brianp Exp $

$Log: alpha.c,v $
 * Revision 1.7  1995/07/20  15:34:37  brianp
 * added type casts to stop warnings with Sun's cc
 *
 * Revision 1.6  1995/06/12  15:35:14  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.5  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.4  1995/05/12  19:22:23  brianp
 * added #include "macros.h"
 *
 * Revision 1.3  1995/05/12  19:20:19  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:15:10  brianp
 * Initial revision
 *
 */


#include "context.h"
#include "list.h"
#include "macros.h"


void glAlphaFunc( GLenum func, GLclampf ref )
{
   if (CC.CompileFlag) {
      gl_save_alphafunc( func, ref );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glAlphaFunc" );
	 return;
      }
      switch (func) {
	 case GL_NEVER:
	 case GL_LESS:
	 case GL_EQUAL:
	 case GL_LEQUAL:
	 case GL_GREATER:
	 case GL_NOTEQUAL:
	 case GL_GEQUAL:
	 case GL_ALWAYS:
	    CC.Color.AlphaFunc = func;
	    CC.Color.AlphaRef = CLAMP( ref, 0.0F, 1.0F );
	    CC.Color.AlphaRefInt = (GLint) (CC.Color.AlphaRef * CC.AlphaScale);
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glAlphaFunc" );
	    break;
      }
   }
}




/*
 * Apply the alpha test to a span of pixels.
 * In/Out:  mask - current pixel mask.  Pixels which fail the alpha test
 *                 will set the corresponding mask flag to 0.
 * Return:  0 = all pixels in the span failed the alpha test.
 *          1 = one or more pixels passed the alpha test.
 */
GLint gl_alpha_test( GLuint n, const GLubyte alpha[], GLubyte mask[] )
{
   GLuint i;

   /* switch cases ordered from most frequent to less frequent */
   switch (CC.Color.AlphaFunc) {
      case GL_LESS:
         for (i=0;i<n;i++) {
	    if ((GLint) alpha[i] >= CC.Color.AlphaRefInt) {
	       mask[i] = 0;
	    }
	 }
	 return 1;
      case GL_LEQUAL:
         for (i=0;i<n;i++) {
	    if ((GLint) alpha[i] > CC.Color.AlphaRefInt) {
	       mask[i] = 0;
	    }
	 }
	 return 1;
      case GL_GEQUAL:
         for (i=0;i<n;i++) {
	    if ((GLint) alpha[i] < CC.Color.AlphaRefInt) {
	       mask[i] = 0;
	    }
	 }
	 return 1;
      case GL_GREATER:
         for (i=0;i<n;i++) {
	    if ((GLint) alpha[i] <= CC.Color.AlphaRefInt) {
	       mask[i] = 0;
	    }
	 }
	 return 1;
      case GL_NOTEQUAL:
         for (i=0;i<n;i++) {
	    if ((GLint) alpha[i] == CC.Color.AlphaRefInt) {
	       mask[i] = 0;
	    }
	 }
	 return 1;
      case GL_EQUAL:
         for (i=0;i<n;i++) {
	    if ((GLint) alpha[i] != CC.Color.AlphaRefInt) {
	       mask[i] = 0;
	    }
	 }
	 return 1;
      case GL_ALWAYS:
	 /* do nothing */
	 return 1;
      case GL_NEVER:
	 for (i=0;i<n;i++) {
	    mask[i] = 0;
	 }
	 return 0;
      default:
	 gl_error( GL_INVALID_ENUM, "Internal error in gl_alpha_test" );
	 break;
   }
   return 1;
}
