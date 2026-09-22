/* $Id: scissor.c,v 1.8 1996/05/08 16:49:37 brianp Exp $ */

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
$Log: scissor.c,v $
 * Revision 1.8  1996/05/08  16:49:37  brianp
 * optimized gl_scissor_pixels()
 *
 * Revision 1.7  1995/08/31  21:25:34  brianp
 * added a (GLint) type cast to gl_scissor_span() to fix a bug
 *
 * Revision 1.6  1995/08/31  18:34:16  brianp
 * added display list / glScissor support
 *
 * Revision 1.5  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.4  1995/05/12  16:57:22  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.3  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/27  22:49:05  brianp
 * modified for PB
 *
 * Revision 1.1  1995/02/24  14:27:12  brianp
 * Initial revision
 *
 */


/*
 * Scissor test.
 */


#include "context.h"
#include "macros.h"
#include "list.h"




void glScissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
   if (CC.CompileFlag) {
      gl_save_scissor( x, y, width, height );
   }
   if (CC.ExecuteFlag) {
      if (width<0 || height<0) {
         gl_error( GL_INVALID_VALUE, "glScissor" );
         return;
      }
      if (INSIDE_BEGIN_END) {
         gl_error( GL_INVALID_OPERATION, "glBegin" );
         return;
      }

      CC.Scissor.X = x;
      CC.Scissor.Y = y;
      CC.Scissor.Width = width;
      CC.Scissor.Height = height;

      /* in device window coords: */
      CC.Scissor.Xmin = x;
      CC.Scissor.Xmax = x+width-1;
      CC.Scissor.Ymin = y;
      CC.Scissor.Ymax = y+height-1;
   }
}



/*
 * Apply the scissor test to a span of pixels.
 * Return:  0 = all pixels in the span are outside the scissor box.
 *          1 = one or more pixels passed the scissor test.
 */
GLint gl_scissor_span( GLuint n, GLint x, GLint y, GLubyte mask[] )
{
   /* first check if whole span is outside the scissor box */
   if (y<CC.Scissor.Ymin || y>CC.Scissor.Ymax
       || x>CC.Scissor.Xmax || x+(GLint)n-1<CC.Scissor.Xmin) {
      return 0;
   }
   else {
      GLuint i;
      /* TODO: this could be better: */
      for (i=0;i<n;i++,x++) {
	 if (x<CC.Scissor.Xmin || x>CC.Scissor.Xmax)
	   mask[i] = 0;
      }
      return 1;
   }
}




/*
 * Apply the scissor test to an array of pixels.
 */
GLuint gl_scissor_pixels( GLuint n, const GLint x[], const GLint y[],
                          GLubyte mask[] )
{
   GLint xmin = CC.Scissor.Xmin;
   GLint xmax = CC.Scissor.Xmax;
   GLint ymin = CC.Scissor.Ymin;
   GLint ymax = CC.Scissor.Ymax;
   GLuint i;

   for (i=0;i<n;i++) {
      mask[i] &= (x[i]>=xmin) & (x[i]<=xmax) & (y[i]>=ymin) & (y[i]<=ymax);
   }

   return 1;
}

