/* feedback.c */

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
$Id: feedback.c,v 1.12 1995/09/07 14:16:35 brianp Exp $

$Log: feedback.c,v $
 * Revision 1.12  1995/09/07  14:16:35  brianp
 * use CC.NewState convention
 *
 * Revision 1.11  1995/07/26  15:03:48  brianp
 * replaced some literals with variables for SunOS 4.x per Asif Khan
 *
 * Revision 1.10  1995/07/21  15:08:49  brianp
 * set feedback buffer count to 0 in glFeedbackBuffer() per Bogdan Sikorski
 *
 * Revision 1.9  1995/06/08  20:52:23  brianp
 * removed HitMin/MaxZ tests from feedback_vertex()
 *
 * Revision 1.8  1995/06/07  19:43:54  brianp
 * glRenderMode() returns -1 if feedback buffer overflows, reset counter to zero
 *
 * Revision 1.7  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.6  1995/05/22  17:18:05  brianp
 * undid prev change to WRITE_RECORD due to overflow logic
 *
 * Revision 1.5  1995/05/19  13:26:50  brianp
 * added display list support for selection/name stack functions
 *
 * Revision 1.4  1995/05/12  17:00:43  brianp
 * changed CC.Mode!=0 to INSIDE_BEGIN_END
 *
 * Revision 1.3  1995/04/18  15:48:23  brianp
 * fixed assignment of NULL to function pointers to prevent warnings on Suns
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:22:05  brianp
 * Initial revision
 *
 */


#include "context.h"
#include "feedback.h"
#include "list.h"
#include "macros.h"




/**********************************************************************/
/*                              Feedback                              */
/**********************************************************************/


#define FB_3D		0x01
#define FB_4D		0x02
#define FB_INDEX	0x04
#define FB_COLOR	0x08
#define FB_TEXTURE	0X10



void glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer )
{
   if (CC.RenderMode==GL_FEEDBACK || INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glFeedbackBuffer" );
      return;
   }

   if (size<0) {
      gl_error( GL_INVALID_VALUE, "glFeedbackBuffer" );
   }

   switch (type) {
      case GL_2D:
	 CC.FeedbackMask = 0;
         CC.FeedbackType = type;
	 break;
      case GL_3D:
	 CC.FeedbackMask = FB_3D;
         CC.FeedbackType = type;
	 break;
      case GL_3D_COLOR:
	 CC.FeedbackMask = FB_3D | (CC.RGBAflag ? FB_COLOR : FB_INDEX);
         CC.FeedbackType = type;
	 break;
      case GL_3D_COLOR_TEXTURE:
	 CC.FeedbackMask = FB_3D | (CC.RGBAflag ? FB_COLOR : FB_INDEX)
	                      | FB_TEXTURE;
         CC.FeedbackType = type;
	 break;
      case GL_4D_COLOR_TEXTURE:
	 CC.FeedbackMask = FB_3D | FB_4D | (CC.RGBAflag ? FB_COLOR : FB_INDEX)
	                      | FB_TEXTURE;
         CC.FeedbackType = type;
	 break;
      default:
	 CC.FeedbackMask = 0;
         gl_error( GL_INVALID_ENUM, "glFeedbackBuffer" );
   }

   CC.FeedbackBufferSize = size;
   CC.FeedbackBuffer = buffer;
   CC.FeedbackCount = 0;
}



void gl_passthrough( GLfloat token )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glPassThrough" );
      return;
   }

   if (CC.RenderMode==GL_FEEDBACK) {
      APPEND_TOKEN( (GLfloat) GL_PASS_THROUGH_TOKEN );
      APPEND_TOKEN( token );
   }
}


void glPassThrough( GLfloat token )
{
   if (CC.CompileFlag) {
      gl_save_passthrough( token );
   }
   if (CC.ExecuteFlag) {
      gl_passthrough( token );
   }
}



/*
 * Put a vertex into the feedback buffer.
 */
void gl_feedback_vertex( GLfloat x, GLfloat y, GLfloat z, GLfloat w,
			 const GLfloat color[4], GLfloat index,
			 const GLfloat texcoord[4] )
{
   APPEND_TOKEN( x );
   APPEND_TOKEN( y );
   if (CC.FeedbackMask & FB_3D) {
      APPEND_TOKEN( z );
   }
   if (CC.FeedbackMask & FB_4D) {
      APPEND_TOKEN( w );
   }
   if (CC.FeedbackMask & FB_INDEX) {
      APPEND_TOKEN( index );
   }
   if (CC.FeedbackMask & FB_COLOR) {
      APPEND_TOKEN( color[0] );
      APPEND_TOKEN( color[1] );
      APPEND_TOKEN( color[2] );
      APPEND_TOKEN( color[3] );
   }
   if (CC.FeedbackMask & FB_TEXTURE) {
      APPEND_TOKEN( texcoord[0] );
      APPEND_TOKEN( texcoord[1] );
      APPEND_TOKEN( texcoord[2] );
      APPEND_TOKEN( texcoord[3] );
   }
}



/**********************************************************************/
/*                              Selection                             */
/**********************************************************************/


void glSelectBuffer( GLsizei size, GLuint *buffer )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glSelectBuffer" );
   }
   if (CC.RenderMode==GL_SELECT) {
      gl_error( GL_INVALID_OPERATION, "glSelectBuffer" );
   }
   CC.SelectBuffer = buffer;
   CC.SelectBufferSize = size;
   CC.SelectBufferCount = 0;

   CC.HitFlag = GL_FALSE;
   CC.HitMinZ = 1.0;
   CC.HitMaxZ = 0.0;
}


void glInitNames( void )
{
   if (CC.CompileFlag) {
      gl_save_initnames();
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glInitNames" );
      }
      CC.NameStackDepth = 0;
      CC.HitFlag = GL_FALSE;
      CC.HitMinZ = 1.0;
      CC.HitMaxZ = 0.0;
   }
}



#define WRITE_RECORD( V )					\
	if (CC.SelectBufferCount < CC.SelectBufferSize) {	\
	   CC.SelectBuffer[CC.SelectBufferCount] = (V);		\
	}							\
	CC.SelectBufferCount++;



static void write_hit_record( void )
{
   if (CC.SelectBufferCount < CC.SelectBufferSize) {
      GLuint i;
      GLuint zmin, zmax, zscale = (~0u);

      /* HitMinZ and HitMaxZ are in [0,1].  Multiply these values by */
      /* 2^32-1 and round to nearest unsigned integer. */

      zmin = (GLuint) ((GLfloat) zscale * CC.HitMinZ);
      zmax = (GLuint) ((GLfloat) zscale * CC.HitMaxZ);

      WRITE_RECORD( CC.NameStackDepth );
      WRITE_RECORD( zmin );
      WRITE_RECORD( zmax );
      for (i=0;i<CC.NameStackDepth;i++) {
         WRITE_RECORD( CC.NameStack[i] );
      }

      CC.SelectHits++;
   }

   /* TODO: correct??? */
   CC.HitFlag = GL_FALSE;
   CC.HitMinZ = 1.0;
   CC.HitMaxZ = -1.0;
}



void glLoadName( GLuint name )
{
   if (CC.CompileFlag) {
      gl_save_loadname( name );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glLoadName" );
	 return;
      }
      if (CC.RenderMode!=GL_SELECT) {
	 return;
      }
      if (CC.NameStackDepth==0) {
	 gl_error( GL_INVALID_OPERATION, "glLoadName" );
	 return;
      }
      if (CC.HitFlag) {
	 write_hit_record();
      }
      if (CC.NameStackDepth<MAX_NAME_STACK_DEPTH) {
	 CC.NameStack[CC.NameStackDepth-1] = name;
      }
      else {
	 CC.NameStack[MAX_NAME_STACK_DEPTH-1] = name;
      }
   }
}


void glPushName( GLuint name )
{
   if (CC.CompileFlag) {
      gl_save_pushname( name );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glPushName" );
	 return;
      }
      if (CC.RenderMode!=GL_SELECT) {
	 return;
      }
      if (CC.HitFlag) {
	 write_hit_record();
      }
      if (CC.NameStackDepth<MAX_NAME_STACK_DEPTH) {
	 CC.NameStack[CC.NameStackDepth++] = name;
      }
      else {
	 gl_error( GL_STACK_OVERFLOW, "glPushName" );
      }
   }
}


void glPopName( void )
{
   if (CC.CompileFlag) {
      gl_save_popname();
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glPopName" );
	 return;
      }
      if (CC.RenderMode!=GL_SELECT) {
	 return;
      }
      if (CC.HitFlag) {
	 write_hit_record();
      }
      if (CC.NameStackDepth>0) {
	 CC.NameStackDepth--;
      }
      else {
	 gl_error( GL_STACK_UNDERFLOW, "glPopName" );
      }
   }
}



/**********************************************************************/
/*                           Render Mode                              */
/**********************************************************************/



GLint glRenderMode( GLenum mode )
{
   GLint result;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glInitNames" );
   }

   switch (CC.RenderMode) {
      case GL_RENDER:
	 result = 0;
	 break;
      case GL_SELECT:
	 if (CC.HitFlag) {
	    write_hit_record();
	 }
	 if (CC.SelectBufferCount > CC.SelectBufferSize) {
	    /* overflow */
	    result = -1;
	 }
	 else {
	    result = CC.SelectHits;
	 }
	 CC.SelectBufferCount = 0;
	 CC.SelectHits = 0;
	 CC.NameStackDepth = 0;
	 break;
      case GL_FEEDBACK:
	 if (CC.FeedbackCount > CC.FeedbackBufferSize) {
	    /* overflow */
	    result = -1;
	 }
	 else {
	    result = CC.FeedbackCount;
	 }
	 CC.FeedbackCount = 0;
	 break;
   }

   switch (mode) {
      case GL_RENDER:
         break;
      case GL_SELECT:
	 if (CC.SelectBufferSize==0) {
	    /* haven't called glSelectBuffer yet */
	    gl_error( GL_INVALID_OPERATION, "glRenderMode" );
	 }
	 break;
      case GL_FEEDBACK:
	 if (CC.FeedbackBufferSize==0) {
	    /* haven't called glFeedbackBuffer yet */
	    gl_error( GL_INVALID_OPERATION, "glRenderMode" );
	 }
	 break;
      default:
	 gl_error( GL_INVALID_ENUM, "glRenderMode" );
	 return 0;
   }

   CC.RenderMode = mode;
   CC.NewState = GL_TRUE;

   return result;
}

