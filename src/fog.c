/* fog.c */

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
$Id: fog.c,v 1.13 1996/02/14 16:57:22 brianp Exp $

$Log: fog.c,v $
 * Revision 1.13  1996/02/14  16:57:22  brianp
 * optimized gl_fog_color|index_vertices() functions
 *
 * Revision 1.12  1995/12/30  17:16:17  brianp
 * gl_fog_color_vertices now takes integer colors instead of floats
 *
 * Revision 1.11  1995/12/20  15:26:39  brianp
 * changed color index array arguments to GLuint
 *
 * Revision 1.10  1995/12/18  17:25:35  brianp
 * use new GLdepth datatype, replace MAX_DEPTH with DEPTH_SCALE
 *
 * Revision 1.9  1995/11/03  22:35:35  brianp
 * replaced gl_fog_color_vertex with gl_fog_color_vertices
 * replaced gl_fog_index_vertex with gl_fog_index_vertices
 *
 * Revision 1.8  1995/07/20  15:35:11  brianp
 * added type casts to stop warnings with Sun's cc
 *
 * Revision 1.7  1995/06/12  15:41:04  brianp
 * removed debugging printf's
 *
 * Revision 1.6  1995/06/12  15:33:54  brianp
 * changed color arrays to GLubyte
 * check for -eyez in gl_fog_index_pixels
 *
 * Revision 1.5  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.4  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.3  1995/02/27  22:48:50  brianp
 * modified for PB
 *
 * Revision 1.2  1995/02/26  22:58:24  brianp
 * fixed glFogi
 *
 * Revision 1.1  1995/02/24  14:23:03  brianp
 * Initial revision
 *
 */


#include <math.h>
#include <stdlib.h>
#include "context.h"
#include "list.h"
#include "macros.h"



void gl_fog( GLenum pname, const GLfloat *params )
{
   GLenum m;

   switch (pname) {
      case GL_FOG_MODE:
         m = (GLenum) (GLint) *params;
	 if (m==GL_LINEAR || m==GL_EXP || m==GL_EXP2) {
	    CC.Fog.Mode = m;
	 }
	 else {
	    gl_error( GL_INVALID_ENUM, "glFog" );
	 }
	 break;
      case GL_FOG_DENSITY:
	 if (*params<0.0) {
	    gl_error( GL_INVALID_VALUE, "glFog" );
	 }
	 else {
	    CC.Fog.Density = *params;
	 }
	 break;
      case GL_FOG_START:
	 CC.Fog.Start = *params;
	 break;
      case GL_FOG_END:
	 CC.Fog.End = *params;
	 break;
      case GL_FOG_INDEX:
	 CC.Fog.Index = *params;
	 break;
      case GL_FOG_COLOR:
	 CC.Fog.Color[0] = params[0];
	 CC.Fog.Color[1] = params[1];
	 CC.Fog.Color[2] = params[2];
	 CC.Fog.Color[3] = params[3];
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glFog" );
   }
}



void glFogf( GLenum pname, GLfloat param )
{
   if (CC.ExecuteFlag) {
      gl_fog( pname, &param );
   }
   if (CC.CompileFlag) {
      gl_save_fog( pname, &param );
   }
}



void glFogi( GLenum pname, GLint param )
{
   GLfloat p;

   p = (GLfloat) param;

   if (CC.ExecuteFlag) {
      gl_fog( pname, &p );
   }
   if (CC.CompileFlag) {
      gl_save_fog( pname, &p );
   }
}



void glFogfv( GLenum pname, const GLfloat *params )
{
   if (CC.ExecuteFlag) {
      gl_fog( pname, params );
   }
   if (CC.CompileFlag) {
      gl_save_fog( pname, params );
   }
}



void glFogiv( GLenum pname, const GLint *params )
{
   GLfloat p[4];

   switch (pname) {
      case GL_FOG_MODE:
      case GL_FOG_DENSITY:
      case GL_FOG_START:
      case GL_FOG_END:
      case GL_FOG_INDEX:
         p[0] = (GLfloat) *params;
	 break;
      case GL_FOG_COLOR:
	 p[0] = INT_TO_FLOAT( params[0] );
	 p[1] = INT_TO_FLOAT( params[1] );
	 p[2] = INT_TO_FLOAT( params[2] );
	 p[3] = INT_TO_FLOAT( params[3] );
	 break;
      default:
	 gl_error( GL_INVALID_ENUM, "glFogi" );
	 return;
   }
   if (CC.ExecuteFlag) {
      gl_fog( pname, p );
   }
   if (CC.CompileFlag) {
      gl_save_fog( pname, p );
   }
}



/*
 * Compute the fogged color for an array of vertices.
 * Input:  n - number of vertices
 *         v - array of vertices
 *         color - the original vertex colors
 * Output:  color - the fogged colors
 */
void gl_fog_color_vertices( GLuint n, GLfloat v[][4], GLfixed color[][4] )
{
   GLuint i;
   GLfloat d;
   GLfloat shift = (GLfloat) (1 << CC.ColorShift);
   GLfloat fogr = CC.Fog.Color[0] * CC.RedScale   * shift;
   GLfloat fogg = CC.Fog.Color[1] * CC.GreenScale * shift;
   GLfloat fogb = CC.Fog.Color[2] * CC.BlueScale  * shift;
   GLfloat foga = CC.Fog.Color[3] * CC.AlphaScale * shift;

   switch (CC.Fog.Mode) {
      case GL_LINEAR:
         d = 1.0F / (CC.Fog.End - CC.Fog.Start);
         for (i=0;i<n;i++) {
            GLfloat f = (CC.Fog.End - ABSF(v[i][2])) * d;
            f = CLAMP( f, 0.0F, 1.0F );
            color[i][0] = f * color[i][0] + (1.0F-f) * fogr;
            color[i][1] = f * color[i][1] + (1.0F-f) * fogg;
            color[i][2] = f * color[i][2] + (1.0F-f) * fogb;
            color[i][3] = f * color[i][3] + (1.0F-f) * foga;
         }
	 break;
      case GL_EXP:
         d = -CC.Fog.Density;
         for (i=0;i<n;i++) {
            GLfloat f = exp( d * ABSF(v[i][2]) );
            f = CLAMP( f, 0.0F, 1.0F );
            color[i][0] = f * color[i][0] + (1.0F-f) * fogr;
            color[i][1] = f * color[i][1] + (1.0F-f) * fogg;
            color[i][2] = f * color[i][2] + (1.0F-f) * fogb;
            color[i][3] = f * color[i][3] + (1.0F-f) * foga;
         }
	 break;
      case GL_EXP2:
         d = -(CC.Fog.Density*CC.Fog.Density);
         for (i=0;i<n;i++) {
            GLfloat z = ABSF(v[i][2]);
            GLfloat f = exp( d * z*z );
            f = CLAMP( f, 0.0F, 1.0F );
            color[i][0] = f * color[i][0] + (1.0F-f) * fogr;
            color[i][1] = f * color[i][1] + (1.0F-f) * fogg;
            color[i][2] = f * color[i][2] + (1.0F-f) * fogb;
            color[i][3] = f * color[i][3] + (1.0F-f) * foga;
         }
	 break;
      default:
         abort();
   }
}



/*
 * Compute the fogged color indexes for an array of vertices.
 * Input:  n - number of vertices
 *         v - array of vertices
 * In/Out: indx - array of vertex color indexes
 */
void gl_fog_index_vertices( GLuint n, GLfloat v[][4], GLuint indx[] )
{
   /* NOTE: the extensive use of casts generates better/faster code for MIPS */
   switch (CC.Fog.Mode) {
      case GL_LINEAR:
         {
            GLfloat d = 1.0F / (CC.Fog.End - CC.Fog.Start);
            GLfloat fogindex = CC.Fog.Index;
            GLfloat fogend = CC.Fog.End;
            GLuint i;
            for (i=0;i<n;i++) {
               GLfloat f = (fogend - ABSF(v[i][2])) * d;
               f = CLAMP( f, 0.0F, 1.0F );
               indx[i] = (GLint) ((GLfloat) (GLint) indx[i] + (1.0F-f) * fogindex);
            }
         }
	 break;
      case GL_EXP:
         {
            GLfloat d = -CC.Fog.Density;
            GLfloat fogindex = CC.Fog.Index;
            GLuint i;
            for (i=0;i<n;i++) {
               GLfloat f = exp( d * ABSF(v[i][2]) );
               f = CLAMP( f, 0.0F, 1.0F );
               indx[i] = (GLint) ((GLfloat) (GLint) indx[i] + (1.0F-f) * fogindex);
            }
         }
	 break;
      case GL_EXP2:
         {
            GLfloat d = -(CC.Fog.Density*CC.Fog.Density);
            GLfloat fogindex = CC.Fog.Index;
            GLuint i;
            for (i=0;i<n;i++) {
               GLfloat z = ABSF(v[i][2]);
               GLfloat f = exp( -d * z*z );
               f = CLAMP( f, 0.0F, 1.0F );
               indx[i] = (GLint) ((GLfloat) (GLint) indx[i] + (1.0F-f) * fogindex);
            }
         }
	 break;
      default:
         abort();
   }
}




/*
 * Apply fog to an array of RGBA pixels.
 * Input:  n - number of pixels
 *         z - array of integer depth values
 *         red, green, blue, alpha - pixel colors
 * Output:  red, green, blue, alpha - fogged pixel colors
 */
void gl_fog_color_pixels( GLuint n, const GLdepth z[], GLubyte red[],
			  GLubyte green[], GLubyte blue[], GLubyte alpha[] )
{
   /* TODO: optimize case when c = 1.0 and d = 0.0 */
   GLfloat d = CC.ProjectionMatrix[14];
   GLfloat c = CC.ProjectionMatrix[10];
   GLfloat winz, ndcz, eyez, f;
   GLuint i;

   GLint fog_red   = (GLint) (CC.Fog.Color[0] * CC.RedScale);
   GLint fog_green = (GLint) (CC.Fog.Color[1] * CC.GreenScale);
   GLint fog_blue  = (GLint) (CC.Fog.Color[2] * CC.BlueScale);
   GLint fog_alpha = (GLint) (CC.Fog.Color[3] * CC.AlphaScale);

   switch (CC.Fog.Mode) {
      case GL_LINEAR:
         for (i=0;i<n;i++) {
	    winz = (GLfloat) z[i] / DEPTH_SCALE;
	    ndcz = (winz - CC.Viewport.Tz) / CC.Viewport.Sz;
	    eyez = -d / (c+ndcz);
	    if (eyez < 0.0)  eyez = -eyez;
	    f = (CC.Fog.End - eyez) / (CC.Fog.End - CC.Fog.Start);
	    f = CLAMP( f, 0.0F, 1.0F );
	    red[i]   = (GLint) (f * (GLfloat) red[i]   + (1.0F-f) * fog_red);
	    green[i] = (GLint) (f * (GLfloat) green[i] + (1.0F-f) * fog_green);
	    blue[i]  = (GLint) (f * (GLfloat) blue[i]  + (1.0F-f) * fog_blue);
	    alpha[i] = (GLint) (f * (GLfloat) alpha[i] + (1.0F-f) * fog_alpha);
	 }
	 break;
      case GL_EXP:
	 for (i=0;i<n;i++) {
	    winz = (GLfloat) z[i] / DEPTH_SCALE;
	    ndcz = (winz - CC.Viewport.Tz) / CC.Viewport.Sz;
	    eyez = -d / (c+ndcz);
	    if (eyez < 0.0)  eyez = -eyez;
	    f = exp( -CC.Fog.Density * eyez );
	    f = CLAMP( f, 0.0F, 1.0F );
	    red[i]   = (GLint) (f * (GLfloat) red[i]   + (1.0F-f) * fog_red);
	    green[i] = (GLint) (f * (GLfloat) green[i] + (1.0F-f) * fog_green);
	    blue[i]  = (GLint) (f * (GLfloat) blue[i]  + (1.0F-f) * fog_blue);
	    alpha[i] = (GLint) (f * (GLfloat) alpha[i] + (1.0F-f) * fog_alpha);
	 }
	 break;
      case GL_EXP2:
	 for (i=0;i<n;i++) {
	    winz = (GLfloat) z[i] / DEPTH_SCALE;
	    ndcz = (winz - CC.Viewport.Tz) / CC.Viewport.Sz;
	    eyez = -d / (c+ndcz);
	    if (eyez < 0.0)  eyez = -eyez;
	    f = exp( -(CC.Fog.Density*CC.Fog.Density * eyez*eyez) );
	    f = CLAMP( f, 0.0F, 1.0F );
	    red[i]   = (GLint) (f * (GLfloat) red[i]   + (1.0F-f) * fog_red);
	    green[i] = (GLint) (f * (GLfloat) green[i] + (1.0F-f) * fog_green);
	    blue[i]  = (GLint) (f * (GLfloat) blue[i]  + (1.0F-f) * fog_blue);
	    alpha[i] = (GLint) (f * (GLfloat) alpha[i] + (1.0F-f) * fog_alpha);
	 }
	 break;
   }
}




/*
 * Apply fog to an array of color index pixels.
 * Input:  n - number of pixels
 *         z - array of integer depth values
 *         index - pixel color indexes
 * Output:  index - fogged pixel color indexes
 */
void gl_fog_index_pixels( GLuint n, const GLdepth z[], GLuint index[] )
{
   /* TODO: optimize case when c = 1.0 and d = 0.0 */
   GLfloat d = CC.ProjectionMatrix[14];
   GLfloat c = CC.ProjectionMatrix[10];
   GLfloat winz, ndcz, eyez, f;
   GLuint i;

   switch (CC.Fog.Mode) {
      case GL_LINEAR:
         for (i=0;i<n;i++) {
	    winz = (GLfloat) z[i] / DEPTH_SCALE;
	    ndcz = (winz - CC.Viewport.Tz) / CC.Viewport.Sz;
	    eyez = -d / (c+ndcz);
	    if (eyez < 0.0)  eyez = -eyez;
	    f = (CC.Fog.End - eyez) / (CC.Fog.End - CC.Fog.Start);
	    f = CLAMP( f, 0.0F, 1.0F );
	    index[i] = (GLuint) ((GLfloat) index[i] + (1.0F-f) * CC.Fog.Index);
	 }
	 break;
      case GL_EXP:
         for (i=0;i<n;i++) {
	    winz = (GLfloat) z[i] / DEPTH_SCALE;
	    ndcz = (winz - CC.Viewport.Tz) / CC.Viewport.Sz;
	    eyez = -d / (c+ndcz);
	    if (eyez < 0.0)  eyez = -eyez;
	    f = exp( -CC.Fog.Density * eyez );
	    f = CLAMP( f, 0.0F, 1.0F );
	    index[i] = (GLuint) ((GLfloat) index[i] + (1.0F-f) * CC.Fog.Index);
	 }
	 break;
      case GL_EXP2:
         for (i=0;i<n;i++) {
	    winz = (GLfloat) z[i] / DEPTH_SCALE;
	    ndcz = (winz - CC.Viewport.Tz) / CC.Viewport.Sz;
	    eyez = -d / (c+ndcz);
	    if (eyez < 0.0)  eyez = -eyez;
	    f = exp( -(CC.Fog.Density*CC.Fog.Density * eyez*eyez) );
	    f = CLAMP( f, 0.0F, 1.0F );
	    index[i] = (GLuint) ((GLfloat) index[i] + (1.0F-f) * CC.Fog.Index);
	 }
	 break;
   }

}

