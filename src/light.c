/* $Id: light.c,v 1.40 1996/05/03 17:48:09 brianp Exp $ */

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
$Log: light.c,v $
 * Revision 1.40  1996/05/03  17:48:09  brianp
 * fixed bug involving 4th element of spot light direction vector
 *
 * Revision 1.39  1996/04/30  12:21:44  brianp
 * changed gl_compute_material_shine_table() per Jacques Leroy
 *
 * Revision 1.38  1996/03/18  17:15:48  brianp
 * material->ShineTable[0] should be 0.0, not 1.0
 *
 * Revision 1.37  1996/03/11  14:16:15  brianp
 * fixed pow(0,0) error in gl_compute_material_shininess_table per Bill Henshaw
 *
 * Revision 1.36  1996/02/26  15:20:22  brianp
 * replaced pow() calls with lookup tables as suggested by Jean-Luc Daems
 *
 * Revision 1.35  1996/02/14  16:56:23  brianp
 * replaced gl_index_shade() with gl_index_shade_vertices()
 *
 * Revision 1.34  1996/02/13  17:53:02  brianp
 * added gl_update_lighting() and gl_color_shade_vertices_fast()
 *
 * Revision 1.33  1996/01/22  15:28:48  brianp
 * fixed a couple bugs in gl_index_shade()
 *
 * Revision 1.32  1996/01/09  19:52:28  brianp
 * fixed an FP overflow bug in gl_init_lighting()
 *
 * Revision 1.31  1996/01/07  22:48:41  brianp
 * removed gl_color_shade()
 * introduced POW macro for computing specular exponent term
 *
 * Revision 1.30  1995/12/30  00:54:26  brianp
 * return integer colors instead of floats in shading functions
 *
 * Revision 1.29  1995/12/20  15:27:03  brianp
 * gl_index_shade changed to return GLuint color indexes instead of GLfloat
 *
 * Revision 1.28  1995/12/19  17:06:11  brianp
 * new implementation of glColorMaterial
 *
 * Revision 1.27  1995/11/02  20:03:05  brianp
 * changed some GLuints to GLints
 *
 * Revision 1.26  1995/11/01  23:18:47  brianp
 * fixed a typo in gl_color_shade()
 *
 * Revision 1.25  1995/11/01  21:46:06  brianp
 * optimized gl_color_shade(), added new gl_color_shade_vertices() function
 *
 * Revision 1.24  1995/10/24  18:36:58  brianp
 * only set CC.NewState when ShadeModel really changes
 *
 * Revision 1.23  1995/10/14  17:41:52  brianp
 * add code to compile glLightModel into display lists
 *
 * Revision 1.22  1995/10/06  13:02:56  brianp
 * fixed glMaterial GL_AMBIENT_AND_DIFFUSE bug
 *
 * Revision 1.21  1995/09/26  17:20:33  brianp
 * fixed uninitialized fmask,bmask bug in gl_material
 *
 * Revision 1.20  1995/09/25  19:24:19  brianp
 * implemented per-vertex glMaterial calls
 *
 * Revision 1.19  1995/09/25  13:21:13  brianp
 * fixed a bug in computing the specular coefficient per Olaf Flebbe
 *
 * Revision 1.18  1995/09/22  16:22:03  brianp
 * added comment about glMaterial not working inside glBegin/glEnd
 *
 * Revision 1.17  1995/09/05  15:35:08  brianp
 * introduced CC.NewState convention
 * many small optimizations made in gl_color_shade() and gl_index_shade()
 *
 * Revision 1.16  1995/07/25  13:27:51  brianp
 * gl_index_shade() returns GLfloats instead of GLuints
 *
 * Revision 1.15  1995/05/30  15:10:45  brianp
 * use ROUND() macro in glGetMaterialiv
 *
 * Revision 1.14  1995/05/29  21:21:53  brianp
 * added glGetMaterial*() functions
 *
 * Revision 1.13  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.12  1995/05/12  17:00:43  brianp
 * changed CC.Mode!=0 to INSIDE_BEGIN_END
 *
 * Revision 1.11  1995/05/12  16:29:51  brianp
 * fixed glGetLightiv()'s prototype
 *
 * Revision 1.10  1995/04/13  19:48:33  brianp
 * fixed GL_LIGHT_MODEL_LOCAL_VIEWER bugs per Armin Liebchen
 *
 * Revision 1.9  1995/04/08  15:25:45  brianp
 * compile glShadeModel
 * fixed pow() domain error
 * added spotlight and attenutation factor to gl_index_shade
 *
 * Revision 1.8  1995/03/28  20:28:03  brianp
 * fixed alpha lighting bug
 *
 * Revision 1.7  1995/03/10  21:41:01  brianp
 * added divide by zero checks
 *
 * Revision 1.6  1995/03/09  21:41:03  brianp
 * new ModelViewInv matrix logic
 *
 * Revision 1.5  1995/03/09  20:07:08  brianp
 * changed gl_transform_point to macro call
 * changed order of arguments in gl_transform_vector
 *
 * Revision 1.4  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.3  1995/02/25  21:01:56  brianp
 * added underflow check for specular coefficient
 *
 * Revision 1.2  1995/02/25  18:52:12  brianp
 * changed NORMALIZE macro
 *
 * Revision 1.1  1995/02/24  14:23:06  brianp
 * Initial revision
 *
 */


#include <assert.h>
#include <math.h>
#include "context.h"
#include "light.h"
#include "list.h"
#include "macros.h"
#include "vb.h"
#include "xform.h"



#ifndef M_PI
#  define M_PI (3.1415926)
#endif

#define DEG2RAD (M_PI/180.0)



void glShadeModel( GLenum mode )
{
   if (CC.CompileFlag) {
      gl_save_shademodel( mode );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
	 gl_error( GL_INVALID_OPERATION, "glShadeModel" );
	 return;
      }

      switch (mode) {
	 case GL_FLAT:
	 case GL_SMOOTH:
            if (CC.Light.ShadeModel!=mode) {
               CC.Light.ShadeModel = mode;
               CC.NewState = GL_TRUE;
            }
	    break;
	 default:
	    gl_error( GL_INVALID_ENUM, "glShadeModel" );
      }
   }
}




void glColorMaterial( GLenum face, GLenum mode )
{
   if (CC.CompileFlag) {
      gl_save_colormaterial( face, mode );
   }
   if (CC.ExecuteFlag) {
      if (INSIDE_BEGIN_END) {
         gl_error( GL_INVALID_OPERATION, "glColorMaterial" );
         return;
      }
      switch (face) {
         case GL_FRONT:
         case GL_BACK:
         case GL_FRONT_AND_BACK:
            CC.Light.ColorMaterialFace = face;
            break;
         default:
            gl_error( GL_INVALID_ENUM, "glColorMaterial" );
            return;
      }
      switch (mode) {
         case GL_EMISSION:
         case GL_AMBIENT:
         case GL_DIFFUSE:
         case GL_SPECULAR:
         case GL_AMBIENT_AND_DIFFUSE:
            CC.Light.ColorMaterialMode = mode;
            break;
         default:
            gl_error( GL_INVALID_ENUM, "glColorMaterial" );
            return;
      }
   }
}



/*
 * pname: GL_AMBIENT, red, green, blue, alpha in [-1,1]
 *        GL_DIFFUSE, red, green, blue, alhpa in [-1,1]
 *        GL_SPECULAR, red, green, blue, alpha in [-1,1]
 *        GL_POSITION, x, y, z, w in homogeneous object coordinates
 *        GL_SPOT_DIRECTION, dx, dy, dz
 *        GL_SPOT_EXPONENT, intensity in [0,128]
 *        GL_SPOT_CUTOFF, angle in [0,90] or 180
 *        GL_CONSTANT_ATTENUATION, factor >= 0
 *        GL_LINEAR_ATTENUATION, factor >= 0
 *        GL_QUADRATIC_ATTENUATION, factor >= 0
 */
void gl_light( GLenum light, GLenum pname, const GLfloat *params )
{
   GLint l;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glShadeModel" );
      return;
   }

   l = (GLint) (light - GL_LIGHT0);

   if (l<0 || l>=MAX_LIGHTS) {
      gl_error( GL_INVALID_ENUM, "glLight" );
      return;
   }

   switch (pname) {
      case GL_AMBIENT:
         COPY_4V( CC.Light.Light[l].Ambient, params );
         break;
      case GL_DIFFUSE:
         COPY_4V( CC.Light.Light[l].Diffuse, params );
         break;
      case GL_SPECULAR:
         COPY_4V( CC.Light.Light[l].Specular, params );
         break;
      case GL_POSITION:
	 /* transform position by ModelView matrix */
	 TRANSFORM_POINT( CC.Light.Light[l].Position, CC.ModelViewMatrix,
                          params );
         break;
      case GL_SPOT_DIRECTION:
	 /* transform direction by inverse modelview */
         {
            GLfloat direction[4];
            direction[0] = params[0];
            direction[1] = params[1];
            direction[2] = params[2];
            direction[3] = 0.0;
            if (!CC.ModelViewInvValid) {
               gl_compute_modelview_inverse();
            }
            gl_transform_vector( CC.Light.Light[l].Direction,
                                 direction, CC.ModelViewInv);
         }
         break;
      case GL_SPOT_EXPONENT:
         if (params[0]<0.0 || params[0]>128.0) {
            gl_error( GL_INVALID_VALUE, "glLight" );
            return;
         }
         CC.Light.Light[l].SpotExponent = params[0];
         gl_compute_spot_exp_table( &CC.Light.Light[l] );
         break;
      case GL_SPOT_CUTOFF:
         if ((params[0]<0.0 || params[0]>90.0) && params[0]!=180.0) {
            gl_error( GL_INVALID_VALUE, "glLight" );
            return;
         }
         CC.Light.Light[l].SpotCutoff = params[0];
         CC.Light.Light[l].CosCutoff = cos(params[0]*DEG2RAD);
         break;
      case GL_CONSTANT_ATTENUATION:
         if (params[0]<0.0) {
            gl_error( GL_INVALID_VALUE, "glLight" );
            return;
         }
         CC.Light.Light[l].ConstantAttenuation = params[0];
         break;
      case GL_LINEAR_ATTENUATION:
         if (params[0]<0.0) {
            gl_error( GL_INVALID_VALUE, "glLight" );
            return;
         }
         CC.Light.Light[l].LinearAttenuation = params[0];
         break;
      case GL_QUADRATIC_ATTENUATION:
         if (params[0]<0.0) {
            gl_error( GL_INVALID_VALUE, "glLight" );
            return;
         }
         CC.Light.Light[l].QuadraticAttenuation = params[0];
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glLight" );
         break;
   }

   CC.NewState = GL_TRUE;
}



/*
 * light: GL_LIGHT0, GL_LIGHT1, ... GL_LIGHTn where n=GL_MAX_LIGHTS-1
 * pname: GL_SPOT_EXPONENT, GL_SPOT_CUTOFF, GL_CONSTANT_ATTENUATION,
 *        GL_LINEAR_ATTENUATION, GL_QUADRATIC_ATTENUATION
 */
void glLightf( GLenum light, GLenum pname, GLfloat param )
{
   if (CC.ExecuteFlag) {
      gl_light( light, pname, &param );
   }
   if (CC.CompileFlag) {
      gl_save_light( light, pname, &param, 1 );
   }
}



void glLighti( GLenum light, GLenum pname, GLint param )
{
   GLfloat fparam;

   fparam = (GLfloat) param;
   if (CC.ExecuteFlag) {
      gl_light( light, pname, &fparam );
   }
   if (CC.CompileFlag) {
      gl_save_light( light, pname, &fparam, 1 );
   }
}



void glLightfv( GLenum light, GLenum pname, const GLfloat *params )
{
   if (CC.ExecuteFlag) {
      gl_light( light, pname, params );
   }
   if (CC.CompileFlag) {
      gl_save_light( light, pname, params, 4 );
   }
}



void glLightiv( GLenum light, GLenum pname, const GLint *params )
{
   GLfloat fparam[4];

   switch (pname) {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_POSITION:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         fparam[3] = (GLfloat) params[3];
         break;
      case GL_SPOT_DIRECTION:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         break;
      case GL_SPOT_EXPONENT:
      case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
         fparam[0] = (GLfloat) params[0];
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glLight" );
         return;
   }
   if (CC.ExecuteFlag) {
      gl_light( light, pname, fparam );
   }
   if (CC.CompileFlag) {
      gl_save_light( light, pname, fparam, 4 );
   }
}



void glGetLightfv( GLenum light, GLenum pname, GLfloat *params )
{
   GLint l;

   l = (GLint) (light - GL_LIGHT0);

   if (l<0 || l>=MAX_LIGHTS) {
      gl_error( GL_INVALID_ENUM, "glGetLightfv" );
      return;
   }

   switch (pname) {
      case GL_AMBIENT:
         COPY_4V( params, CC.Light.Light[l].Ambient );
         break;
      case GL_DIFFUSE:
         COPY_4V( params, CC.Light.Light[l].Diffuse );
         break;
      case GL_SPECULAR:
         COPY_4V( params, CC.Light.Light[l].Specular );
         break;
      case GL_POSITION:
         COPY_4V( params, CC.Light.Light[l].Position );
         break;
      case GL_SPOT_DIRECTION:
         COPY_3V( params, CC.Light.Light[l].Direction );
         break;
      case GL_SPOT_EXPONENT:
         params[0] = CC.Light.Light[l].SpotExponent;
         break;
      case GL_SPOT_CUTOFF:
         params[0] = CC.Light.Light[l].SpotCutoff;
         break;
      case GL_CONSTANT_ATTENUATION:
         params[0] = CC.Light.Light[l].ConstantAttenuation;
         break;
      case GL_LINEAR_ATTENUATION:
         params[0] = CC.Light.Light[l].LinearAttenuation;
         break;
      case GL_QUADRATIC_ATTENUATION:
         params[0] = CC.Light.Light[l].QuadraticAttenuation;
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetLightfv" );
         break;
   }
}



void glGetLightiv( GLenum light, GLenum pname, GLint *params )
{
   /* TODO */
}



/**********************************************************************/
/***                        Light Model                             ***/
/**********************************************************************/


void gl_lightmodel( GLenum pname, const GLfloat *params )
{
   switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:
         COPY_4V( CC.Light.Model.Ambient, params );
         break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
         if (params[0]==0.0)
            CC.Light.Model.LocalViewer = GL_FALSE;
         else
            CC.Light.Model.LocalViewer = GL_TRUE;
         break;
      case GL_LIGHT_MODEL_TWO_SIDE:
         if (params[0]==0.0)
            CC.Light.Model.TwoSide = GL_FALSE;
         else
            CC.Light.Model.TwoSide = GL_TRUE;
         CC.NewState = GL_TRUE;
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glLightModel" );
         break;
   }
   CC.NewState = GL_TRUE;
}



/*
 * pname: GL_LIGHT_MODEL_LOCAL_VIEWER, GL_LIGHT_MODEL_TWO_SIDE
 */
void glLightModelf( GLenum pname, GLfloat param )
{
   if (CC.CompileFlag) {
      gl_save_lightmodel( pname, &param );
   }
   if (CC.ExecuteFlag) {
      gl_lightmodel( pname, &param );
   }
}



void glLightModeli( GLenum pname, GLint param )
{
   GLfloat fparam = (GLfloat) param;
   if (CC.CompileFlag) {
      gl_save_lightmodel( pname, &fparam );
   }
   if (CC.ExecuteFlag) {
      gl_lightmodel( pname, &fparam );
   }
}



/*
 * pname: GL_LIGHT_MODEL_AMBIENT red, green, blue in [1.0, -1.0]
 *        GL_LIGHT_MODEL_LOCAL_VIEWER mode where 0.0 = point, else parallel
 *        GL_LIGHT_MODEL_TWO_SIDE mode where 0.0 = one-sided, else 2-sided
 */
void glLightModelfv( GLenum pname, const GLfloat *params )
{
   if (CC.CompileFlag) {
      gl_save_lightmodel( pname, params );
   }
   if (CC.ExecuteFlag) {
      gl_lightmodel( pname, params );
   }
}



void glLightModeliv( GLenum pname, const GLint *params )
{
   GLfloat fparam[4];

   switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
         fparam[0] = (GLfloat) params[0];
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glLightModeliv" );
         return;
   }
   if (CC.CompileFlag) {
      gl_save_lightmodel( pname, fparam );
   }
   if (CC.ExecuteFlag) {
      gl_lightmodel( pname, fparam );
   }
}




/********** MATERIAL **********/



void gl_material( GLenum face, GLenum pname, const GLfloat *params )
{
   GLuint bitmask = 0, fmask = 0, bmask = 0;
   struct gl_material *mat;

   if (face!=GL_FRONT && face!=GL_BACK && face!=GL_FRONT_AND_BACK) {
      gl_error( GL_INVALID_ENUM, "glMaterial" );
      return;
   }

   if (face==GL_FRONT || face==GL_FRONT_AND_BACK) {
      fmask = 0xfff;
   }
   if (face==GL_BACK || face==GL_FRONT_AND_BACK) {
      bmask = 0xfff;
   }

   /* Make a bitmask indicating what material attribute(s) we're updating */
   switch (pname) {
      case GL_AMBIENT:
         bitmask |= (FRONT_AMBIENT_BIT & fmask) | (BACK_AMBIENT_BIT & bmask);
         break;
      case GL_DIFFUSE:
         bitmask |= (FRONT_DIFFUSE_BIT & fmask) | (BACK_DIFFUSE_BIT & bmask);
         break;
      case GL_SPECULAR:
         bitmask |= (FRONT_SPECULAR_BIT & fmask) | (BACK_SPECULAR_BIT & bmask);
         break;
      case GL_EMISSION:
         bitmask |= (FRONT_EMISSION_BIT & fmask) | (BACK_EMISSION_BIT & bmask);
         break;
      case GL_SHININESS:
         bitmask |= (FRONT_SHININESS_BIT & fmask) | (BACK_SHININESS_BIT & bmask);
         break;
      case GL_AMBIENT_AND_DIFFUSE:
         bitmask |= ((FRONT_AMBIENT_BIT | FRONT_DIFFUSE_BIT) & fmask)
                  | ((BACK_AMBIENT_BIT | BACK_DIFFUSE_BIT) & bmask);
         break;
      case GL_COLOR_INDEXES:
         bitmask |= (FRONT_INDEXES_BIT & fmask) | (BACK_INDEXES_BIT & bmask);
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glMaterial(pname)" );
         return;
   }

   if (INSIDE_BEGIN_END) {
      /* Save per-vertex material changes in the Vertex Buffer.
       * The update_material function will eventually update the global
       * CC.Light.Material values.
       */
      mat = VB.Material[VB.Count];
      VB.MaterialMask[VB.Count] |= bitmask;
      VB.MaterialChanges = GL_TRUE;
   }
   else {
      /* just update the global material property */
      mat = CC.Light.Material;
      CC.NewState = GL_TRUE;
   }

   if (bitmask & FRONT_AMBIENT_BIT) {
      COPY_4V( mat[0].Ambient, params );
   }
   if (bitmask & BACK_AMBIENT_BIT) {
      COPY_4V( mat[1].Ambient, params );
   }
   if (bitmask & FRONT_DIFFUSE_BIT) {
      COPY_4V( mat[0].Diffuse, params );
   }
   if (bitmask & BACK_DIFFUSE_BIT) {
      COPY_4V( mat[1].Diffuse, params );
   }
   if (bitmask & FRONT_SPECULAR_BIT) {
      COPY_4V( mat[0].Specular, params );
   }
   if (bitmask & BACK_SPECULAR_BIT) {
      COPY_4V( mat[1].Specular, params );
   }
   if (bitmask & FRONT_EMISSION_BIT) {
      COPY_4V( mat[0].Emission, params );
   }
   if (bitmask & BACK_EMISSION_BIT) {
      COPY_4V( mat[1].Emission, params );
   }
   if (bitmask & FRONT_SHININESS_BIT) {
      mat[0].Shininess = CLAMP( params[0], 0.0, 128.0 );
      gl_compute_material_shine_table( &mat[0] );
   }
   if (bitmask & BACK_SHININESS_BIT) {
      mat[1].Shininess = CLAMP( params[0], 0.0, 128.0 );
      gl_compute_material_shine_table( &mat[1] );
   }
   if (bitmask & FRONT_INDEXES_BIT) {
      mat[0].AmbientIndex = params[0];
      mat[0].DiffuseIndex = params[1];
      mat[0].SpecularIndex = params[2];
   }
   if (bitmask & BACK_INDEXES_BIT) {
      mat[1].AmbientIndex = params[0];
      mat[1].DiffuseIndex = params[1];
      mat[1].SpecularIndex = params[2];
   }
}



void glMaterialf( GLenum face, GLenum pname, GLfloat param )
{
   if (CC.CompileFlag) {
      gl_save_material( face, pname, &param );
   }
   if (CC.ExecuteFlag) {
      gl_material( face, pname, &param );
   }
}



void glMateriali( GLenum face, GLenum pname, GLint param )
{
   GLfloat fparam;

   fparam = (GLfloat) param;

   if (CC.CompileFlag) {
      gl_save_material( face, pname, &fparam );
   }
   if (CC.ExecuteFlag) {
      gl_material( face, pname, &fparam );
   }
}



void glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{
   if (CC.CompileFlag) {
      gl_save_material( face, pname, params );
   }
   if (CC.ExecuteFlag) {
      gl_material( face, pname, params );
   }
}



void glMaterialiv( GLenum face, GLenum pname, const GLint *params )
{
   GLfloat fparam[4];

   switch (pname) {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
      case GL_EMISSION:
      case GL_AMBIENT_AND_DIFFUSE:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_SHININESS:
         fparam[0] = (GLfloat) params[0];
         break;
      case GL_COLOR_INDEXES:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         break;
   }
   if (CC.CompileFlag) {
      gl_save_material( face, pname, fparam );
   }
   if (CC.ExecuteFlag) {
      gl_material( face, pname, fparam );
   }
}



void glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
   GLuint f;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetMaterialfv" );
      return;
   }
   if (face==GL_FRONT) {
      f = 0;
   }
   else if (face==GL_BACK) {
      f = 1;
   }
   else {
      gl_error( GL_INVALID_ENUM, "glGetMaterialfv(face)" );
      return;
   }
   switch (pname) {
      case GL_AMBIENT:
         COPY_4V( params, CC.Light.Material[f].Ambient );
         break;
      case GL_DIFFUSE:
         COPY_4V( params, CC.Light.Material[f].Diffuse );
	 break;
      case GL_SPECULAR:
         COPY_4V( params, CC.Light.Material[f].Specular );
	 break;
      case GL_EMISSION:
	 COPY_4V( params, CC.Light.Material[f].Emission );
	 break;
      case GL_SHININESS:
	 *params = CC.Light.Material[f].Shininess;
	 break;
      case GL_COLOR_INDEXES:
	 params[0] = CC.Light.Material[f].AmbientIndex;
	 params[1] = CC.Light.Material[f].DiffuseIndex;
	 params[2] = CC.Light.Material[f].SpecularIndex;
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetMaterialfv(pname)" );
   }
   CC.NewState = GL_TRUE;
}



void glGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
   GLuint f;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetMaterialiv" );
      return;
   }
   if (face==GL_FRONT) {
      f = 0;
   }
   else if (face==GL_BACK) {
      f = 1;
   }
   else {
      gl_error( GL_INVALID_ENUM, "glGetMaterialiv(face)" );
      return;
   }
   switch (pname) {
      case GL_AMBIENT:
         params[0] = FLOAT_TO_INT( CC.Light.Material[f].Ambient[0] );
         params[1] = FLOAT_TO_INT( CC.Light.Material[f].Ambient[1] );
         params[2] = FLOAT_TO_INT( CC.Light.Material[f].Ambient[2] );
         params[3] = FLOAT_TO_INT( CC.Light.Material[f].Ambient[3] );
         break;
      case GL_DIFFUSE:
         params[0] = FLOAT_TO_INT( CC.Light.Material[f].Diffuse[0] );
         params[1] = FLOAT_TO_INT( CC.Light.Material[f].Diffuse[1] );
         params[2] = FLOAT_TO_INT( CC.Light.Material[f].Diffuse[2] );
         params[3] = FLOAT_TO_INT( CC.Light.Material[f].Diffuse[3] );
	 break;
      case GL_SPECULAR:
         params[0] = FLOAT_TO_INT( CC.Light.Material[f].Specular[0] );
         params[1] = FLOAT_TO_INT( CC.Light.Material[f].Specular[1] );
         params[2] = FLOAT_TO_INT( CC.Light.Material[f].Specular[2] );
         params[3] = FLOAT_TO_INT( CC.Light.Material[f].Specular[3] );
	 break;
      case GL_EMISSION:
         params[0] = FLOAT_TO_INT( CC.Light.Material[f].Emission[0] );
         params[1] = FLOAT_TO_INT( CC.Light.Material[f].Emission[1] );
         params[2] = FLOAT_TO_INT( CC.Light.Material[f].Emission[2] );
         params[3] = FLOAT_TO_INT( CC.Light.Material[f].Emission[3] );
	 break;
      case GL_SHININESS:
         *params = ROUNDF( CC.Light.Material[f].Shininess );
	 break;
      case GL_COLOR_INDEXES:
	 params[0] = ROUNDF( CC.Light.Material[f].AmbientIndex );
	 params[1] = ROUNDF( CC.Light.Material[f].DiffuseIndex );
	 params[2] = ROUNDF( CC.Light.Material[f].SpecularIndex );
	 break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetMaterialfv(pname)" );
   }
}




/**********************************************************************/
/*****                  Lighting computation                      *****/
/**********************************************************************/


/*
 * Notes:
 *   When two-sided lighting is enabled we compute the color (or index)
 *   for both the front and back side of the primitive.  Then, when the
 *   orientation of the facet is later learned, we can determine which
 *   color (or index) to use for rendering.
 */



/*
 * Whenever the spotlight exponent for a light changes we must call
 * this function to recompute the exponent lookup table.
 */
void gl_compute_spot_exp_table( struct gl_light *l )
{
   int i;
   double exponent = l->SpotExponent;

   l->SpotExpTable[0][0] = 0.0;
   for (i=1;i<EXP_TABLE_SIZE;i++) {
      l->SpotExpTable[i][0] = pow(i/(double)(EXP_TABLE_SIZE-1), exponent);
   }
   for (i=0;i<EXP_TABLE_SIZE-1;i++) {
      l->SpotExpTable[i][1] = l->SpotExpTable[i+1][0] - l->SpotExpTable[i][0];
   }
   l->SpotExpTable[EXP_TABLE_SIZE-1][1] = 0.0;
}



/*
 * Whenever the shininess of a material changes we must call this
 * function to recompute the exponential lookup table.
 */
void gl_compute_material_shine_table( struct gl_material *m )
{
   int i;
   double exponent = m->Shininess;

   m->ShineTable[0] = 0.0;
   for (i=1;i<SHINE_TABLE_SIZE;i++) {
      double x = pow( i/(double)(SHINE_TABLE_SIZE-1), exponent );
      if (x<1.0e-10) {
         m->ShineTable[i] = 0.0;
      }
      else {
         m->ShineTable[i] = x;
      }
   }
}



/*
 * Examine current lighting parameters to determine if the optimized lighting
 * function can be used.  Also, precompute some lighting values which are
 * used by gl_color_shade_vertices_fast().
 */
void gl_update_lighting( void )
{
   GLint i;

   if (!CC.Light.Enabled) {
      /* If lighting is not enabled, we can skip all this. */
      return;
   }

   /* base color = material_emission + global_ambient */
   CC.Light.BaseColor[0] = CC.Light.Material[0].Emission[0]
             + CC.Light.Model.Ambient[0] * CC.Light.Material[0].Ambient[0];
   CC.Light.BaseColor[1] = CC.Light.Material[0].Emission[1]
             + CC.Light.Model.Ambient[1] * CC.Light.Material[0].Ambient[1];
   CC.Light.BaseColor[2] = CC.Light.Material[0].Emission[2]
             + CC.Light.Model.Ambient[2] * CC.Light.Material[0].Ambient[2];
   CC.Light.BaseColor[3] = MIN2( CC.Light.Material[0].Diffuse[3], 1.0F );

   /* Compute CC.Light.LastEnabled */
   CC.Light.LastEnabled = -1;
   for (i=0;i<MAX_LIGHTS;i++) {
      if (CC.Light.Light[i].Enabled) {
         CC.Light.LastEnabled = i;
      }
   }

   for (i=0;i<=CC.Light.LastEnabled;i++) {
      if (CC.Light.Light[i].Enabled) {
         /* Add each light's ambient component to base color */
         CC.Light.BaseColor[0] += CC.Light.Light[i].Ambient[0]
                                * CC.Light.Material[0].Ambient[0];
         CC.Light.BaseColor[1] += CC.Light.Light[i].Ambient[1]
                                * CC.Light.Material[0].Ambient[1];
         CC.Light.BaseColor[2] += CC.Light.Light[i].Ambient[2]
                                * CC.Light.Material[0].Ambient[2];
         /* diffuse[light] = diffuse[light] * material_diffuse */
         CC.Light.Diffuse[i][0] = CC.Light.Light[i].Diffuse[0]
                                * CC.Light.Material[0].Diffuse[0];
         CC.Light.Diffuse[i][1] = CC.Light.Light[i].Diffuse[1]
                                * CC.Light.Material[0].Diffuse[1];
         CC.Light.Diffuse[i][2] = CC.Light.Light[i].Diffuse[2]
                                * CC.Light.Material[0].Diffuse[2];
         /* specular[light] = specular[light] * material_specular */
         CC.Light.Specular[i][0] = CC.Light.Light[i].Specular[0]
                                 * CC.Light.Material[0].Specular[0];
         CC.Light.Specular[i][1] = CC.Light.Light[i].Specular[1]
                                 * CC.Light.Material[0].Specular[1];
         CC.Light.Specular[i][2] = CC.Light.Light[i].Specular[2]
                                 * CC.Light.Material[0].Specular[2];
      }
   }

   /* Compute normalized position and direction vectors for each light */
   for (i=0;i<=CC.Light.LastEnabled;i++) {
      COPY_3V( CC.Light.Light[i].NormPosition, CC.Light.Light[i].Position );
      NORMALIZE_3V( CC.Light.Light[i].NormPosition );
      COPY_3V( CC.Light.Light[i].NormDirection, CC.Light.Light[i].Direction );
      NORMALIZE_3V( CC.Light.Light[i].NormDirection );
   }

   /* Determine if the fast lighting function can be used */
   CC.Light.Fast = GL_TRUE;
   for (i=0;i<=CC.Light.LastEnabled;i++) {
      if (CC.Light.Light[i].Enabled) {
         if (   CC.Light.Light[i].Position[3]!=0.0F
             || CC.Light.Light[i].SpotCutoff!=180.0F
             || CC.Light.BaseColor[0]<0.0F
             || CC.Light.BaseColor[1]<0.0F
             || CC.Light.BaseColor[2]<0.0F
             || CC.Light.BaseColor[3]<0.0F
             || CC.Light.Diffuse[i][0]<0.0F
             || CC.Light.Diffuse[i][1]<0.0F
             || CC.Light.Diffuse[i][2]<0.0F
             || CC.Light.Specular[i][0]<0.0F
             || CC.Light.Specular[i][1]<0.0F
             || CC.Light.Specular[i][2]<0.0F) {
            CC.Light.Fast = GL_FALSE;
            break;
         }
      }
   }
   if (CC.Light.Model.TwoSide || CC.Light.Model.LocalViewer
       || CC.Light.ColorMaterialEnabled) {
      CC.Light.Fast = GL_FALSE;
   }
}





/*
 * Use current lighting/material settings to compute the RGBA colors of
 * an array of vertexes.
 * Input:  n - number of vertexes to process
 *         vertex - array of vertex positions in eye coordinates
 *         normal - array of surface normal vectors
 *         twoside - 0 = front face shading only, 1 = two-sided lighting
 * Output:  frontcolor - array of resulting front-face colors
 *          backcolor - array of resulting back-face colors
 */
void gl_color_shade_vertices( GLuint n,
                              GLfloat vertex[][4],
                              GLfloat normal[][3],
                              GLuint twoside,
                              GLfixed frontcolor[][4],
                              GLfixed backcolor[][4] )
{
   GLint side, i, j;
   GLfloat rscale, gscale, bscale, ascale;

   /* Compute scale factor to go from floats in [0,1] to integers or fixed
    * point values:
    */
   rscale = (GLfloat) ( (GLint) CC.RedScale   << CC.ColorShift );
   gscale = (GLfloat) ( (GLint) CC.GreenScale << CC.ColorShift );
   bscale = (GLfloat) ( (GLint) CC.BlueScale  << CC.ColorShift );
   ascale = (GLfloat) ( (GLint) CC.AlphaScale << CC.ColorShift );


   for (side=0;side<=twoside;side++) {
      GLfloat ambient[MAX_LIGHTS][3];
      GLfloat r0, g0, b0, a0;
      GLfixed A;

      /*** Compute color contribution from global lighting ***/
      r0 = CC.Light.Material[side].Emission[0]
         + CC.Light.Model.Ambient[0] * CC.Light.Material[side].Ambient[0];
      g0 = CC.Light.Material[side].Emission[1]
         + CC.Light.Model.Ambient[1] * CC.Light.Material[side].Ambient[1];
      b0 = CC.Light.Material[side].Emission[2]
         + CC.Light.Model.Ambient[2] * CC.Light.Material[side].Ambient[2];

      /* Alpha is simple, same for all vertices */
      a0 = CC.Light.Material[side].Diffuse[3];
      A = (GLfixed) (CLAMP( a0, 0.0F, 1.0F ) * ascale);

      /* Compute ambient color from each light.  Same for all vertices. */
      for (i=0;i<=CC.Light.LastEnabled;i++) {
         ambient[i][0] = CC.Light.Light[i].Ambient[0]
                       * CC.Light.Material[side].Ambient[0];
         ambient[i][1] = CC.Light.Light[i].Ambient[1]
                       * CC.Light.Material[side].Ambient[1];
         ambient[i][2] = CC.Light.Light[i].Ambient[2]
                       * CC.Light.Material[side].Ambient[2];
      }


      for (j=0;j<n;j++) {
         GLfloat R, G, B;
         GLfloat norm[3];

         if (side==0) {
            /* shade frontside */
            norm[0] = normal[j][0];
            norm[1] = normal[j][1];
            norm[2] = normal[j][2];
         }
         else {
            /* shade backside */
            norm[0] = -normal[j][0];
            norm[1] = -normal[j][1];
            norm[2] = -normal[j][2];
         }

         R = r0;
         G = g0;
         B = b0;

         /* Add contribution from each light source */
         for (i=0;i<=CC.Light.LastEnabled;i++) {

            if (CC.Light.Light[i].Enabled) {
               GLfloat attenuation;
               GLfloat l[3];  /* unit vector from vertex to light */
               GLfloat l_dot_norm;  /* dot product of l and norm */

               /* compute l and attenuation */
               if (CC.Light.Light[i].Position[3]==0.0) {
                  /* directional light */
                  /* Effectively, l is a vector from the origin to the light */
                  l[0] = CC.Light.Light[i].NormPosition[0];
                  l[1] = CC.Light.Light[i].NormPosition[1];
                  l[2] = CC.Light.Light[i].NormPosition[2];
                  attenuation = 1.0F;
               }
               else {
                  /* positional light */
                  GLfloat d;     /* distance from vertex to light */
                  l[0] = CC.Light.Light[i].Position[0] - vertex[j][0];
                  l[1] = CC.Light.Light[i].Position[1] - vertex[j][1];
                  l[2] = CC.Light.Light[i].Position[2] - vertex[j][2];
                  d = (GLfloat) sqrt( l[0]*l[0] + l[1]*l[1] + l[2]*l[2] );
                  if (d>0.001F) {
                     GLfloat invd = 1.0F / d;
                     l[0] *= invd;
                     l[1] *= invd;
                     l[2] *= invd;
                  }
                  attenuation = 1.0F / (CC.Light.Light[i].ConstantAttenuation
                              + d * (CC.Light.Light[i].LinearAttenuation
                              + d * CC.Light.Light[i].QuadraticAttenuation));
               }

               l_dot_norm = DOT3( l, norm );

               /* diffuse and specular terms */
               if (l_dot_norm<=0.0F) {
                  /* surface faces away from light, no diffuse or specular */
                  R += attenuation * ambient[i][0];
                  G += attenuation * ambient[i][1];
                  B += attenuation * ambient[i][2];
                  /* done with this light */
               }
               else {
                  GLfloat s[3], dot, t, spotlight_effect;
                  GLfloat diffuseR, diffuseG, diffuseB;
                  GLfloat specularR, specularG, specularB;
                  
                  /* spotlight factor */
                  if (CC.Light.Light[i].SpotCutoff==180.0F) {
                     /* not a spot light */
                     spotlight_effect = 1.0F;
                  }
                  else {
                     GLfloat v[3], dot;
                     v[0] = -l[0];  /* v points from light to vertex */
                     v[1] = -l[1];
                     v[2] = -l[2];
                     dot = DOT3( v, CC.Light.Light[i].NormDirection );
                     if (dot<=0.0F || dot<CC.Light.Light[i].CosCutoff) {
                        /* outside of cone */
                        spotlight_effect = 0.0F;
                     }
                     else {
                        double x = dot * (EXP_TABLE_SIZE-1);
                        int k = (int) x;
                        spotlight_effect = CC.Light.Light[i].SpotExpTable[k][0]
                           + (x-k)*CC.Light.Light[i].SpotExpTable[k][1];
                     }
                  }

                  /* diffuse term */
                  diffuseR = l_dot_norm * CC.Light.Light[i].Diffuse[0]
                               * CC.Light.Material[side].Diffuse[0];
                  diffuseG = l_dot_norm * CC.Light.Light[i].Diffuse[1]
                               * CC.Light.Material[side].Diffuse[1];
                  diffuseB = l_dot_norm * CC.Light.Light[i].Diffuse[2]
                               * CC.Light.Material[side].Diffuse[2];

                  /* specular term */
                  if (CC.Light.Model.LocalViewer) {
                     GLfloat v[3];
                     v[0] = vertex[j][0];
                     v[1] = vertex[j][1];
                     v[2] = vertex[j][2];
                     NORMALIZE_3V( v );
                     s[0] = l[0] - v[0];
                     s[1] = l[1] - v[1];
                     s[2] = l[2] - v[2];
                  }
                  else {
                     s[0] = l[0];
                     s[1] = l[1];
                     s[2] = l[2] + 1.0F;
                  }
                  /* attention: s is not normalized, done later if needed */
                  dot = DOT3(s,norm);

                  if (dot<=0.0) {
                     specularR = 0.0F;
                     specularG = 0.0F;
                     specularB = 0.0F;
                  }
                  else {
                     GLfloat spec_coef;
                     /* now `correct' the dot product */
                     dot = dot / sqrt(s[0]*s[0]+s[1]*s[1]+s[2]*s[2]);
                     if (dot>1.0F) {
                        /* only happens if normal vector length > 1.0 */
                        spec_coef = pow( dot,
                                         CC.Light.Material[side].Shininess );
                     }
                     else {
                        /* use table lookup approximation */
                        int k = (int) (dot * (GLfloat) (SHINE_TABLE_SIZE-1));
                        spec_coef = CC.Light.Material[side].ShineTable[k];
                     }
                     if (spec_coef<1.0e-10) {
                        specularR = 0.0F;
                        specularG = 0.0F;
                        specularB = 0.0F;
                     }
                     else {
                        specularR = spec_coef * CC.Light.Light[i].Specular[0]
                                    * CC.Light.Material[side].Specular[0];
                        specularG = spec_coef * CC.Light.Light[i].Specular[1]
                                    * CC.Light.Material[side].Specular[1];
                        specularB = spec_coef * CC.Light.Light[i].Specular[2]
                                    * CC.Light.Material[side].Specular[2];
                     }
                  }
                  t = attenuation * spotlight_effect;
                  R += t * (ambient[i][0] + diffuseR + specularR);
                  G += t * (ambient[i][1] + diffuseG + specularG);
                  B += t * (ambient[i][2] + diffuseB + specularB);
               }

            } /*if*/

         } /*for loop over lights*/

         if (side==0) {
            /* clamp and convert to integer or fixed point */
            frontcolor[j][0] = (GLfixed) (CLAMP( R, 0.0F, 1.0F ) * rscale);
            frontcolor[j][1] = (GLfixed) (CLAMP( G, 0.0F, 1.0F ) * gscale);
            frontcolor[j][2] = (GLfixed) (CLAMP( B, 0.0F, 1.0F ) * bscale);
            frontcolor[j][3] = A;
         }
         else {
            /* clamp and convert to integer or fixed point */
            backcolor[j][0] = (GLfixed) (CLAMP( R, 0.0F, 1.0F ) * rscale);
            backcolor[j][1] = (GLfixed) (CLAMP( G, 0.0F, 1.0F ) * gscale);
            backcolor[j][2] = (GLfixed) (CLAMP( B, 0.0F, 1.0F ) * bscale);
            backcolor[j][3] = A;
         }
      } /*loop over vertices*/

   } /*for side*/
}



/*
 * This is an optimized version of the above function.
 */
void gl_color_shade_vertices_fast( GLuint n,
                                   GLfloat vertex[][4],
                                   GLfloat normal[][3],
                                   GLuint twoside,
                                   GLfixed frontcolor[][4],
                                   GLfixed backcolor[][4] )
{
   GLint i, j;
   GLfloat rscale, gscale, bscale, ascale;
   GLint ishininess = CC.Light.Material[0].Shininess;
   GLdouble dshininess = CC.Light.Material[0].Shininess;
   GLfixed A;

   /* Compute scale factor to go from floats in [0,1] to integers or fixed
    * point values:
    */
   rscale = (GLfloat) ( (GLint) CC.RedScale   << CC.ColorShift );
   gscale = (GLfloat) ( (GLint) CC.GreenScale << CC.ColorShift );
   bscale = (GLfloat) ( (GLint) CC.BlueScale  << CC.ColorShift );
   ascale = (GLfloat) ( (GLint) CC.AlphaScale << CC.ColorShift );

   /* Alpha is easy to compute, same for all vertices */
   A = (GLfixed) ( CC.Light.BaseColor[3] * ascale);

   /* Loop over vertices */
   for (j=0;j<n;j++) {
      GLfloat R, G, B;
      GLfloat nx, ny, nz;

      /* the normal vector */
      nx = normal[j][0];
      ny = normal[j][1];
      nz = normal[j][2];

      /* base color from global illumination and enabled light's ambient */
      R = CC.Light.BaseColor[0];
      G = CC.Light.BaseColor[1];
      B = CC.Light.BaseColor[2];

      /* Add contribution from each light source */
      for (i=CC.Light.LastEnabled;i>=0;i--) {
         if (CC.Light.Light[i].Enabled) {
            GLfloat lx, ly, lz;  /* unit vector from vertex to light */
            GLfloat l_dot_norm;  /* dot product of l and norm */

            lx = CC.Light.Light[i].NormPosition[0];
            ly = CC.Light.Light[i].NormPosition[1];
            lz = CC.Light.Light[i].NormPosition[2];

            l_dot_norm = lx*nx + ly*ny + lz*nz;

            /* diffuse and specular terms */
            if (l_dot_norm>0.0F) {
               GLfloat dot;

               /* add diffuse term */
               R += l_dot_norm * CC.Light.Diffuse[i][0];
               G += l_dot_norm * CC.Light.Diffuse[i][1];
               B += l_dot_norm * CC.Light.Diffuse[i][2];

               /* dot product of n and s, s = l + <0,0,1> */
               dot = nx*lx + ny*ly + nz*(lz+1.0F);

               /* specular term */
               if (dot>0.0F) {
                  /* now `correct' the dot product */
                  dot = dot / sqrt(lx*lx+ly*ly+(lz+1.0F)*(lz+1.0F));
                  if (dot>1.0F) {
                     /* only happens if normal vector length > 1.0 */
                     GLfloat spec_coef = pow( dot,
                                              CC.Light.Material[0].Shininess );
                     if (spec_coef>1.0e-10F) {
                        R += spec_coef * CC.Light.Specular[i][0];
                        G += spec_coef * CC.Light.Specular[i][1];
                        B += spec_coef * CC.Light.Specular[i][2];
                     }
                  }
                  else {
                     /* use table lookup approximation */
                     int k = (int) (dot * (GLfloat) (SHINE_TABLE_SIZE-1));
                     GLfloat spec_coef = CC.Light.Material[0].ShineTable[k];
                     R += spec_coef * CC.Light.Specular[i][0];
                     G += spec_coef * CC.Light.Specular[i][1];
                     B += spec_coef * CC.Light.Specular[i][2];
                  }
               }
            }

         } /*if*/

      } /*for loop over lights*/

      /* clamp and convert to integer or fixed point */
      frontcolor[j][0] = (GLfixed) (MIN2( R, 1.0F ) * rscale);
      frontcolor[j][1] = (GLfixed) (MIN2( G, 1.0F ) * gscale);
      frontcolor[j][2] = (GLfixed) (MIN2( B, 1.0F ) * bscale);
      frontcolor[j][3] = A;

   } /*loop over vertices*/
}



/*
 * Use current lighting/material settings to compute the color indexes
 * for an array of vertices.
 * Input:  n - number of vertices to shade
 *         vertex - array of [n] vertex position in viewing coordinates
 *         normal - array of [n] surface normal vector
 *         twoside - 0 = front face shading only, 1 = two-sided lighting
 * Output:  frontindex - resulting array of [n] front-face color indexes
 *          backindex - resulting array of [n] back-face color indexes
 */
void gl_index_shade_vertices( GLuint n,
                              GLfloat vertex[][4],
                              GLfloat normal[][3],
                              GLuint twoside,
                              GLuint frontindex[],
                              GLuint backindex[] )
{
   GLint side, i, j;

   for (side=0;side<=twoside;side++) {
      GLint ishininess = CC.Light.Material[side].Shininess;
      GLdouble dshininess = CC.Light.Material[side].Shininess;

      /* loop over vertices */
      for (j=0;j<n;j++) {
         GLfloat d_ci, s_ci;
         GLfloat d_a, s_a, index;
         GLfloat diffuse, specular;  /* accumulated diffuse and specular terms */
         GLfloat norm[3];

         if (side==0) {
            /* shade frontside */
            norm[0] = normal[j][0];
            norm[1] = normal[j][1];
            norm[2] = normal[j][2];
         }
         else {
            /* shade backside */
            norm[0] = -normal[j][0];
            norm[1] = -normal[j][1];
            norm[2] = -normal[j][2];
         }

         diffuse = specular = 0.0;

         /* Accumulate diffuse and specular from each light source */
         for (i=0;i<=CC.Light.LastEnabled;i++) {

            if (CC.Light.Light[i].Enabled) {
               GLfloat attenuation;
               GLfloat spotlight_effect;
               GLfloat l[3];  /* unit vector from vertex to light */
               GLfloat l_dot_norm;  /* dot product of l and norm */

               /* compute l and  attenuation */
               if (CC.Light.Light[i].Position[3]==0.0) {
                  /* directional light */
                  /* Effectively, l is a vector from the origin to the light. */
                  l[0] = CC.Light.Light[i].NormPosition[0];
                  l[1] = CC.Light.Light[i].NormPosition[1];
                  l[2] = CC.Light.Light[i].NormPosition[2];
                  attenuation = 1.0F;
               }
               else {
                  /* positional light */
                  GLfloat d;     /* distance from vertex to light */
                  l[0] = CC.Light.Light[i].Position[0] - vertex[j][0];
                  l[1] = CC.Light.Light[i].Position[1] - vertex[j][1];
                  l[2] = CC.Light.Light[i].Position[2] - vertex[j][2];
                  d = (GLfloat) sqrt( l[0]*l[0] + l[1]*l[1] + l[2]*l[2] );
                  if (d>0.001) {
                     GLfloat invd = 1.0F / d;
                     l[0] *= invd;
                     l[1] *= invd;
                     l[2] *= invd;
                  }
                  attenuation = 1.0F / (CC.Light.Light[i].ConstantAttenuation
                                 + d * (CC.Light.Light[i].LinearAttenuation
                                 + d * CC.Light.Light[i].QuadraticAttenuation));
               }

               l_dot_norm = DOT3( l, norm );

               if (l_dot_norm>0.0F) {

                  /* spotlight factor */
                  if (CC.Light.Light[i].SpotCutoff==180.0F) {
                     /* not a spot light */
                     spotlight_effect = 1.0F;
                  }
                  else {
                     GLfloat v[3], dot;
                     v[0] = -l[0];  /* v points from light to vertex */
                     v[1] = -l[1];
                     v[2] = -l[2];
                     dot = DOT3( v, CC.Light.Light[i].NormDirection );
                     if (dot<=0.0F || dot<CC.Light.Light[i].CosCutoff) {
                        /* outside of cone */
                        spotlight_effect = 0.0F;
                     }
                     else {
                        double x = dot * (EXP_TABLE_SIZE-1);
                        int k = (int) x;
                        spotlight_effect = CC.Light.Light[i].SpotExpTable[k][0]
                                  + (x-k)*CC.Light.Light[i].SpotExpTable[k][1];
                     }
                  }

                  /* accumulate diffuse term */
                  d_ci = 0.30F * CC.Light.Light[i].Diffuse[0]
                       + 0.59F * CC.Light.Light[i].Diffuse[1]
                       + 0.11F * CC.Light.Light[i].Diffuse[2];
                  diffuse += l_dot_norm * d_ci * spotlight_effect * attenuation;

                  /* accumulate specular term */
                  {
                     GLfloat s[3], dot, spec_coef;

                     /* specular term */
                     if (CC.Light.Model.LocalViewer) {
                        GLfloat v[3];
                        v[0] = vertex[j][0];
                        v[1] = vertex[j][1];
                        v[2] = vertex[j][2];
                        NORMALIZE_3V( v );
                        s[0] = l[0] - v[0];
                        s[1] = l[1] - v[1];
                        s[2] = l[2] - v[2];
                     }
                     else {
                        s[0] = l[0];
                        s[1] = l[1];
                        s[2] = l[2] + 1.0F;
                     }
                     /* attention: s is not normalized, done later if necessary */
                     dot = DOT3(s,norm);

                     if (dot<=0.0F) {
                        spec_coef = 0.0;
                     }
                     else {
                        /* now `correct' the dot product */
                        dot = dot / sqrt(s[0]*s[0]+s[1]*s[1]+s[2]*s[2]);
                        if (dot>1.0F) {
                           spec_coef = pow( dot,
                                           CC.Light.Material[side].Shininess );
                        }
                        else {
                           int k = (int) (dot * (GLfloat)(SHINE_TABLE_SIZE-1));
                           GLfloat spec_coef = CC.Light.Material[side].ShineTable[k];
                        }
                     }
                     s_ci = 0.30F * CC.Light.Light[i].Specular[0]
                          + 0.59F * CC.Light.Light[i].Specular[1]
                          + 0.11F * CC.Light.Light[i].Specular[2];
                     specular += spec_coef * s_ci * spotlight_effect * attenuation;
                  }
               }

            } /* if */

         } /* for */

         /* Now compute color index */
         if (specular>1.0F)
            specular = 1.0F;

         d_a = CC.Light.Material[side].DiffuseIndex
             - CC.Light.Material[side].AmbientIndex;
         s_a = CC.Light.Material[side].SpecularIndex
             - CC.Light.Material[side].AmbientIndex;

         index = CC.Light.Material[side].AmbientIndex
               + diffuse * (1.0-specular) * d_a
               + specular * s_a;

         if (index>CC.Light.Material[side].SpecularIndex) {
            index = CC.Light.Material[side].SpecularIndex;
         }

         if (side==0) {
            frontindex[j] = (GLuint) (GLint) index;
         }
         else {
            backindex[j] = (GLuint) (GLint) index;
         }

      } /*for vertex*/

   } /*for side*/
}
