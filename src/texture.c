/* texture.c */

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
$Id: texture.c,v 1.23 1996/02/02 23:22:32 brianp Exp $

$Log: texture.c,v $
 * Revision 1.23  1996/02/02  23:22:32  brianp
 * check that width and height = 2^k+2*border per Frederic Devernay
 *
 * Revision 1.22  1995/11/22  13:38:01  brianp
 * removed texcoord clamping optimization
 *
 * Revision 1.21  1995/11/13  21:45:53  brianp
 * fixed bug in texture border logic
 * added F suffix to many constants
 *
 * Revision 1.20  1995/11/09  16:57:52  brianp
 * fixed bugs in apply_texture for GL_BLEND per Johan Nouvel
 *
 * Revision 1.19  1995/11/03  17:39:30  brianp
 * removed unused variable, added casts for C++ compilation
 *
 * Revision 1.18  1995/10/14  17:42:45  brianp
 * compile glTexEnv, glTexParameter, and glTexGen into display lists
 *
 * Revision 1.17  1995/08/31  21:28:59  brianp
 * new TexGenEnabled bitfield
 * fixed a bug in spherical normal generation
 *
 * Revision 1.16  1995/07/24  20:35:40  brianp
 * replaced memset() with MEMSET() and memcpy() with MEMCPY()
 *
 * Revision 1.15  1995/07/20  15:36:06  brianp
 * added casts to PROD macro to suppress warnings with Sun's cc
 *
 * Revision 1.14  1995/06/12  15:43:06  brianp
 * changed color arrays to GLubyte
 *
 * Revision 1.13  1995/05/30  15:41:23  brianp
 * fixed GL_LUMINANCE bug in unpack_texture()
 *
 * Revision 1.12  1995/05/30  15:11:13  brianp
 * added more error checking to glGetTexLevelParamater[fi]v()
 * added glGetTexImage() stub
 *
 * Revision 1.11  1995/05/29  21:23:38  brianp
 * added glGetTexEnv*(), glGetTexGen*(), glGetTex[Level]Parameter*() functions
 *
 * Revision 1.10  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.9  1995/05/18  14:46:51  brianp
 * fixed texture DeleteFlag bug
 *
 * Revision 1.8  1995/05/16  14:09:44  brianp
 * more data types supported by glTexImage1/2D()
 *
 * Revision 1.7  1995/05/15  16:09:41  brianp
 * store textures as GLubytes instead of GLfloats
 * implemented GL_MODULATE, GL_DECAL, GL_BLEND and GL_REPLACE_EXT modes
 * save 1D and 2D textures in display lists
 *
 * Revision 1.6  1995/03/17  20:14:50  brianp
 * small bug fixes
 *
 * Revision 1.5  1995/03/10  15:19:23  brianp
 * added divide by zero check to gl_texgen
 *
 * Revision 1.4  1995/03/09  21:33:12  brianp
 * removed #include stdio.h
 *
 * Revision 1.3  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/03/02  19:11:30  brianp
 * modified gl_texgen and gl_texture_span_1d
 *
 * Revision 1.1  1995/02/24  14:28:31  brianp
 * Initial revision
 *
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "list.h"
#include "macros.h"
#include "pb.h"


#ifndef NULL
#  define NULL 0
#endif



/**********************************************************************/
/*                       Texture Environment                          */
/**********************************************************************/



void gl_texenv( GLenum target, GLenum pname, const GLfloat *param )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glTexEnv" );
      return;
   }

   if (target!=GL_TEXTURE_ENV) {
      gl_error( GL_INVALID_ENUM, "glTexEnv(target)" );
      return;
   }

   if (pname==GL_TEXTURE_ENV_MODE) {
      GLenum mode = (GLenum) (GLint) *param;
      switch (mode) {
	 case GL_MODULATE:
	 case GL_BLEND:
	 case GL_DECAL:
	 case GL_REPLACE_EXT:
	    CC.Texture.EnvMode = mode;
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glTexEnv(param)" );
	    return;
      }
   }
   else if (pname==GL_TEXTURE_ENV_COLOR) {
      CC.Texture.EnvColor[0] = CLAMP( param[0], 0.0, 1.0 );
      CC.Texture.EnvColor[1] = CLAMP( param[1], 0.0, 1.0 );
      CC.Texture.EnvColor[2] = CLAMP( param[2], 0.0, 1.0 );
      CC.Texture.EnvColor[3] = CLAMP( param[3], 0.0, 1.0 );
   }
   else {
      gl_error( GL_INVALID_ENUM, "glTexEnv(pname)" );
      return;
   }
}



void glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
   if (CC.CompileFlag) {
      gl_save_texenv( target, pname, &param );
   }
   if (CC.ExecuteFlag) {
      gl_texenv( target, pname, &param );
   }
}



void glTexEnvi( GLenum target, GLenum pname, GLint param )
{
   GLfloat p = (GLfloat) param;
   if (CC.CompileFlag) {
      gl_save_texenv( target, pname, &p );
   }
   if (CC.ExecuteFlag) {
      gl_texenv( target, pname, &p );
   }
}



void glTexEnvfv( GLenum target, GLenum pname, const GLfloat *param )
{
   if (CC.CompileFlag) {
      gl_save_texenv( target, pname, param );
   }
   if (CC.ExecuteFlag) {
      gl_texenv( target, pname, param );
   }
}



void glTexEnviv( GLenum target, GLenum pname, const GLint *param )
{
   GLfloat p[4];
   p[0] = INT_TO_FLOAT( param[0] );
   p[1] = INT_TO_FLOAT( param[1] );
   p[2] = INT_TO_FLOAT( param[2] );
   p[3] = INT_TO_FLOAT( param[3] );
   if (CC.CompileFlag) {
      gl_save_texenv( target, pname, p );
   }
   if (CC.ExecuteFlag) {
      gl_texenv( target, pname, p );
   }
}




void glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
   if (target!=GL_TEXTURE_ENV) {
      gl_error( GL_INVALID_ENUM, "glGetTexEnvfv(target)" );
      return;
   }
   switch (pname) {
      case GL_TEXTURE_ENV_MODE:
         *params = (GLfloat) CC.Texture.EnvMode;
	 break;
      case GL_TEXTURE_ENV_COLOR:
	 COPY_4V( params, CC.Texture.EnvColor );
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetTexEnvfv(pname)" );
   }
}


void glGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
   if (target!=GL_TEXTURE_ENV) {
      gl_error( GL_INVALID_ENUM, "glGetTexEnvfv(target)" );
      return;
   }
   switch (pname) {
      case GL_TEXTURE_ENV_MODE:
         *params = (GLint) CC.Texture.EnvMode;
	 break;
      case GL_TEXTURE_ENV_COLOR:
	 params[0] = FLOAT_TO_INT( CC.Texture.EnvColor[0] );
	 params[1] = FLOAT_TO_INT( CC.Texture.EnvColor[1] );
	 params[2] = FLOAT_TO_INT( CC.Texture.EnvColor[2] );
	 params[3] = FLOAT_TO_INT( CC.Texture.EnvColor[3] );
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetTexEnvfv(pname)" );
   }
}




/**********************************************************************/
/*                       Texture Parameters                           */
/**********************************************************************/


void gl_texparameter( GLenum target, GLenum pname, const GLfloat *params )
{
   GLenum eparam = (GLenum) (GLint) params[0];

   if (target==GL_TEXTURE_1D) {
      switch (pname) {
	 case GL_TEXTURE_MIN_FILTER:
	    if (eparam==GL_NEAREST || eparam==GL_LINEAR
		|| eparam==GL_NEAREST_MIPMAP_NEAREST
		|| eparam==GL_LINEAR_MIPMAP_NEAREST
		|| eparam==GL_NEAREST_MIPMAP_LINEAR
		|| eparam==GL_LINEAR_MIPMAP_LINEAR) {
	       CC.Texture.MinFilter1D = eparam;
	    }
	    else {
	       gl_error( GL_INVALID_VALUE, "glTexParameter(param)" );
	    }
	    break;
	 case GL_TEXTURE_MAG_FILTER:
	    if (eparam==GL_NEAREST || eparam==GL_LINEAR) {
	       CC.Texture.MagFilter1D = eparam;
	    }
	    else {
	       gl_error( GL_INVALID_VALUE, "glTexParameter(param)" );
	    }
	    break;
	 case GL_TEXTURE_WRAP_S:
	    if (eparam==GL_CLAMP || eparam==GL_REPEAT) {
	       CC.Texture.WrapS1D = eparam;
	    }
	    else {
	       gl_error( GL_INVALID_VALUE, "glTexParameter(param)" );
	    }
	    break;
	 case GL_TEXTURE_WRAP_T:
	    if (eparam==GL_CLAMP || eparam==GL_REPEAT) {
	       CC.Texture.WrapT1D = eparam;
	    }
	    else {
	       gl_error( GL_INVALID_VALUE, "glTexParameter(param)" );
	    }
	    break;
         case GL_TEXTURE_BORDER_COLOR:
            CC.Texture.BorderColor[0] = CLAMP( params[0], 0.0, 1.0 );
            CC.Texture.BorderColor[1] = CLAMP( params[1], 0.0, 1.0 );
            CC.Texture.BorderColor[2] = CLAMP( params[2], 0.0, 1.0 );
            CC.Texture.BorderColor[3] = CLAMP( params[3], 0.0, 1.0 );
            break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glTexParameter(pname)" );
      }
   }
   else if (target==GL_TEXTURE_2D) {
      switch (pname) {
	 case GL_TEXTURE_MIN_FILTER:
	    if (eparam==GL_NEAREST || eparam==GL_LINEAR
		|| eparam==GL_NEAREST_MIPMAP_NEAREST
		|| eparam==GL_LINEAR_MIPMAP_NEAREST
		|| eparam==GL_NEAREST_MIPMAP_LINEAR
		|| eparam==GL_LINEAR_MIPMAP_LINEAR) {
	       CC.Texture.MinFilter2D = eparam;
	    }
	    else {
	       gl_error( GL_INVALID_VALUE, "glTexParameter(param)" );
	    }
	    break;
	 case GL_TEXTURE_MAG_FILTER:
	    if (eparam==GL_NEAREST || eparam==GL_LINEAR) {
	       CC.Texture.MagFilter2D = eparam;
	    }
	    else {
	       gl_error( GL_INVALID_VALUE, "glTexParameter(param)" );
	    }
	    break;
	 case GL_TEXTURE_WRAP_S:
	    if (eparam==GL_CLAMP || eparam==GL_REPEAT) {
	       CC.Texture.WrapS2D = eparam;
	    }
	    else {
	       gl_error( GL_INVALID_VALUE, "glTexParameter(param)" );
	    }
	    break;
	 case GL_TEXTURE_WRAP_T:
	    if (eparam==GL_CLAMP || eparam==GL_REPEAT) {
	       CC.Texture.WrapT2D = eparam;
	    }
	    else {
	       gl_error( GL_INVALID_VALUE, "glTexParameter(param)" );
	    }
	    break;
         case GL_TEXTURE_BORDER_COLOR:
            CC.Texture.BorderColor[0] = CLAMP( params[0], 0.0, 1.0 );
            CC.Texture.BorderColor[1] = CLAMP( params[1], 0.0, 1.0 );
            CC.Texture.BorderColor[2] = CLAMP( params[2], 0.0, 1.0 );
            CC.Texture.BorderColor[3] = CLAMP( params[3], 0.0, 1.0 );
            break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glTexParameter(pname)" );
      }
   }
   else {
      gl_error( GL_INVALID_ENUM, "glTexParameter(target)" );
   }
}



void glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
   if (CC.CompileFlag) {
      gl_save_texparameter( target, pname, &param );
   }
   if (CC.ExecuteFlag) {
      gl_texparameter( target, pname, &param );
   }
}



void glTexParameteri( GLenum target, GLenum pname, GLint param )
{
   GLfloat fparam = param;
   if (CC.CompileFlag) {
      gl_save_texparameter( target, pname, &fparam );
   }
   if (CC.ExecuteFlag) {
      gl_texparameter( target, pname, &fparam );
   }
}



void glTexParameterfv( GLenum target, GLenum pname, const GLfloat *params )
{
   if (CC.CompileFlag) {
      gl_save_texparameter( target, pname, params );
   }
   if (CC.ExecuteFlag) {
      gl_texparameter( target, pname, params );
   }
}



void glTexParameteriv( GLenum target, GLenum pname, const GLint *params )
{
   GLfloat p[4];
   if (pname==GL_TEXTURE_BORDER_COLOR) {
      p[0] = INT_TO_FLOAT( params[0] );
      p[1] = INT_TO_FLOAT( params[1] );
      p[2] = INT_TO_FLOAT( params[2] );
      p[3] = INT_TO_FLOAT( params[3] );
   }
   else {
      p[0] = (GLfloat) params[0];
      p[1] = (GLfloat) params[1];
      p[2] = (GLfloat) params[2];
      p[3] = (GLfloat) params[3];
   }
   if (CC.CompileFlag) {
      gl_save_texparameter( target, pname, p );
   }
   if (CC.ExecuteFlag) {
      gl_texparameter( target, pname, p );
   }
}


void glGetTexLevelParameterfv( GLenum target, GLint level,
			       GLenum pname, GLfloat *params )
{
   if (level<0 || level>=MAX_TEXTURE_LEVELS) {
      gl_error( GL_INVALID_VALUE, "glGetTexLevelParameterfv" );
      return;
   }

   switch (target) {
      case GL_TEXTURE_1D:
         switch (pname) {
	    case GL_TEXTURE_WIDTH:
	       *params = (GLfloat) CC.TextureWidth1D[level];
	       break;
	    case GL_TEXTURE_COMPONENTS:
	       *params = (GLfloat) CC.TextureComponents1D[level];
	       break;
	    case GL_TEXTURE_BORDER:
	       *params = (GLfloat) CC.TextureBorder1D[level];
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glGetTexLevelParameterfv(pname)" );
	 }
	 break;
      case GL_TEXTURE_2D:
	 switch (pname) {
	    case GL_TEXTURE_WIDTH:
	       *params = (GLfloat) CC.TextureWidth2D[level];
	       break;
	    case GL_TEXTURE_HEIGHT:
	       *params = (GLfloat) CC.TextureHeight2D[level];
	       break;
	    case GL_TEXTURE_COMPONENTS:
	       *params = (GLfloat) CC.TextureComponents2D[level];
	       break;
	    case GL_TEXTURE_BORDER:
	       *params = (GLfloat) CC.TextureBorder2D[level];
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glGetTexLevelParameterfv(pname)" );
	 }
	 break;
     default:
	 gl_error( GL_INVALID_ENUM, "glGetTexLevelParameterfv(target)" );
   }	 
}


void glGetTexLevelParameteriv( GLenum target, GLint level,
			       GLenum pname, GLint *params )
{
   if (level<0 || level>=MAX_TEXTURE_LEVELS) {
      gl_error( GL_INVALID_VALUE, "glGetTexLevelParameteriv" );
      return;
   }

   switch (target) {
      case GL_TEXTURE_1D:
         switch (pname) {
	    case GL_TEXTURE_WIDTH:
	       *params = CC.TextureWidth1D[level];
	       break;
	    case GL_TEXTURE_COMPONENTS:
	       *params = CC.TextureComponents1D[level];
	       break;
	    case GL_TEXTURE_BORDER:
	       *params = CC.TextureBorder1D[level];
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glGetTexLevelParameteriv(pname)" );
	 }
	 break;
      case GL_TEXTURE_2D:
	 switch (pname) {
	    case GL_TEXTURE_WIDTH:
	       *params = CC.TextureWidth2D[level];
	       break;
	    case GL_TEXTURE_HEIGHT:
	       *params = CC.TextureHeight2D[level];
	       break;
	    case GL_TEXTURE_COMPONENTS:
	       *params = CC.TextureComponents2D[level];
	       break;
	    case GL_TEXTURE_BORDER:
	       *params = CC.TextureBorder2D[level];
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glGetTexLevelParameteriv(pname)" );
	 }
	 break;
     default:
	 gl_error( GL_INVALID_ENUM, "glGetTexLevelParameteriv(target)" );
   }	 
}



void glGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
   switch (target) {
      case GL_TEXTURE_1D:
         switch (pname) {
	    case GL_TEXTURE_MAG_FILTER:
	       *params = (GLfloat) CC.Texture.MagFilter1D;
	       break;
	    case GL_TEXTURE_MIN_FILTER:
	       *params = (GLfloat) CC.Texture.MinFilter1D;
	       break;
	    case GL_TEXTURE_WRAP_S:
	       *params = (GLfloat) CC.Texture.WrapS1D;
	       break;
	    case GL_TEXTURE_WRAP_T:
	       *params = (GLfloat) CC.Texture.WrapT1D;
	       break;
	    case GL_TEXTURE_BORDER_COLOR:
	       COPY_4V( params, CC.Texture.BorderColor );
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glGetTexParameterfv(pname)" );
	 }
         break;
      case GL_TEXTURE_2D:
         switch (pname) {
	    case GL_TEXTURE_MAG_FILTER:
	       *params = (GLfloat) CC.Texture.MagFilter2D;
	       break;
	    case GL_TEXTURE_MIN_FILTER:
	       *params = (GLfloat) CC.Texture.MinFilter2D;
	       break;
	    case GL_TEXTURE_WRAP_S:
	       *params = (GLfloat) CC.Texture.WrapS2D;
	       break;
	    case GL_TEXTURE_WRAP_T:
	       *params = (GLfloat) CC.Texture.WrapT2D;
	       break;
	    case GL_TEXTURE_BORDER_COLOR:
	       COPY_4V( params, CC.Texture.BorderColor );
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glGetTexParameterfv(pname)" );
	 }
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetTexParameterfv(target)" );
   }
}


void glGetTexParameteriv( GLenum target, GLenum pname, GLint *params )
{
   switch (target) {
      case GL_TEXTURE_1D:
         switch (pname) {
	    case GL_TEXTURE_MAG_FILTER:
	       *params = (GLint) CC.Texture.MagFilter1D;
	       break;
	    case GL_TEXTURE_MIN_FILTER:
	       *params = (GLint) CC.Texture.MinFilter1D;
	       break;
	    case GL_TEXTURE_WRAP_S:
	       *params = (GLint) CC.Texture.WrapS1D;
	       break;
	    case GL_TEXTURE_WRAP_T:
	       *params = (GLint) CC.Texture.WrapT1D;
	       break;
	    case GL_TEXTURE_BORDER_COLOR:
	       params[0] = FLOAT_TO_INT( CC.Texture.BorderColor[0] );
	       params[1] = FLOAT_TO_INT( CC.Texture.BorderColor[1] );
	       params[2] = FLOAT_TO_INT( CC.Texture.BorderColor[2] );
	       params[3] = FLOAT_TO_INT( CC.Texture.BorderColor[3] );
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glGetTexParameteriv(pname)" );
	 }
         break;
      case GL_TEXTURE_2D:
         switch (pname) {
	    case GL_TEXTURE_MAG_FILTER:
	       *params = (GLint) CC.Texture.MagFilter2D;
	       break;
	    case GL_TEXTURE_MIN_FILTER:
	       *params = (GLint) CC.Texture.MinFilter2D;
	       break;
	    case GL_TEXTURE_WRAP_S:
	       *params = (GLint) CC.Texture.WrapS2D;
	       break;
	    case GL_TEXTURE_WRAP_T:
	       *params = (GLint) CC.Texture.WrapT2D;
	       break;
	    case GL_TEXTURE_BORDER_COLOR:
	       params[0] = FLOAT_TO_INT( CC.Texture.BorderColor[0] );
	       params[1] = FLOAT_TO_INT( CC.Texture.BorderColor[1] );
	       params[2] = FLOAT_TO_INT( CC.Texture.BorderColor[2] );
	       params[3] = FLOAT_TO_INT( CC.Texture.BorderColor[3] );
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glGetTexParameteriv(pname)" );
	 }
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetTexParameteriv(target)" );
   }
}




/**********************************************************************/
/*                    Texture Coord Generation                        */
/**********************************************************************/


void gl_texgen( GLenum coord, GLenum pname, const GLfloat *params )
{
   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glTexGenfv" );
      return;
   }

   switch( coord ) {
      case GL_S:
         if (pname==GL_TEXTURE_GEN_MODE) {
	    GLenum mode = (GLenum) (GLint) *params;
	    if (mode==GL_OBJECT_LINEAR ||
		mode==GL_EYE_LINEAR ||
		mode==GL_SPHERE_MAP) {
	       CC.Texture.GenModeS = mode;
	    }
	    else {
	       gl_error( GL_INVALID_ENUM, "glTexGenfv(param)" );
	       return;
	    }
	 }
	 else if (pname==GL_OBJECT_PLANE) {
	    CC.Texture.ObjectPlaneS[0] = params[0];
	    CC.Texture.ObjectPlaneS[1] = params[1];
	    CC.Texture.ObjectPlaneS[2] = params[2];
	    CC.Texture.ObjectPlaneS[3] = params[3];
	 }
	 else if (pname==GL_EYE_PLANE) {
	    /* TODO:  xform plane by modelview??? */
	    CC.Texture.EyePlaneS[0] = params[0];
	    CC.Texture.EyePlaneS[1] = params[1];
	    CC.Texture.EyePlaneS[2] = params[2];
	    CC.Texture.EyePlaneS[3] = params[3];
	 }
	 else {
	    gl_error( GL_INVALID_ENUM, "glTexGenfv(pname)" );
	    return;
	 }
	 break;
      case GL_T:
         if (pname==GL_TEXTURE_GEN_MODE) {
	    GLenum mode = (GLenum) (GLint) *params;
	    if (mode==GL_OBJECT_LINEAR ||
		mode==GL_EYE_LINEAR ||
		mode==GL_SPHERE_MAP) {
	       CC.Texture.GenModeT = mode;
	    }
	    else {
	       gl_error( GL_INVALID_ENUM, "glTexGenfv(param)" );
	       return;
	    }
	 }
	 else if (pname==GL_OBJECT_PLANE) {
	    CC.Texture.ObjectPlaneT[0] = params[0];
	    CC.Texture.ObjectPlaneT[1] = params[1];
	    CC.Texture.ObjectPlaneT[2] = params[2];
	    CC.Texture.ObjectPlaneT[3] = params[3];
	 }
	 else if (pname==GL_EYE_PLANE) {
	    CC.Texture.EyePlaneT[0] = params[0];
	    CC.Texture.EyePlaneT[1] = params[1];
	    CC.Texture.EyePlaneT[2] = params[2];
	    CC.Texture.EyePlaneT[3] = params[3];
	 }
	 else {
	    gl_error( GL_INVALID_ENUM, "glTexGenfv(pname)" );
	    return;
	 }
	 break;
      case GL_R:
         if (pname==GL_TEXTURE_GEN_MODE) {
	    GLenum mode = (GLenum) (GLint) *params;
	    if (mode==GL_OBJECT_LINEAR ||
		mode==GL_EYE_LINEAR) {
	       CC.Texture.GenModeR = mode;
	    }
	    else {
	       gl_error( GL_INVALID_ENUM, "glTexGenfv(param)" );
	       return;
	    }
	 }
	 else if (pname==GL_OBJECT_PLANE) {
	    CC.Texture.ObjectPlaneR[0] = params[0];
	    CC.Texture.ObjectPlaneR[1] = params[1];
	    CC.Texture.ObjectPlaneR[2] = params[2];
	    CC.Texture.ObjectPlaneR[3] = params[3];
	 }
	 else if (pname==GL_EYE_PLANE) {
	    CC.Texture.EyePlaneR[0] = params[0];
	    CC.Texture.EyePlaneR[1] = params[1];
	    CC.Texture.EyePlaneR[2] = params[2];
	    CC.Texture.EyePlaneR[3] = params[3];
	 }
	 else {
	    gl_error( GL_INVALID_ENUM, "glTexGenfv(pname)" );
	    return;
	 }
	 break;
      case GL_Q:
         if (pname==GL_TEXTURE_GEN_MODE) {
	    GLenum mode = (GLenum) (GLint) *params;
	    if (mode==GL_OBJECT_LINEAR ||
		mode==GL_EYE_LINEAR) {
	       CC.Texture.GenModeQ = mode;
	    }
	    else {
	       gl_error( GL_INVALID_ENUM, "glTexGenfv(param)" );
	       return;
	    }
	 }
	 else if (pname==GL_OBJECT_PLANE) {
	    CC.Texture.ObjectPlaneQ[0] = params[0];
	    CC.Texture.ObjectPlaneQ[1] = params[1];
	    CC.Texture.ObjectPlaneQ[2] = params[2];
	    CC.Texture.ObjectPlaneQ[3] = params[3];
	 }
	 else if (pname==GL_EYE_PLANE) {
	    CC.Texture.EyePlaneQ[0] = params[0];
	    CC.Texture.EyePlaneQ[1] = params[1];
	    CC.Texture.EyePlaneQ[2] = params[2];
	    CC.Texture.EyePlaneQ[3] = params[3];
	 }
	 else {
	    gl_error( GL_INVALID_ENUM, "glTexGenfv(pname)" );
	    return;
	 }
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glTexGenfv(coord)" );
	 return;
   }

   CC.NewState = GL_TRUE;
}



void glTexGend( GLenum coord, GLenum pname, GLdouble param )
{
   GLfloat p = (GLfloat) param;
   if (CC.CompileFlag) {
      gl_save_texgen( coord, pname, &p );
   }
   if (CC.ExecuteFlag) {
      gl_texgen( coord, pname, &p );
   }
}


void glTexGenf( GLenum coord, GLenum pname, GLfloat param )
{
   if (CC.CompileFlag) {
      gl_save_texgen( coord, pname, &param );
   }
   if (CC.ExecuteFlag) {
      gl_texgen( coord, pname, &param );
   }
}


void glTexGeni( GLenum coord, GLenum pname, GLint param )
{
   GLfloat p = (GLfloat) param;
   if (CC.CompileFlag) {
      gl_save_texgen( coord, pname, &p );
   }
   if (CC.ExecuteFlag) {
      gl_texgen( coord, pname, &p );
   }
}


void glTexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{
   GLfloat p[4];
   p[0] = params[0];
   p[1] = params[1];
   p[2] = params[2];
   p[3] = params[3];
   if (CC.CompileFlag) {
      gl_save_texgen( coord, pname, p );
   }
   if (CC.ExecuteFlag) {
      gl_texgen( coord, pname, p );
   }
}


void glTexGeniv( GLenum coord, GLenum pname, const GLint *params )
{
   GLfloat p[4];
   p[0] = params[0];
   p[1] = params[1];
   p[2] = params[2];
   p[3] = params[3];
   if (CC.CompileFlag) {
      gl_save_texgen( coord, pname, p );
   }
   if (CC.ExecuteFlag) {
      gl_texgen( coord, pname, p );
   }
}


void glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
   if (CC.CompileFlag) {
      gl_save_texgen( coord, pname, params );
   }
   if (CC.ExecuteFlag) {
      gl_texgen( coord, pname, params );
   }
}


void glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{
   /* TODO */
}

void glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{
   /* TODO */
}

void glGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{
   /* TODO */
}



/*
 * Perform automatic texture coordinate generation.
 * Input:  obj - vertex in object coordinate system
 *         eye - vertex in eye coordinate system
 *         normal - normal vector in eye coordinate system
 * Output:  texcoord - the resuling texture coordinate, if TexGen enabled.
 */
void gl_do_texgen( const GLfloat obj[4],
                   const GLfloat eye[4],
                   const GLfloat normal[3],
                   GLfloat texcoord[4] )
{
   GLfloat u[3], two_nn, m, fx, fy, fz;

   if (CC.Texture.TexGenEnabled & S_BIT) {
      switch( CC.Texture.GenModeS) {
	 case GL_OBJECT_LINEAR:
            texcoord[0] = DOT4( obj, CC.Texture.ObjectPlaneS );
	    break;
	 case GL_EYE_LINEAR:
	    texcoord[0] = DOT4( eye, CC.Texture.EyePlaneS );
	    break;
	 case GL_SPHERE_MAP:
            COPY_3V( u, eye );
            NORMALIZE_3V( u );
	    two_nn = 2.0*DOT3(normal,normal);
	    fx = u[0] - two_nn * u[0];
	    fy = u[1] - two_nn * u[1];
	    fz = u[2] - two_nn * u[2];
	    m = 2.0 * sqrt( fx*fx + fy*fy + (fz+1.0)*(fz+1.0) );
	    if (m==0.0) {
	       texcoord[0] = 0.0;
	    }
	    else {
	       texcoord[0] = fx / m + 0.5;
	    }
	    break;
      }
   }

   if (CC.Texture.TexGenEnabled & T_BIT) {
      switch( CC.Texture.GenModeT) {
	 case GL_OBJECT_LINEAR:
	    texcoord[1] = DOT4( obj, CC.Texture.ObjectPlaneT );
	    break;
	 case GL_EYE_LINEAR:
	    texcoord[1] = DOT4( eye, CC.Texture.EyePlaneT );
	    break;
	 case GL_SPHERE_MAP:
	    /* TODO: safe to assume that m and fy valid from above??? */
	    if (m==0.0) {
	       texcoord[1] = 0.0;
	    }
	    else {
	       texcoord[1] = fy / m + 0.5;
	    }
	    break;
      }
   }

   if (CC.Texture.TexGenEnabled & R_BIT) {
      switch( CC.Texture.GenModeR) {
	 case GL_OBJECT_LINEAR:
	    texcoord[2] = DOT4( obj, CC.Texture.ObjectPlaneR );
	    break;
	 case GL_EYE_LINEAR:
	    texcoord[2] = DOT4( eye, CC.Texture.EyePlaneR );
	    break;
      }
   }

   if (CC.Texture.TexGenEnabled & Q_BIT) {
      switch( CC.Texture.GenModeQ) {
	 case GL_OBJECT_LINEAR:
	    texcoord[3] = DOT4( obj, CC.Texture.ObjectPlaneQ );
	    break;
	 case GL_EYE_LINEAR:
	    texcoord[3] = DOT4( eye, CC.Texture.EyePlaneQ );
	    break;
      }
   }
}



/**********************************************************************/
/*                           Texture Image                            */
/**********************************************************************/


/*
 * Convert the texture image given to glTexImage1D or glTexImage2D into
 * an array of GLubytes.
 * Return:  address of texture image or NULL if error.
 */
static GLubyte *unpack_texture( GLint components, GLsizei width, GLsizei height,
			        GLenum format, GLenum type, const GLvoid *pixels )
{
   GLboolean rflag, gflag, bflag, aflag, lflag;
   GLuint elements;
   GLuint i, row;
   GLubyte *texture, *texptr;
   GLboolean scale_or_bias;

   scale_or_bias = CC.Pixel.RedScale  !=1.0F || CC.Pixel.RedBias  !=0.0F
                || CC.Pixel.GreenScale!=1.0F || CC.Pixel.GreenBias!=0.0F
		|| CC.Pixel.BlueScale !=1.0F || CC.Pixel.BlueBias !=0.0F
		|| CC.Pixel.AlphaScale!=1.0F || CC.Pixel.AlphaBias!=0.0F;

   switch (format) {
      case GL_COLOR_INDEX:
         elements = 1;
	 rflag = gflag = bflag = aflag = lflag = GL_FALSE;
	 break;
      case GL_RED:
	 elements = 1;
	 rflag = GL_TRUE;
	 gflag = bflag = aflag = lflag = GL_FALSE;
	 break;
      case GL_GREEN:
	 elements = 1;
	 gflag = GL_TRUE;
	 rflag = bflag = aflag = lflag = GL_FALSE;
	 break;
      case GL_BLUE:
	 elements = 1;
	 bflag = GL_TRUE;
	 rflag = gflag = aflag = lflag = GL_FALSE;
	 break;
      case GL_ALPHA:
	 elements = 1;
	 aflag = GL_TRUE;
	 rflag = gflag = bflag = lflag = GL_FALSE;
	 break;
      case GL_RGB:
         elements = 3;
	 rflag = gflag = bflag = GL_TRUE;
	 aflag = lflag = GL_FALSE;
	 break;
      case GL_RGBA:
         elements = 4;
	 rflag = gflag = bflag = aflag = GL_TRUE;
	 lflag = GL_FALSE;
	 break;
      case GL_LUMINANCE:
         elements = 1;
	 rflag = gflag = bflag = aflag = GL_FALSE;
	 lflag = GL_TRUE;
	 break;
      case GL_LUMINANCE_ALPHA:
         elements = 2;
	 lflag = aflag = GL_TRUE;
	 rflag = gflag = bflag = GL_FALSE;
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glTexImage1/2D(format)" );
	 return NULL;
   }

   /* Allocate texture memory */
   texture = (GLubyte *) malloc( width * height * components );
   if (!texture) {
      gl_error( GL_OUT_OF_MEMORY, "glTexImage1/2D" );
      return NULL;
   }
   texptr = texture;

   /* TODO: obey glPixelStore parameters! */

   /* Build texture map image row by row */
   for (row=0;row<height;row++) {
      if (type==GL_UNSIGNED_BYTE && format==GL_RGB && components==3
	  && !scale_or_bias) {
	 /*
	  * A frequent and simple case
	  */
	 GLubyte *src = (GLubyte *) pixels + row * width * 3;
	 MEMCPY( texptr, src, 3*width );
	 texptr += 3*width;
      }
      else if (type==GL_UNSIGNED_BYTE && format==GL_RGBA && components==4
	  && !scale_or_bias) {
	 /*
	  * Another frequent and simple case
	  */
	 GLubyte *src = (GLubyte *) pixels + row * width * 4;
	 MEMCPY( texptr, src, 4*width );
	 texptr += 4*width;
      }
      else {
	 /*
	  * General solution
	  */
	 GLfloat red[MAX_TEXTURE_SIZE], green[MAX_TEXTURE_SIZE];
	 GLfloat blue[MAX_TEXTURE_SIZE], alpha[MAX_TEXTURE_SIZE];

	 switch (type) {
	    case GL_UNSIGNED_BYTE:
	       {
		  GLubyte *src = (GLubyte *) pixels + row * width * elements;
		  for (i=0;i<width;i++) {
		     if (lflag) {
			red[i] = green[i] = blue[i] = UBYTE_TO_FLOAT(*src++);
		     }
		     else {
			red[i]   = rflag ? UBYTE_TO_FLOAT(*src++) : 0.0F;
			green[i] = gflag ? UBYTE_TO_FLOAT(*src++) : 0.0F;
			blue[i]  = bflag ? UBYTE_TO_FLOAT(*src++) : 0.0F;
		     }
		     alpha[i] = aflag ? UBYTE_TO_FLOAT(*src++) : 1.0F;
		  }
	       }
	       break;
	    case GL_BYTE:
	       {
		  GLbyte *src = (GLbyte *) pixels + row * width * elements;
		  for (i=0;i<width;i++) {
		     if (lflag) {
			red[i] = green[i] = blue[i] = BYTE_TO_FLOAT(*src++);
		     }
		     else {
			red[i]   = rflag ? BYTE_TO_FLOAT(*src++) : 0.0F;
			green[i] = gflag ? BYTE_TO_FLOAT(*src++) : 0.0F;
			blue[i]  = bflag ? BYTE_TO_FLOAT(*src++) : 0.0F;
		     }
		     alpha[i] = aflag ? BYTE_TO_FLOAT(*src++) : 1.0F;
		  }
	       }
	       break;
	    case GL_UNSIGNED_SHORT:
	       {
		  GLushort *src = (GLushort *) pixels + row * width * elements;
		  for (i=0;i<width;i++) {
		     if (lflag) {
			red[i] = green[i] = blue[i] = USHORT_TO_FLOAT(*src++);
		     }
		     else {
			red[i]   = rflag ? USHORT_TO_FLOAT(*src++) : 0.0F;
			green[i] = gflag ? USHORT_TO_FLOAT(*src++) : 0.0F;
			blue[i]  = bflag ? USHORT_TO_FLOAT(*src++) : 0.0F;
		     }
		     alpha[i] = aflag ? USHORT_TO_FLOAT(*src++) : 1.0F;
		  }
	       }
	       break;
	    case GL_SHORT:
	       {
		  GLshort *src = (GLshort *) pixels + row * width * elements;
		  for (i=0;i<width;i++) {
		     if (lflag) {
			red[i] = green[i] = blue[i] = SHORT_TO_FLOAT(*src++);
		     }
		     else {
			red[i]   = rflag ? SHORT_TO_FLOAT(*src++) : 0.0F;
			green[i] = gflag ? SHORT_TO_FLOAT(*src++) : 0.0F;
			blue[i]  = bflag ? SHORT_TO_FLOAT(*src++) : 0.0F;
		     }
		     alpha[i] = aflag ? SHORT_TO_FLOAT(*src++) : 1.0F;
		  }
	       }
	       break;
	    /*TODO: implement rest of data types */
	    case GL_FLOAT:
	       {
		  GLfloat *src = (GLfloat *) pixels + row * width * elements;
		  for (i=0;i<width;i++) {
		     if (lflag) {
			red[i] = green[i] = blue[i] = *src++;
		     }
		     else {
			red[i]   = rflag ? *src++ : 0.0F;
			green[i] = gflag ? *src++ : 0.0F;
			blue[i]  = bflag ? *src++ : 0.0F;
		     }
		     alpha[i] = aflag ? *src++ : 1.0F;
		  }
	       }
	       break;
	    default:
	       gl_error( GL_INVALID_ENUM, "glTexImage1/2D(type)" );
	 } /* switch */

	 /* apply scale and/or bias */
	 if (scale_or_bias) {
	    for (i=0;i<width;i++) {
	       register GLfloat r, g, b, a;
	       r = red[i]   * CC.Pixel.RedScale   + CC.Pixel.RedBias;
	       g = green[i] * CC.Pixel.GreenScale + CC.Pixel.GreenBias;
	       b = blue[i]  * CC.Pixel.BlueScale  + CC.Pixel.BlueBias;
	       a = alpha[i] * CC.Pixel.AlphaScale + CC.Pixel.AlphaBias;
	       red[i]   = CLAMP( r, 0.0F, 1.0F );
	       green[i] = CLAMP( g, 0.0F, 1.0F );
	       blue[i]  = CLAMP( b, 0.0F, 1.0F );
	       alpha[i] = CLAMP( a, 0.0F, 1.0F );
	    }
	 }

	 /* save 8-bit components */
	 switch (components) {
	    case 1:  /* red */
	       for (i=0;i<width;i++) {
		  *texptr++ = (GLubyte) (GLint) (red[i] * 255.0F);
	       }
	       break;
	    case 2:  /* red, alpha */
	       for (i=0;i<width;i++) {
		  *texptr++ = (GLubyte) (GLint) (red[i] * 255.0F);
		  *texptr++ = (GLubyte) (GLint) (alpha[i] * 255.0F);
	       }
	       break;
	    case 3:  /* red, green, blue */
	       for (i=0;i<width;i++) {
		  *texptr++ = (GLubyte) (GLint) (red[i] * 255.0F);
		  *texptr++ = (GLubyte) (GLint) (green[i] * 255.0F);
		  *texptr++ = (GLubyte) (GLint) (blue[i] * 255.0F);
	       }
	       break;
	    case 4:  /* red, green, blue, alpha */
	       for (i=0;i<width;i++) {
		  *texptr++ = (GLubyte) (GLint) (red[i] * 255.0F);
		  *texptr++ = (GLubyte) (GLint) (green[i] * 255.0F);
		  *texptr++ = (GLubyte) (GLint) (blue[i] * 255.0F);
		  *texptr++ = (GLubyte) (GLint) (alpha[i] * 255.0F);
	       }
	       break;
	 } /* switch components */

      } /* general solution */

   } /* for row */

   return texture;
}


void gl_teximage1d( GLint level, GLint components,
		    GLsizei width, GLint border,
		    const GLubyte *pixels )
{
   /* check if we can delete the current image */
   if (CC.TextureImage1D[level] && CC.TextureImage1DDeleteFlag[level]) {
      free( CC.TextureImage1D[level] );
   }

   CC.TextureComponents1D[level] = components;
   CC.TextureWidth1D[level] = width + border;
   CC.TextureBorder1D[level] = border;
   CC.TextureImage1D[level] = (GLubyte *) pixels;
}



void glTexImage1D( GLenum target, GLint level, GLint components,
		   GLsizei width, GLint border, GLenum format,
		   GLenum type, const GLvoid *pixels )
{
   GLubyte *texture;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glTexImage1D" );
      return;
   }
   if (target!=GL_TEXTURE_1D) {
      gl_error( GL_INVALID_ENUM, "glTexImage1D" );
      return;
   }
   if (level<0 || level>=MAX_TEXTURE_LEVELS) {
      gl_error( GL_INVALID_VALUE, "glTexImage1D(level)" );
      return;
   }
   if (components<0 || components>4) {
      gl_error( GL_INVALID_VALUE, "glTexImage1D(components)" );
      return;
   }
   if (width<=2*border || width>2+MAX_TEXTURE_SIZE) {
      gl_error( GL_INVALID_VALUE, "glTexImage1D(width)" );
      return;
   }
   if (border!=0 && border!=1) {
      gl_error( GL_INVALID_VALUE, "glTexImage1D(border)" );
      return;
   }
   {
      GLint i, w;
      w = width - 2*border;
      i = 1;
      while ( w > i ) {
         i *= 2;
      }
      if (w != i) {
         gl_error( GL_INVALID_VALUE, "glTexImage1D(width != 2^k + 2*border))");
         return;
      }
   }

   /* Convert texture image to GLubytes */
   texture = unpack_texture( components, width+border, 1,
                             format, type, pixels );
   if (!texture) {
      return;
   }

   /* install the texture */
   if (CC.ExecuteFlag) {
      gl_teximage1d( level, components, width, border, texture );
   }
   if (CC.CompileFlag) {
      gl_save_teximage1d( level, components, width, border, texture );
      /* Mark the image as don't delete, it'll get deleted if/when the */
      /* display list is deallocated. */
      CC.TextureImage1DDeleteFlag[level] = GL_FALSE;
   }
   else {
      /* Mark this image as deletable because it's not in a display list. */
      CC.TextureImage1DDeleteFlag[level] = GL_TRUE;
   }
}



void gl_teximage2d( GLint level, GLint components,
		    GLsizei width, GLsizei height, GLint border,
		    const GLubyte *pixels )
{
   /* check if we can delete the current image */
   if (CC.TextureImage2D[level] && CC.TextureImage2DDeleteFlag[level]) {
      free( CC.TextureImage2D[level] );
   }

   CC.TextureComponents2D[level] = components;
   CC.TextureWidth2D[level] = width + border;
   CC.TextureHeight2D[level] = height + border;
   CC.TextureBorder2D[level] = border;
   CC.TextureImage2D[level] = (GLubyte *) pixels;
}



void glTexImage2D( GLenum target, GLint level, GLint components,
		   GLsizei width, GLsizei height, GLint border,
		   GLenum format, GLenum type, const GLvoid *pixels )
{
   GLubyte *texture;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glTexImage2D" );
      return;
   }
   if (target!=GL_TEXTURE_2D) {
      gl_error( GL_INVALID_ENUM, "glTexImage2D(target)" );
      return;
   }
   if (level<0 || level>=MAX_TEXTURE_LEVELS) {
      gl_error( GL_INVALID_VALUE, "glTexImage2D(level)" );
      return;
   }
   if (components<0 || components>4) {
      gl_error( GL_INVALID_VALUE, "glTexImage2D(components)" );
      return;
   }
   if (width<=2*border || width>2+MAX_TEXTURE_SIZE) {
      gl_error( GL_INVALID_VALUE, "glTexImage2D(width)" );
      return;
   }
   if (height<=2*border || height>2+MAX_TEXTURE_SIZE) {
      gl_error( GL_INVALID_VALUE, "glTexImage2D(height)" );
      return;
   }
   if (border!=0 && border!=1) {
      gl_error( GL_INVALID_VALUE, "glTexImage2D(border)" );
      return;
   }
   {
      GLint i, w;
      w = width - 2*border;
      i = 1;
      while ( w > i ) {
         i *= 2;
      }
      if (w != i) {
         gl_error(GL_INVALID_VALUE, "glTexImage2D(width != 2^k + 2*border))");
         return;
      }
   }
   {
      GLint i, h;
      h = height - 2*border;
      i = 1;
      while ( h > i ) {
         i *= 2;
      }
      if (h != i) {
         gl_error(GL_INVALID_VALUE, "glTexImage2D(height != 2^k + 2*border))");
         return;
      }
   }

   /* Convert texture image to GLubytes */
   texture = unpack_texture( components, width+border, height+border,
                             format, type, pixels );
   if (!texture) {
      return;
   }

   /* install the texture */
   if (CC.ExecuteFlag) {
      gl_teximage2d( level, components, width, height, border, texture );
   }
   if (CC.CompileFlag) {
      gl_save_teximage2d( level, components, width, height, border, texture );
      /* Mark the image as don't delete, it'll get deleted if/when the */
      /* display list is deallocated. */
      CC.TextureImage2DDeleteFlag[level] = GL_FALSE;
   }
   else {
      /* Mark this image as deletable because it's not in a display list. */
      CC.TextureImage2DDeleteFlag[level] = GL_TRUE;
   }
}



void glGetTexImage( GLenum target, GLint level, GLenum format,
		    GLenum type, GLvoid *pixels )
{
   /* TODO */
}




/**********************************************************************/
/*                   Perform Texture Mapping                          */
/**********************************************************************/



/*
 * Combine incoming fragment color with texel color to produce output color.
 */
static void apply_texture( GLuint n, GLint components,
	 GLubyte red[], GLubyte green[], GLubyte blue[], GLubyte alpha[],
	 GLubyte tred[], GLubyte tgreen[], GLubyte tblue[], GLubyte talpha[] )
{
   register GLuint i;
   GLint envred, envgreen, envblue;

   /* Texture environment color */
   envred   = (GLint) (CC.Texture.EnvColor[0] * 255.0F);
   envgreen = (GLint) (CC.Texture.EnvColor[1] * 255.0F);
   envblue  = (GLint) (CC.Texture.EnvColor[2] * 255.0F);

   if (CC.RedScale!=255.0) {
      /* This is a hack!  Rescale input colors from [0,scale] to [0,255]. */
      GLfloat rscale = 255.0 / CC.RedScale;
      GLfloat gscale = 255.0 / CC.GreenScale;
      GLfloat bscale = 255.0 / CC.BlueScale;
      GLfloat ascale = 255.0 / CC.AlphaScale;
      for (i=0;i<n;i++) {
	 red[i]   = (GLint) (red[i]   * rscale);
	 green[i] = (GLint) (green[i] * gscale);
	 blue[i]  = (GLint) (blue[i]  * bscale);
	 alpha[i] = (GLint) (alpha[i] * ascale);
      }
   }

/*
 * Use (A*(B+1)) >> 8 as a fast approximation of (A*B)/255 for A and B in [0,255]
 */
#define PROD(A,B)   (((GLint)(A) * (GLint)(B)+1) >> 8)

   switch (CC.Texture.EnvMode) {
      case GL_MODULATE:
         switch (components) {
	    case 1:  /* luminance */
	       for (i=0;i<n;i++) {
		  /* Cv = LtCf */
		  red[i]   = PROD( tred[i], red[i] );
		  green[i] = PROD( tred[i], green[i] );
		  blue[i]  = PROD( tred[i], blue[i] );
		  /* Av = Af */
	       }
	       break;
	    case 2: /* luminance, alpha */
	       for (i=0;i<n;i++) {
		  /* Cv = LtCf */
		  red[i]   = PROD( tred[i], red[i] );
		  green[i] = PROD( tred[i], green[i] );
		  blue[i]  = PROD( tred[i], blue[i] );
		  /* Av = AtAf */
		  alpha[i] = PROD( talpha[i], alpha[i] );
	       }
	       break;
	    case 3: /* rgb */
	       for (i=0;i<n;i++) {
		  /* Cv = CtCf */
		  red[i]   = PROD( tred[i],   red[i] );
		  green[i] = PROD( tgreen[i], green[i] );
		  blue[i]  = PROD( tblue[i],  blue[i] );
		  /* Av = Af */
	       }
	       break;
	    case 4: /* rgb, alpha */
	       for (i=0;i<n;i++) {
		  /* Cv = CtCf */
		  red[i]   = PROD( tred[i],   red[i] );
		  green[i] = PROD( tgreen[i], green[i] );
		  blue[i]  = PROD( tblue[i],  blue[i] );
		  /* Av = AtAf */
		  alpha[i] = PROD( talpha[i], alpha[i] );
	       }
	       break;
	 } /* components */
	 break;
      case GL_DECAL:
         switch (components) {
	    case 3:
	       for (i=0;i<n;i++) {
		  /* Cv = Ct */
		  red[i]   = tred[i];
		  green[i] = tgreen[i];
		  blue[i]  = tblue[i];
		  /* Av = Af */
	       }
	       break;
	    case 4:
	       for (i=0;i<n;i++) {
		  /* Cv = (1-At)Cf + AtCt */
		  GLint t = talpha[i], s = 255 - t;
		  red[i]   = PROD(s,red[i])   + PROD(t,tred[i]);
		  green[i] = PROD(s,green[i]) + PROD(t,tgreen[i]);
		  blue[i]  = PROD(s,blue[i])  + PROD(t,tblue[i]);
		  /* Av = Af */
	       }
	       break;
	 }
	 break;
      case GL_BLEND:
	 switch (components) {
	    case 1:
	       for (i=0;i<n;i++) {
		  /* Cv = (1-Lt)Cf + LtCc */
		  GLint t = tred[i], s = 255 - t;
		  red[i]   = PROD(s,red[i])   + PROD(t,envred);
		  green[i] = PROD(s,green[i]) + PROD(t,envgreen);
		  blue[i]  = PROD(s,blue[i])  + PROD(t,envblue);
		  /* Av = Af */
	       }
	       break;
	    case 2:
	       for (i=0;i<n;i++) {
		  /* Cv = (1-Lt)Cf + LtCc */
		  GLint t = tred[i], s = 255 - t;
		  red[i]   = PROD(s,red[i])   + PROD(t,envred);
		  green[i] = PROD(s,green[i]) + PROD(t,envgreen);
		  blue[i]  = PROD(s,blue[i])  + PROD(t,envblue);
		  /* Av = AtAf */
		  alpha[i] = PROD(talpha[i], alpha[i]);
	       }
	       break;
	    case 3:
	       for (i=0;i<n;i++) {
		  /* Cv = (1-Ct)Cf + CtCc */
		  red[i]   = PROD((255-tred[i]),  red[i]  ) + PROD(tred[i],  envred  );
		  green[i] = PROD((255-tgreen[i]),green[i]) + PROD(tgreen[i],envgreen);
		  blue[i]  = PROD((255-tblue[i]), blue[i] ) + PROD(tblue[i], envblue );
		  /* Av = Af */
	       }
	       break;
	    case 4:
	       for (i=0;i<n;i++) {
		  /* Cv = (1-Ct)Cf + CtCc */
		  red[i]   = PROD((255-tred[i]),  red[i]  ) + PROD(tred[i],  envred  );
		  green[i] = PROD((255-tgreen[i]),green[i]) + PROD(tgreen[i],envgreen);
		  blue[i]  = PROD((255-tblue[i]), blue[i] ) + PROD(tblue[i], envblue );
		  /* Av = AtAf */
		  alpha[i] = PROD(talpha[i], alpha[i]);
	       }
	       break;
	 }
	 break;
      case GL_REPLACE_EXT:
	 switch (components) {
	    case 1:
	       for (i=0;i<n;i++) {
		  /* Cv = Lt */
		  red[i] = green[i] = blue[i] = tred[i];
		  /* Av = Af */
	       }
	       break;
	    case 2:
	       for (i=0;i<n;i++) {
		  /* Cv = Lt */
		  red[i] = green[i] = blue[i] = tred[i];
		  /* Av = At */
		  alpha[i] = talpha[i];
	       }
	       break;
	    case 3:
	       for (i=0;i<n;i++) {
		  /* Cv = Ct */
		  red[i]   = tred[i];
		  green[i] = tgreen[i];
		  blue[i]  = tblue[i];
		  /* Av = Af */
	       }
	       break;
	    case 4:
	       for (i=0;i<n;i++) {
		  /* Cv = Ct */
		  red[i]   = tred[i];
		  green[i] = tgreen[i];
		  blue[i]  = tblue[i];
		  /* Av = At */
		  alpha[i] = talpha[i];
	       }
	       break;
	 }
	 break;
   }
#undef PROD

   if (CC.RedScale!=255.0) {
      /* This is a hack!  Rescale input colors from [0,255] to [0,scale]. */
      GLfloat rscale = CC.RedScale   / 255.0;
      GLfloat gscale = CC.GreenScale / 255.0;
      GLfloat bscale = CC.BlueScale  / 255.0;
      GLfloat ascale = CC.AlphaScale / 255.0;
      for (i=0;i<n;i++) {
	 red[i]   = (GLint) (red[i]   * rscale);
	 green[i] = (GLint) (green[i] * gscale);
	 blue[i]  = (GLint) (blue[i]  * bscale);
	 alpha[i] = (GLint) (alpha[i] * ascale);
      }
   }


}


/*
 * Apply the 1-D texture to an array of pixels.
 */
void gl_texture_pixels_1d( GLuint n, GLfloat s[], 
			   GLubyte red[], GLubyte green[],
			   GLubyte blue[], GLubyte alpha[] )
{
   GLubyte tred[PB_SIZE], tgreen[PB_SIZE];
   GLubyte tblue[PB_SIZE], talpha[PB_SIZE];
   GLint alpha_offset;
   GLuint level;
   GLuint i;

   level = 0;

   if (!CC.TextureImage1D[level]) {
      /* no texture image */
      return;
   }

   /* alpha_offset = offset of the alpha value in the texel of a texture image */
   if (CC.TextureComponents1D[level]==2) {
      alpha_offset = 1;
   }
   else if (CC.TextureComponents1D[level]==4) {
      alpha_offset = 3;
   }
   else {
      alpha_offset = 0;
   }

   /*
    * Compute texel colors.
    */
   for (i=0;i<n;i++) {
      GLfloat ss;
      GLboolean border;

      border = GL_FALSE;   /* Use border pixel color? */

      /* S COORDINATE */
      if (CC.Texture.WrapS1D==GL_REPEAT) {
         ss = s[i];
         while (ss<0.0F)  ss += 1.0F;
         while (ss>1.0F)  ss -= 1.0F;
/*	 ss = s[i] - (GLfloat) (GLint) s[i];*/
      }
      else {
	 /* GL_CLAMP */
	 if (s[i]<0.0F || s[i]>1.0F) {
	    if (CC.TextureBorder1D[level]) {
	       /* use border texel */
	       border = GL_TRUE;
	    }
	    else {
	       /* use border color */
	       ss = (s[i]<0.0F) ? 0.0F : 1.0F;
	    }
	 }
	 else {
	    ss = s[i];
	 }
      }

      if (border) {
	 tred[i]   = (GLint) (CC.Texture.BorderColor[0] * 255.0F);
	 tgreen[i] = (GLint) (CC.Texture.BorderColor[1] * 255.0F);
	 tblue[i]  = (GLint) (CC.Texture.BorderColor[2] * 255.0F);
	 talpha[i] = (GLint) (CC.Texture.BorderColor[3] * 255.0F);
      }
      else {
	 GLuint x, p;
	 if (ss<1.0F) {
	    x = (GLuint) (CC.TextureWidth1D[level] * ss);
	 }
	 else {
	    x = CC.TextureWidth1D[level]-1;
	 }

	 p = CC.TextureComponents1D[level] * x;
	 tred[i]   = CC.TextureImage1D[level][p];
	 tgreen[i] = CC.TextureImage1D[level][p+1];
	 tblue[i]  = CC.TextureImage1D[level][p+2];
	 talpha[i] = CC.TextureImage1D[level][p+alpha_offset];
      }
   }

   apply_texture( n, CC.TextureComponents1D[level],
		  red, green, blue, alpha, tred, tgreen, tblue, talpha );
}



/*
 * Apply the 2-D texture to an array of pixels.
 */
void gl_texture_pixels_2d( GLuint n,
			   GLfloat s[], GLfloat t[],
			   GLubyte red[], GLubyte green[],
			   GLubyte blue[], GLubyte alpha[] )
{
   GLubyte tred[PB_SIZE], tgreen[PB_SIZE];
   GLubyte tblue[PB_SIZE], talpha[PB_SIZE];
   GLint alpha_offset;
   GLuint level;
   GLuint i;

   level = 0;

   if (!CC.TextureImage2D[level]) {
      /* no texture image */
      return;
   }

   /* alpha_offset = offset of the alpha value in the texel of a texture image */
   if (CC.TextureComponents2D[level]==2) {
      alpha_offset = 1;
   }
   else if (CC.TextureComponents2D[level]==4) {
      alpha_offset = 3;
   }
   else {
      alpha_offset = 0;
   }

   /*
    * Compute texel colors.
    */
   for (i=0;i<n;i++) {
      GLfloat ss, tt;
      GLboolean border;

      border = GL_FALSE;   /* Use border pixel color? */

      /* S COORDINATE */
      if (CC.Texture.WrapS2D==GL_REPEAT) {
         ss = s[i];
         while (ss<0.0F)  ss += 1.0F;
         while (ss>1.0F)  ss -= 1.0F;
/*         ss = s[i] - (GLfloat) (GLint) s[i];*/
      }
      else {
	 /* GL_CLAMP */
	 if (s[i]<0.0F || s[i]>1.0F) {
	    if (CC.TextureBorder2D[level]) {
	       /* use border texel */
	       border = GL_TRUE;
	    }
	    else {
	       /* use border color */
	       ss = (s[i]<0.0F) ? 0.0F : 1.0F;
	    }
	 }
	 else {
	    ss = s[i];
	 }
      }

      /* T COORDINATE */
      if (CC.Texture.WrapT2D==GL_REPEAT) {
         tt = t[i];
         while (tt<0.0F)  tt += 1.0F;
         while (tt>1.0F)  tt -= 1.0F;
/*         tt = t[i] - (GLfloat) (GLint) t[i];*/
      }
      else {
	 /* GL_CLAMP */
	 if (t[i]<0.0F || t[i]>1.0F) {
	    if (CC.TextureBorder2D[level]) {
	       /* use border texel */
	       border = GL_TRUE;
	    }
	    else {
	       /* use border color */
	       tt = (t[i]<0.0F) ? 0.0F : 1.0F;
	    }
	 }
	 else {
	    tt = t[i];
	 }
      }

      if (border) {
	 tred[i]   = (GLint) (CC.Texture.BorderColor[0] * 255.0F);
	 tgreen[i] = (GLint) (CC.Texture.BorderColor[1] * 255.0F);
	 tblue[i]  = (GLint) (CC.Texture.BorderColor[2] * 255.0F);
	 talpha[i] = (GLint) (CC.Texture.BorderColor[3] * 255.0F);
      }
      else {
	 GLuint x, y, p;
	 if (ss<1.0F) {
	    x = (GLuint) (CC.TextureWidth2D[level] * ss);
	 }
	 else {
	    x = CC.TextureWidth2D[level]-1;
	 }
	 if (tt<1.0F) {
	    y = (GLuint) (CC.TextureHeight2D[level] * tt);
	 }
	 else {
	    y = CC.TextureHeight2D[level]-1;
	 }      

	 p = CC.TextureComponents2D[level] * (CC.TextureWidth2D[level] * y + x);
	 tred[i]   = CC.TextureImage2D[level][p];
	 tgreen[i] = CC.TextureImage2D[level][p+1];
	 tblue[i]  = CC.TextureImage2D[level][p+2];
	 talpha[i] = CC.TextureImage2D[level][p+alpha_offset];
      }
   }

   apply_texture( n, CC.TextureComponents2D[level],
		  red, green, blue, alpha, tred, tgreen, tblue, talpha );
}

