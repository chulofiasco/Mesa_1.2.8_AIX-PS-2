/* get.c */

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
$Id: get.c,v 1.23 1996/02/26 15:07:16 brianp Exp $

$Log: get.c,v $
 * Revision 1.23  1996/02/26  15:07:16  brianp
 * replaced CC.Current.Color with CC.Current.IntColor
 *
 * Revision 1.22  1996/02/19  21:50:16  brianp
 * glGet(GL_ALPHA_BITS) now returns correct value for softare alpha buffering
 *
 * Revision 1.21  1996/02/15  16:01:01  brianp
 * added vertex array extension support
 *
 * Revision 1.20  1996/01/17  15:16:34  brianp
 * glGet(GL_INDEX_MODE) returned wrong result
 *
 * Revision 1.19  1996/01/11  20:14:38  brianp
 * use new GLstencil datatype
 *
 * Revision 1.18  1995/12/18  17:25:55  brianp
 * use new GLdepth and GLaccum datatypes
 *
 * Revision 1.17  1995/10/27  20:29:02  brianp
 * added glPolygonOffsetEXT() support
 *
 * Revision 1.16  1995/10/14  16:29:18  brianp
 * replaced dd_buffer_info with DD.buffer_size
 *
 * Revision 1.15  1995/08/31  21:29:33  brianp
 * new TexGenEnabled bitfield
 *
 * Revision 1.14  1995/07/12  21:19:02  brianp
 * bug fix:  glGet(GL_DEPTH_BITS) returned bytes, not bits (per Tor Stenvaag)
 *
 * Revision 1.13  1995/06/08  20:51:57  brianp
 * added missing RenderMode to glGetDoublev()
 *
 * Revision 1.12  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.11  1995/05/17  13:52:37  brianp
 * implemented glIndexMask(0) and glColorMask(0,0,0,0)
 *
 * Revision 1.10  1995/05/15  15:24:07  brianp
 * replaced CC.CurrentListNum with gl_list_index()
 *
 * Revision 1.9  1995/05/12  19:26:43  brianp
 * replaced CC.Mode!=0 with INSIDE_BEGIN_END
 *
 * Revision 1.8  1995/05/12  17:00:43  brianp
 * changed CC.Mode!=0 to INSIDE_BEGIN_END
 *
 * Revision 1.7  1995/03/27  20:31:53  brianp
 * new Texture.Enabled scheme
 *
 * Revision 1.6  1995/03/24  15:28:47  brianp
 * replaced ACCUM_BITS with ACC_TYPE
 *
 * Revision 1.5  1995/03/22  21:37:22  brianp
 * removed BufferDepth from CC
 *
 * Revision 1.4  1995/03/10  16:25:53  brianp
 * updated for blending extensions
 *
 * Revision 1.3  1995/03/10  14:26:58  brianp
 * implemented get(GL_CURRENT_RASTER_TEXTURE_COORDS)
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:23:16  brianp
 * Initial revision
 *
 */


/* glGet functions */


#include <string.h>
#include "context.h"
#include "dd.h"
#include "list.h"
#include "macros.h"


#define FLOAT_TO_BOOL(X)	( (X)==0.0F ? GL_FALSE : GL_TRUE )
#define INT_TO_BOOL(I)		( (I)==0 ? GL_FALSE : GL_TRUE )
#define ENUM_TO_BOOL(E)		( (E)==0 ? GL_FALSE : GL_TRUE )
/*#define FLOAT_TO_INT(I)		((GLint) (I * 2147483647.0))*/



/*
 * Given a color component scale value such as CC.RedScale, CC.BlueScale,
 * etc. return the number of bits implied by that scaling.  This is
 * usually expressed by:  scale = 2^bits - 1.
 * Example:  if RedScale==63.0, bits must be 6.
 * Input:  a color component scale value
 * Return:  number of bits implied by the scale value.
 */
static GLuint bits( GLfloat scale )
{
   GLuint iscale, bits;

   iscale = (GLuint) scale;
   for (bits=0; iscale>0; bits++) {
      iscale = iscale >> 1;
   }
   return bits;
}



void glGetBooleanv( GLenum pname, GLboolean *params )
{
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetBooleanv" );
      return;
   }

   switch (pname) {
      case GL_ACCUM_RED_BITS:
      case GL_ACCUM_GREEN_BITS:
      case GL_ACCUM_BLUE_BITS:
      case GL_ACCUM_ALPHA_BITS:
         *params = INT_TO_BOOL(sizeof(GLaccum)*8);
         break;
      case GL_ACCUM_CLEAR_VALUE:
         params[0] = FLOAT_TO_BOOL(CC.Accum.ClearColor[0]);
         params[1] = FLOAT_TO_BOOL(CC.Accum.ClearColor[1]);
         params[2] = FLOAT_TO_BOOL(CC.Accum.ClearColor[2]);
         params[3] = FLOAT_TO_BOOL(CC.Accum.ClearColor[3]);
         break;
      case GL_ALPHA_BIAS:
         *params = FLOAT_TO_BOOL(CC.Pixel.AlphaBias);
         break;
      case GL_ALPHA_BITS:
         if (CC.FrontAlphaEnabled || CC.BackAlphaEnabled) {
            *params = GL_TRUE;
         }
         else {
            *params = GL_FALSE;
         }
         break;
      case GL_ALPHA_SCALE:
         *params = FLOAT_TO_BOOL(CC.Pixel.AlphaScale);
         break;
      case GL_ALPHA_TEST:
         *params = CC.Color.AlphaEnabled;
         break;
      case GL_ALPHA_TEST_FUNC:
         *params = ENUM_TO_BOOL(CC.Color.AlphaFunc);
         break;
      case GL_ALPHA_TEST_REF:
         *params = FLOAT_TO_BOOL(CC.Color.AlphaRef);
         break;
      case GL_ATTRIB_STACK_DEPTH:
         *params = INT_TO_BOOL(CC.AttribStackDepth);
         break;
      case GL_AUTO_NORMAL:
         *params = CC.Eval.AutoNormal;
         break;
      case GL_AUX_BUFFERS:
         *params = (NUM_AUX_BUFFERS) ? GL_TRUE : GL_FALSE;
         break;
      case GL_BLEND:
         *params = CC.Color.BlendEnabled;
         break;
      case GL_BLEND_DST:
         *params = ENUM_TO_BOOL(CC.Color.BlendDst);
         break;
      case GL_BLEND_SRC:
         *params = ENUM_TO_BOOL(CC.Color.BlendSrc);
         break;
      case GL_BLEND_EQUATION_EXT:
	 *params = ENUM_TO_BOOL( CC.Color.BlendEquation );
	 break;
      case GL_BLEND_COLOR_EXT:
	 params[0] = FLOAT_TO_BOOL( CC.Color.BlendColor[0] );
	 params[1] = FLOAT_TO_BOOL( CC.Color.BlendColor[1] );
	 params[2] = FLOAT_TO_BOOL( CC.Color.BlendColor[2] );
	 params[3] = FLOAT_TO_BOOL( CC.Color.BlendColor[3] );
	 break;
      case GL_BLUE_BIAS:
         *params = FLOAT_TO_BOOL(CC.Pixel.BlueBias);
         break;
      case GL_BLUE_BITS:
         *params = INT_TO_BOOL( bits( CC.BlueScale ) );
         break;
      case GL_BLUE_SCALE:
         *params = FLOAT_TO_BOOL(CC.Pixel.BlueScale);
         break;
      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:
         *params = CC.Transform.ClipEnabled[pname-GL_CLIP_PLANE0];
         break;
      case GL_COLOR_CLEAR_VALUE:
         params[0] = FLOAT_TO_BOOL(CC.Color.ClearColor[0]);
         params[1] = FLOAT_TO_BOOL(CC.Color.ClearColor[1]);
         params[2] = FLOAT_TO_BOOL(CC.Color.ClearColor[2]);
         params[3] = FLOAT_TO_BOOL(CC.Color.ClearColor[3]);
         break;
      case GL_COLOR_MATERIAL:
         *params = CC.Light.ColorMaterialEnabled;
         break;
      case GL_COLOR_MATERIAL_FACE:
         *params = ENUM_TO_BOOL(CC.Light.ColorMaterialFace);
         break;
      case GL_COLOR_MATERIAL_PARAMETER:
         *params = ENUM_TO_BOOL(CC.Light.ColorMaterialMode);
         break;
      case GL_COLOR_WRITEMASK:
         params[0] = (CC.Color.ColorMask & 8) ? GL_TRUE : GL_FALSE;
         params[1] = (CC.Color.ColorMask & 4) ? GL_TRUE : GL_FALSE;
         params[2] = (CC.Color.ColorMask & 2) ? GL_TRUE : GL_FALSE;
         params[3] = (CC.Color.ColorMask & 1) ? GL_TRUE : GL_FALSE;
         break;
      case GL_CULL_FACE:
         *params = CC.Polygon.CullFlag;
         break;
      case GL_CULL_FACE_MODE:
         *params = ENUM_TO_BOOL(CC.Polygon.CullFaceMode);
         break;
      case GL_CURRENT_COLOR:
         params[0] = INT_TO_BOOL(CC.Current.IntColor[0]);
         params[1] = INT_TO_BOOL(CC.Current.IntColor[1]);
         params[2] = INT_TO_BOOL(CC.Current.IntColor[2]);
         params[3] = INT_TO_BOOL(CC.Current.IntColor[3]);
         break;
      case GL_CURRENT_INDEX:
         *params = INT_TO_BOOL(CC.Current.Index);
         break;
      case GL_CURRENT_NORMAL:
         params[0] = FLOAT_TO_BOOL(CC.Current.Normal[0]);
         params[1] = FLOAT_TO_BOOL(CC.Current.Normal[1]);
         params[2] = FLOAT_TO_BOOL(CC.Current.Normal[2]);
         break;
      case GL_CURRENT_RASTER_COLOR:
	 params[0] = FLOAT_TO_BOOL(CC.Current.RasterColor[0]);
	 params[1] = FLOAT_TO_BOOL(CC.Current.RasterColor[1]);
	 params[2] = FLOAT_TO_BOOL(CC.Current.RasterColor[2]);
	 params[3] = FLOAT_TO_BOOL(CC.Current.RasterColor[3]);
	 break;
      case GL_CURRENT_RASTER_DISTANCE:
	 *params = FLOAT_TO_BOOL(CC.Current.RasterDistance);
	 break;
      case GL_CURRENT_RASTER_INDEX:
	 *params = FLOAT_TO_BOOL(CC.Current.RasterIndex);
	 break;
      case GL_CURRENT_RASTER_POSITION:
	 params[0] = FLOAT_TO_BOOL(CC.Current.RasterPos[0]);
	 params[1] = FLOAT_TO_BOOL(CC.Current.RasterPos[1]);
	 params[2] = FLOAT_TO_BOOL(CC.Current.RasterPos[2]);
	 params[3] = FLOAT_TO_BOOL(CC.Current.RasterPos[3]);
	 break;
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
         params[0] = FLOAT_TO_BOOL(CC.Current.RasterTexCoord[0]);
         params[1] = FLOAT_TO_BOOL(CC.Current.RasterTexCoord[1]);
         params[2] = FLOAT_TO_BOOL(CC.Current.RasterTexCoord[2]);
         params[3] = FLOAT_TO_BOOL(CC.Current.RasterTexCoord[3]);
	 break;
      case GL_CURRENT_RASTER_POSITION_VALID:
         *params = CC.Current.RasterPosValid;
	 break;
      case GL_CURRENT_TEXTURE_COORDS:
         params[0] = FLOAT_TO_BOOL(CC.Current.TexCoord[0]);
         params[1] = FLOAT_TO_BOOL(CC.Current.TexCoord[1]);
         params[2] = FLOAT_TO_BOOL(CC.Current.TexCoord[2]);
         params[3] = FLOAT_TO_BOOL(CC.Current.TexCoord[3]);
	 break;
      case GL_DEPTH_BIAS:
         *params = FLOAT_TO_BOOL(CC.Pixel.DepthBias);
	 break;
      case GL_DEPTH_BITS:
	 *params = INT_TO_BOOL(8*sizeof(GLdepth));
	 break;
      case GL_DEPTH_CLEAR_VALUE:
         *params = FLOAT_TO_BOOL(CC.Depth.Clear);
	 break;
      case GL_DEPTH_FUNC:
         *params = ENUM_TO_BOOL(CC.Depth.Func);
	 break;
      case GL_DEPTH_RANGE:
         params[0] = FLOAT_TO_BOOL(CC.Viewport.Near);
         params[1] = FLOAT_TO_BOOL(CC.Viewport.Far);
	 break;
      case GL_DEPTH_SCALE:
         *params = FLOAT_TO_BOOL(CC.Pixel.DepthScale);
	 break;
      case GL_DEPTH_TEST:
         *params = CC.Depth.Test;
	 break;
      case GL_DEPTH_WRITEMASK:
	 *params = CC.Depth.Mask;
	 break;
      case GL_DITHER:
	 *params = CC.Color.DitherFlag;
	 break;
      case GL_DOUBLEBUFFER:
	 *params = CC.DBflag;
	 break;
      case GL_DRAW_BUFFER:
	 *params = ENUM_TO_BOOL(CC.Color.DrawBuffer);
	 break;
      case GL_EDGE_FLAG:
	 *params = CC.Current.EdgeFlag;
	 break;
      case GL_FOG:
	 *params = CC.Fog.Enabled;
	 break;
      case GL_FOG_COLOR:
         params[0] = FLOAT_TO_BOOL(CC.Fog.Color[0]);
         params[1] = FLOAT_TO_BOOL(CC.Fog.Color[1]);
         params[2] = FLOAT_TO_BOOL(CC.Fog.Color[2]);
         params[3] = FLOAT_TO_BOOL(CC.Fog.Color[3]);
	 break;
      case GL_FOG_DENSITY:
         *params = FLOAT_TO_BOOL(CC.Fog.Density);
	 break;
      case GL_FOG_END:
         *params = FLOAT_TO_BOOL(CC.Fog.End);
	 break;
      case GL_FOG_HINT:
	 *params = ENUM_TO_BOOL(CC.Hint.Fog);
	 break;
      case GL_FOG_INDEX:
	 *params = FLOAT_TO_BOOL(CC.Fog.Index);
	 break;
      case GL_FOG_MODE:
	 *params = ENUM_TO_BOOL(CC.Fog.Mode);
	 break;
      case GL_FOG_START:
         *params = FLOAT_TO_BOOL(CC.Fog.End);
	 break;
      case GL_FRONT_FACE:
	 *params = ENUM_TO_BOOL(CC.Polygon.FrontFace);
	 break;
      case GL_GREEN_BIAS:
         *params = FLOAT_TO_BOOL(CC.Pixel.GreenBias);
	 break;
      case GL_GREEN_BITS:
         *params = INT_TO_BOOL( bits( CC.GreenScale ) );
	 break;
      case GL_GREEN_SCALE:
         *params = FLOAT_TO_BOOL(CC.Pixel.GreenScale);
	 break;
      case GL_INDEX_BITS:
	 {
	    GLuint width, height, depth;
	    (*DD.buffer_size)( &width, &height, &depth );
	    *params = INT_TO_BOOL(depth);
	 }
	 break;
      case GL_INDEX_CLEAR_VALUE:
	 *params = INT_TO_BOOL(CC.Color.ClearIndex);
	 break;
      case GL_INDEX_MODE:
	 *params = CC.RGBAflag ? GL_FALSE : GL_TRUE;
	 break;
      case GL_INDEX_OFFSET:
	 *params = INT_TO_BOOL(CC.Pixel.IndexOffset);
	 break;
      case GL_INDEX_SHIFT:
	 *params = INT_TO_BOOL(CC.Pixel.IndexShift);
	 break;
      case GL_INDEX_WRITEMASK:
	 *params = INT_TO_BOOL(CC.Color.IndexMask);
	 break;
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
	 *params = CC.Light.Light[pname-GL_LIGHT0].Enabled;
	 break;
      case GL_LIGHTING:
	 *params = CC.Light.Enabled;
	 break;
      case GL_LIGHT_MODEL_AMBIENT:
	 params[0] = FLOAT_TO_BOOL(CC.Light.Model.Ambient[0]);
	 params[1] = FLOAT_TO_BOOL(CC.Light.Model.Ambient[1]);
	 params[2] = FLOAT_TO_BOOL(CC.Light.Model.Ambient[2]);
	 params[3] = FLOAT_TO_BOOL(CC.Light.Model.Ambient[3]);
	 break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
	 *params = CC.Light.Model.LocalViewer;
	 break;
      case GL_LIGHT_MODEL_TWO_SIDE:
	 *params = CC.Light.Model.TwoSide;
	 break;
      case GL_LINE_SMOOTH:
	 *params = CC.Line.SmoothFlag;
	 break;
      case GL_LINE_SMOOTH_HINT:
	 *params = ENUM_TO_BOOL(CC.Hint.LineSmooth);
	 break;
      case GL_LINE_STIPPLE:
	 *params = CC.Line.StippleFlag;
	 break;
      case GL_LINE_STIPPLE_PATTERN:
	 *params = INT_TO_BOOL(CC.Line.StipplePattern);
	 break;
      case GL_LINE_STIPPLE_REPEAT:
	 *params = INT_TO_BOOL(CC.Line.StippleFactor);
	 break;
      case GL_LINE_WIDTH:
	 *params = FLOAT_TO_BOOL(CC.Line.Width);
	 break;
      case GL_LINE_WIDTH_GRANULARITY:
	 *params = FLOAT_TO_BOOL(LINE_WIDTH_GRANULARITY);
	 break;
      case GL_LINE_WIDTH_RANGE:
	 params[0] = FLOAT_TO_BOOL(MIN_LINE_WIDTH);
	 params[1] = FLOAT_TO_BOOL(MAX_LINE_WIDTH);
	 break;
      case GL_LIST_BASE:
	 *params = INT_TO_BOOL(CC.List.ListBase);
	 break;
      case GL_LIST_INDEX:
	 *params = INT_TO_BOOL( gl_list_index() );
	 break;
      case GL_LIST_MODE:
	 *params = ENUM_TO_BOOL( CC.ExecuteFlag
				  ? GL_COMPILE_AND_EXECUTE : GL_COMPILE );
	 break;
      case GL_LOGIC_OP:
	 *params = CC.Color.LogicOpEnabled;
	 break;
      case GL_LOGIC_OP_MODE:
	 *params = ENUM_TO_BOOL(CC.Color.LogicOp);
	 break;
      case GL_MAP1_COLOR_4:
	 *params = CC.Eval.Map1Color4;
	 break;
      case GL_MAP1_GRID_DOMAIN:
	 params[0] = FLOAT_TO_BOOL(CC.Eval.MapGrid1u1);
	 params[1] = FLOAT_TO_BOOL(CC.Eval.MapGrid1u2);
	 break;
      case GL_MAP1_GRID_SEGMENTS:
	 *params = INT_TO_BOOL(CC.Eval.MapGrid1un);
	 break;
      case GL_MAP1_INDEX:
	 *params = CC.Eval.Map1Index;
	 break;
      case GL_MAP1_NORMAL:
	 *params = CC.Eval.Map1Normal;
	 break;
      case GL_MAP1_TEXTURE_COORD_1:
	 *params = CC.Eval.Map1TextureCoord1;
	 break;
      case GL_MAP1_TEXTURE_COORD_2:
	 *params = CC.Eval.Map1TextureCoord2;
	 break;
      case GL_MAP1_TEXTURE_COORD_3:
	 *params = CC.Eval.Map1TextureCoord3;
	 break;
      case GL_MAP1_TEXTURE_COORD_4:
	 *params = CC.Eval.Map1TextureCoord4;
	 break;
      case GL_MAP1_VERTEX_3:
	 *params = CC.Eval.Map1Vertex3;
	 break;
      case GL_MAP1_VERTEX_4:
	 *params = CC.Eval.Map1Vertex4;
	 break;
      case GL_MAP2_COLOR_4:
	 *params = CC.Eval.Map2Color4;
	 break;
      case GL_MAP2_GRID_DOMAIN:
	 params[0] = FLOAT_TO_BOOL(CC.Eval.MapGrid2u1);
	 params[1] = FLOAT_TO_BOOL(CC.Eval.MapGrid2u2);
	 params[2] = FLOAT_TO_BOOL(CC.Eval.MapGrid2v1);
	 params[3] = FLOAT_TO_BOOL(CC.Eval.MapGrid2v2);
	 break;
      case GL_MAP2_GRID_SEGMENTS:
	 params[0] = INT_TO_BOOL(CC.Eval.MapGrid2un);
	 params[1] = INT_TO_BOOL(CC.Eval.MapGrid2vn);
	 break;
      case GL_MAP2_INDEX:
	 *params = CC.Eval.Map2Index;
	 break;
      case GL_MAP2_NORMAL:
	 *params = CC.Eval.Map2Normal;
	 break;
      case GL_MAP2_TEXTURE_COORD_1:
	 *params = CC.Eval.Map2TextureCoord1;
	 break;
      case GL_MAP2_TEXTURE_COORD_2:
	 *params = CC.Eval.Map2TextureCoord2;
	 break;
      case GL_MAP2_TEXTURE_COORD_3:
	 *params = CC.Eval.Map2TextureCoord3;
	 break;
      case GL_MAP2_TEXTURE_COORD_4:
	 *params = CC.Eval.Map2TextureCoord4;
	 break;
      case GL_MAP2_VERTEX_3:
	 *params = CC.Eval.Map2Vertex3;
	 break;
      case GL_MAP2_VERTEX_4:
	 *params = CC.Eval.Map2Vertex4;
	 break;
      case GL_MAP_COLOR:
	 *params = CC.Pixel.MapColorFlag;
	 break;
      case GL_MAP_STENCIL:
	 *params = CC.Pixel.MapStencilFlag;
	 break;
      case GL_MATRIX_MODE:
	 *params = ENUM_TO_BOOL( CC.Transform.MatrixMode );
	 break;
      case GL_MAX_ATTRIB_STACK_DEPTH:
	 *params = INT_TO_BOOL(MAX_ATTRIB_STACK_DEPTH);
	 break;
      case GL_MAX_CLIP_PLANES:
	 *params = INT_TO_BOOL(MAX_CLIP_PLANES);
	 break;
      case GL_MAX_EVAL_ORDER:
	 *params = INT_TO_BOOL(MAX_EVAL_ORDER);
	 break;
      case GL_MAX_LIGHTS:
	 *params = INT_TO_BOOL(MAX_LIGHTS);
	 break;
      case GL_MAX_LIST_NESTING:
	 *params = INT_TO_BOOL(MAX_LIST_NESTING);
	 break;
      case GL_MAX_MODELVIEW_STACK_DEPTH:
	 *params = INT_TO_BOOL(MAX_MODELVIEW_STACK_DEPTH);
	 break;
      case GL_MAX_NAME_STACK_DEPTH:
	 *params = INT_TO_BOOL(MAX_NAME_STACK_DEPTH);
	 break;
      case GL_MAX_PIXEL_MAP_TABLE:
	 *params = INT_TO_BOOL(MAX_PIXEL_MAP_TABLE);
	 break;
      case GL_MAX_PROJECTION_STACK_DEPTH:
	 *params = INT_TO_BOOL(MAX_PROJECTION_STACK_DEPTH);
	 break;
      case GL_MAX_TEXTURE_SIZE:
	 *params = INT_TO_BOOL(MAX_TEXTURE_SIZE);
	 break;
      case GL_MAX_TEXTURE_STACK_DEPTH:
	 *params = INT_TO_BOOL(MAX_TEXTURE_STACK_DEPTH);
	 break;
      case GL_MAX_VIEWPORT_DIMS:
	 params[0] = INT_TO_BOOL(MAX_WIDTH);
	 params[1] = INT_TO_BOOL(MAX_HEIGHT);
	 break;
      case GL_MODELVIEW_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = FLOAT_TO_BOOL(CC.ModelViewMatrix[i]);
	 }
	 break;
      case GL_MODELVIEW_STACK_DEPTH:
	 *params = INT_TO_BOOL(CC.ModelViewStackDepth);
	 break;
      case GL_NAME_STACK_DEPTH:
	 *params = INT_TO_BOOL(CC.NameStackDepth);
	 break;
      case GL_NORMALIZE:
	 *params = CC.Transform.Normalize;
	 break;
      case GL_PACK_ALIGNMENT:
	 *params = INT_TO_BOOL(CC.PackAlignment);
	 break;
      case GL_PACK_LSB_FIRST:
	 *params = CC.PackLSBFirst;
	 break;
      case GL_PACK_ROW_LENGTH:
	 *params = INT_TO_BOOL(CC.PackRowLength);
	 break;
      case GL_PACK_SKIP_PIXELS:
	 *params = INT_TO_BOOL(CC.PackSkipPixels);
	 break;
      case GL_PACK_SKIP_ROWS:
	 *params = INT_TO_BOOL(CC.PackSkipRows);
	 break;
      case GL_PACK_SWAP_BYTES:
	 *params = CC.PackSwapBytes;
	 break;
      case GL_PERSPECTIVE_CORRECTION_HINT:
	 *params = ENUM_TO_BOOL(CC.Hint.PerspectiveCorrection);
	 break;
      case GL_PIXEL_MAP_A_TO_A_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapAtoAsize);
	 break;
      case GL_PIXEL_MAP_B_TO_B_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapBtoBsize);
	 break;
      case GL_PIXEL_MAP_G_TO_G_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapGtoGsize);
	 break;
      case GL_PIXEL_MAP_I_TO_A_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapItoAsize);
	 break;
      case GL_PIXEL_MAP_I_TO_B_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapItoBsize);
	 break;
      case GL_PIXEL_MAP_I_TO_G_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapItoGsize);
	 break;
      case GL_PIXEL_MAP_I_TO_I_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapItoIsize);
	 break;
      case GL_PIXEL_MAP_I_TO_R_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapItoRsize);
	 break;
      case GL_PIXEL_MAP_R_TO_R_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapRtoRsize);
	 break;
      case GL_PIXEL_MAP_S_TO_S_SIZE:
	 *params = INT_TO_BOOL(CC.Pixel.MapStoSsize);
	 break;
      case GL_POINT_SIZE:
	 *params = FLOAT_TO_BOOL(CC.Point.Size );
	 break;
      case GL_POINT_SIZE_GRANULARITY:
	 *params = FLOAT_TO_BOOL(POINT_SIZE_GRANULARITY );
	 break;
      case GL_POINT_SIZE_RANGE:
	 params[0] = FLOAT_TO_BOOL(MIN_POINT_SIZE );
	 params[1] = FLOAT_TO_BOOL(MAX_POINT_SIZE );
	 break;
      case GL_POINT_SMOOTH:
	 *params = CC.Point.SmoothFlag;
	 break;
      case GL_POINT_SMOOTH_HINT:
	 *params = ENUM_TO_BOOL(CC.Hint.PointSmooth);
	 break;
      case GL_POLYGON_MODE:
	 params[0] = ENUM_TO_BOOL(CC.Polygon.FrontMode);
	 params[1] = ENUM_TO_BOOL(CC.Polygon.BackMode);
	 break;
      case GL_POLYGON_OFFSET_BIAS_EXT:
         *params = FLOAT_TO_BOOL( CC.Polygon.OffsetBias );
         break;
      case GL_POLYGON_OFFSET_FACTOR_EXT:
         *params = FLOAT_TO_BOOL( CC.Polygon.OffsetFactor );
         break;
      case GL_POLYGON_SMOOTH:
	 *params = CC.Polygon.SmoothFlag;
	 break;
      case GL_POLYGON_SMOOTH_HINT:
	 *params = ENUM_TO_BOOL(CC.Hint.PolygonSmooth);
	 break;
      case GL_POLYGON_STIPPLE:
	 *params = CC.Polygon.StippleFlag;
	 break;
      case GL_PROJECTION_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = FLOAT_TO_BOOL(CC.ProjectionMatrix[i]);
	 }
	 break;
      case GL_PROJECTION_STACK_DEPTH:
	 *params = INT_TO_BOOL(CC.ProjectionStackDepth);
	 break;
      case GL_READ_BUFFER:
	 *params = ENUM_TO_BOOL(CC.Pixel.ReadBuffer);
	 break;
      case GL_RED_BIAS:
         *params = FLOAT_TO_BOOL(CC.Pixel.RedBias);
	 break;
      case GL_RED_BITS:
         *params = INT_TO_BOOL( bits( CC.RedScale ) );
	 break;
      case GL_RED_SCALE:
         *params = FLOAT_TO_BOOL(CC.Pixel.RedScale);
	 break;
      case GL_RENDER_MODE:
	 *params = ENUM_TO_BOOL(CC.RenderMode);
	 break;
      case GL_RGBA_MODE:
         *params = CC.RGBAflag;
	 break;
      case GL_SCISSOR_BOX:
	 params[0] = INT_TO_BOOL(CC.Scissor.X);
	 params[1] = INT_TO_BOOL(CC.Scissor.Y);
	 params[2] = INT_TO_BOOL(CC.Scissor.Width);
	 params[3] = INT_TO_BOOL(CC.Scissor.Height);
	 break;
      case GL_SCISSOR_TEST:
	 *params = CC.Scissor.Enabled;
	 break;
      case GL_SHADE_MODEL:
	 *params = ENUM_TO_BOOL(CC.Light.ShadeModel);
	 break;
      case GL_STENCIL_BITS:
	 *params = INT_TO_BOOL(8*sizeof(GLstencil));
	 break;
      case GL_STENCIL_CLEAR_VALUE:
	 *params = INT_TO_BOOL(CC.Stencil.Clear);
	 break;
      case GL_STENCIL_FAIL:
	 *params = ENUM_TO_BOOL(CC.Stencil.FailFunc);
	 break;
      case GL_STENCIL_FUNC:
	 *params = ENUM_TO_BOOL(CC.Stencil.Function);
	 break;
      case GL_STENCIL_PASS_DEPTH_FAIL:
	 *params = ENUM_TO_BOOL(CC.Stencil.ZFailFunc);
	 break;
      case GL_STENCIL_PASS_DEPTH_PASS:
	 *params = ENUM_TO_BOOL(CC.Stencil.ZPassFunc);
	 break;
      case GL_STENCIL_REF:
	 *params = INT_TO_BOOL(CC.Stencil.Ref);
	 break;
      case GL_STENCIL_TEST:
	 *params = CC.Stencil.Enabled;
	 break;
      case GL_STENCIL_VALUE_MASK:
	 *params = INT_TO_BOOL(CC.Stencil.ValueMask);
	 break;
      case GL_STENCIL_WRITEMASK:
	 *params = INT_TO_BOOL(CC.Stencil.WriteMask);
	 break;
      case GL_STEREO:
	 *params = GL_FALSE;    /* TODO */
	 break;
      case GL_SUBPIXEL_BITS:
	 *params = INT_TO_BOOL(0);  /* TODO */
	 break;
      case GL_TEXTURE_1D:
	 *params = (CC.Texture.Enabled & 1) ? GL_TRUE : GL_FALSE;
	 break;
      case GL_TEXTURE_2D:
	 *params = (CC.Texture.Enabled & 2) ? GL_TRUE : GL_FALSE;
	 break;
      case GL_TEXTURE_ENV_COLOR:
	 params[0] = FLOAT_TO_BOOL(CC.Texture.EnvColor[0]);
	 params[1] = FLOAT_TO_BOOL(CC.Texture.EnvColor[1]);
	 params[2] = FLOAT_TO_BOOL(CC.Texture.EnvColor[2]);
	 params[3] = FLOAT_TO_BOOL(CC.Texture.EnvColor[3]);
	 break;
      case GL_TEXTURE_ENV_MODE:
	 *params = ENUM_TO_BOOL(CC.Texture.EnvMode);
	 break;
      case GL_TEXTURE_GEN_S:
	 *params = (CC.Texture.TexGenEnabled & S_BIT) ? GL_TRUE : GL_FALSE;
	 break;
      case GL_TEXTURE_GEN_T:
	 *params = (CC.Texture.TexGenEnabled & T_BIT) ? GL_TRUE : GL_FALSE;
	 break;
      case GL_TEXTURE_GEN_R:
	 *params = (CC.Texture.TexGenEnabled & R_BIT) ? GL_TRUE : GL_FALSE;
	 break;
      case GL_TEXTURE_GEN_Q:
	 *params = (CC.Texture.TexGenEnabled & Q_BIT) ? GL_TRUE : GL_FALSE;
	 break;
      case GL_TEXTURE_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = FLOAT_TO_BOOL(CC.TextureMatrix[i]);
	 }
	 break;
      case GL_TEXTURE_STACK_DEPTH:
	 *params = INT_TO_BOOL(CC.TextureStackDepth);
	 break;
      case GL_UNPACK_ALIGNMENT:
	 *params = INT_TO_BOOL(CC.UnpackAlignment);
	 break;
      case GL_UNPACK_LSB_FIRST:
	 *params = CC.UnpackLSBFirst;
	 break;
      case GL_UNPACK_ROW_LENGTH:
	 *params = INT_TO_BOOL(CC.UnpackRowLength);
	 break;
      case GL_UNPACK_SKIP_PIXELS:
	 *params = INT_TO_BOOL(CC.UnpackSkipPixels);
	 break;
      case GL_UNPACK_SKIP_ROWS:
	 *params = INT_TO_BOOL(CC.UnpackSkipRows);
	 break;
      case GL_UNPACK_SWAP_BYTES:
	 *params = CC.UnpackSwapBytes;
	 break;
      case GL_VIEWPORT:
	 params[0] = INT_TO_BOOL(CC.Viewport.X);
	 params[1] = INT_TO_BOOL(CC.Viewport.Y);
	 params[2] = INT_TO_BOOL(CC.Viewport.Width);
	 params[3] = INT_TO_BOOL(CC.Viewport.Height);
	 break;
      case GL_ZOOM_X:
	 *params = FLOAT_TO_BOOL(CC.Pixel.ZoomX);
	 break;
      case GL_ZOOM_Y:
	 *params = FLOAT_TO_BOOL(CC.Pixel.ZoomY);
	 break;
      case GL_VERTEX_ARRAY_SIZE_EXT:
         *params = INT_TO_BOOL(CC.Array.VertexSize);
         break;
      case GL_VERTEX_ARRAY_TYPE_EXT:
         *params = ENUM_TO_BOOL(CC.Array.VertexType);
         break;
      case GL_VERTEX_ARRAY_STRIDE_EXT:
         *params = INT_TO_BOOL(CC.Array.VertexStride);
         break;
      case GL_VERTEX_ARRAY_COUNT_EXT:
         *params = INT_TO_BOOL(CC.Array.VertexCount);
         break;
      case GL_NORMAL_ARRAY_TYPE_EXT:
         *params = ENUM_TO_BOOL(CC.Array.NormalType);
         break;
      case GL_NORMAL_ARRAY_STRIDE_EXT:
         *params = INT_TO_BOOL(CC.Array.NormalStride);
         break;
      case GL_NORMAL_ARRAY_COUNT_EXT:
         *params = INT_TO_BOOL(CC.Array.NormalCount);
         break;
      case GL_COLOR_ARRAY_SIZE_EXT:
         *params = INT_TO_BOOL(CC.Array.ColorSize);
         break;
      case GL_COLOR_ARRAY_TYPE_EXT:
         *params = ENUM_TO_BOOL(CC.Array.ColorType);
         break;
      case GL_COLOR_ARRAY_STRIDE_EXT:
         *params = INT_TO_BOOL(CC.Array.ColorStride);
         break;
      case GL_COLOR_ARRAY_COUNT_EXT:
         *params = INT_TO_BOOL(CC.Array.ColorCount);
         break;
      case GL_INDEX_ARRAY_TYPE_EXT:
         *params = ENUM_TO_BOOL(CC.Array.IndexType);
         break;
      case GL_INDEX_ARRAY_STRIDE_EXT:
         *params = INT_TO_BOOL(CC.Array.IndexStride);
         break;
      case GL_INDEX_ARRAY_COUNT_EXT:
         *params = INT_TO_BOOL(CC.Array.IndexCount);
         break;
      case GL_TEXTURE_COORD_ARRAY_SIZE_EXT:
         *params = INT_TO_BOOL(CC.Array.TexCoordSize);
         break;
      case GL_TEXTURE_COORD_ARRAY_TYPE_EXT:
         *params = ENUM_TO_BOOL(CC.Array.TexCoordType);
         break;
      case GL_TEXTURE_COORD_ARRAY_STRIDE_EXT:
         *params = INT_TO_BOOL(CC.Array.TexCoordStride);
         break;
      case GL_TEXTURE_COORD_ARRAY_COUNT_EXT:
         *params = INT_TO_BOOL(CC.Array.TexCoordCount);
         break;
      case GL_EDGE_FLAG_ARRAY_STRIDE_EXT:
         *params = INT_TO_BOOL(CC.Array.EdgeFlagStride);
         break;
      case GL_EDGE_FLAG_ARRAY_COUNT_EXT:
         *params = INT_TO_BOOL(CC.Array.EdgeFlagCount);
         break;

      default:
         gl_error( GL_INVALID_ENUM, "glGetBooleanv" );
   }
}




void glGetDoublev( GLenum pname, GLdouble *params )
{
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetDoublev" );
      return;
   }

   switch (pname) {
      case GL_ACCUM_RED_BITS:
      case GL_ACCUM_GREEN_BITS:
      case GL_ACCUM_BLUE_BITS:
      case GL_ACCUM_ALPHA_BITS:
         *params = (GLdouble) (sizeof(GLaccum)*8);
         break;
      case GL_ACCUM_CLEAR_VALUE:
         params[0] = (GLdouble) CC.Accum.ClearColor[0];
         params[1] = (GLdouble) CC.Accum.ClearColor[1];
         params[2] = (GLdouble) CC.Accum.ClearColor[2];
         params[3] = (GLdouble) CC.Accum.ClearColor[3];
         break;
      case GL_ALPHA_BIAS:
         *params = (GLdouble) CC.Pixel.AlphaBias;
         break;
      case GL_ALPHA_BITS:
         if (CC.FrontAlphaEnabled || CC.BackAlphaEnabled) {
            *params = 8*sizeof(GLubyte);
         }
         else {
            *params = 0.0;
         }
         break;
      case GL_ALPHA_SCALE:
         *params = (GLdouble) CC.Pixel.AlphaScale;
         break;
      case GL_ALPHA_TEST:
         *params = (GLdouble) CC.Color.AlphaEnabled;
         break;
      case GL_ALPHA_TEST_FUNC:
         *params = (GLdouble) CC.Color.AlphaFunc;
         break;
      case GL_ALPHA_TEST_REF:
         *params = (GLdouble) CC.Color.AlphaRef;
         break;
      case GL_ATTRIB_STACK_DEPTH:
         *params = (GLdouble ) CC.AttribStackDepth;
         break;
      case GL_AUTO_NORMAL:
         *params = (GLdouble) CC.Eval.AutoNormal;
         break;
      case GL_AUX_BUFFERS:
         *params = (GLdouble) NUM_AUX_BUFFERS;
         break;
      case GL_BLEND:
         *params = (GLdouble) CC.Color.BlendEnabled;
         break;
      case GL_BLEND_DST:
         *params = (GLdouble) CC.Color.BlendDst;
         break;
      case GL_BLEND_SRC:
         *params = (GLdouble) CC.Color.BlendSrc;
         break;
      case GL_BLEND_EQUATION_EXT:
	 *params = (GLdouble) CC.Color.BlendEquation;
	 break;
      case GL_BLEND_COLOR_EXT:
	 params[0] = (GLdouble) CC.Color.BlendColor[0];
	 params[1] = (GLdouble) CC.Color.BlendColor[1];
	 params[2] = (GLdouble) CC.Color.BlendColor[2];
	 params[3] = (GLdouble) CC.Color.BlendColor[3];
	 break;
      case GL_BLUE_BIAS:
         *params = (GLdouble) CC.Pixel.BlueBias;
         break;
      case GL_BLUE_BITS:
         *params = (GLdouble) bits( CC.BlueScale );
         break;
      case GL_BLUE_SCALE:
         *params = (GLdouble) CC.Pixel.BlueScale;
         break;
      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:
         *params = (GLdouble) CC.Transform.ClipEnabled[pname-GL_CLIP_PLANE0];
         break;
      case GL_COLOR_CLEAR_VALUE:
         params[0] = (GLdouble) CC.Color.ClearColor[0];
         params[1] = (GLdouble) CC.Color.ClearColor[1];
         params[2] = (GLdouble) CC.Color.ClearColor[2];
         params[3] = (GLdouble) CC.Color.ClearColor[3];
         break;
      case GL_COLOR_MATERIAL:
         *params = (GLdouble) CC.Light.ColorMaterialEnabled;
         break;
      case GL_COLOR_MATERIAL_FACE:
         *params = (GLdouble) CC.Light.ColorMaterialFace;
         break;
      case GL_COLOR_MATERIAL_PARAMETER:
         *params = (GLdouble) CC.Light.ColorMaterialMode;
         break;
      case GL_COLOR_WRITEMASK:
         params[0] = (CC.Color.ColorMask & 8) ? 1.0 : 0.0;
         params[1] = (CC.Color.ColorMask & 4) ? 1.0 : 0.0;
         params[2] = (CC.Color.ColorMask & 2) ? 1.0 : 0.0;
         params[3] = (CC.Color.ColorMask & 1) ? 1.0 : 0.0;
         break;
      case GL_CULL_FACE:
         *params = (GLdouble) CC.Polygon.CullFlag;
         break;
      case GL_CULL_FACE_MODE:
         *params = (GLdouble) CC.Polygon.CullFaceMode;
         break;
      case GL_CURRENT_COLOR:
         params[0] = (GLdouble) CC.Current.IntColor[0] / CC.RedScale;
         params[1] = (GLdouble) CC.Current.IntColor[1] / CC.GreenScale;
         params[2] = (GLdouble) CC.Current.IntColor[2] / CC.BlueScale;
         params[3] = (GLdouble) CC.Current.IntColor[3] / CC.AlphaScale;
         break;
      case GL_CURRENT_INDEX:
         *params = (GLdouble) CC.Current.Index;
         break;
      case GL_CURRENT_NORMAL:
         params[0] = (GLdouble) CC.Current.Normal[0];
         params[1] = (GLdouble) CC.Current.Normal[1];
         params[2] = (GLdouble) CC.Current.Normal[2];
         break;
      case GL_CURRENT_RASTER_COLOR:
	 params[0] = (GLdouble) CC.Current.RasterColor[0];
	 params[1] = (GLdouble) CC.Current.RasterColor[1];
	 params[2] = (GLdouble) CC.Current.RasterColor[2];
	 params[3] = (GLdouble) CC.Current.RasterColor[3];
	 break;
      case GL_CURRENT_RASTER_DISTANCE:
	 params[0] = (GLdouble) CC.Current.RasterDistance;
	 break;
      case GL_CURRENT_RASTER_INDEX:
	 *params = (GLdouble) CC.Current.RasterIndex;
	 break;
      case GL_CURRENT_RASTER_POSITION:
	 params[0] = (GLdouble) CC.Current.RasterPos[0];
	 params[1] = (GLdouble) CC.Current.RasterPos[1];
	 params[2] = (GLdouble) CC.Current.RasterPos[2];
	 params[3] = (GLdouble) CC.Current.RasterPos[3];
	 break;
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
	 params[0] = (GLdouble) CC.Current.RasterTexCoord[0];
	 params[1] = (GLdouble) CC.Current.RasterTexCoord[1];
	 params[2] = (GLdouble) CC.Current.RasterTexCoord[2];
	 params[3] = (GLdouble) CC.Current.RasterTexCoord[3];
	 break;
      case GL_CURRENT_RASTER_POSITION_VALID:
	 *params = (GLdouble) CC.Current.RasterPosValid;
	 break;
      case GL_CURRENT_TEXTURE_COORDS:
	 params[0] = (GLdouble) CC.Current.TexCoord[0];
	 params[1] = (GLdouble) CC.Current.TexCoord[1];
	 params[2] = (GLdouble) CC.Current.TexCoord[2];
	 params[3] = (GLdouble) CC.Current.TexCoord[3];
	 break;
      case GL_DEPTH_BIAS:
	 *params = (GLdouble) CC.Pixel.DepthBias;
	 break;
      case GL_DEPTH_BITS:
	 *params = (GLdouble) (8*sizeof(GLdepth));
	 break;
      case GL_DEPTH_CLEAR_VALUE:
	 *params = (GLdouble) CC.Depth.Clear;
	 break;
      case GL_DEPTH_FUNC:
	 *params = (GLdouble) CC.Depth.Func;
	 break;
      case GL_DEPTH_RANGE:
         params[0] = (GLdouble) CC.Viewport.Near;
         params[1] = (GLdouble) CC.Viewport.Far;
	 break;
      case GL_DEPTH_SCALE:
	 *params = (GLdouble) CC.Pixel.DepthScale;
	 break;
      case GL_DEPTH_TEST:
	 *params = (GLdouble) CC.Depth.Test;
	 break;
      case GL_DEPTH_WRITEMASK:
	 *params = (GLdouble) CC.Depth.Mask;
	 break;
      case GL_DITHER:
	 *params = (GLdouble) CC.Color.DitherFlag;
	 break;
      case GL_DOUBLEBUFFER:
	 *params = (GLdouble) CC.DBflag;
	 break;
      case GL_DRAW_BUFFER:
	 *params = (GLdouble) CC.Color.DrawBuffer;
	 break;
      case GL_EDGE_FLAG:
	 *params = (GLdouble) CC.Current.EdgeFlag;
	 break;
      case GL_FOG:
	 *params = (GLdouble) CC.Fog.Enabled;
	 break;
      case GL_FOG_COLOR:
	 params[0] = (GLdouble) CC.Fog.Color[0];
	 params[1] = (GLdouble) CC.Fog.Color[1];
	 params[2] = (GLdouble) CC.Fog.Color[2];
	 params[3] = (GLdouble) CC.Fog.Color[3];
	 break;
      case GL_FOG_DENSITY:
	 *params = (GLdouble) CC.Fog.Density;
	 break;
      case GL_FOG_END:
	 *params = (GLdouble) CC.Fog.End;
	 break;
      case GL_FOG_HINT:
	 *params = (GLdouble) CC.Hint.Fog;
	 break;
      case GL_FOG_INDEX:
	 *params = (GLdouble) CC.Fog.Index;
	 break;
      case GL_FOG_MODE:
	 *params = (GLdouble) CC.Fog.Mode;
	 break;
      case GL_FOG_START:
	 *params = (GLdouble) CC.Fog.Start;
	 break;
      case GL_FRONT_FACE:
	 *params = (GLdouble) CC.Polygon.FrontFace;
	 break;
      case GL_GREEN_BIAS:
         *params = (GLdouble) CC.Pixel.GreenBias;
         break;
      case GL_GREEN_BITS:
         *params = (GLdouble) bits( CC.GreenScale );
         break;
      case GL_GREEN_SCALE:
         *params = (GLdouble) CC.Pixel.GreenScale;
         break;
      case GL_INDEX_BITS:
	 {
	    GLuint width, height, depth;
	    (*DD.buffer_size)( &width, &height, &depth );
	    *params = (GLdouble) depth;
	 }
	 break;
      case GL_INDEX_CLEAR_VALUE:
         *params = (GLdouble) CC.Color.ClearIndex;
	 break;
      case GL_INDEX_MODE:
	 *params = CC.RGBAflag ? 0.0 : 1.0;
	 break;
      case GL_INDEX_OFFSET:
	 *params = (GLdouble) CC.Pixel.IndexOffset;
	 break;
      case GL_INDEX_SHIFT:
	 *params = (GLdouble) CC.Pixel.IndexShift;
	 break;
      case GL_INDEX_WRITEMASK:
	 *params = (GLdouble) CC.Color.IndexMask;
	 break;
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
	 *params = (GLdouble) CC.Light.Light[pname-GL_LIGHT0].Enabled;
	 break;
      case GL_LIGHTING:
	 *params = (GLdouble) CC.Light.Enabled;
	 break;
      case GL_LIGHT_MODEL_AMBIENT:
	 params[0] = (GLdouble) CC.Light.Model.Ambient[0];
	 params[1] = (GLdouble) CC.Light.Model.Ambient[1];
	 params[2] = (GLdouble) CC.Light.Model.Ambient[2];
	 params[3] = (GLdouble) CC.Light.Model.Ambient[3];
	 break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
	 *params = (GLdouble) CC.Light.Model.LocalViewer;
	 break;
      case GL_LIGHT_MODEL_TWO_SIDE:
	 *params = (GLdouble) CC.Light.Model.TwoSide;
	 break;
      case GL_LINE_SMOOTH:
	 *params = (GLdouble) CC.Line.SmoothFlag;
	 break;
      case GL_LINE_SMOOTH_HINT:
	 *params = (GLdouble) CC.Hint.LineSmooth;
	 break;
      case GL_LINE_STIPPLE:
	 *params = (GLdouble) CC.Line.StippleFlag;
	 break;
      case GL_LINE_STIPPLE_PATTERN:
         *params = (GLdouble) CC.Line.StipplePattern;
         break;
      case GL_LINE_STIPPLE_REPEAT:
         *params = (GLdouble) CC.Line.StippleFactor;
         break;
      case GL_LINE_WIDTH:
	 *params = (GLdouble) CC.Line.Width;
	 break;
      case GL_LINE_WIDTH_GRANULARITY:
	 *params = (GLdouble) LINE_WIDTH_GRANULARITY;
	 break;
      case GL_LINE_WIDTH_RANGE:
	 params[0] = (GLdouble) MIN_LINE_WIDTH;
	 params[1] = (GLdouble) MAX_LINE_WIDTH;
	 break;
      case GL_LIST_BASE:
	 *params = (GLdouble) CC.List.ListBase;
	 break;
      case GL_LIST_INDEX:
	 *params = (GLdouble) gl_list_index();
	 break;
      case GL_LIST_MODE:
	 *params = CC.ExecuteFlag ? (GLdouble) GL_COMPILE_AND_EXECUTE
	   			  : (GLdouble) GL_COMPILE;
	 break;
      case GL_LOGIC_OP:
	 *params = (GLdouble) CC.Color.LogicOpEnabled;
	 break;
      case GL_LOGIC_OP_MODE:
         *params = (GLdouble) CC.Color.LogicOp;
	 break;
      case GL_MAP1_COLOR_4:
	 *params = (GLdouble) CC.Eval.Map1Color4;
	 break;
      case GL_MAP1_GRID_DOMAIN:
	 params[0] = (GLdouble) CC.Eval.MapGrid1u1;
	 params[1] = (GLdouble) CC.Eval.MapGrid1u2;
	 break;
      case GL_MAP1_GRID_SEGMENTS:
	 *params = (GLdouble) CC.Eval.MapGrid1un;
	 break;
      case GL_MAP1_INDEX:
	 *params = (GLdouble) CC.Eval.Map1Index;
	 break;
      case GL_MAP1_NORMAL:
	 *params = (GLdouble) CC.Eval.Map1Normal;
	 break;
      case GL_MAP1_TEXTURE_COORD_1:
	 *params = (GLdouble) CC.Eval.Map1TextureCoord1;
	 break;
      case GL_MAP1_TEXTURE_COORD_2:
	 *params = (GLdouble) CC.Eval.Map1TextureCoord2;
	 break;
      case GL_MAP1_TEXTURE_COORD_3:
	 *params = (GLdouble) CC.Eval.Map1TextureCoord3;
	 break;
      case GL_MAP1_TEXTURE_COORD_4:
	 *params = (GLdouble) CC.Eval.Map1TextureCoord4;
	 break;
      case GL_MAP1_VERTEX_3:
	 *params = (GLdouble) CC.Eval.Map1Vertex3;
	 break;
      case GL_MAP1_VERTEX_4:
	 *params = (GLdouble) CC.Eval.Map1Vertex4;
	 break;
      case GL_MAP2_COLOR_4:
	 *params = (GLdouble) CC.Eval.Map2Color4;
	 break;
      case GL_MAP2_GRID_DOMAIN:
	 params[0] = (GLdouble) CC.Eval.MapGrid2u1;
	 params[1] = (GLdouble) CC.Eval.MapGrid2u2;
	 params[2] = (GLdouble) CC.Eval.MapGrid2v1;
	 params[3] = (GLdouble) CC.Eval.MapGrid2v2;
	 break;
      case GL_MAP2_GRID_SEGMENTS:
	 params[0] = (GLdouble) CC.Eval.MapGrid2un;
	 params[1] = (GLdouble) CC.Eval.MapGrid2vn;
	 break;
      case GL_MAP2_INDEX:
	 *params = (GLdouble) CC.Eval.Map2Index;
	 break;
      case GL_MAP2_NORMAL:
	 *params = (GLdouble) CC.Eval.Map2Normal;
	 break;
      case GL_MAP2_TEXTURE_COORD_1:
	 *params = (GLdouble) CC.Eval.Map2TextureCoord1;
	 break;
      case GL_MAP2_TEXTURE_COORD_2:
	 *params = (GLdouble) CC.Eval.Map2TextureCoord2;
	 break;
      case GL_MAP2_TEXTURE_COORD_3:
	 *params = (GLdouble) CC.Eval.Map2TextureCoord3;
	 break;
      case GL_MAP2_TEXTURE_COORD_4:
	 *params = (GLdouble) CC.Eval.Map2TextureCoord4;
	 break;
      case GL_MAP2_VERTEX_3:
	 *params = (GLdouble) CC.Eval.Map2Vertex3;
	 break;
      case GL_MAP2_VERTEX_4:
	 *params = (GLdouble) CC.Eval.Map2Vertex4;
	 break;
      case GL_MAP_COLOR:
	 *params = (GLdouble) CC.Pixel.MapColorFlag;
	 break;
      case GL_MAP_STENCIL:
	 *params = (GLdouble) CC.Pixel.MapStencilFlag;
	 break;
      case GL_MATRIX_MODE:
	 *params = (GLdouble) CC.Transform.MatrixMode;
	 break;
      case GL_MAX_ATTRIB_STACK_DEPTH:
	 *params = (GLdouble) MAX_ATTRIB_STACK_DEPTH;
	 break;
      case GL_MAX_CLIP_PLANES:
	 *params = (GLdouble) MAX_CLIP_PLANES;
	 break;
      case GL_MAX_EVAL_ORDER:
	 *params = (GLdouble) MAX_EVAL_ORDER;
	 break;
      case GL_MAX_LIGHTS:
	 *params = (GLdouble) MAX_LIGHTS;
	 break;
      case GL_MAX_LIST_NESTING:
	 *params = (GLdouble) MAX_LIST_NESTING;
	 break;
      case GL_MAX_MODELVIEW_STACK_DEPTH:
	 *params = (GLdouble) MAX_MODELVIEW_STACK_DEPTH;
	 break;
      case GL_MAX_NAME_STACK_DEPTH:
	 *params = (GLdouble) MAX_NAME_STACK_DEPTH;
	 break;
      case GL_MAX_PIXEL_MAP_TABLE:
	 *params = (GLdouble) MAX_PIXEL_MAP_TABLE;
	 break;
      case GL_MAX_PROJECTION_STACK_DEPTH:
	 *params = (GLdouble) MAX_PROJECTION_STACK_DEPTH;
	 break;
      case GL_MAX_TEXTURE_SIZE:
	 *params = (GLdouble) MAX_TEXTURE_SIZE;
	 break;
      case GL_MAX_TEXTURE_STACK_DEPTH:
	 *params = (GLdouble) MAX_TEXTURE_STACK_DEPTH;
	 break;
      case GL_MAX_VIEWPORT_DIMS:
         params[0] = (GLdouble) MAX_WIDTH;
         params[1] = (GLdouble) MAX_HEIGHT;
         break;
      case GL_MODELVIEW_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = (GLdouble) CC.ModelViewMatrix[i];
	 }
	 break;
      case GL_MODELVIEW_STACK_DEPTH:
	 *params = (GLdouble) CC.ModelViewStackDepth;
	 break;
      case GL_NAME_STACK_DEPTH:
	 *params = (GLdouble) CC.NameStackDepth;
	 break;
      case GL_NORMALIZE:
	 *params = (GLdouble) CC.Transform.Normalize;
	 break;
      case GL_PACK_ALIGNMENT:
	 *params = (GLdouble) CC.PackAlignment;
	 break;
      case GL_PACK_LSB_FIRST:
	 *params = (GLdouble) CC.PackLSBFirst;
	 break;
      case GL_PACK_ROW_LENGTH:
	 *params = (GLdouble) CC.PackRowLength;
	 break;
      case GL_PACK_SKIP_PIXELS:
	 *params = (GLdouble) CC.PackSkipPixels;
	 break;
      case GL_PACK_SKIP_ROWS:
	 *params = (GLdouble) CC.PackSkipRows;
	 break;
      case GL_PACK_SWAP_BYTES:
	 *params = (GLdouble) CC.PackSwapBytes;
	 break;
      case GL_PERSPECTIVE_CORRECTION_HINT:
	 *params = (GLdouble) CC.Hint.PerspectiveCorrection;
	 break;
      case GL_PIXEL_MAP_A_TO_A_SIZE:
	 *params = (GLdouble) CC.Pixel.MapAtoAsize;
	 break;
      case GL_PIXEL_MAP_B_TO_B_SIZE:
	 *params = (GLdouble) CC.Pixel.MapBtoBsize;
	 break;
      case GL_PIXEL_MAP_G_TO_G_SIZE:
	 *params = (GLdouble) CC.Pixel.MapGtoGsize;
	 break;
      case GL_PIXEL_MAP_I_TO_A_SIZE:
	 *params = (GLdouble) CC.Pixel.MapItoAsize;
	 break;
      case GL_PIXEL_MAP_I_TO_B_SIZE:
	 *params = (GLdouble) CC.Pixel.MapItoBsize;
	 break;
      case GL_PIXEL_MAP_I_TO_G_SIZE:
	 *params = (GLdouble) CC.Pixel.MapItoGsize;
	 break;
      case GL_PIXEL_MAP_I_TO_I_SIZE:
	 *params = (GLdouble) CC.Pixel.MapItoIsize;
	 break;
      case GL_PIXEL_MAP_I_TO_R_SIZE:
	 *params = (GLdouble) CC.Pixel.MapItoRsize;
	 break;
      case GL_PIXEL_MAP_R_TO_R_SIZE:
	 *params = (GLdouble) CC.Pixel.MapRtoRsize;
	 break;
      case GL_PIXEL_MAP_S_TO_S_SIZE:
	 *params = (GLdouble) CC.Pixel.MapStoSsize;
	 break;
      case GL_POINT_SIZE:
         *params = (GLdouble) CC.Point.Size;
         break;
      case GL_POINT_SIZE_GRANULARITY:
	 *params = (GLdouble) POINT_SIZE_GRANULARITY;
	 break;
      case GL_POINT_SIZE_RANGE:
	 params[0] = (GLdouble) MIN_POINT_SIZE;
	 params[1] = (GLdouble) MAX_POINT_SIZE;
	 break;
      case GL_POINT_SMOOTH:
	 *params = (GLdouble) CC.Point.SmoothFlag;
	 break;
      case GL_POINT_SMOOTH_HINT:
	 *params = (GLdouble) CC.Hint.PointSmooth;
	 break;
      case GL_POLYGON_MODE:
	 params[0] = (GLdouble) CC.Polygon.FrontMode;
	 params[1] = (GLdouble) CC.Polygon.BackMode;
	 break;
      case GL_POLYGON_OFFSET_BIAS_EXT:
         *params = (GLdouble) CC.Polygon.OffsetBias;
         break;
      case GL_POLYGON_OFFSET_FACTOR_EXT:
         *params = (GLdouble) CC.Polygon.OffsetFactor;
         break;
      case GL_POLYGON_SMOOTH:
	 *params = (GLdouble) CC.Polygon.SmoothFlag;
	 break;
      case GL_POLYGON_SMOOTH_HINT:
	 *params = (GLdouble) CC.Hint.PolygonSmooth;
	 break;
      case GL_POLYGON_STIPPLE:
	 for (i=0;i<32;i++) {		/* RIGHT? */
	    params[i] = (GLdouble) CC.PolygonStipple[i];
	 }
	 break;
      case GL_PROJECTION_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = (GLdouble) CC.ProjectionMatrix[i];
	 }
	 break;
      case GL_PROJECTION_STACK_DEPTH:
	 *params = (GLdouble) CC.ProjectionStackDepth;
	 break;
      case GL_READ_BUFFER:
	 *params = (GLdouble) CC.Pixel.ReadBuffer;
	 break;
      case GL_RED_BIAS:
         *params = (GLdouble) CC.Pixel.RedBias;
         break;
      case GL_RED_BITS:
         *params = (GLdouble) bits( CC.RedScale );
         break;
      case GL_RED_SCALE:
         *params = (GLdouble) CC.Pixel.RedScale;
         break;
      case GL_RENDER_MODE:
	 *params = (GLdouble) CC.RenderMode;
	 break;
      case GL_RGBA_MODE:
	 *params = (GLdouble) CC.RGBAflag;
	 break;
      case GL_SCISSOR_BOX:
	 params[0] = (GLdouble) CC.Scissor.X;
	 params[1] = (GLdouble) CC.Scissor.Y;
	 params[2] = (GLdouble) CC.Scissor.Width;
	 params[3] = (GLdouble) CC.Scissor.Height;
	 break;
      case GL_SCISSOR_TEST:
	 *params = (GLdouble) CC.Scissor.Enabled;
	 break;
      case GL_SHADE_MODEL:
	 *params = (GLdouble) CC.Light.ShadeModel;
	 break;
      case GL_STENCIL_BITS:
         *params = (GLdouble) (8*sizeof(GLstencil));
         break;
      case GL_STENCIL_CLEAR_VALUE:
	 *params = (GLdouble) CC.Stencil.Clear;
	 break;
      case GL_STENCIL_FAIL:
	 *params = (GLdouble) CC.Stencil.FailFunc;
	 break;
      case GL_STENCIL_FUNC:
	 *params = (GLdouble) CC.Stencil.Function;
	 break;
      case GL_STENCIL_PASS_DEPTH_FAIL:
	 *params = (GLdouble) CC.Stencil.ZFailFunc;
	 break;
      case GL_STENCIL_PASS_DEPTH_PASS:
	 *params = (GLdouble) CC.Stencil.ZPassFunc;
	 break;
      case GL_STENCIL_REF:
	 *params = (GLdouble) CC.Stencil.Ref;
	 break;
      case GL_STENCIL_TEST:
	 *params = (GLdouble) CC.Stencil.Enabled;
	 break;
      case GL_STENCIL_VALUE_MASK:
	 *params = (GLdouble) CC.Stencil.ValueMask;
	 break;
      case GL_STENCIL_WRITEMASK:
	 *params = (GLdouble) CC.Stencil.WriteMask;
	 break;
      case GL_STEREO:
	 *params = 0.0;   /* TODO */
	 break;
      case GL_SUBPIXEL_BITS:
	 *params = 0.0;   /* TODO */
	 break;
      case GL_TEXTURE_1D:
	 *params = (CC.Texture.Enabled & 1) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_2D:
	 *params = (CC.Texture.Enabled & 2) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_ENV_COLOR:
	 params[0] = (GLdouble) CC.Texture.EnvColor[0];
	 params[1] = (GLdouble) CC.Texture.EnvColor[1];
	 params[2] = (GLdouble) CC.Texture.EnvColor[2];
	 params[3] = (GLdouble) CC.Texture.EnvColor[3];
	 break;
      case GL_TEXTURE_ENV_MODE:
	 *params = (GLdouble) CC.Texture.EnvMode;
	 break;
      case GL_TEXTURE_GEN_S:
	 *params = (CC.Texture.TexGenEnabled & S_BIT) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_GEN_T:
	 *params = (CC.Texture.TexGenEnabled & T_BIT) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_GEN_R:
	 *params = (CC.Texture.TexGenEnabled & R_BIT) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_GEN_Q:
	 *params = (CC.Texture.TexGenEnabled & Q_BIT) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_MATRIX:
         for (i=0;i<16;i++) {
	    params[i] = (GLdouble) CC.TextureMatrix[i];
	 }
	 break;
      case GL_TEXTURE_STACK_DEPTH:
	 *params = (GLdouble) CC.TextureStackDepth;
	 break;
      case GL_UNPACK_ALIGNMENT:
	 *params = (GLdouble) CC.UnpackAlignment;
	 break;
      case GL_UNPACK_LSB_FIRST:
	 *params = (GLdouble) CC.UnpackLSBFirst;
	 break;
      case GL_UNPACK_ROW_LENGTH:
	 *params = (GLdouble) CC.UnpackRowLength;
	 break;
      case GL_UNPACK_SKIP_PIXELS:
	 *params = (GLdouble) CC.UnpackSkipPixels;
	 break;
      case GL_UNPACK_SKIP_ROWS:
	 *params = (GLdouble) CC.UnpackSkipRows;
	 break;
      case GL_UNPACK_SWAP_BYTES:
	 *params = (GLdouble) CC.UnpackSwapBytes;
	 break;
      case GL_VIEWPORT:
	 params[0] = (GLdouble) CC.Viewport.X;
	 params[1] = (GLdouble) CC.Viewport.Y;
	 params[2] = (GLdouble) CC.Viewport.Width;
	 params[3] = (GLdouble) CC.Viewport.Height;
	 break;
      case GL_ZOOM_X:
	 *params = (GLdouble) CC.Pixel.ZoomX;
	 break;
      case GL_ZOOM_Y:
	 *params = (GLdouble) CC.Pixel.ZoomY;
	 break;
      case GL_VERTEX_ARRAY_SIZE_EXT:
         *params = (GLdouble) CC.Array.VertexSize;
         break;
      case GL_VERTEX_ARRAY_TYPE_EXT:
         *params = (GLdouble) CC.Array.VertexType;
         break;
      case GL_VERTEX_ARRAY_STRIDE_EXT:
         *params = (GLdouble) CC.Array.VertexStride;
         break;
      case GL_VERTEX_ARRAY_COUNT_EXT:
         *params = (GLdouble) CC.Array.VertexCount;
         break;
      case GL_NORMAL_ARRAY_TYPE_EXT:
         *params = (GLdouble) CC.Array.NormalType;
         break;
      case GL_NORMAL_ARRAY_STRIDE_EXT:
         *params = (GLdouble) CC.Array.NormalStride;
         break;
      case GL_NORMAL_ARRAY_COUNT_EXT:
         *params = (GLdouble) CC.Array.NormalCount;
         break;
      case GL_COLOR_ARRAY_SIZE_EXT:
         *params = (GLdouble) CC.Array.ColorSize;
         break;
      case GL_COLOR_ARRAY_TYPE_EXT:
         *params = (GLdouble) CC.Array.ColorType;
         break;
      case GL_COLOR_ARRAY_STRIDE_EXT:
         *params = (GLdouble) CC.Array.ColorStride;
         break;
      case GL_COLOR_ARRAY_COUNT_EXT:
         *params = (GLdouble) CC.Array.ColorCount;
         break;
      case GL_INDEX_ARRAY_TYPE_EXT:
         *params = (GLdouble) CC.Array.IndexType;
         break;
      case GL_INDEX_ARRAY_STRIDE_EXT:
         *params = (GLdouble) CC.Array.IndexStride;
         break;
      case GL_INDEX_ARRAY_COUNT_EXT:
         *params = (GLdouble) CC.Array.IndexCount;
         break;
      case GL_TEXTURE_COORD_ARRAY_SIZE_EXT:
         *params = (GLdouble) CC.Array.TexCoordSize;
         break;
      case GL_TEXTURE_COORD_ARRAY_TYPE_EXT:
         *params = (GLdouble) CC.Array.TexCoordType;
         break;
      case GL_TEXTURE_COORD_ARRAY_STRIDE_EXT:
         *params = (GLdouble) CC.Array.TexCoordStride;
         break;
      case GL_TEXTURE_COORD_ARRAY_COUNT_EXT:
         *params = (GLdouble) CC.Array.TexCoordCount;
         break;
      case GL_EDGE_FLAG_ARRAY_STRIDE_EXT:
         *params = (GLdouble) CC.Array.EdgeFlagStride;
         break;
      case GL_EDGE_FLAG_ARRAY_COUNT_EXT:
         *params = (GLdouble) CC.Array.EdgeFlagCount;
         break;

      default:
         gl_error( GL_INVALID_ENUM, "glGetDoublev" );
   }
}




void glGetFloatv( GLenum pname, GLfloat *params )
{
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetFloatv" );
      return;
   }
   switch (pname) {
      case GL_ACCUM_RED_BITS:
      case GL_ACCUM_GREEN_BITS:
      case GL_ACCUM_BLUE_BITS:
      case GL_ACCUM_ALPHA_BITS:
         *params = (GLfloat) (sizeof(GLaccum)*8);
         break;
      case GL_ACCUM_CLEAR_VALUE:
         params[0] = CC.Accum.ClearColor[0];
         params[1] = CC.Accum.ClearColor[1];
         params[2] = CC.Accum.ClearColor[2];
         params[3] = CC.Accum.ClearColor[3];
         break;
      case GL_ALPHA_BIAS:
         *params = CC.Pixel.AlphaBias;
         break;
      case GL_ALPHA_BITS:
         if (CC.FrontAlphaEnabled || CC.BackAlphaEnabled) {
            *params = 8*sizeof(GLubyte);
         }
         else {
            *params = 0.0F;
         }
         break;
      case GL_ALPHA_SCALE:
         *params = CC.Pixel.AlphaScale;
         break;
      case GL_ALPHA_TEST:
         *params = (GLfloat) CC.Color.AlphaEnabled;
         break;
      case GL_ALPHA_TEST_FUNC:
         *params = (GLfloat) CC.Color.AlphaFunc;
         break;
      case GL_ALPHA_TEST_REF:
         *params = (GLfloat) CC.Color.AlphaRef;
         break;
      case GL_ATTRIB_STACK_DEPTH:
         *params = (GLfloat ) CC.AttribStackDepth;
         break;
      case GL_AUTO_NORMAL:
         *params = (GLfloat) CC.Eval.AutoNormal;
         break;
      case GL_AUX_BUFFERS:
         *params = (GLfloat) NUM_AUX_BUFFERS;
         break;
      case GL_BLEND:
         *params = (GLfloat) CC.Color.BlendEnabled;
         break;
      case GL_BLEND_DST:
         *params = (GLfloat) CC.Color.BlendDst;
         break;
      case GL_BLEND_SRC:
         *params = (GLfloat) CC.Color.BlendSrc;
         break;
      case GL_BLEND_EQUATION_EXT:
	 *params = (GLfloat) CC.Color.BlendEquation;
	 break;
      case GL_BLEND_COLOR_EXT:
	 params[0] = CC.Color.BlendColor[0];
	 params[1] = CC.Color.BlendColor[1];
	 params[2] = CC.Color.BlendColor[2];
	 params[3] = CC.Color.BlendColor[3];
	 break;
      case GL_BLUE_BIAS:
         *params = CC.Pixel.BlueBias;
         break;
      case GL_BLUE_BITS:
         *params = (GLfloat) bits( CC.BlueScale );
         break;
      case GL_BLUE_SCALE:
         *params = CC.Pixel.BlueScale;
         break;
      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:
         *params = (GLfloat) CC.Transform.ClipEnabled[pname-GL_CLIP_PLANE0];
         break;
      case GL_COLOR_CLEAR_VALUE:
         params[0] = (GLfloat) CC.Color.ClearColor[0];
         params[1] = (GLfloat) CC.Color.ClearColor[1];
         params[2] = (GLfloat) CC.Color.ClearColor[2];
         params[3] = (GLfloat) CC.Color.ClearColor[3];
         break;
      case GL_COLOR_MATERIAL:
         *params = (GLfloat) CC.Light.ColorMaterialEnabled;
         break;
      case GL_COLOR_MATERIAL_FACE:
         *params = (GLfloat) CC.Light.ColorMaterialFace;
         break;
      case GL_COLOR_MATERIAL_PARAMETER:
         *params = (GLfloat) CC.Light.ColorMaterialMode;
         break;
      case GL_COLOR_WRITEMASK:
         params[0] = (CC.Color.ColorMask & 8) ? 1.0F : 0.0F;
         params[1] = (CC.Color.ColorMask & 4) ? 1.0F : 0.0F;
         params[2] = (CC.Color.ColorMask & 2) ? 1.0F : 0.0F;
         params[3] = (CC.Color.ColorMask & 1) ? 1.0F : 0.0F;
         break;
      case GL_CULL_FACE:
         *params = (GLfloat) CC.Polygon.CullFlag;
         break;
      case GL_CULL_FACE_MODE:
         *params = (GLfloat) CC.Polygon.CullFaceMode;
         break;
      case GL_CURRENT_COLOR:
         params[0] = CC.Current.IntColor[0] / CC.RedScale;
         params[1] = CC.Current.IntColor[1] / CC.GreenScale;
         params[2] = CC.Current.IntColor[2] / CC.BlueScale;
         params[3] = CC.Current.IntColor[3] / CC.AlphaScale;
         break;
      case GL_CURRENT_INDEX:
         *params = (GLfloat) CC.Current.Index;
         break;
      case GL_CURRENT_NORMAL:
         params[0] = CC.Current.Normal[0];
         params[1] = CC.Current.Normal[1];
         params[2] = CC.Current.Normal[2];
         break;
      case GL_CURRENT_RASTER_COLOR:
	 params[0] = CC.Current.RasterColor[0];
	 params[1] = CC.Current.RasterColor[1];
	 params[2] = CC.Current.RasterColor[2];
	 params[3] = CC.Current.RasterColor[3];
	 break;
      case GL_CURRENT_RASTER_DISTANCE:
	 params[0] = CC.Current.RasterDistance;
	 break;
      case GL_CURRENT_RASTER_INDEX:
	 *params = (GLfloat) CC.Current.RasterIndex;
	 break;
      case GL_CURRENT_RASTER_POSITION:
	 params[0] = CC.Current.RasterPos[0];
	 params[1] = CC.Current.RasterPos[1];
	 params[2] = CC.Current.RasterPos[2];
	 params[3] = CC.Current.RasterPos[3];
	 break;
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
	 params[0] = CC.Current.RasterTexCoord[0];
	 params[1] = CC.Current.RasterTexCoord[1];
	 params[2] = CC.Current.RasterTexCoord[2];
	 params[3] = CC.Current.RasterTexCoord[3];
	 break;
      case GL_CURRENT_RASTER_POSITION_VALID:
	 *params = (GLfloat) CC.Current.RasterPosValid;
	 break;
      case GL_CURRENT_TEXTURE_COORDS:
	 params[0] = (GLfloat) CC.Current.TexCoord[0];
	 params[1] = (GLfloat) CC.Current.TexCoord[1];
	 params[2] = (GLfloat) CC.Current.TexCoord[2];
	 params[3] = (GLfloat) CC.Current.TexCoord[3];
	 break;
      case GL_DEPTH_BIAS:
	 *params = (GLfloat) CC.Pixel.DepthBias;
	 break;
      case GL_DEPTH_BITS:
	 *params = (GLfloat) (8*sizeof(GLdepth));
	 break;
      case GL_DEPTH_CLEAR_VALUE:
	 *params = (GLfloat) CC.Depth.Clear;
	 break;
      case GL_DEPTH_FUNC:
	 *params = (GLfloat) CC.Depth.Func;
	 break;
      case GL_DEPTH_RANGE:
         params[0] = (GLfloat) CC.Viewport.Near;
         params[1] = (GLfloat) CC.Viewport.Far;
	 break;
      case GL_DEPTH_SCALE:
	 *params = (GLfloat) CC.Pixel.DepthScale;
	 break;
      case GL_DEPTH_TEST:
	 *params = (GLfloat) CC.Depth.Test;
	 break;
      case GL_DEPTH_WRITEMASK:
	 *params = (GLfloat) CC.Depth.Mask;
	 break;
      case GL_DITHER:
	 *params = (GLfloat) CC.Color.DitherFlag;
	 break;
      case GL_DOUBLEBUFFER:
	 *params = (GLfloat) CC.DBflag;
	 break;
      case GL_DRAW_BUFFER:
	 *params = (GLfloat) CC.Color.DrawBuffer;
	 break;
      case GL_EDGE_FLAG:
	 *params = (GLfloat) CC.Current.EdgeFlag;
	 break;
      case GL_FOG:
	 *params = (GLfloat) CC.Fog.Enabled;
	 break;
      case GL_FOG_COLOR:
	 params[0] = CC.Fog.Color[0];
	 params[1] = CC.Fog.Color[1];
	 params[2] = CC.Fog.Color[2];
	 params[3] = CC.Fog.Color[3];
	 break;
      case GL_FOG_DENSITY:
	 *params = CC.Fog.Density;
	 break;
      case GL_FOG_END:
	 *params = CC.Fog.End;
	 break;
      case GL_FOG_HINT:
	 *params = (GLfloat) CC.Hint.Fog;
	 break;
      case GL_FOG_INDEX:
	 *params = CC.Fog.Index;
	 break;
      case GL_FOG_MODE:
	 *params = (GLfloat) CC.Fog.Mode;
	 break;
      case GL_FOG_START:
	 *params = CC.Fog.Start;
	 break;
      case GL_FRONT_FACE:
	 *params = (GLfloat) CC.Polygon.FrontFace;
	 break;
      case GL_GREEN_BIAS:
         *params = (GLfloat) CC.Pixel.GreenBias;
         break;
      case GL_GREEN_BITS:
         *params = (GLfloat) bits( CC.GreenScale );
         break;
      case GL_GREEN_SCALE:
         *params = (GLfloat) CC.Pixel.GreenScale;
         break;
      case GL_INDEX_BITS:
	 {
	    GLuint width, height, depth;
	    (*DD.buffer_size)( &width, &height, &depth );
	    *params = (GLfloat) depth;
	 }
	 break;
      case GL_INDEX_CLEAR_VALUE:
         *params = (GLfloat) CC.Color.ClearIndex;
	 break;
      case GL_INDEX_MODE:
	 *params = CC.RGBAflag ? 0.0F : 1.0F;
	 break;
      case GL_INDEX_OFFSET:
	 *params = (GLfloat) CC.Pixel.IndexOffset;
	 break;
      case GL_INDEX_SHIFT:
	 *params = (GLfloat) CC.Pixel.IndexShift;
	 break;
      case GL_INDEX_WRITEMASK:
	 *params = (GLfloat) CC.Color.IndexMask;
	 break;
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
	 *params = (GLfloat) CC.Light.Light[pname-GL_LIGHT0].Enabled;
	 break;
      case GL_LIGHTING:
	 *params = (GLfloat) CC.Light.Enabled;
	 break;
      case GL_LIGHT_MODEL_AMBIENT:
	 params[0] = CC.Light.Model.Ambient[0];
	 params[1] = CC.Light.Model.Ambient[1];
	 params[2] = CC.Light.Model.Ambient[2];
	 params[3] = CC.Light.Model.Ambient[3];
	 break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
	 *params = (GLfloat) CC.Light.Model.LocalViewer;
	 break;
      case GL_LIGHT_MODEL_TWO_SIDE:
	 *params = (GLfloat) CC.Light.Model.TwoSide;
	 break;
      case GL_LINE_SMOOTH:
	 *params = (GLfloat) CC.Line.SmoothFlag;
	 break;
      case GL_LINE_SMOOTH_HINT:
	 *params = (GLfloat) CC.Hint.LineSmooth;
	 break;
      case GL_LINE_STIPPLE:
	 *params = (GLfloat) CC.Line.StippleFlag;
	 break;
      case GL_LINE_STIPPLE_PATTERN:
         *params = (GLfloat) CC.Line.StipplePattern;
         break;
      case GL_LINE_STIPPLE_REPEAT:
         *params = (GLfloat) CC.Line.StippleFactor;
         break;
      case GL_LINE_WIDTH:
	 *params = (GLfloat) CC.Line.Width;
	 break;
      case GL_LINE_WIDTH_GRANULARITY:
	 *params = (GLfloat) LINE_WIDTH_GRANULARITY;
	 break;
      case GL_LINE_WIDTH_RANGE:
	 params[0] = (GLfloat) MIN_LINE_WIDTH;
	 params[1] = (GLfloat) MAX_LINE_WIDTH;
	 break;
      case GL_LIST_BASE:
	 *params = (GLfloat) CC.List.ListBase;
	 break;
      case GL_LIST_INDEX:
	 *params = (GLfloat) gl_list_index();
	 break;
      case GL_LIST_MODE:
	 *params = CC.ExecuteFlag ? (GLfloat) GL_COMPILE_AND_EXECUTE
	   			  : (GLfloat) GL_COMPILE;
	 break;
      case GL_LOGIC_OP:
	 *params = (GLfloat) CC.Color.LogicOpEnabled;
	 break;
      case GL_LOGIC_OP_MODE:
         *params = (GLfloat) CC.Color.LogicOp;
	 break;
      case GL_MAP1_COLOR_4:
	 *params = (GLfloat) CC.Eval.Map1Color4;
	 break;
      case GL_MAP1_GRID_DOMAIN:
	 params[0] = CC.Eval.MapGrid1u1;
	 params[1] = CC.Eval.MapGrid1u2;
	 break;
      case GL_MAP1_GRID_SEGMENTS:
	 *params = (GLfloat) CC.Eval.MapGrid1un;
	 break;
      case GL_MAP1_INDEX:
	 *params = (GLfloat) CC.Eval.Map1Index;
	 break;
      case GL_MAP1_NORMAL:
	 *params = (GLfloat) CC.Eval.Map1Normal;
	 break;
      case GL_MAP1_TEXTURE_COORD_1:
	 *params = (GLfloat) CC.Eval.Map1TextureCoord1;
	 break;
      case GL_MAP1_TEXTURE_COORD_2:
	 *params = (GLfloat) CC.Eval.Map1TextureCoord2;
	 break;
      case GL_MAP1_TEXTURE_COORD_3:
	 *params = (GLfloat) CC.Eval.Map1TextureCoord3;
	 break;
      case GL_MAP1_TEXTURE_COORD_4:
	 *params = (GLfloat) CC.Eval.Map1TextureCoord4;
	 break;
      case GL_MAP1_VERTEX_3:
	 *params = (GLfloat) CC.Eval.Map1Vertex3;
	 break;
      case GL_MAP1_VERTEX_4:
	 *params = (GLfloat) CC.Eval.Map1Vertex4;
	 break;
      case GL_MAP2_COLOR_4:
	 *params = (GLfloat) CC.Eval.Map2Color4;
	 break;
      case GL_MAP2_GRID_DOMAIN:
	 params[0] = CC.Eval.MapGrid2u1;
	 params[1] = CC.Eval.MapGrid2u2;
	 params[2] = CC.Eval.MapGrid2v1;
	 params[3] = CC.Eval.MapGrid2v2;
	 break;
      case GL_MAP2_GRID_SEGMENTS:
	 params[0] = (GLfloat) CC.Eval.MapGrid2un;
	 params[1] = (GLfloat) CC.Eval.MapGrid2vn;
	 break;
      case GL_MAP2_INDEX:
	 *params = (GLfloat) CC.Eval.Map2Index;
	 break;
      case GL_MAP2_NORMAL:
	 *params = (GLfloat) CC.Eval.Map2Normal;
	 break;
      case GL_MAP2_TEXTURE_COORD_1:
	 *params = CC.Eval.Map2TextureCoord1;
	 break;
      case GL_MAP2_TEXTURE_COORD_2:
	 *params = CC.Eval.Map2TextureCoord2;
	 break;
      case GL_MAP2_TEXTURE_COORD_3:
	 *params = CC.Eval.Map2TextureCoord3;
	 break;
      case GL_MAP2_TEXTURE_COORD_4:
	 *params = CC.Eval.Map2TextureCoord4;
	 break;
      case GL_MAP2_VERTEX_3:
	 *params = (GLfloat) CC.Eval.Map2Vertex3;
	 break;
      case GL_MAP2_VERTEX_4:
	 *params = (GLfloat) CC.Eval.Map2Vertex4;
	 break;
      case GL_MAP_COLOR:
	 *params = (GLfloat) CC.Pixel.MapColorFlag;
	 break;
      case GL_MAP_STENCIL:
	 *params = (GLfloat) CC.Pixel.MapStencilFlag;
	 break;
      case GL_MATRIX_MODE:
	 *params = (GLfloat) CC.Transform.MatrixMode;
	 break;
      case GL_MAX_ATTRIB_STACK_DEPTH:
	 *params = (GLfloat) MAX_ATTRIB_STACK_DEPTH;
	 break;
      case GL_MAX_CLIP_PLANES:
	 *params = (GLfloat) MAX_CLIP_PLANES;
	 break;
      case GL_MAX_EVAL_ORDER:
	 *params = (GLfloat) MAX_EVAL_ORDER;
	 break;
      case GL_MAX_LIGHTS:
	 *params = (GLfloat) MAX_LIGHTS;
	 break;
      case GL_MAX_LIST_NESTING:
	 *params = (GLfloat) MAX_LIST_NESTING;
	 break;
      case GL_MAX_MODELVIEW_STACK_DEPTH:
	 *params = (GLfloat) MAX_MODELVIEW_STACK_DEPTH;
	 break;
      case GL_MAX_NAME_STACK_DEPTH:
	 *params = (GLfloat) MAX_NAME_STACK_DEPTH;
	 break;
      case GL_MAX_PIXEL_MAP_TABLE:
	 *params = (GLfloat) MAX_PIXEL_MAP_TABLE;
	 break;
      case GL_MAX_PROJECTION_STACK_DEPTH:
	 *params = (GLfloat) MAX_PROJECTION_STACK_DEPTH;
	 break;
      case GL_MAX_TEXTURE_SIZE:
	 *params = (GLfloat) MAX_TEXTURE_SIZE;
	 break;
      case GL_MAX_TEXTURE_STACK_DEPTH:
	 *params = (GLfloat) MAX_TEXTURE_STACK_DEPTH;
	 break;
      case GL_MAX_VIEWPORT_DIMS:
         params[0] = (GLfloat) MAX_WIDTH;
         params[1] = (GLfloat) MAX_HEIGHT;
         break;
      case GL_MODELVIEW_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = CC.ModelViewMatrix[i];
	 }
	 break;
      case GL_MODELVIEW_STACK_DEPTH:
	 *params = (GLfloat) CC.ModelViewStackDepth;
	 break;
      case GL_NAME_STACK_DEPTH:
	 *params = (GLfloat) CC.NameStackDepth;
	 break;
      case GL_NORMALIZE:
	 *params = (GLfloat) CC.Transform.Normalize;
	 break;
      case GL_PACK_ALIGNMENT:
	 *params = (GLfloat) CC.PackAlignment;
	 break;
      case GL_PACK_LSB_FIRST:
	 *params = (GLfloat) CC.PackLSBFirst;
	 break;
      case GL_PACK_ROW_LENGTH:
	 *params = (GLfloat) CC.PackRowLength;
	 break;
      case GL_PACK_SKIP_PIXELS:
	 *params = (GLfloat) CC.PackSkipPixels;
	 break;
      case GL_PACK_SKIP_ROWS:
	 *params = (GLfloat) CC.PackSkipRows;
	 break;
      case GL_PACK_SWAP_BYTES:
	 *params = (GLfloat) CC.PackSwapBytes;
	 break;
      case GL_PERSPECTIVE_CORRECTION_HINT:
	 *params = (GLfloat) CC.Hint.PerspectiveCorrection;
	 break;
      case GL_PIXEL_MAP_A_TO_A_SIZE:
	 *params = (GLfloat) CC.Pixel.MapAtoAsize;
	 break;
      case GL_PIXEL_MAP_B_TO_B_SIZE:
	 *params = (GLfloat) CC.Pixel.MapBtoBsize;
	 break;
      case GL_PIXEL_MAP_G_TO_G_SIZE:
	 *params = (GLfloat) CC.Pixel.MapGtoGsize;
	 break;
      case GL_PIXEL_MAP_I_TO_A_SIZE:
	 *params = (GLfloat) CC.Pixel.MapItoAsize;
	 break;
      case GL_PIXEL_MAP_I_TO_B_SIZE:
	 *params = (GLfloat) CC.Pixel.MapItoBsize;
	 break;
      case GL_PIXEL_MAP_I_TO_G_SIZE:
	 *params = (GLfloat) CC.Pixel.MapItoGsize;
	 break;
      case GL_PIXEL_MAP_I_TO_I_SIZE:
	 *params = (GLfloat) CC.Pixel.MapItoIsize;
	 break;
      case GL_PIXEL_MAP_I_TO_R_SIZE:
	 *params = (GLfloat) CC.Pixel.MapItoRsize;
	 break;
      case GL_PIXEL_MAP_R_TO_R_SIZE:
	 *params = (GLfloat) CC.Pixel.MapRtoRsize;
	 break;
      case GL_PIXEL_MAP_S_TO_S_SIZE:
	 *params = (GLfloat) CC.Pixel.MapStoSsize;
	 break;
      case GL_POINT_SIZE:
         *params = (GLfloat) CC.Point.Size;
         break;
      case GL_POINT_SIZE_GRANULARITY:
	 *params = (GLfloat) POINT_SIZE_GRANULARITY;
	 break;
      case GL_POINT_SIZE_RANGE:
	 params[0] = (GLfloat) MIN_POINT_SIZE;
	 params[1] = (GLfloat) MAX_POINT_SIZE;
	 break;
      case GL_POINT_SMOOTH:
	 *params = (GLfloat) CC.Point.SmoothFlag;
	 break;
      case GL_POINT_SMOOTH_HINT:
	 *params = (GLfloat) CC.Hint.PointSmooth;
	 break;
      case GL_POLYGON_MODE:
	 params[0] = (GLfloat) CC.Polygon.FrontMode;
	 params[1] = (GLfloat) CC.Polygon.BackMode;
	 break;
      case GL_POLYGON_OFFSET_BIAS_EXT:
         *params = CC.Polygon.OffsetBias;
         break;
      case GL_POLYGON_OFFSET_FACTOR_EXT:
         *params = CC.Polygon.OffsetFactor;
         break;
      case GL_POLYGON_SMOOTH:
	 *params = (GLfloat) CC.Polygon.SmoothFlag;
	 break;
      case GL_POLYGON_SMOOTH_HINT:
	 *params = (GLfloat) CC.Hint.PolygonSmooth;
	 break;
      case GL_POLYGON_STIPPLE:
	 for (i=0;i<32;i++) {		/* RIGHT? */
	    params[i] = (GLfloat) CC.PolygonStipple[i];
	 }
	 break;
      case GL_PROJECTION_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = CC.ProjectionMatrix[i];
	 }
	 break;
      case GL_PROJECTION_STACK_DEPTH:
	 *params = (GLfloat) CC.ProjectionStackDepth;
	 break;
      case GL_READ_BUFFER:
	 *params = (GLfloat) CC.Pixel.ReadBuffer;
	 break;
      case GL_RED_BIAS:
         *params = CC.Pixel.RedBias;
         break;
      case GL_RED_BITS:
         *params = (GLfloat) bits( CC.RedScale );
         break;
      case GL_RED_SCALE:
         *params = CC.Pixel.RedScale;
         break;
      case GL_RENDER_MODE:
	 *params = (GLfloat) CC.RenderMode;
	 break;
      case GL_RGBA_MODE:
	 *params = (GLfloat) CC.RGBAflag;
	 break;
      case GL_SCISSOR_BOX:
	 params[0] = (GLfloat) CC.Scissor.X;
	 params[1] = (GLfloat) CC.Scissor.Y;
	 params[2] = (GLfloat) CC.Scissor.Width;
	 params[3] = (GLfloat) CC.Scissor.Height;
	 break;
      case GL_SCISSOR_TEST:
	 *params = (GLfloat) CC.Scissor.Enabled;
	 break;
      case GL_SHADE_MODEL:
	 *params = (GLfloat) CC.Light.ShadeModel;
	 break;
      case GL_STENCIL_BITS:
         *params = (GLfloat) (8*sizeof(GLstencil));
         break;
      case GL_STENCIL_CLEAR_VALUE:
	 *params = (GLfloat) CC.Stencil.Clear;
	 break;
      case GL_STENCIL_FAIL:
	 *params = (GLfloat) CC.Stencil.FailFunc;
	 break;
      case GL_STENCIL_FUNC:
	 *params = (GLfloat) CC.Stencil.Function;
	 break;
      case GL_STENCIL_PASS_DEPTH_FAIL:
	 *params = (GLfloat) CC.Stencil.ZFailFunc;
	 break;
      case GL_STENCIL_PASS_DEPTH_PASS:
	 *params = (GLfloat) CC.Stencil.ZPassFunc;
	 break;
      case GL_STENCIL_REF:
	 *params = (GLfloat) CC.Stencil.Ref;
	 break;
      case GL_STENCIL_TEST:
	 *params = (GLfloat) CC.Stencil.Enabled;
	 break;
      case GL_STENCIL_VALUE_MASK:
	 *params = (GLfloat) CC.Stencil.ValueMask;
	 break;
      case GL_STENCIL_WRITEMASK:
	 *params = (GLfloat) CC.Stencil.WriteMask;
	 break;
      case GL_STEREO:
	 *params = 0.0F;  /* TODO */
	 break;
      case GL_SUBPIXEL_BITS:
	 *params = 0.0F;  /* TODO */
	 break;
      case GL_TEXTURE_1D:
	 *params = (CC.Texture.Enabled & 1) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_2D:
	 *params = (CC.Texture.Enabled & 2) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_ENV_COLOR:
	 params[0] = CC.Texture.EnvColor[0];
	 params[1] = CC.Texture.EnvColor[1];
	 params[2] = CC.Texture.EnvColor[2];
	 params[3] = CC.Texture.EnvColor[3];
	 break;
      case GL_TEXTURE_ENV_MODE:
	 *params = (GLfloat) CC.Texture.EnvMode;
	 break;
      case GL_TEXTURE_GEN_S:
	 *params = (CC.Texture.TexGenEnabled & S_BIT) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_GEN_T:
	 *params = (CC.Texture.TexGenEnabled & T_BIT) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_GEN_R:
	 *params = (CC.Texture.TexGenEnabled & R_BIT) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_GEN_Q:
	 *params = (CC.Texture.TexGenEnabled & Q_BIT) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_MATRIX:
         for (i=0;i<16;i++) {
	    params[i] = CC.TextureMatrix[i];
	 }
	 break;
      case GL_TEXTURE_STACK_DEPTH:
	 *params = (GLfloat) CC.TextureStackDepth;
	 break;
      case GL_UNPACK_ALIGNMENT:
	 *params = (GLfloat) CC.UnpackAlignment;
	 break;
      case GL_UNPACK_LSB_FIRST:
	 *params = (GLfloat) CC.UnpackLSBFirst;
	 break;
      case GL_UNPACK_ROW_LENGTH:
	 *params = (GLfloat) CC.UnpackRowLength;
	 break;
      case GL_UNPACK_SKIP_PIXELS:
	 *params = (GLfloat) CC.UnpackSkipPixels;
	 break;
      case GL_UNPACK_SKIP_ROWS:
	 *params = (GLfloat) CC.UnpackSkipRows;
	 break;
      case GL_UNPACK_SWAP_BYTES:
	 *params = (GLfloat) CC.UnpackSwapBytes;
	 break;
      case GL_VIEWPORT:
	 params[0] = (GLfloat) CC.Viewport.X;
	 params[1] = (GLfloat) CC.Viewport.Y;
	 params[2] = (GLfloat) CC.Viewport.Width;
	 params[3] = (GLfloat) CC.Viewport.Height;
	 break;
      case GL_ZOOM_X:
	 *params = (GLfloat) CC.Pixel.ZoomX;
	 break;
      case GL_ZOOM_Y:
	 *params = (GLfloat) CC.Pixel.ZoomY;
	 break;
      case GL_VERTEX_ARRAY_SIZE_EXT:
         *params = (GLfloat) CC.Array.VertexSize;
         break;
      case GL_VERTEX_ARRAY_TYPE_EXT:
         *params = (GLfloat) CC.Array.VertexType;
         break;
      case GL_VERTEX_ARRAY_STRIDE_EXT:
         *params = (GLfloat) CC.Array.VertexStride;
         break;
      case GL_VERTEX_ARRAY_COUNT_EXT:
         *params = (GLfloat) CC.Array.VertexCount;
         break;
      case GL_NORMAL_ARRAY_TYPE_EXT:
         *params = (GLfloat) CC.Array.NormalType;
         break;
      case GL_NORMAL_ARRAY_STRIDE_EXT:
         *params = (GLfloat) CC.Array.NormalStride;
         break;
      case GL_NORMAL_ARRAY_COUNT_EXT:
         *params = (GLfloat) CC.Array.NormalCount;
         break;
      case GL_COLOR_ARRAY_SIZE_EXT:
         *params = (GLfloat) CC.Array.ColorSize;
         break;
      case GL_COLOR_ARRAY_TYPE_EXT:
         *params = (GLfloat) CC.Array.ColorType;
         break;
      case GL_COLOR_ARRAY_STRIDE_EXT:
         *params = (GLfloat) CC.Array.ColorStride;
         break;
      case GL_COLOR_ARRAY_COUNT_EXT:
         *params = (GLfloat) CC.Array.ColorCount;
         break;
      case GL_INDEX_ARRAY_TYPE_EXT:
         *params = (GLfloat) CC.Array.IndexType;
         break;
      case GL_INDEX_ARRAY_STRIDE_EXT:
         *params = (GLfloat) CC.Array.IndexStride;
         break;
      case GL_INDEX_ARRAY_COUNT_EXT:
         *params = (GLfloat) CC.Array.IndexCount;
         break;
      case GL_TEXTURE_COORD_ARRAY_SIZE_EXT:
         *params = (GLfloat) CC.Array.TexCoordSize;
         break;
      case GL_TEXTURE_COORD_ARRAY_TYPE_EXT:
         *params = (GLfloat) CC.Array.TexCoordType;
         break;
      case GL_TEXTURE_COORD_ARRAY_STRIDE_EXT:
         *params = (GLfloat) CC.Array.TexCoordStride;
         break;
      case GL_TEXTURE_COORD_ARRAY_COUNT_EXT:
         *params = (GLfloat) CC.Array.TexCoordCount;
         break;
      case GL_EDGE_FLAG_ARRAY_STRIDE_EXT:
         *params = (GLfloat) CC.Array.EdgeFlagStride;
         break;
      case GL_EDGE_FLAG_ARRAY_COUNT_EXT:
         *params = (GLfloat) CC.Array.EdgeFlagCount;
         break;

      default:
         gl_error( GL_INVALID_ENUM, "glGetFloatv" );
   }
}




void glGetIntegerv( GLenum pname, GLint *params )
{
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glGetIntegerv" );
      return;
   }
   switch (pname) {
      case GL_ACCUM_RED_BITS:
      case GL_ACCUM_GREEN_BITS:
      case GL_ACCUM_BLUE_BITS:
      case GL_ACCUM_ALPHA_BITS:
         *params = (GLint) (sizeof(GLaccum)*8);
         break;
      case GL_ACCUM_CLEAR_VALUE:
         params[0] = FLOAT_TO_INT( CC.Accum.ClearColor[0] );
         params[1] = FLOAT_TO_INT( CC.Accum.ClearColor[1] );
         params[2] = FLOAT_TO_INT( CC.Accum.ClearColor[2] );
         params[3] = FLOAT_TO_INT( CC.Accum.ClearColor[3] );
         break;
      case GL_ALPHA_BIAS:
         *params = (GLint) CC.Pixel.AlphaBias;
         break;
      case GL_ALPHA_BITS:
         if (CC.FrontAlphaEnabled || CC.BackAlphaEnabled) {
            *params = 8*sizeof(GLubyte);
         }
         else {
            *params = 0;
         }
         break;
      case GL_ALPHA_SCALE:
         *params = (GLint) CC.Pixel.AlphaScale;
         break;
      case GL_ALPHA_TEST:
         *params = (GLint) CC.Color.AlphaEnabled;
         break;
      case GL_ALPHA_TEST_REF:
         *params = FLOAT_TO_INT( CC.Color.AlphaRef );
         break;
      case GL_ALPHA_TEST_FUNC:
         *params = (GLint) CC.Color.AlphaFunc;
         break;
      case GL_ATTRIB_STACK_DEPTH:
         *params = (GLint) CC.AttribStackDepth;
         break;
      case GL_AUTO_NORMAL:
         *params = (GLint) CC.Eval.AutoNormal;
         break;
      case GL_AUX_BUFFERS:
         *params = (GLint) NUM_AUX_BUFFERS;
         break;
      case GL_BLEND:
         *params = (GLint) CC.Color.BlendEnabled;
         break;
      case GL_BLEND_DST:
         *params = (GLint) CC.Color.BlendDst;
         break;
      case GL_BLEND_SRC:
         *params = (GLint) CC.Color.BlendSrc;
         break;
      case GL_BLEND_EQUATION_EXT:
	 *params = (GLint) CC.Color.BlendEquation;
	 break;
      case GL_BLEND_COLOR_EXT:
	 params[0] = FLOAT_TO_INT( CC.Color.BlendColor[0] );
	 params[1] = FLOAT_TO_INT( CC.Color.BlendColor[1] );
	 params[2] = FLOAT_TO_INT( CC.Color.BlendColor[2] );
	 params[3] = FLOAT_TO_INT( CC.Color.BlendColor[3] );
	 break;
      case GL_BLUE_BIAS:
         *params = (GLint) CC.Pixel.BlueBias;
         break;
      case GL_BLUE_BITS:
         *params = (GLint) bits( CC.BlueScale );
         break;
      case GL_BLUE_SCALE:
         *params = (GLint) CC.Pixel.BlueScale;
         break;
      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:
         i = (GLint) (pname - GL_CLIP_PLANE0);
         *params = (GLint) CC.Transform.ClipEnabled[i];
         break;
      case GL_COLOR_CLEAR_VALUE:
         params[0] = FLOAT_TO_INT( CC.Color.ClearColor[0] );
         params[1] = FLOAT_TO_INT( CC.Color.ClearColor[1] );
         params[2] = FLOAT_TO_INT( CC.Color.ClearColor[2] );
         params[3] = FLOAT_TO_INT( CC.Color.ClearColor[3] );
         break;
      case GL_COLOR_MATERIAL:
         *params = (GLint) CC.Light.ColorMaterialEnabled;
         break;
      case GL_COLOR_MATERIAL_FACE:
         *params = (GLint) CC.Light.ColorMaterialFace;
         break;
      case GL_COLOR_MATERIAL_PARAMETER:
         *params = (GLint) CC.Light.ColorMaterialMode;
         break;
      case GL_COLOR_WRITEMASK:
         params[0] = (CC.Color.ColorMask & 8) ? 1 : 0;
         params[1] = (CC.Color.ColorMask & 4) ? 1 : 0;
         params[2] = (CC.Color.ColorMask & 2) ? 1 : 0;
         params[3] = (CC.Color.ColorMask & 1) ? 1 : 0;
         break;
      case GL_CULL_FACE:
         *params = (GLint) CC.Polygon.CullFlag;
         break;
      case GL_CULL_FACE_MODE:
         *params = (GLint) CC.Polygon.CullFaceMode;
         break;
      case GL_CURRENT_COLOR:
         params[0] = FLOAT_TO_INT( (CC.Current.IntColor[0]/CC.RedScale) );
         params[1] = FLOAT_TO_INT( (CC.Current.IntColor[1]/CC.GreenScale) );
         params[2] = FLOAT_TO_INT( (CC.Current.IntColor[2]/CC.BlueScale) );
         params[3] = FLOAT_TO_INT( (CC.Current.IntColor[3]/CC.AlphaScale) );
         break;
      case GL_CURRENT_INDEX:
         *params = (GLint) CC.Current.Index;
         break;
      case GL_CURRENT_NORMAL:
         params[0] = FLOAT_TO_INT( CC.Current.Normal[0] );
         params[1] = FLOAT_TO_INT( CC.Current.Normal[1] );
         params[2] = FLOAT_TO_INT( CC.Current.Normal[2] );
         break;
      case GL_CURRENT_RASTER_COLOR:
	 params[0] = FLOAT_TO_INT( CC.Current.RasterColor[0] );
	 params[1] = FLOAT_TO_INT( CC.Current.RasterColor[1] );
	 params[2] = FLOAT_TO_INT( CC.Current.RasterColor[2] );
	 params[3] = FLOAT_TO_INT( CC.Current.RasterColor[3] );
	 break;
      case GL_CURRENT_RASTER_DISTANCE:
	 params[0] = (GLint) CC.Current.RasterDistance;
	 break;
      case GL_CURRENT_RASTER_INDEX:
	 *params = (GLint) CC.Current.RasterIndex;
	 break;
      case GL_CURRENT_RASTER_POSITION:
	 params[0] = (GLint) CC.Current.RasterPos[0];
	 params[1] = (GLint) CC.Current.RasterPos[1];
	 params[2] = (GLint) CC.Current.RasterPos[2];
	 params[3] = (GLint) CC.Current.RasterPos[3];
	 break;
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
	 params[0] = (GLint) CC.Current.RasterTexCoord[0];
	 params[1] = (GLint) CC.Current.RasterTexCoord[1];
	 params[2] = (GLint) CC.Current.RasterTexCoord[2];
	 params[3] = (GLint) CC.Current.RasterTexCoord[3];
	 break;
      case GL_CURRENT_RASTER_POSITION_VALID:
	 *params = (GLint) CC.Current.RasterPosValid;
	 break;
      case GL_CURRENT_TEXTURE_COORDS:
         params[0] = (GLint) CC.Current.TexCoord[0];
         params[1] = (GLint) CC.Current.TexCoord[1];
         params[2] = (GLint) CC.Current.TexCoord[2];
         params[3] = (GLint) CC.Current.TexCoord[3];
	 break;
      case GL_DEPTH_BIAS:
         *params = (GLint) CC.Pixel.DepthBias;
	 break;
      case GL_DEPTH_BITS:
	 *params = 8*sizeof(GLdepth);
	 break;
      case GL_DEPTH_CLEAR_VALUE:
         *params = (GLint) CC.Depth.Clear;
	 break;
      case GL_DEPTH_FUNC:
         *params = (GLint) CC.Depth.Func;
	 break;
      case GL_DEPTH_RANGE:
         params[0] = (GLint) CC.Viewport.Near;
         params[1] = (GLint) CC.Viewport.Far;
	 break;
      case GL_DEPTH_SCALE:
         *params = (GLint) CC.Pixel.DepthScale;
	 break;
      case GL_DEPTH_TEST:
         *params = (GLint) CC.Depth.Test;
	 break;
      case GL_DEPTH_WRITEMASK:
	 *params = (GLint) CC.Depth.Mask;
	 break;
      case GL_DITHER:
	 *params = (GLint) CC.Color.DitherFlag;
	 break;
      case GL_DOUBLEBUFFER:
	 *params = (GLint) CC.DBflag;
	 break;
      case GL_DRAW_BUFFER:
	 *params = (GLint) CC.Color.DrawBuffer;
	 break;
      case GL_EDGE_FLAG:
	 *params = (GLint) CC.Current.EdgeFlag;
	 break;
      case GL_FOG:
	 *params = (GLint) CC.Fog.Enabled;
	 break;
      case GL_FOG_COLOR:
	 params[0] = FLOAT_TO_INT( CC.Fog.Color[0] );
	 params[1] = FLOAT_TO_INT( CC.Fog.Color[1] );
	 params[2] = FLOAT_TO_INT( CC.Fog.Color[2] );
	 params[3] = FLOAT_TO_INT( CC.Fog.Color[3] );
	 break;
      case GL_FOG_DENSITY:
	 *params = (GLint) CC.Fog.Density;
	 break;
      case GL_FOG_END:
	 *params = (GLint) CC.Fog.End;
	 break;
      case GL_FOG_HINT:
	 *params = (GLint) CC.Hint.Fog;
	 break;
      case GL_FOG_INDEX:
	 *params = (GLint) CC.Fog.Index;
	 break;
      case GL_FOG_MODE:
	 *params = (GLint) CC.Fog.Mode;
	 break;
      case GL_FOG_START:
	 *params = (GLint) CC.Fog.Start;
	 break;
      case GL_FRONT_FACE:
	 *params = (GLint) CC.Polygon.FrontFace;
	 break;
      case GL_GREEN_BIAS:
         *params = (GLint) CC.Pixel.GreenBias;
         break;
      case GL_GREEN_BITS:
         *params = (GLint) bits( CC.GreenScale );
         break;
      case GL_GREEN_SCALE:
         *params = (GLint) CC.Pixel.GreenScale;
         break;
      case GL_INDEX_BITS:
	 {
	    GLuint width, height, depth;
	    (*DD.buffer_size)( &width, &height, &depth );
	    *params = (GLint) depth;
	 }
         break;
      case GL_INDEX_CLEAR_VALUE:
         *params = (GLint) CC.Color.ClearIndex;
         break;
      case GL_INDEX_MODE:
	 *params = CC.RGBAflag ? 0 : 1;
	 break;
      case GL_INDEX_OFFSET:
	 *params = CC.Pixel.IndexOffset;
	 break;
      case GL_INDEX_SHIFT:
	 *params = CC.Pixel.IndexShift;
	 break;
      case GL_INDEX_WRITEMASK:
	 *params = (GLint) CC.Color.IndexMask;
	 break;
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
	 *params = (GLint) CC.Light.Light[pname-GL_LIGHT0].Enabled;
	 break;
      case GL_LIGHTING:
	 *params = (GLint) CC.Light.Enabled;
	 break;
      case GL_LIGHT_MODEL_AMBIENT:
	 params[0] = FLOAT_TO_INT( CC.Light.Model.Ambient[0] );
	 params[1] = FLOAT_TO_INT( CC.Light.Model.Ambient[1] );
	 params[2] = FLOAT_TO_INT( CC.Light.Model.Ambient[2] );
	 params[3] = FLOAT_TO_INT( CC.Light.Model.Ambient[3] );
	 break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
	 *params = (GLint) CC.Light.Model.LocalViewer;
	 break;
      case GL_LIGHT_MODEL_TWO_SIDE:
	 *params = (GLint) CC.Light.Model.TwoSide;
	 break;
      case GL_LINE_SMOOTH:
	 *params = (GLint) CC.Line.SmoothFlag;
	 break;
      case GL_LINE_SMOOTH_HINT:
	 *params = (GLint) CC.Hint.LineSmooth;
	 break;
      case GL_LINE_STIPPLE:
	 *params = (GLint) CC.Line.StippleFlag;
	 break;
      case GL_LINE_STIPPLE_PATTERN:
         *params = (GLint) CC.Line.StipplePattern;
         break;
      case GL_LINE_STIPPLE_REPEAT:
         *params = (GLint) CC.Line.StippleFactor;
         break;
      case GL_LINE_WIDTH:
	 *params = (GLint) CC.Line.Width;
	 break;
      case GL_LINE_WIDTH_GRANULARITY:
	 *params = (GLint) LINE_WIDTH_GRANULARITY;
	 break;
      case GL_LINE_WIDTH_RANGE:
	 params[0] = (GLint) MIN_LINE_WIDTH;
	 params[1] = (GLint) MAX_LINE_WIDTH;
	 break;
      case GL_LIST_BASE:
	 *params = (GLint) CC.List.ListBase;
	 break;
      case GL_LIST_INDEX:
	 *params = (GLint) gl_list_index();
	 break;
      case GL_LIST_MODE:
	 *params = CC.ExecuteFlag ? (GLint) GL_COMPILE_AND_EXECUTE
	   			  : (GLint) GL_COMPILE;
	 break;
      case GL_LOGIC_OP:
	 *params = (GLint) CC.Color.LogicOpEnabled;
	 break;
      case GL_LOGIC_OP_MODE:
         *params = (GLint) CC.Color.LogicOp;
         break;
      case GL_MAP1_COLOR_4:
	 *params = (GLint) CC.Eval.Map1Color4;
	 break;
      case GL_MAP1_GRID_DOMAIN:
	 params[0] = (GLint) CC.Eval.MapGrid1u1;
	 params[1] = (GLint) CC.Eval.MapGrid1u2;
	 break;
      case GL_MAP1_GRID_SEGMENTS:
	 *params = (GLint) CC.Eval.MapGrid1un;
	 break;
      case GL_MAP1_INDEX:
	 *params = (GLint) CC.Eval.Map1Index;
	 break;
      case GL_MAP1_NORMAL:
	 *params = (GLint) CC.Eval.Map1Normal;
	 break;
      case GL_MAP1_TEXTURE_COORD_1:
	 *params = (GLint) CC.Eval.Map1TextureCoord1;
	 break;
      case GL_MAP1_TEXTURE_COORD_2:
	 *params = (GLint) CC.Eval.Map1TextureCoord2;
	 break;
      case GL_MAP1_TEXTURE_COORD_3:
	 *params = (GLint) CC.Eval.Map1TextureCoord3;
	 break;
      case GL_MAP1_TEXTURE_COORD_4:
	 *params = (GLint) CC.Eval.Map1TextureCoord4;
	 break;
      case GL_MAP1_VERTEX_3:
	 *params = (GLint) CC.Eval.Map1Vertex3;
	 break;
      case GL_MAP1_VERTEX_4:
	 *params = (GLint) CC.Eval.Map1Vertex4;
	 break;
      case GL_MAP2_COLOR_4:
	 *params = (GLint) CC.Eval.Map2Color4;
	 break;
      case GL_MAP2_GRID_DOMAIN:
	 params[0] = (GLint) CC.Eval.MapGrid2u1;
	 params[1] = (GLint) CC.Eval.MapGrid2u2;
	 params[2] = (GLint) CC.Eval.MapGrid2v1;
	 params[3] = (GLint) CC.Eval.MapGrid2v2;
	 break;
      case GL_MAP2_GRID_SEGMENTS:
	 params[0] = (GLint) CC.Eval.MapGrid2un;
	 params[1] = (GLint) CC.Eval.MapGrid2vn;
	 break;
      case GL_MAP2_INDEX:
	 *params = (GLint) CC.Eval.Map2Index;
	 break;
      case GL_MAP2_NORMAL:
	 *params = (GLint) CC.Eval.Map2Normal;
	 break;
      case GL_MAP2_TEXTURE_COORD_1:
	 *params = (GLint) CC.Eval.Map2TextureCoord1;
	 break;
      case GL_MAP2_TEXTURE_COORD_2:
	 *params = (GLint) CC.Eval.Map2TextureCoord2;
	 break;
      case GL_MAP2_TEXTURE_COORD_3:
	 *params = (GLint) CC.Eval.Map2TextureCoord3;
	 break;
      case GL_MAP2_TEXTURE_COORD_4:
	 *params = (GLint) CC.Eval.Map2TextureCoord4;
	 break;
      case GL_MAP2_VERTEX_3:
	 *params = (GLint) CC.Eval.Map2Vertex3;
	 break;
      case GL_MAP2_VERTEX_4:
	 *params = (GLint) CC.Eval.Map2Vertex4;
	 break;
      case GL_MAP_COLOR:
	 *params = (GLint) CC.Pixel.MapColorFlag;
	 break;
      case GL_MAP_STENCIL:
	 *params = (GLint) CC.Pixel.MapStencilFlag;
	 break;
      case GL_MATRIX_MODE:
	 *params = (GLint) CC.Transform.MatrixMode;
	 break;
      case GL_MAX_ATTRIB_STACK_DEPTH:
         *params = (GLint) MAX_ATTRIB_STACK_DEPTH;
         break;
      case GL_MAX_CLIP_PLANES:
         *params = (GLint) MAX_CLIP_PLANES;
         break;
      case GL_MAX_EVAL_ORDER:
	 *params = (GLint) MAX_EVAL_ORDER;
	 break;
      case GL_MAX_LIGHTS:
         *params = (GLint) MAX_LIGHTS;
         break;
      case GL_MAX_LIST_NESTING:
         *params = (GLint) MAX_LIST_NESTING;
         break;
      case GL_MAX_MODELVIEW_STACK_DEPTH:
         *params = (GLint) MAX_MODELVIEW_STACK_DEPTH;
         break;
      case GL_MAX_NAME_STACK_DEPTH:
	 *params = (GLint) MAX_NAME_STACK_DEPTH;
	 break;
      case GL_MAX_PIXEL_MAP_TABLE:
	 *params = (GLint) MAX_PIXEL_MAP_TABLE;
	 break;
      case GL_MAX_PROJECTION_STACK_DEPTH:
         *params = (GLint) MAX_PROJECTION_STACK_DEPTH;
         break;
      case GL_MAX_TEXTURE_SIZE:
	 *params = (GLint) MAX_TEXTURE_SIZE;
	 break;
      case GL_MAX_TEXTURE_STACK_DEPTH:
	 *params = (GLint) MAX_TEXTURE_STACK_DEPTH;
	 break;
      case GL_MAX_VIEWPORT_DIMS:
         params[0] = (GLint) MAX_WIDTH;
         params[1] = (GLint) MAX_HEIGHT;
         break;
      case GL_MODELVIEW_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = (GLint) CC.ModelViewMatrix[i];
	 }
	 break;
      case GL_MODELVIEW_STACK_DEPTH:
	 *params = (GLint) CC.ModelViewStackDepth;
	 break;
      case GL_NAME_STACK_DEPTH:
	 *params = (GLint) CC.NameStackDepth;
	 break;
      case GL_NORMALIZE:
	 *params = (GLint) CC.Transform.Normalize;
	 break;
      case GL_PACK_ALIGNMENT:
	 *params = CC.PackAlignment;
	 break;
      case GL_PACK_LSB_FIRST:
	 *params = (GLint) CC.PackLSBFirst;
	 break;
      case GL_PACK_ROW_LENGTH:
	 *params = CC.PackRowLength;
	 break;
      case GL_PACK_SKIP_PIXELS:
	 *params = CC.PackSkipPixels;
	 break;
      case GL_PACK_SKIP_ROWS:
	 *params = CC.PackSkipRows;
	 break;
      case GL_PACK_SWAP_BYTES:
	 *params = (GLint) CC.PackSwapBytes;
	 break;
      case GL_PERSPECTIVE_CORRECTION_HINT:
	 *params = (GLint) CC.Hint.PerspectiveCorrection;
	 break;
      case GL_PIXEL_MAP_A_TO_A_SIZE:
	 *params = CC.Pixel.MapAtoAsize;
	 break;
      case GL_PIXEL_MAP_B_TO_B_SIZE:
	 *params = CC.Pixel.MapBtoBsize;
	 break;
      case GL_PIXEL_MAP_G_TO_G_SIZE:
	 *params = CC.Pixel.MapGtoGsize;
	 break;
      case GL_PIXEL_MAP_I_TO_A_SIZE:
	 *params = CC.Pixel.MapItoAsize;
	 break;
      case GL_PIXEL_MAP_I_TO_B_SIZE:
	 *params = CC.Pixel.MapItoBsize;
	 break;
      case GL_PIXEL_MAP_I_TO_G_SIZE:
	 *params = CC.Pixel.MapItoGsize;
	 break;
      case GL_PIXEL_MAP_I_TO_I_SIZE:
	 *params = CC.Pixel.MapItoIsize;
	 break;
      case GL_PIXEL_MAP_I_TO_R_SIZE:
	 *params = CC.Pixel.MapItoRsize;
	 break;
      case GL_PIXEL_MAP_R_TO_R_SIZE:
	 *params = CC.Pixel.MapRtoRsize;
	 break;
      case GL_PIXEL_MAP_S_TO_S_SIZE:
	 *params = CC.Pixel.MapStoSsize;
	 break;
      case GL_POINT_SIZE:
         *params = (GLint) CC.Point.Size;
         break;
      case GL_POINT_SIZE_GRANULARITY:
	 *params = (GLint) POINT_SIZE_GRANULARITY;
	 break;
      case GL_POINT_SIZE_RANGE:
	 params[0] = (GLint) MIN_POINT_SIZE;
	 params[1] = (GLint) MAX_POINT_SIZE;
	 break;
      case GL_POINT_SMOOTH:
	 *params = (GLint) CC.Point.SmoothFlag;
	 break;
      case GL_POINT_SMOOTH_HINT:
	 *params = (GLint) CC.Hint.PointSmooth;
	 break;
      case GL_POLYGON_MODE:
	 params[0] = (GLint) CC.Polygon.FrontMode;
	 params[1] = (GLint) CC.Polygon.BackMode;
	 break;
      case GL_POLYGON_OFFSET_BIAS_EXT:
         *params = (GLint) CC.Polygon.OffsetBias;
         break;
      case GL_POLYGON_OFFSET_FACTOR_EXT:
         *params = (GLint) CC.Polygon.OffsetFactor;
         break;
      case GL_POLYGON_SMOOTH:
	 *params = (GLint) CC.Polygon.SmoothFlag;
	 break;
      case GL_POLYGON_SMOOTH_HINT:
	 *params = (GLint) CC.Hint.PolygonSmooth;
	 break;
      case GL_POLYGON_STIPPLE:
	 for (i=0;i<32;i++) {		/* RIGHT? */
	    params[i] = (GLint) CC.PolygonStipple[i];
	 }
	 break;
      case GL_PROJECTION_MATRIX:
	 for (i=0;i<16;i++) {
	    params[i] = (GLint) CC.ProjectionMatrix[i];
	 }
	 break;
      case GL_PROJECTION_STACK_DEPTH:
	 *params = (GLint) CC.ProjectionStackDepth;
	 break;
      case GL_READ_BUFFER:
	 *params = (GLint) CC.Pixel.ReadBuffer;
	 break;
      case GL_RED_BIAS:
         *params = (GLint) CC.Pixel.RedBias;
         break;
      case GL_RED_BITS:
         *params = (GLint) bits( CC.RedScale );
         break;
      case GL_RED_SCALE:
         *params = (GLint) CC.Pixel.RedScale;
         break;
      case GL_RENDER_MODE:
	 *params = (GLint) CC.RenderMode;
	 break;
      case GL_RGBA_MODE:
	 *params = (GLint) CC.RGBAflag;
	 break;
      case GL_SCISSOR_BOX:
	 params[0] = (GLint) CC.Scissor.X;
	 params[1] = (GLint) CC.Scissor.Y;
	 params[2] = (GLint) CC.Scissor.Width;
	 params[3] = (GLint) CC.Scissor.Height;
	 break;
      case GL_SCISSOR_TEST:
	 *params = (GLint) CC.Scissor.Enabled;
	 break;
      case GL_SHADE_MODEL:
	 *params = (GLint) CC.Light.ShadeModel;
	 break;
      case GL_STENCIL_BITS:
         *params = 8*sizeof(GLstencil);
         break;
      case GL_STENCIL_CLEAR_VALUE:
	 *params = (GLint) CC.Stencil.Clear;
	 break;
      case GL_STENCIL_FAIL:
	 *params = (GLint) CC.Stencil.FailFunc;
	 break;
      case GL_STENCIL_FUNC:
	 *params = (GLint) CC.Stencil.Function;
	 break;
      case GL_STENCIL_PASS_DEPTH_FAIL:
	 *params = (GLint) CC.Stencil.ZFailFunc;
	 break;
      case GL_STENCIL_PASS_DEPTH_PASS:
	 *params = (GLint) CC.Stencil.ZPassFunc;
	 break;
      case GL_STENCIL_REF:
	 *params = (GLint) CC.Stencil.Ref;
	 break;
      case GL_STENCIL_TEST:
	 *params = (GLint) CC.Stencil.Enabled;
	 break;
      case GL_STENCIL_VALUE_MASK:
	 *params = (GLint) CC.Stencil.ValueMask;
	 break;
      case GL_STENCIL_WRITEMASK:
	 *params = (GLint) CC.Stencil.WriteMask;
	 break;
      case GL_STEREO:
	 *params = 0;  /* TODO */
	 break;
      case GL_SUBPIXEL_BITS:
	 *params = 0;  /* TODO */
	 break;
      case GL_TEXTURE_1D:
	 *params = (CC.Texture.Enabled & 1) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_2D:
	 *params = (CC.Texture.Enabled & 2) ? 1.0 : 0.0;
	 break;
      case GL_TEXTURE_ENV_COLOR:
	 params[0] = FLOAT_TO_INT( CC.Texture.EnvColor[0] );
	 params[1] = FLOAT_TO_INT( CC.Texture.EnvColor[1] );
	 params[2] = FLOAT_TO_INT( CC.Texture.EnvColor[2] );
	 params[3] = FLOAT_TO_INT( CC.Texture.EnvColor[3] );
	 break;
      case GL_TEXTURE_ENV_MODE:
	 *params = (GLint) CC.Texture.EnvMode;
	 break;
      case GL_TEXTURE_GEN_S:
	 *params = (CC.Texture.TexGenEnabled & S_BIT) ? 1 : 0;
	 break;
      case GL_TEXTURE_GEN_T:
	 *params = (CC.Texture.TexGenEnabled & T_BIT) ? 1 : 0;
	 break;
      case GL_TEXTURE_GEN_R:
	 *params = (CC.Texture.TexGenEnabled & R_BIT) ? 1 : 0;
	 break;
      case GL_TEXTURE_GEN_Q:
	 *params = (CC.Texture.TexGenEnabled & Q_BIT) ? 1 : 0;
	 break;
      case GL_TEXTURE_MATRIX:
         for (i=0;i<16;i++) {
	    params[i] = (GLint) CC.TextureMatrix[i];
	 }
	 break;
      case GL_TEXTURE_STACK_DEPTH:
	 *params = (GLint) CC.TextureStackDepth;
	 break;
      case GL_UNPACK_ALIGNMENT:
	 *params = CC.UnpackAlignment;
	 break;
      case GL_UNPACK_LSB_FIRST:
	 *params = (GLint) CC.UnpackLSBFirst;
	 break;
      case GL_UNPACK_ROW_LENGTH:
	 *params = CC.UnpackRowLength;
	 break;
      case GL_UNPACK_SKIP_PIXELS:
	 *params = CC.UnpackSkipPixels;
	 break;
      case GL_UNPACK_SKIP_ROWS:
	 *params = CC.UnpackSkipRows;
	 break;
      case GL_UNPACK_SWAP_BYTES:
	 *params = (GLint) CC.UnpackSwapBytes;
	 break;
      case GL_VIEWPORT:
         params[0] = (GLint) CC.Viewport.X;
         params[1] = (GLint) CC.Viewport.Y;
         params[2] = (GLint) CC.Viewport.Width;
         params[3] = (GLint) CC.Viewport.Height;
         break;
      case GL_ZOOM_X:
	 *params = (GLint) CC.Pixel.ZoomX;
	 break;
      case GL_ZOOM_Y:
	 *params = (GLint) CC.Pixel.ZoomY;
	 break;
      case GL_VERTEX_ARRAY_SIZE_EXT:
         *params = CC.Array.VertexSize;
         break;
      case GL_VERTEX_ARRAY_TYPE_EXT:
         *params = CC.Array.VertexType;
         break;
      case GL_VERTEX_ARRAY_STRIDE_EXT:
         *params = CC.Array.VertexStride;
         break;
      case GL_VERTEX_ARRAY_COUNT_EXT:
         *params = CC.Array.VertexCount;
         break;
      case GL_NORMAL_ARRAY_TYPE_EXT:
         *params = CC.Array.NormalType;
         break;
      case GL_NORMAL_ARRAY_STRIDE_EXT:
         *params = CC.Array.NormalStride;
         break;
      case GL_NORMAL_ARRAY_COUNT_EXT:
         *params = CC.Array.NormalCount;
         break;
      case GL_COLOR_ARRAY_SIZE_EXT:
         *params = CC.Array.ColorSize;
         break;
      case GL_COLOR_ARRAY_TYPE_EXT:
         *params = CC.Array.ColorType;
         break;
      case GL_COLOR_ARRAY_STRIDE_EXT:
         *params = CC.Array.ColorStride;
         break;
      case GL_COLOR_ARRAY_COUNT_EXT:
         *params = CC.Array.ColorCount;
         break;
      case GL_INDEX_ARRAY_TYPE_EXT:
         *params = CC.Array.IndexType;
         break;
      case GL_INDEX_ARRAY_STRIDE_EXT:
         *params = CC.Array.IndexStride;
         break;
      case GL_INDEX_ARRAY_COUNT_EXT:
         *params = CC.Array.IndexCount;
         break;
      case GL_TEXTURE_COORD_ARRAY_SIZE_EXT:
         *params = CC.Array.TexCoordSize;
         break;
      case GL_TEXTURE_COORD_ARRAY_TYPE_EXT:
         *params = CC.Array.TexCoordType;
         break;
      case GL_TEXTURE_COORD_ARRAY_STRIDE_EXT:
         *params = CC.Array.TexCoordStride;
         break;
      case GL_TEXTURE_COORD_ARRAY_COUNT_EXT:
         *params = CC.Array.TexCoordCount;
         break;
      case GL_EDGE_FLAG_ARRAY_STRIDE_EXT:
         *params = CC.Array.EdgeFlagStride;
         break;
      case GL_EDGE_FLAG_ARRAY_COUNT_EXT:
         *params = CC.Array.EdgeFlagCount;
         break;

      default:
         gl_error( GL_INVALID_ENUM, "glGetIntegerv" );
   }
}

