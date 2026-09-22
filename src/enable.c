/* $Id: enable.c,v 1.28 1996/04/25 20:40:48 brianp Exp $ */

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
$Log: enable.c,v $
 * Revision 1.28  1996/04/25  20:40:48  brianp
 * call DD.alloc_depth_buffer() instead of gl_alloc_depth_buffer()
 *
 * Revision 1.27  1996/04/23  22:11:35  brianp
 * set CC.VertexFunc to gl_nop_vertex when lighting or texturing en/disabled
 *
 * Revision 1.26  1996/02/26  15:07:16  brianp
 * replaced CC.Current.Color with CC.Current.IntColor
 *
 * Revision 1.25  1996/02/15  16:11:24  brianp
 * don't compile vertex array enable/disables into display lists
 *
 * Revision 1.24  1996/02/15  16:01:49  brianp
 * added vertex array extension support
 *
 * Revision 1.23  1996/02/13  17:47:19  brianp
 * removed code to compute CC.Light.LastEnabled
 *
 * Revision 1.22  1996/01/02  22:11:53  brianp
 * signal CC.NewState when enable/disabling dithering
 *
 * Revision 1.21  1995/12/20  17:29:12  brianp
 * replaced call to gl_color with call to gl_material
 *
 * Revision 1.20  1995/11/01  21:45:45  brianp
 * update CC.Light.LastEnabled when enabling/disabling lights
 *
 * Revision 1.19  1995/10/27  20:29:02  brianp
 * added glPolygonOffsetEXT() support
 *
 * Revision 1.18  1995/10/19  15:47:26  brianp
 * only set NewState flag when really needed
 *
 * Revision 1.17  1995/10/14  16:30:49  brianp
 * added DD.dither call
 *
 * Revision 1.16  1995/10/04  22:02:38  brianp
 * set CC.NewState when GL_LIGHTING is enabled/disabled
 *
 * Revision 1.15  1995/09/22  21:49:52  brianp
 * don't allow glEnable(GL_TEXTURE_xD) in CI mode
 *
 * Revision 1.14  1995/09/15  18:40:25  brianp
 * only signal CC.NewState when necessary
 *
 * Revision 1.13  1995/08/31  21:29:53  brianp
 * new TexGenEnabled bitfield
 * introduced CC.NewState convention
 *
 * Revision 1.12  1995/06/21  15:10:47  brianp
 * allocate stencil buffer when stencil test is enabled
 *
 * Revision 1.11  1995/06/20  22:11:18  brianp
 * fixed depth buffer allocation bug in glEnable(), per Asif Khan
 *
 * Revision 1.10  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.9  1995/05/12  19:26:43  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.8  1995/03/27  20:31:26  brianp
 * new Texture.Enabled scheme
 *
 * Revision 1.7  1995/03/24  20:03:21  brianp
 * added gl_update_pixel_logic call to GL_BLEND case
 *
 * Revision 1.6  1995/03/24  17:00:22  brianp
 * added gl_update_pixel_logic
 *
 * Revision 1.5  1995/03/09  21:41:39  brianp
 * call gl_udpate_rasterflags for GL_CULL_FACE
 *
 * Revision 1.4  1995/03/09  19:07:39  brianp
 * removed gl_disable, fixed color_material bug
 *
 * Revision 1.3  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.2  1995/02/24  15:25:46  brianp
 * added error checks to gl_enable and gl_disable
 *
 * Revision 1.1  1995/02/24  14:21:54  brianp
 * Initial revision
 *
 */


#include <string.h>
#include "context.h"
#include "dd.h"
#include "depth.h"
#include "draw.h"
#include "light.h"
#include "list.h"
#include "macros.h"
#include "stencil.h"




/*
 * Perform glEnable and glDisable calls.
 */
void gl_enable( GLenum cap, GLboolean state )
{
   GLuint i, p;

   if (INSIDE_BEGIN_END) {
      if (state) {
	 gl_error( GL_INVALID_OPERATION, "glEnable" );
      }
      else {
	 gl_error( GL_INVALID_OPERATION, "glDisable" );
      }
      return;
   }

   switch (cap) {
      case GL_ALPHA_TEST:
         if (CC.Color.AlphaEnabled!=state) {
            CC.Color.AlphaEnabled = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_AUTO_NORMAL:
	 CC.Eval.AutoNormal = state;
	 break;
      case GL_BLEND:
         if (CC.Color.BlendEnabled!=state) {
            CC.Color.BlendEnabled = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:
	 CC.Transform.ClipEnabled[cap-GL_CLIP_PLANE0] = state;
	 /* Check if any clip planes enabled */
         CC.Transform.AnyClip = GL_FALSE;
         for (p=0;p<MAX_CLIP_PLANES;p++) {
            if (CC.Transform.ClipEnabled[p]) {
               CC.Transform.AnyClip = GL_TRUE;
               break;
            }
         }
	 break;
      case GL_COLOR_MATERIAL:
         if (CC.Light.ColorMaterialEnabled!=state) {
            CC.Light.ColorMaterialEnabled = state;
            if (state) {
               GLfloat color[4];
               color[0] = CC.Current.IntColor[0] / CC.RedScale;
               color[1] = CC.Current.IntColor[1] / CC.GreenScale;
               color[2] = CC.Current.IntColor[2] / CC.BlueScale;
               color[3] = CC.Current.IntColor[3] / CC.AlphaScale;
               /* update material with current color */
               gl_material( CC.Light.ColorMaterialFace,
                            CC.Light.ColorMaterialMode, color );
            }
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_CULL_FACE:
         if (CC.Polygon.CullFlag!=state) {
            CC.Polygon.CullFlag = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_DEPTH_TEST:
	 if (CC.Depth.Test!=state) {
            CC.Depth.Test = state;
            CC.NewState = GL_TRUE;
         }
	 if (state && !CC.DepthBuffer) {
	    /* need to allocate a depth buffer now */
	    (*DD.alloc_depth_buffer)();
	 }
         break;
      case GL_DITHER:
	 CC.Color.DitherFlag = state;
         if (DD.dither) {
            (*DD.dither)( state );
         }
         CC.NewState = GL_TRUE;
	 break;
      case GL_FOG:
	 if (CC.Fog.Enabled!=state) {
            CC.Fog.Enabled = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
         CC.Light.Light[cap-GL_LIGHT0].Enabled = state;
         CC.NewState = GL_TRUE;
         break;
      case GL_LIGHTING:
         if (CC.Light.Enabled!=state) {
            CC.Light.Enabled = state;
            CC.NewState = GL_TRUE;
            CC.VertexFunc = gl_nop_vertex;
         }
         break;
      case GL_LINE_SMOOTH:
	 if (CC.Line.SmoothFlag!=state) {
            CC.Line.SmoothFlag = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_LINE_STIPPLE:
	 if (CC.Line.StippleFlag!=state) {
            CC.Line.StippleFlag = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_LOGIC_OP:
	 if (CC.Color.LogicOpEnabled!=state) {
            CC.NewState = GL_TRUE;
         }
	 CC.Color.LogicOpEnabled = state;
	 break;
      case GL_MAP1_COLOR_4:
	 CC.Eval.Map1Color4 = state;
	 break;
      case GL_MAP1_INDEX:
	 CC.Eval.Map1Index = state;
	 break;
      case GL_MAP1_NORMAL:
	 CC.Eval.Map1Normal = state;
	 break;
      case GL_MAP1_TEXTURE_COORD_1:
	 CC.Eval.Map1TextureCoord1 = state;
	 break;
      case GL_MAP1_TEXTURE_COORD_2:
	 CC.Eval.Map1TextureCoord2 = state;
	 break;
      case GL_MAP1_TEXTURE_COORD_3:
	 CC.Eval.Map1TextureCoord3 = state;
	 break;
      case GL_MAP1_TEXTURE_COORD_4:
	 CC.Eval.Map1TextureCoord4 = state;
	 break;
      case GL_MAP1_VERTEX_3:
	 CC.Eval.Map1Vertex3 = state;
	 break;
      case GL_MAP1_VERTEX_4:
	 CC.Eval.Map1Vertex4 = state;
	 break;
      case GL_MAP2_COLOR_4:
	 CC.Eval.Map2Color4 = state;
	 break;
      case GL_MAP2_INDEX:
	 CC.Eval.Map2Index = state;
	 break;
      case GL_MAP2_NORMAL:
	 CC.Eval.Map2Normal = state;
	 break;
      case GL_MAP2_TEXTURE_COORD_1: 
	 CC.Eval.Map2TextureCoord1 = state;
	 break;
      case GL_MAP2_TEXTURE_COORD_2:
	 CC.Eval.Map2TextureCoord2 = state;
	 break;
      case GL_MAP2_TEXTURE_COORD_3:
	 CC.Eval.Map2TextureCoord3 = state;
	 break;
      case GL_MAP2_TEXTURE_COORD_4:
	 CC.Eval.Map2TextureCoord4 = state;
	 break;
      case GL_MAP2_VERTEX_3:
	 CC.Eval.Map2Vertex3 = state;
	 break;
      case GL_MAP2_VERTEX_4:
	 CC.Eval.Map2Vertex4 = state;
	 break;
      case GL_NORMALIZE:
	 CC.Transform.Normalize = state;
	 break;
      case GL_POINT_SMOOTH:
	 if (CC.Point.SmoothFlag!=state) {
            CC.Point.SmoothFlag = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_POLYGON_SMOOTH:
	 if (CC.Polygon.SmoothFlag!=state) {
            CC.Polygon.SmoothFlag = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_POLYGON_STIPPLE:
	 if (CC.Polygon.StippleFlag!=state) {
            CC.Polygon.StippleFlag = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_POLYGON_OFFSET_EXT:
         if (CC.Polygon.OffsetEnabled!=state) {
            CC.Polygon.OffsetEnabled = state;
            CC.NewState = GL_TRUE;
         }
         break;
      case GL_SCISSOR_TEST:
         if (CC.Scissor.Enabled!=state) {
            CC.Scissor.Enabled = state;
            CC.NewState = GL_TRUE;
         }
	 break;
      case GL_STENCIL_TEST:
	 if (CC.Stencil.Enabled!=state) {
            CC.Stencil.Enabled = state;
            CC.NewState = GL_TRUE;
         }
	 if (state && !CC.StencilBuffer) {
	    /* need to allocate a stencil buffer now */
	    gl_alloc_stencil_buffer();
	 }
	 break;
      case GL_TEXTURE_1D:
         if (CC.RGBAflag) {
            /* texturing only works in RGB mode */
            if (state) {
               CC.Texture.Enabled |= 1;
            }
            else {
               CC.Texture.Enabled &= 2;
            }
            CC.NewState = GL_TRUE;
            CC.VertexFunc = gl_nop_vertex;
         }
	 break;
      case GL_TEXTURE_2D:
         if (CC.RGBAflag) {
            /* texturing only works in RGB mode */
            if (state) {
               CC.Texture.Enabled |= 2;
            }
            else {
               CC.Texture.Enabled &= 1;
            }
            CC.NewState = GL_TRUE;
            CC.VertexFunc = gl_nop_vertex;
         }
	 break;
      case GL_TEXTURE_GEN_Q:
         if (state) {
            CC.Texture.TexGenEnabled |= Q_BIT;
         }
         else {
            CC.Texture.TexGenEnabled &= ~Q_BIT;
         }
         CC.NewState = GL_TRUE;
	 break;
      case GL_TEXTURE_GEN_R:
         if (state) {
            CC.Texture.TexGenEnabled |= R_BIT;
         }
         else {
            CC.Texture.TexGenEnabled &= ~R_BIT;
         }
         CC.NewState = GL_TRUE;
	 break;
      case GL_TEXTURE_GEN_S:
	 if (state) {
            CC.Texture.TexGenEnabled |= S_BIT;
         }
         else {
            CC.Texture.TexGenEnabled &= ~S_BIT;
         }
         CC.NewState = GL_TRUE;
	 break;
      case GL_TEXTURE_GEN_T:
         if (state) {
            CC.Texture.TexGenEnabled |= T_BIT;
         }
         else {
            CC.Texture.TexGenEnabled &= ~T_BIT;
         }
         CC.NewState = GL_TRUE;
	 break;
      case GL_VERTEX_ARRAY_EXT:
         CC.Array.VertexEnabled = state;
         break;
      case GL_NORMAL_ARRAY_EXT:
         CC.Array.NormalEnabled = state;
         break;
      case GL_COLOR_ARRAY_EXT:
         CC.Array.ColorEnabled = state;
         break;
      case GL_INDEX_ARRAY_EXT:
         CC.Array.IndexEnabled = state;
         break;
      case GL_TEXTURE_COORD_ARRAY_EXT:
         CC.Array.TexCoordEnabled = state;
         break;
      case GL_EDGE_FLAG_ARRAY_EXT:
         CC.Array.EdgeFlagEnabled = state;
         break;
      default:
	 if (state) {
	    gl_error( GL_INVALID_ENUM, "glEnable" );
	 }
	 else {
	    gl_error( GL_INVALID_ENUM, "glDisable" );
	 }
         break;
   }
}




void glEnable( GLenum cap )
{
   if (CC.CompileFlag) {
      switch (cap) {
         case GL_VERTEX_ARRAY_EXT:
         case GL_NORMAL_ARRAY_EXT:
         case GL_COLOR_ARRAY_EXT:
         case GL_INDEX_ARRAY_EXT:
         case GL_TEXTURE_COORD_ARRAY_EXT:
         case GL_EDGE_FLAG_ARRAY_EXT:
            /* A special case: these don't get put into display lists */
            break;
         default:
            gl_save_enable( cap );
      }
   }
   if (CC.ExecuteFlag) {
      gl_enable( cap, GL_TRUE );
   }
}



void glDisable( GLenum cap )
{
   if (CC.CompileFlag) {
      switch (cap) {
         case GL_VERTEX_ARRAY_EXT:
         case GL_NORMAL_ARRAY_EXT:
         case GL_COLOR_ARRAY_EXT:
         case GL_INDEX_ARRAY_EXT:
         case GL_TEXTURE_COORD_ARRAY_EXT:
         case GL_EDGE_FLAG_ARRAY_EXT:
            /* A special case: these don't get put into display lists */
            break;
         default:
            gl_save_disable( cap );
      }
   }
   if (CC.ExecuteFlag) {
      gl_enable( cap, GL_FALSE );
   }
}



GLboolean glIsEnabled( GLenum cap )
{
   switch (cap) {
      case GL_ALPHA_TEST:
         return CC.Color.AlphaEnabled;
      case GL_AUTO_NORMAL:
	 return CC.Eval.AutoNormal;
      case GL_BLEND:
         return CC.Color.BlendEnabled;
      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:
	 return CC.Transform.ClipEnabled[cap-GL_CLIP_PLANE0];
      case GL_COLOR_MATERIAL:
	 return CC.Light.ColorMaterialEnabled;
      case GL_CULL_FACE:
         return CC.Polygon.CullFlag;
      case GL_DEPTH_TEST:
         return CC.Depth.Test;
      case GL_DITHER:
	 return CC.Color.DitherFlag;
      case GL_FOG:
	 return CC.Fog.Enabled;
      case GL_LIGHTING:
         return CC.Light.Enabled;
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
         return CC.Light.Light[cap-GL_LIGHT0].Enabled;
      case GL_LINE_SMOOTH:
	 return CC.Line.SmoothFlag;
      case GL_LINE_STIPPLE:
	 return CC.Line.StippleFlag;
      case GL_LOGIC_OP:
	 return CC.Color.LogicOpEnabled;
      case GL_MAP1_COLOR_4:
	 return CC.Eval.Map1Color4;
      case GL_MAP1_INDEX:
	 return CC.Eval.Map1Index;
      case GL_MAP1_NORMAL:
	 return CC.Eval.Map1Normal;
      case GL_MAP1_TEXTURE_COORD_1:
	 return CC.Eval.Map1TextureCoord1;
      case GL_MAP1_TEXTURE_COORD_2:
	 return CC.Eval.Map1TextureCoord2;
      case GL_MAP1_TEXTURE_COORD_3:
	 return CC.Eval.Map1TextureCoord3;
      case GL_MAP1_TEXTURE_COORD_4:
	 return CC.Eval.Map1TextureCoord4;
      case GL_MAP1_VERTEX_3:
	 return CC.Eval.Map1Vertex3;
      case GL_MAP1_VERTEX_4:
	 return CC.Eval.Map1Vertex4;
      case GL_MAP2_COLOR_4:
	 return CC.Eval.Map2Color4;
      case GL_MAP2_INDEX:
	 return CC.Eval.Map2Index;
      case GL_MAP2_NORMAL:
	 return CC.Eval.Map2Normal;
      case GL_MAP2_TEXTURE_COORD_1: 
	 return CC.Eval.Map2TextureCoord1;
      case GL_MAP2_TEXTURE_COORD_2:
	 return CC.Eval.Map2TextureCoord2;
      case GL_MAP2_TEXTURE_COORD_3:
	 return CC.Eval.Map2TextureCoord3;
      case GL_MAP2_TEXTURE_COORD_4:
	 return CC.Eval.Map2TextureCoord4;
      case GL_MAP2_VERTEX_3:
	 return CC.Eval.Map2Vertex3;
      case GL_MAP2_VERTEX_4:
	 return CC.Eval.Map2Vertex4;
      case GL_NORMALIZE:
	 return CC.Transform.Normalize;
      case GL_POINT_SMOOTH:
	 return CC.Point.SmoothFlag;
      case GL_POLYGON_SMOOTH:
	 return CC.Polygon.SmoothFlag;
      case GL_POLYGON_STIPPLE:
	 return CC.Polygon.StippleFlag;
      case GL_POLYGON_OFFSET_EXT:
	 return CC.Polygon.OffsetEnabled;
      case GL_SCISSOR_TEST:
	 return CC.Scissor.Enabled;
      case GL_STENCIL_TEST:
	 return CC.Stencil.Enabled;
      case GL_TEXTURE_1D:
	 return (CC.Texture.Enabled & 1) ? GL_TRUE : GL_FALSE;
      case GL_TEXTURE_2D:
	 return (CC.Texture.Enabled & 2) ? GL_TRUE : GL_FALSE;
      case GL_TEXTURE_GEN_Q:
	 return (CC.Texture.TexGenEnabled & Q_BIT) ? GL_TRUE : GL_FALSE;
      case GL_TEXTURE_GEN_R:
	 return (CC.Texture.TexGenEnabled & R_BIT) ? GL_TRUE : GL_FALSE;
      case GL_TEXTURE_GEN_S:
	 return (CC.Texture.TexGenEnabled & S_BIT) ? GL_TRUE : GL_FALSE;
      case GL_TEXTURE_GEN_T:
	 return (CC.Texture.TexGenEnabled & T_BIT) ? GL_TRUE : GL_FALSE;
      case GL_VERTEX_ARRAY_EXT:
         return CC.Array.VertexEnabled;
      case GL_NORMAL_ARRAY_EXT:
         return CC.Array.NormalEnabled;
      case GL_COLOR_ARRAY_EXT:
         return CC.Array.ColorEnabled;
      case GL_INDEX_ARRAY_EXT:
         return CC.Array.IndexEnabled;
      case GL_TEXTURE_COORD_ARRAY_EXT:
         return CC.Array.TexCoordEnabled;
      case GL_EDGE_FLAG_ARRAY_EXT:
         return CC.Array.EdgeFlagEnabled;
      default:
	 gl_error( GL_INVALID_ENUM, "glIsEnabled" );
	 return GL_FALSE;
   }
}

