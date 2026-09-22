/* $Id: attrib.c,v 1.12 1996/04/18 14:53:32 brianp Exp $ */

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
$Log: attrib.c,v $
 * Revision 1.12  1996/04/18  14:53:32  brianp
 * added error checking for GL_INVALID_OPERATION
 *
 * Revision 1.11  1996/03/22  20:54:08  brianp
 * removed CC.ClipSpans stuff
 *
 * Revision 1.10  1995/11/01  23:18:22  brianp
 * update CC.Light.LastEnabled when poping GL_ENABLE_BIT
 *
 * Revision 1.9  1995/10/27  20:29:02  brianp
 * added glPolygonOffsetEXT() support
 *
 * Revision 1.8  1995/08/31  21:32:45  brianp
 * new TexGenEnabled bitfield
 * introduced CC.NewState convention
 *
 * Revision 1.7  1995/07/24  18:58:55  brianp
 * added CC.ClipSpans logic to glPopAttrib()
 *
 * Revision 1.6  1995/07/17  15:36:32  brianp
 * fixed malloc size problem for stipples per Steve Freitag (stevef@ultranet.com)
 * added missing code for popping GL_POLYGON_STIPPLE_BIT attribute
 *
 * Revision 1.5  1995/05/22  21:02:41  brianp
 * Release 1.2
 *
 * Revision 1.4  1995/03/27  20:30:12  brianp
 * new Texture.Enabled sheme
 *
 * Revision 1.3  1995/03/24  17:00:59  brianp
 * added gl_update_pixel_logic
 *
 * Revision 1.2  1995/03/04  19:29:44  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/24  14:16:38  brianp
 * Initial revision
 *
 */


#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "list.h"
#include "macros.h"


#define MALLOC_STRUCT(T)  (struct T *) malloc( sizeof(struct T) )



static struct gl_attrib_node *new_attrib_node( GLbitfield kind )
{
   struct gl_attrib_node *an;

   an = (struct gl_attrib_node *) malloc( sizeof(struct gl_attrib_node) );
   if (an) {
      an->kind = kind;
   }
   return an;
}



void gl_push_attrib( GLbitfield mask )
{
   struct gl_attrib_node *newnode;
   struct gl_attrib_node *head;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glPushAttrib" );
      return;
   }

   if (CC.AttribStackDepth>=MAX_ATTRIB_STACK_DEPTH) {
      gl_error( GL_STACK_OVERFLOW, "glPushAttrib" );
      return;
   }

   /* Build linked list of attribute nodes which save all attribute */
   /* groups specified by the mask. */
   head = NULL;

   if (mask & GL_ACCUM_BUFFER_BIT) {
      struct gl_accum_attrib *attr;

      attr = MALLOC_STRUCT( gl_accum_attrib );
      MEMCPY( attr, &CC.Accum, sizeof(struct gl_accum_attrib) );
      newnode = new_attrib_node( GL_ACCUM_BUFFER_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_COLOR_BUFFER_BIT) {
      struct gl_colorbuffer_attrib *attr;
      attr = MALLOC_STRUCT( gl_colorbuffer_attrib );
      MEMCPY( attr, &CC.Color, sizeof(struct gl_colorbuffer_attrib) );
      newnode = new_attrib_node( GL_COLOR_BUFFER_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_CURRENT_BIT) {
      struct gl_current_attrib *attr;
      attr = MALLOC_STRUCT( gl_current_attrib );
      MEMCPY( attr, &CC.Current, sizeof(struct gl_current_attrib) );
      newnode = new_attrib_node( GL_CURRENT_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_DEPTH_BUFFER_BIT) {
      struct gl_depthbuffer_attrib *attr;
      attr = MALLOC_STRUCT( gl_depthbuffer_attrib );
      MEMCPY( attr, &CC.Depth, sizeof(struct gl_depthbuffer_attrib) );
      newnode = new_attrib_node( GL_DEPTH_BUFFER_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_ENABLE_BIT) {
      struct gl_enable_attrib *attr;
      GLuint i;
      attr = MALLOC_STRUCT( gl_enable_attrib );
      /* Copy enable flags from all other attributes into the enable struct. */
      attr->AlphaTest = CC.Color.AlphaEnabled;
      attr->AutoNormal = CC.Eval.AutoNormal;
      attr->Blend = CC.Color.BlendEnabled;
      for (i=0;i<MAX_CLIP_PLANES;i++) {
         attr->ClipPlane[i] = CC.Transform.ClipEnabled[i];
      }
      attr->ColorMaterial = CC.Light.ColorMaterialEnabled;
      attr->CullFace = CC.Polygon.CullFlag;
      attr->DepthTest = CC.Depth.Test;
      attr->Dither = CC.Color.DitherFlag;
      attr->Fog = CC.Fog.Enabled;
      for (i=0;i<MAX_LIGHTS;i++) {
         attr->Light[i] = CC.Light.Light[i].Enabled;
      }
      attr->Lighting = CC.Light.Enabled;
      attr->LineSmooth = CC.Line.SmoothFlag;
      attr->LineStipple = CC.Line.StippleFlag;
      attr->LogicOp = CC.Color.LogicOpEnabled;
      attr->Map1Color4 = CC.Eval.Map1Color4;
      attr->Map1Index = CC.Eval.Map1Index;
      attr->Map1Normal = CC.Eval.Map1Normal;
      attr->Map1TextureCoord1 = CC.Eval.Map1TextureCoord1;
      attr->Map1TextureCoord2 = CC.Eval.Map1TextureCoord2;
      attr->Map1TextureCoord3 = CC.Eval.Map1TextureCoord3;
      attr->Map1TextureCoord4 = CC.Eval.Map1TextureCoord4;
      attr->Map1Vertex3 = CC.Eval.Map1Vertex3;
      attr->Map1Vertex4 = CC.Eval.Map1Vertex4;
      attr->Map2Color4 = CC.Eval.Map2Color4;
      attr->Map2Index = CC.Eval.Map2Index;
      attr->Map2Normal = CC.Eval.Map2Normal;
      attr->Map2TextureCoord1 = CC.Eval.Map2TextureCoord1;
      attr->Map2TextureCoord2 = CC.Eval.Map2TextureCoord2;
      attr->Map2TextureCoord3 = CC.Eval.Map2TextureCoord3;
      attr->Map2TextureCoord4 = CC.Eval.Map2TextureCoord4;
      attr->Map2Vertex3 = CC.Eval.Map2Vertex3;
      attr->Map2Vertex4 = CC.Eval.Map2Vertex4;
      attr->Normalize = CC.Transform.Normalize;
      attr->PointSmooth = CC.Point.SmoothFlag;
      attr->PolygonOffset = CC.Polygon.OffsetEnabled;
      attr->PolygonSmooth = CC.Polygon.SmoothFlag;
      attr->PolygonStipple = CC.Polygon.StippleFlag;
      attr->Scissor = CC.Scissor.Enabled;
      attr->Stencil = CC.Stencil.Enabled;
      attr->Texture = CC.Texture.Enabled;
      attr->TexGen = CC.Texture.TexGenEnabled;
      newnode = new_attrib_node( GL_ENABLE_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_EVAL_BIT) {
      struct gl_eval_attrib *attr;
      attr = MALLOC_STRUCT( gl_eval_attrib );
      MEMCPY( attr, &CC.Eval, sizeof(struct gl_eval_attrib) );
      newnode = new_attrib_node( GL_EVAL_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_FOG_BIT) {
      struct gl_fog_attrib *attr;
      attr = MALLOC_STRUCT( gl_fog_attrib );
      MEMCPY( attr, &CC.Fog, sizeof(struct gl_fog_attrib) );
      newnode = new_attrib_node( GL_FOG_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_HINT_BIT) {
      struct gl_hint_attrib *attr;
      attr = MALLOC_STRUCT( gl_hint_attrib );
      MEMCPY( attr, &CC.Hint, sizeof(struct gl_hint_attrib) );
      newnode = new_attrib_node( GL_HINT_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_LIGHTING_BIT) {
      struct gl_light_attrib *attr;
      attr = MALLOC_STRUCT( gl_light_attrib );
      MEMCPY( attr, &CC.Light, sizeof(struct gl_light_attrib) );
      newnode = new_attrib_node( GL_LIGHTING_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_LINE_BIT) {
      struct gl_line_attrib *attr;
      attr = MALLOC_STRUCT( gl_line_attrib );
      MEMCPY( attr, &CC.Line, sizeof(struct gl_line_attrib) );
      newnode = new_attrib_node( GL_LINE_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_LIST_BIT) {
      struct gl_list_attrib *attr;
      attr = MALLOC_STRUCT( gl_list_attrib );
      MEMCPY( attr, &CC.List, sizeof(struct gl_list_attrib) );
      newnode = new_attrib_node( GL_LIST_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_PIXEL_MODE_BIT) {
      struct gl_pixel_attrib *attr;
      attr = MALLOC_STRUCT( gl_pixel_attrib );
      MEMCPY( attr, &CC.Pixel, sizeof(struct gl_pixel_attrib) );
      newnode = new_attrib_node( GL_PIXEL_MODE_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_POINT_BIT) {
      struct gl_point_attrib *attr;
      attr = MALLOC_STRUCT( gl_point_attrib );
      MEMCPY( attr, &CC.Point, sizeof(struct gl_point_attrib) );
      newnode = new_attrib_node( GL_POINT_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_POLYGON_BIT) {
      struct gl_polygon_attrib *attr;
      attr = MALLOC_STRUCT( gl_polygon_attrib );
      MEMCPY( attr, &CC.Polygon, sizeof(struct gl_polygon_attrib) );
      newnode = new_attrib_node( GL_POLYGON_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_POLYGON_STIPPLE_BIT) {
      GLuint *stipple;
      stipple = (GLuint *) malloc( 32*sizeof(GLuint) );
      MEMCPY( stipple, CC.PolygonStipple, 32*sizeof(GLuint) );
      newnode = new_attrib_node( GL_POLYGON_STIPPLE_BIT );
      newnode->data = stipple;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_SCISSOR_BIT) {
      struct gl_scissor_attrib *attr;
      attr = MALLOC_STRUCT( gl_scissor_attrib );
      MEMCPY( attr, &CC.Scissor, sizeof(struct gl_scissor_attrib) );
      newnode = new_attrib_node( GL_SCISSOR_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_STENCIL_BUFFER_BIT) {
      struct gl_stencil_attrib *attr;
      attr = MALLOC_STRUCT( gl_stencil_attrib );
      MEMCPY( attr, &CC.Stencil, sizeof(struct gl_stencil_attrib) );
      newnode = new_attrib_node( GL_STENCIL_BUFFER_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_TEXTURE_BIT) {
      struct gl_texture_attrib *attr;
      attr = MALLOC_STRUCT( gl_texture_attrib );
      MEMCPY( attr, &CC.Texture, sizeof(struct gl_texture_attrib) );
      newnode = new_attrib_node( GL_TEXTURE_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_TRANSFORM_BIT) {
      struct gl_transform_attrib *attr;
      attr = MALLOC_STRUCT( gl_transform_attrib );
      MEMCPY( attr, &CC.Transform, sizeof(struct gl_transform_attrib) );
      newnode = new_attrib_node( GL_TRANSFORM_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   if (mask & GL_VIEWPORT_BIT) {
      struct gl_viewport_attrib *attr;
      attr = MALLOC_STRUCT( gl_viewport_attrib );
      MEMCPY( attr, &CC.Viewport, sizeof(struct gl_viewport_attrib) );
      newnode = new_attrib_node( GL_VIEWPORT_BIT );
      newnode->data = attr;
      newnode->next = head;
      head = newnode;
   }

   /* etc... */

   CC.AttribStack[CC.AttribStackDepth] = head;
   CC.AttribStackDepth++;
}



void gl_pop_attrib( void )
{
   struct gl_attrib_node *attr, *next;
   struct gl_enable_attrib *enable;
   GLuint i;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glPopAttrib" );
      return;
   }

   if (CC.AttribStackDepth==0) {
      gl_error( GL_STACK_UNDERFLOW, "glPopAttrib" );
      return;
   }

   CC.AttribStackDepth--;
   attr = CC.AttribStack[CC.AttribStackDepth];

   while (attr) {
      switch (attr->kind) {
         case GL_ACCUM_BUFFER_BIT:
            MEMCPY( &CC.Accum, attr->data, sizeof(struct gl_accum_attrib) );
            break;
         case GL_COLOR_BUFFER_BIT:
            MEMCPY( &CC.Color, attr->data,
		    sizeof(struct gl_colorbuffer_attrib) );
            break;
         case GL_CURRENT_BIT:
            MEMCPY( &CC.Current, attr->data,
		    sizeof(struct gl_current_attrib) );
            break;
         case GL_DEPTH_BUFFER_BIT:
            MEMCPY( &CC.Depth, attr->data,
		    sizeof(struct gl_depthbuffer_attrib) );
            break;
         case GL_ENABLE_BIT:
            enable = (struct gl_enable_attrib *) attr->data;
            CC.Color.AlphaEnabled = enable->AlphaTest;
            CC.Transform.Normalize = enable->AutoNormal;
            CC.Color.BlendEnabled = enable->Blend;
	    for (i=0;i<MAX_CLIP_PLANES;i++) {
               CC.Transform.ClipEnabled[i] = enable->ClipPlane[i];
	    }
	    CC.Light.ColorMaterialEnabled = enable->ColorMaterial;
	    CC.Polygon.CullFlag = enable->CullFace;
	    CC.Depth.Test = enable->DepthTest;
	    CC.Color.DitherFlag = enable->Dither;
	    CC.Fog.Enabled = enable->Fog;
            CC.Light.LastEnabled = -1;
	    for (i=0;i<MAX_LIGHTS;i++) {
	       CC.Light.Light[i].Enabled = enable->Light[i];
               if (enable->Light[i]) {
                  CC.Light.LastEnabled = i;
               }
	    }
	    CC.Light.Enabled = enable->Lighting;
	    CC.Line.SmoothFlag = enable->LineSmooth;
	    CC.Line.StippleFlag = enable->LineStipple;
	    CC.Color.LogicOpEnabled = enable->LogicOp;
	    CC.Eval.Map1Color4 = enable->Map1Color4;
	    CC.Eval.Map1Index = enable->Map1Index;
	    CC.Eval.Map1Normal = enable->Map1Normal;
	    CC.Eval.Map1TextureCoord1 = enable->Map1TextureCoord1;
	    CC.Eval.Map1TextureCoord2 = enable->Map1TextureCoord2;
	    CC.Eval.Map1TextureCoord3 = enable->Map1TextureCoord3;
	    CC.Eval.Map1TextureCoord4 = enable->Map1TextureCoord4;
	    CC.Eval.Map1Vertex3 = enable->Map1Vertex3;
	    CC.Eval.Map1Vertex4 = enable->Map1Vertex4;
	    CC.Eval.Map2Color4 = enable->Map2Color4;
	    CC.Eval.Map2Index = enable->Map2Index;
	    CC.Eval.Map2Normal = enable->Map2Normal;
	    CC.Eval.Map2TextureCoord1 = enable->Map2TextureCoord1;
	    CC.Eval.Map2TextureCoord2 = enable->Map2TextureCoord2;
	    CC.Eval.Map2TextureCoord3 = enable->Map2TextureCoord3;
	    CC.Eval.Map2TextureCoord4 = enable->Map2TextureCoord4;
	    CC.Eval.Map2Vertex3 = enable->Map2Vertex3;
	    CC.Eval.Map2Vertex4 = enable->Map2Vertex4;
	    CC.Transform.Normalize = enable->Normalize;
	    CC.Point.SmoothFlag = enable->PointSmooth;
	    CC.Polygon.OffsetEnabled = enable->PolygonOffset;
	    CC.Polygon.SmoothFlag = enable->PolygonSmooth;
	    CC.Polygon.StippleFlag = enable->PolygonStipple;
	    CC.Scissor.Enabled = enable->Scissor;
	    CC.Stencil.Enabled = enable->Stencil;
	    CC.Texture.Enabled = enable->Texture;
	    CC.Texture.TexGenEnabled = enable->TexGen;
            break;
         case GL_EVAL_BIT:
            MEMCPY( &CC.Eval, attr->data, sizeof(struct gl_eval_attrib) );
            break;
         case GL_FOG_BIT:
            MEMCPY( &CC.Fog, attr->data, sizeof(struct gl_fog_attrib) );
            break;
         case GL_HINT_BIT:
            MEMCPY( &CC.Hint, attr->data, sizeof(struct gl_hint_attrib) );
            break;
         case GL_LIGHTING_BIT:
            MEMCPY( &CC.Light, attr->data, sizeof(struct gl_light_attrib) );
            break;
         case GL_LINE_BIT:
            MEMCPY( &CC.Line, attr->data, sizeof(struct gl_line_attrib) );
            break;
         case GL_LIST_BIT:
            MEMCPY( &CC.List, attr->data, sizeof(struct gl_list_attrib) );
            break;
         case GL_PIXEL_MODE_BIT:
            MEMCPY( &CC.Pixel, attr->data, sizeof(struct gl_pixel_attrib) );
            break;
         case GL_POINT_BIT:
            MEMCPY( &CC.Point, attr->data, sizeof(struct gl_point_attrib) );
            break;
         case GL_POLYGON_BIT:
            MEMCPY( &CC.Polygon, attr->data,
		    sizeof(struct gl_polygon_attrib) );
            break;
	 case GL_POLYGON_STIPPLE_BIT:
	    MEMCPY( CC.PolygonStipple, attr->data, 32*sizeof(GLuint) );
	    break;
         case GL_SCISSOR_BIT:
            MEMCPY( &CC.Scissor, attr->data,
		    sizeof(struct gl_scissor_attrib) );
            break;
         case GL_STENCIL_BUFFER_BIT:
            MEMCPY( &CC.Stencil, attr->data,
		    sizeof(struct gl_stencil_attrib) );
            break;
         case GL_TRANSFORM_BIT:
            MEMCPY( &CC.Transform, attr->data,
		    sizeof(struct gl_transform_attrib) );
            break;
         case GL_TEXTURE_BIT:
            MEMCPY( &CC.Texture, attr->data,
		    sizeof(struct gl_texture_attrib) );
            break;
         case GL_VIEWPORT_BIT:
            MEMCPY( &CC.Viewport, attr->data,
		    sizeof(struct gl_viewport_attrib) );
            break;
         default:
            /* ERROR! */
            break;
      }

      next = attr->next;
      free( (void *) attr->data );
      free( (void *) attr );
      attr = next;
   }

   CC.NewState = GL_TRUE;
}



void glPushAttrib( GLbitfield mask )
{
   if (CC.CompileFlag) {
      gl_save_pushattrib( mask );
   }
   if (CC.ExecuteFlag) {
      gl_push_attrib( mask );
   }
}



void glPopAttrib( void )
{
   if (CC.CompileFlag) {
      gl_save_popattrib();
   }
   if (CC.ExecuteFlag) {
      gl_pop_attrib();
   }
}
