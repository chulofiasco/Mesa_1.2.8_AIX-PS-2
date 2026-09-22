/* $Id: misc.c,v 1.36 1996/05/01 20:46:01 brianp Exp $ */

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
$Log: misc.c,v $
 * Revision 1.36  1996/05/01  20:46:01  brianp
 * added GL_MESA_window_pos extension code
 *
 * Revision 1.35  1996/04/25  20:45:48  brianp
 * replaced gl_clear_depth_buffer() call with DD.clear_depth_buffer()
 * only call DD.finish and DD.flush if pointers are non-null
 * updated version string to 1.2.8
 *
 * Revision 1.34  1996/03/01  20:04:20  brianp
 * changed gl_error() for glGetString()
 *
 * Revision 1.33  1996/02/19  21:50:00  brianp
 * added support for software alpha buffering
 *
 * Revision 1.32  1996/02/15  16:02:23  brianp
 * added GL_EXT_vertex_array to extensions string
 *
 * Revision 1.31  1996/02/06  03:23:54  brianp
 * removed gamma correction code
 *
 * Revision 1.30  1996/01/22  15:37:30  brianp
 * changed version string to 1.2.6
 * added DD/NeXT work-around, per Michael Johnson
 *
 * Revision 1.29  1996/01/05  01:18:01  brianp
 * added profiling to glClear
 *
 * Revision 1.28  1995/11/30  00:19:19  brianp
 * changed version string to 1.2.5
 *
 * Revision 1.27  1995/11/22  13:35:40  brianp
 * check CC.NewState in glClear
 *
 * Revision 1.26  1995/11/13  22:08:29  brianp
 * removed comma from extensions string
 *
 * Revision 1.25  1995/11/03  17:39:48  brianp
 * removed unused variables
 *
 * Revision 1.24  1995/10/27  20:29:19  brianp
 * added GL_EXT_polygon_offset to extensions string
 *
 * Revision 1.23  1995/10/19  15:48:05  brianp
 * added gamma support
 * changed DD.clear_color arguments to GLubytes
 *
 * Revision 1.22  1995/10/14  16:29:48  brianp
 * new glReadBuffer and glDrawBuffer implementations
 * added glColor/IndexMask support when clearing color buffer
 *
 * Revision 1.21  1995/09/20  18:20:58  brianp
 * prototype device driver changes described
 *
 * Revision 1.20  1995/09/13  14:51:52  brianp
 * moved glGetError to context.c
 *
 * Revision 1.19  1995/09/08  14:08:51  brianp
 * use GL_QUADS instead of GL_POLYGON in glRect functions
 * updated version string to 1.2.3
 *
 * Revision 1.18  1995/07/24  18:55:49  brianp
 * added dd_finish()
 *
 * Revision 1.17  1995/07/18  20:23:58  brianp
 * updated version string for 1.2.2
 *
 * Revision 1.16  1995/06/29  22:10:47  brianp
 * don't call dd_clear_index() when in RGB mode
 * don't call dd_clear_color() when in CI mode
 *
 * Revision 1.15  1995/06/09  21:48:38  brianp
 * changed version string to 1.2.1
 *
 * Revision 1.14  1995/05/24  13:00:15  brianp
 * updated version query functions to return 1.2
 *
 * Revision 1.13  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.12  1995/05/17  13:52:37  brianp
 * implemented glIndexMask(0) and glColorMask(0,0,0,0)
 *
 * Revision 1.11  1995/05/12  16:57:22  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.10  1995/05/12  16:28:41  brianp
 * added const to glGetString prototype
 *
 * Revision 1.9  1995/04/17  14:48:19  brianp
 * updated glGetString for 1.1.4 beta
 *
 * Revision 1.8  1995/03/30  21:08:25  brianp
 * glClear limited to scissor box, not viewport!
 *
 * Revision 1.7  1995/03/13  20:55:59  brianp
 * new read buffer logic
 *
 * Revision 1.6  1995/03/10  16:25:19  brianp
 * updated glGetString for blending extensions
 *
 * Revision 1.5  1995/03/08  15:15:04  brianp
 * removed garbage characters from tail of file
 *
 * Revision 1.4  1995/03/08  15:10:02  brianp
 * added support for dd_clear_index and dd_clear_color
 *
 * Revision 1.3  1995/03/07  14:21:10  brianp
 * updated for new XSetForeground/GC scheme
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:24:57  brianp
 * Initial revision
 *
 */


/*
 * Miscellaneous functions
 */


#include <stdlib.h>
#include <string.h>
#include "accum.h"
#include "alphabuf.h"
#include "context.h"
#include "depth.h"
#include "dd.h"
#include "list.h"
#include "macros.h"
#include "masking.h"
#include "stencil.h"



/*
 * This is a kludge to get Mesa to work on NeXTs.
 * Contributed by Michael B. Johnson  (wave@welles.ThoughtPort.Net)
 */

#ifdef __NeXT__
struct dd_function_table DD;
#endif





void
glClearIndex( GLfloat c )
{
   if (CC.CompileFlag) {
      gl_save_clearindex( c );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glClearIndex" );
	 return;
      }
      CC.Color.ClearIndex = (GLuint) c;
      if (!CC.RGBAflag) {
	 /* it's OK to call glClearIndex in RGBA mode but it should be a NOP */
	 (*DD.clear_index)( CC.Color.ClearIndex );
      }
   }
}



void
glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
   if (CC.CompileFlag) {
      gl_save_clearcolor( red, green, blue, alpha );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glClearColor" );
	 return;
      }

      CC.Color.ClearColor[0] = CLAMP( red,   0.0F, 1.0F );
      CC.Color.ClearColor[1] = CLAMP( green, 0.0F, 1.0F );
      CC.Color.ClearColor[2] = CLAMP( blue,  0.0F, 1.0F );
      CC.Color.ClearColor[3] = CLAMP( alpha, 0.0F, 1.0F );

      if (CC.RGBAflag) {
         GLubyte r = (GLint) (CC.Color.ClearColor[0] * CC.RedScale);
         GLubyte g = (GLint) (CC.Color.ClearColor[1] * CC.GreenScale);
         GLubyte b = (GLint) (CC.Color.ClearColor[2] * CC.BlueScale);
         GLubyte a = (GLint) (CC.Color.ClearColor[3] * CC.AlphaScale);
	 (*DD.clear_color)( r, g, b, a );
      }
   }
}




/*
 * Clear the color buffer when glColorMask or glIndexMask is in effect.
 */
static void
clear_color_buffer_with_masking( void )
{
   GLint x, y, height, width;

   /* Compute region to clear */
   if (CC.Scissor.Enabled) {
      x = CC.Scissor.Xmin;
      y = CC.Scissor.Ymin;
      height = CC.Scissor.Ymax - CC.Scissor.Ymin + 1;
      width = CC.Scissor.Xmax - CC.Scissor.Xmin + 1;
   }
   else {
      x = 0;
      y = 0;
      height = CC.BufferHeight;
      width = CC.BufferWidth;
   }

   if (CC.RGBAflag) {
      /* RGBA mode */
      GLubyte red[MAX_WIDTH], green[MAX_WIDTH];
      GLubyte blue[MAX_WIDTH], alpha[MAX_WIDTH];
      GLubyte r = CC.Color.ClearColor[0] * CC.RedScale;
      GLubyte g = CC.Color.ClearColor[1] * CC.GreenScale;
      GLubyte b = CC.Color.ClearColor[2] * CC.BlueScale;
      GLubyte a = CC.Color.ClearColor[3] * CC.AlphaScale;
      GLint i;
      for (i=0;i<height;i++,y++) {
         MEMSET( red,   (int) r, width );
         MEMSET( green, (int) g, width );
         MEMSET( blue,  (int) b, width );
         MEMSET( alpha, (int) a, width );
         gl_mask_color_span( width, x, y, red, green, blue, alpha );
         (*DD.write_color_span)( width, x, y, red, green, blue, alpha, NULL );
         if (CC.RasterMask & ALPHABUF_BIT) {
            gl_write_alpha_span( width, x, y, alpha, NULL );
         }
      }
   }
   else {
      /* Color index mode */
      GLuint indx[MAX_WIDTH];
      GLubyte mask[MAX_WIDTH];
      GLint i, j;
      MEMSET( mask, 1, width );
      for (i=0;i<height;i++,y++) {
         for (j=0;j<width;j++) {
            indx[j] = CC.Color.ClearIndex;
         }
         gl_mask_index_span( width, x, y, indx );
         (*DD.write_index_span)( width, x, y, indx, mask );
      }
   }
}




void
glClear( GLbitfield mask )
{
   if (CC.CompileFlag) {
      gl_save_clear( mask );
   }
   if (CC.ExecuteFlag) {
#ifdef PROFILE
      GLdouble t0 = gl_time();
#endif

      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glClear" );
	 return;
      }

      if (CC.NewState) {
         gl_update_state();
      }

      if (mask & GL_DEPTH_BUFFER_BIT) {
	 (*DD.clear_depth_buffer)();
      }

      if (mask & GL_ACCUM_BUFFER_BIT) {
	 gl_clear_accum_buffer();
      }

      if (mask & GL_COLOR_BUFFER_BIT) {
         if (CC.Color.SWmasking) {
            clear_color_buffer_with_masking();
         }
         else {
            (*DD.clear)( !CC.Scissor.Enabled,
                         CC.Scissor.X, CC.Scissor.Y,
                         CC.Scissor.Width, CC.Scissor.Height );
            if (CC.RasterMask & ALPHABUF_BIT) {
               gl_clear_alpha_buffers();
            }
         }
      }

      if (mask & GL_STENCIL_BUFFER_BIT) {
	 gl_clear_stencil_buffer();
      }

#ifdef PROFILE
      CC.ClearTime += gl_time() - t0;
      CC.ClearCount++;
#endif

   }
}



void
glIndexMask( GLuint mask )
{
   if (CC.CompileFlag) {
      gl_save_indexmask( mask );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glIndexMask" );
	 return;
      }
      CC.Color.IndexMask = mask;
      CC.NewState = GL_TRUE;
   }
}



void
glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
   if (CC.CompileFlag) {
      gl_save_colormask( red, green, blue, alpha );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glColorMask" );
	 return;
      }
      CC.Color.ColorMask = (red << 3) | (green << 2) | (blue << 1) | alpha;
      CC.NewState = GL_TRUE;
   }
}



const GLubyte *
glGetString( GLenum name )
{
   static char *vendor = "Brian Paul";
   static char *renderer = "Mesa";
   static char *version = "1.2.8";
   static char *extensions = "GL_EXT_blend_color GL_EXT_blend_minmax GL_EXT_blend_logic_op GL_EXT_blend_subtract GL_EXT_polygon_offset GL_EXT_vertex_array GL_MESA_window_pos";

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetString" );
      return (GLubyte *) 0;
   }

   switch (name) {
      case GL_VENDOR:
         return (GLubyte *) vendor;
      case GL_RENDERER:
         return (GLubyte *) renderer;
      case GL_VERSION:
         return (GLubyte *) version;
      case GL_EXTENSIONS:
         return (GLubyte *) extensions;
      default:
         gl_error( GL_INVALID_ENUM, "glGetString" );
         return (GLubyte *) 0;
   }
}



void
glFinish( void )
{
   /* Don't compile into display list */
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glFinish" );
      return;
   }
   if (DD.finish) {
      (*DD.finish)();
   }
}



void
glFlush( void )
{
   /* Don't compile into display list */
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glFlush" );
      return;
   }
   if (DD.flush) {
      (*DD.flush)();
   }
}



void glHint( GLenum target, GLenum mode )
{
   if (CC.CompileFlag) {
      gl_save_hint( target, mode );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glHint" );
	 return;
      }
      if (mode!=GL_DONT_CARE && mode!=GL_FASTEST && mode!=GL_NICEST) {
	 gl_error( GL_INVALID_ENUM, "glHint(mode)" );
	 return;
      }
      switch (target) {
	 case GL_FOG_HINT:
	    CC.Hint.Fog = mode;
	    break;
	 case GL_LINE_SMOOTH_HINT:
	    CC.Hint.LineSmooth = mode;
	    break;
	 case GL_PERSPECTIVE_CORRECTION_HINT:
	    CC.Hint.PerspectiveCorrection = mode;
	    break;
	 case GL_POINT_SMOOTH_HINT:
	    CC.Hint.PointSmooth = mode;
	    break;
	 case GL_POLYGON_SMOOTH_HINT:
	    CC.Hint.PolygonSmooth = mode;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glHint(target)" );
      }
   }
}



void glDrawBuffer( GLenum mode )
{
   if (CC.CompileFlag) {
      gl_save_drawbuffer( mode );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glDrawBuffer" );
	 return;
      }
      switch (mode) {
	 case GL_FRONT:
	 case GL_FRONT_LEFT:
            if ( (*DD.set_buffer)( GL_FRONT ) == GL_FALSE ) {
               gl_error( GL_INVALID_ENUM, "glDrawBuffer" );
               return;
            }
            CC.Color.DrawBuffer = mode;
            CC.AlphaBuffer = CC.FrontAlphaBuffer;
            break;
	 case GL_BACK:
	 case GL_BACK_LEFT:
            if ( (*DD.set_buffer)( GL_BACK ) == GL_FALSE) {
               gl_error( GL_INVALID_ENUM, "glDrawBuffer" );
               return;
            }
            CC.Color.DrawBuffer = mode;
            CC.AlphaBuffer = CC.BackAlphaBuffer;
            break;
	 case GL_NONE:
	 case GL_FRONT_RIGHT:
	 case GL_BACK_RIGHT:
	 case GL_LEFT:
	 case GL_RIGHT:
	 case GL_FRONT_AND_BACK:
	 case GL_AUX0:
            gl_error( GL_INVALID_OPERATION, "glDrawBuffer" );
            break;
         default:
            gl_error( GL_INVALID_ENUM, "glDrawBuffer" );
      }
   }
}



void glReadBuffer( GLenum mode )
{
   if (CC.CompileFlag) {
      gl_save_readbuffer( mode );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glReadBuffer" );
	 return;
      }
      switch (mode) {
	 case GL_FRONT:
	 case GL_FRONT_LEFT:
            if ( (*DD.set_buffer)( GL_FRONT ) == GL_FALSE) {
               gl_error( GL_INVALID_ENUM, "glReadBuffer" );
               return;
            }
            CC.Pixel.ReadBuffer = mode;
            CC.AlphaBuffer = CC.FrontAlphaBuffer;
            break;
	 case GL_BACK:
	 case GL_BACK_LEFT:
            if ( (*DD.set_buffer)( GL_BACK ) == GL_FALSE) {
               gl_error( GL_INVALID_ENUM, "glReadBuffer" );
               return;
            }
            CC.Pixel.ReadBuffer = mode;
            CC.AlphaBuffer = CC.BackAlphaBuffer;
            break;
	 case GL_FRONT_RIGHT:
	 case GL_BACK_RIGHT:
	 case GL_LEFT:
	 case GL_RIGHT:
	 case GL_AUX0:
	    gl_error( GL_INVALID_OPERATION, "glReadBuffer" );
            break;
         default:
            gl_error( GL_INVALID_ENUM, "glReadBuffer" );
      }
   }

   /* Remember, the draw buffer is the default state */
   (void) (*DD.set_buffer)( CC.Color.DrawBuffer );
}



void glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
   /*
    * TODO:  optimize all glRect calls to issue gl_save_vertex and/or
    * gl_vertex calls, etc. depending on CC.ExecuteFlag and
    * CC.CompileFlag.
    */

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glRectd" );
      return;
   }
   glBegin( GL_QUADS );
   glVertex2d( x1, y1 );
   glVertex2d( x2, y1 );
   glVertex2d( x2, y2 );
   glVertex2d( x1, y2 );
   glEnd();
}


void glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glRectf" );
      return;
   }
   glBegin( GL_QUADS );
   glVertex2f( x1, y1 );
   glVertex2f( x2, y1 );
   glVertex2f( x2, y2 );
   glVertex2f( x1, y2 );
   glEnd();
}


void glRecti( GLint x1, GLint y1, GLint x2, GLint y2 )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glRecti" );
      return;
   }
   glBegin( GL_QUADS );
   glVertex2i( x1, y1 );
   glVertex2i( x2, y1 );
   glVertex2i( x2, y2 );
   glVertex2i( x1, y2 );
   glEnd();
}


void glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glRects" );
      return;
   }
   glBegin( GL_QUADS );
   glVertex2s( x1, y1 );
   glVertex2s( x2, y1 );
   glVertex2s( x2, y2 );
   glVertex2s( x1, y2 );
   glEnd();
}


void glRectdv( const GLdouble *v1, const GLdouble *v2 )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glRectdv" );
      return;
   }
   glBegin( GL_QUADS );
   glVertex2d( v1[0], v1[1] );
   glVertex2d( v2[0], v1[1] );
   glVertex2d( v2[0], v2[1] );
   glVertex2d( v1[0], v2[1] );
   glEnd();
}


void glRectfv( const GLfloat *v1, const GLfloat *v2 )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glRectfv" );
      return;
   }
   glBegin( GL_QUADS );
   glVertex2f( v1[0], v1[1] );
   glVertex2f( v2[0], v1[1] );
   glVertex2f( v2[0], v2[1] );
   glVertex2f( v1[0], v2[1] );
   glEnd();
}


void glRectiv( const GLint *v1, const GLint *v2 )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glRectiv" );
      return;
   }
   glBegin( GL_QUADS );
   glVertex2i( v1[0], v1[1] );
   glVertex2i( v2[0], v1[1] );
   glVertex2i( v2[0], v2[1] );
   glVertex2i( v1[0], v2[1] );
   glEnd();
}


void glRectsv( const GLshort *v1, const GLshort *v2 )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glRectsv" );
      return;
   }
   glBegin( GL_QUADS );
   glVertex2s( v1[0], v1[1] );
   glVertex2s( v2[0], v1[1] );
   glVertex2s( v2[0], v2[1] );
   glVertex2s( v1[0], v2[1] );
   glEnd();
}


