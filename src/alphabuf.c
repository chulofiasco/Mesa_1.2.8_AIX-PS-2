/* $Id: alphabuf.c,v 1.1 1996/02/19 21:53:51 brianp Exp $ */

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
$Log: alphabuf.c,v $
 * Revision 1.1  1996/02/19  21:53:51  brianp
 * Initial revision
 *
 */



/*
 * Software alpha planes.  Many frame buffers don't have alpha bits so
 * we simulate them in software.
 */



#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "macros.h"




#define ALPHA_ADDR(X,Y)  (CC.AlphaBuffer + (Y) * CC.BufferWidth + (X))



/*
 * Allocate a new front and back alpha buffer.
 */
void gl_alloc_alpha_buffers( void )
{
   GLint bytes = CC.BufferWidth * CC.BufferHeight * sizeof(GLubyte);

   if (CC.FrontAlphaEnabled) {
      if (CC.FrontAlphaBuffer) {
         free( CC.FrontAlphaBuffer );
      }
      CC.FrontAlphaBuffer = (GLubyte *) malloc( bytes );
      if (!CC.FrontAlphaBuffer) {
         /* out of memory */
         gl_error( GL_OUT_OF_MEMORY, "Couldn't allocate front alpha buffer" );
      }
   }
   if (CC.BackAlphaEnabled) {
      if (CC.BackAlphaBuffer) {
         free( CC.BackAlphaBuffer );
      }
      CC.BackAlphaBuffer = (GLubyte *) malloc( bytes );
      if (!CC.BackAlphaBuffer) {
         /* out of memory */
         gl_error( GL_OUT_OF_MEMORY, "Couldn't allocate back alpha buffer" );
      }
   }
   if (CC.Color.DrawBuffer==GL_FRONT) {
      CC.AlphaBuffer = CC.FrontAlphaBuffer;
   }
   if (CC.Color.DrawBuffer==GL_BACK) {
      CC.AlphaBuffer = CC.BackAlphaBuffer;
   }
}



/*
 * Clear the front or back alpha planes.
 */
void gl_clear_alpha_buffers( void )
{
   GLint buffer;

   for (buffer=0;buffer<2;buffer++) {
      GLubyte *abuffer = NULL;
      if (buffer==0 && CC.Color.DrawBuffer==GL_FRONT
          && CC.FrontAlphaEnabled && CC.FrontAlphaBuffer) {
         abuffer = CC.FrontAlphaBuffer;
      }
      else if (buffer==1 && CC.Color.DrawBuffer==GL_BACK
          && CC.BackAlphaEnabled && CC.BackAlphaBuffer) {
         abuffer = CC.BackAlphaBuffer;
      }
      if (abuffer) {
         GLubyte aclear = (GLint) (CC.Color.ClearColor[3] * CC.AlphaScale);
         if (CC.Scissor.Enabled) {
            GLint i, j;
            for (j=0;j<CC.Scissor.Height;j++) {
               GLubyte *aptr = ALPHA_ADDR(CC.Scissor.Xmin, CC.Scissor.Ymin+j);
               for (i=0;i<CC.Scissor.Width;i++) {
                  *aptr++ = aclear;
               }
            }
         }
         else {
            MEMSET( abuffer, aclear, CC.BufferWidth * CC.BufferHeight );
         }
      }
   }
}



void gl_write_alpha_span( GLuint n, GLint x, GLint y,
                          GLubyte alpha[], GLubyte mask[] )
{
   GLubyte *aptr = ALPHA_ADDR( x, y );
   GLuint i;

   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            *aptr = alpha[i];
         }
         aptr++;
      }
   }
   else {
      for (i=0;i<n;i++) {
         *aptr++ = alpha[i];
      }
   }
}


void gl_write_mono_alpha_span( GLuint n, GLint x, GLint y,
                               GLubyte alpha, GLubyte mask[] )
{
   GLubyte *aptr = ALPHA_ADDR( x, y );
   GLuint i;

   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            *aptr = alpha;
         }
         aptr++;
      }
   }
   else {
      for (i=0;i<n;i++) {
         *aptr++ = alpha;
      }
   }
}


void gl_write_alpha_pixels( GLuint n, const GLint x[], const GLint y[],
                            const GLubyte alpha[], const GLubyte mask[] )
{
   GLuint i;

   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            GLubyte *aptr = ALPHA_ADDR( x[i], y[i] );
            *aptr = alpha[i];
         }
      }
   }
   else {
      for (i=0;i<n;i++) {
         GLubyte *aptr = ALPHA_ADDR( x[i], y[i] );
         *aptr = alpha[i];
      }
   }
}


void gl_write_mono_alpha_pixels( GLuint n, const GLint x[], const GLint y[],
                                 GLubyte alpha, const GLubyte mask[] )
{
   GLuint i;

   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            GLubyte *aptr = ALPHA_ADDR( x[i], y[i] );
            *aptr = alpha;
         }
      }
   }
   else {
      for (i=0;i<n;i++) {
         GLubyte *aptr = ALPHA_ADDR( x[i], y[i] );
         *aptr = alpha;
      }
   }
}



void gl_read_alpha_span( GLuint n, GLint x, GLint y, GLubyte alpha[] )
{
   GLubyte *aptr = ALPHA_ADDR( x, y );
   GLuint i;
   for (i=0;i<n;i++) {
      alpha[i] = *aptr++;
   }
}


void gl_read_alpha_pixels( GLuint n, const GLint x[], const GLint y[],
                           GLubyte alpha[], const GLubyte mask[] )
{
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *aptr = ALPHA_ADDR( x[i], y[i] );
         alpha[i] = *aptr;
      }
   }
}



