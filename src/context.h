/* $Id: context.h,v 1.59 1996/05/20 15:02:44 brianp Exp $ */

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
$Log: context.h,v $
 * Revision 1.59  1996/05/20  15:02:44  brianp
 * Removed Reserved[] display list array
 *
 * Revision 1.58  1996/05/09  16:32:27  brianp
 * removed ComputePlane field
 *
 * Revision 1.57  1996/05/08  16:48:45  brianp
 * added LineFunc2 field for Mondello driver per Peter McDermott
 *
 * Revision 1.56  1996/04/25  20:47:02  brianp
 * added prototype for future gl_new_context() and new CC fields
 *
 * Revision 1.55  1996/03/22  20:54:47  brianp
 * replaced CC.ClipSpans with WINCLIP_BIT RasterMask flag
 *
 * Revision 1.54  1996/02/26  15:14:32  brianp
 * removed CC.Current.Color, replaced by CC.Current.IntColor
 *
 * Revision 1.53  1996/02/26  15:13:24  brianp
 * added lookup tables and code for optimized lighting
 *
 * Revision 1.52  1996/02/19  21:49:26  brianp
 * added support for software alpha buffering
 *
 * Revision 1.51  1996/02/15  16:01:20  brianp
 * added vertex array extension support
 *
 * Revision 1.50  1996/02/13  17:46:21  brianp
 * added fields to gl_light_attrib struct for lighting optimizations
 *
 * Revision 1.49  1996/02/06  04:13:02  brianp
 * added CC.Polygon.CullBits code
 *
 * Revision 1.48  1996/02/06  03:24:36  brianp
 * removed gamma correction code
 *
 * Revision 1.47  1996/01/29  19:05:44  brianp
 * added Orientation field for unfilled polygons
 *
 * Revision 1.46  1996/01/22  15:35:12  brianp
 * rename MutableColors to MutablePixels, added MonoPixels flag
 *
 * Revision 1.45  1996/01/19  18:08:49  brianp
 * use a smaller FIXED_SHIFT if using 32-bit depth buffer
 *
 * Revision 1.44  1996/01/11  20:14:22  brianp
 * use new GLstencil datatype
 *
 * Revision 1.43  1996/01/05  01:21:20  brianp
 * added profiling
 *
 * Revision 1.42  1996/01/02  22:13:40  brianp
 * removed old fixed-point macros
 *
 * Revision 1.41  1995/12/30  00:47:55  brianp
 * added EightBitColor, ColorShift, IntColor fields to CC
 *
 * Revision 1.40  1995/12/19  22:15:12  brianp
 * added RasterOffsetX and RasterOffsetY fields
 *
 * Revision 1.39  1995/12/18  17:11:50  brianp
 * added GLdepth and GLaccum datatypes
 *
 * Revision 1.38  1995/12/14  19:57:23  brianp
 * added GLfixed datatype and macros
 *
 * Revision 1.37  1995/11/22  13:34:49  brianp
 * added MutableColors flag
 *
 * Revision 1.36  1995/11/13  21:28:24  brianp
 * added comment about TextureWidth/Height
 *
 * Revision 1.35  1995/11/02  14:57:14  brianp
 * changed LastEnabled to GLint type
 *
 * Revision 1.34  1995/11/01  21:44:15  brianp
 * added CC.Light.LastEnabled
 *
 * Revision 1.33  1995/10/27  20:28:27  brianp
 * added glPolygonOffsetEXT() support
 *
 * Revision 1.32  1995/10/19  15:45:38  brianp
 * added gamma support
 * new arguments to gl_new_context()
 *
 * Revision 1.31  1995/10/17  21:34:03  brianp
 * added typedefs for points_func, line_func, and polygon_func
 *
 * Revision 1.30  1995/10/14  16:26:13  brianp
 * enable dithering by default
 * added SWmasking code
 *
 * Revision 1.29  1995/09/25  16:31:29  brianp
 * reorganized front and back material indexing
 *
 * Revision 1.28  1995/09/15  18:48:52  brianp
 * introduced CC.NewState convention
 * cleaned up comments, removed dead code
 *
 * Revision 1.27  1995/08/01  22:36:12  brianp
 * fixed an extension-compatibility symbol problem
 *
 * Revision 1.26  1995/08/01  22:23:04  brianp
 * added definitions for extension symbols when using non-Mesa header files
 *
 * Revision 1.25  1995/07/25  16:41:54  brianp
 * made changes for using CC.VertexFunc pointer
 *
 * Revision 1.24  1995/07/17  22:32:31  brianp
 * added ClipSpans flag to handle viewports which extend beyond the window
 *
 * Revision 1.23  1995/06/05  20:26:24  brianp
 * added Unfilled field to gl_polygon struct
 *
 * Revision 1.22  1995/05/31  14:32:37  brianp
 * changed BufferWidth, BufferHeight to GLint
 *
 * Revision 1.21  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.20  1995/05/17  13:52:37  brianp
 * implemented glIndexMask(0) and glColorMask(0,0,0,0)
 *
 * Revision 1.19  1995/05/17  13:17:22  brianp
 * changed default CC.Mode value to allow use of real OpenGL headers
 * removed need for CC.MajorMode variable
 *
 * Revision 1.18  1995/05/15  16:06:28  brianp
 * implemented shared/nonshared display lists
 *
 * Revision 1.17  1995/05/12  16:30:14  brianp
 * Texture images stored as bytes, not floats
 *
 * Revision 1.16  1995/04/17  13:51:19  brianp
 * added gl_copy_context() function
 *
 * Revision 1.15  1995/04/11  14:04:05  brianp
 * moved CC.write... functions to dd
 *
 * Revision 1.14  1995/03/30  21:08:04  brianp
 * updated to use pointers to CC.write_* functions
 *
 * Revision 1.13  1995/03/27  20:31:26  brianp
 * new Texture.Enabled scheme
 *
 * Revision 1.12  1995/03/24  16:59:41  brianp
 * added gl_update_pixel_logic
 *
 * Revision 1.11  1995/03/24  15:30:10  brianp
 * cleaned up CC struct and removed vertex buffer arrays
 *
 * Revision 1.10  1995/03/22  21:37:22  brianp
 * removed BufferDepth from CC
 *
 * Revision 1.9  1995/03/10  16:26:43  brianp
 * updated for bleding extensions
 *
 * Revision 1.8  1995/03/09  21:39:51  brianp
 * added ModelViewInvValid flag
 *
 * Revision 1.7  1995/03/08  15:10:02  brianp
 * added support for dd_logicop
 *
 * Revision 1.6  1995/03/07  14:20:28  brianp
 * removed ClearColorInt field
 *
 * Revision 1.5  1995/03/04  19:25:29  brianp
 * 1.1 beta revision
 *
 * Revision 1.4  1995/03/02  19:17:02  brianp
 * new RasterMask logic, fixed some comments
 *
 * Revision 1.3  1995/02/27  22:48:33  brianp
 * modified for PB
 *
 * Revision 1.2  1995/02/27  15:07:32  brianp
 * added Vcolor/Vindex scheme
 *
 * Revision 1.1  1995/02/24  14:18:47  brianp
 * Initial revision
 *
 */


#ifndef CONTEXT_H
#define CONTEXT_H


#include "GL/gl.h"
#include "config.h"




/*
 * Depth buffer data type:
 */
#if DEPTH_BITS==16
   typedef GLushort GLdepth;
#elif DEPTH_BITS==32
   typedef GLint GLdepth;
#else
   illegal number of depth bits
#endif


/*
 * Accumulation buffer data type:
 */
#if ACCUM_BITS==8
   typedef GLubyte GLaccum;
#elif ACCUM_BITS==16
   typedef GLushort GLaccum;
#else
   illegal number of accumulation bits
#endif


/*
 * Stencil buffer data type:
 */
#if STENCIL_BITS==8
   typedef GLubyte GLstencil;
#else
   illegal number of stencil bits
#endif



/*
 * Fixed point arithmetic type:
 */
typedef int GLfixed;

#if DEPTH_BITS==32
#  define FIXED_SHIFT      7
#  define FixedToUns(F)    ((unsigned)(F) >> 7u)
#  define FloatToFixed(X)  ( (int) ((X) * 128.0F) )
#else
#  define FIXED_SHIFT      15
#  define FixedToUns(F)    ((unsigned)(F) >> 15u)
#  define FloatToFixed(X)  ( (int) ((X) * 32768.0F) )
#endif
#define IntToFixed(I)    ((I) << FIXED_SHIFT)
#define FixedToInt(F)    ((F) >> FIXED_SHIFT)



/*
 * Specular exponent and material shininess lookup table sizes:
 */
#define EXP_TABLE_SIZE 512
#define SHINE_TABLE_SIZE 200

struct gl_light {
	GLfloat Ambient[4];		/* ambient color */
	GLfloat Diffuse[4];		/* diffuse color */
	GLfloat Specular[4];		/* specular color */
	GLfloat Position[4];		/* position in eye coordinates */
	GLfloat NormPosition[3];	/* normalized position */
	GLfloat Direction[4];		/* spotlight dir in eye coordinates */
        GLfloat NormDirection[3];	/* normalized direction */
	GLfloat SpotExponent;
        GLfloat SpotExpTable[EXP_TABLE_SIZE][2];  /* to replace a pow() call */
	GLfloat SpotCutoff;		/* in degress */
        GLfloat CosCutoff;		/* = cos(SpotCutoff) */
	GLfloat ConstantAttenuation;
	GLfloat LinearAttenuation;
	GLfloat QuadraticAttenuation;
	GLboolean Enabled;		/* On/off flag */
};


struct gl_lightmodel {
	GLfloat Ambient[4];		/* ambient color */
	GLboolean LocalViewer;		/* Local (or infinite) view point? */
	GLboolean TwoSide;		/* Two (or one) sided lighting? */
};


struct gl_material {
	GLfloat Ambient[4];
	GLfloat Diffuse[4];
	GLfloat Specular[4];
	GLfloat Emission[4];
	GLfloat Shininess;
	GLfloat AmbientIndex;	/* for color index lighting */
	GLfloat DiffuseIndex;	/* for color index lighting */
	GLfloat SpecularIndex;	/* for color index lighting */
        GLfloat ShineTable[SHINE_TABLE_SIZE];  /* to replace a pow() call */
};



/*
 * Attribute structures:
 *    We define a struct for each attribute group to make pushing and
 *    popping attributes easy.  Also it's a good organization.
 */


struct gl_accum_attrib {
	GLfloat ClearColor[4];	/* Accumulation buffer clear color */
};


/* Vertex array extension */
struct gl_array_attrib {
	GLint VertexSize;
	GLenum VertexType;
	GLsizei VertexStride;		/* user-specified stride */
	GLsizei VertexStrideB;		/* actual stride in bytes */
	GLsizei VertexCount;
	void *VertexPtr;
	GLboolean VertexEnabled;

	GLenum NormalType;
	GLsizei NormalStride;		/* user-specified stride */
	GLsizei NormalStrideB;		/* actual stride in bytes */
	GLsizei NormalCount;
	void *NormalPtr;
	GLboolean NormalEnabled;

	GLint ColorSize;
	GLenum ColorType;
	GLsizei ColorStride;		/* user-specified stride */
	GLsizei ColorStrideB;		/* actual stride in bytes */
	GLsizei ColorCount;
	void *ColorPtr;
	GLboolean ColorEnabled;

	GLenum IndexType;
	GLsizei IndexStride;		/* user-specified stride */
	GLsizei IndexStrideB;		/* actual stride in bytes */
	GLsizei IndexCount;
	void *IndexPtr;
	GLboolean IndexEnabled;

	GLint TexCoordSize;
	GLenum TexCoordType;
	GLsizei TexCoordStride;		/* user-specified stride */
	GLsizei TexCoordStrideB;	/* actual stride in bytes */
	GLsizei TexCoordCount;
	void *TexCoordPtr;
	GLboolean TexCoordEnabled;

	GLsizei EdgeFlagStride;		/* user-specified stride */
	GLsizei EdgeFlagStrideB;	/* actual stride in bytes */
	GLsizei EdgeFlagCount;
	GLboolean *EdgeFlagPtr;
	GLboolean EdgeFlagEnabled;
};


struct gl_colorbuffer_attrib {
	GLuint ClearIndex;		/* Index to use for glClear */
	GLfloat ClearColor[4];		/* Color to use for glClear */

	GLuint IndexMask;		/* Color index write mask */
	GLuint ColorMask;		/* bit 3=red,2=green,1=blue,0=alpha*/
        GLboolean SWmasking;		/* Do color/CI masking in software? */

	GLenum DrawBuffer;		/* Which buffer to draw into */

	/* alpha testing */
	GLboolean AlphaEnabled;		/* Alpha test enabled flag */
	GLenum AlphaFunc;		/* Alpha test function */
	GLfloat AlphaRef;		/* Alpha reference value */
	GLint AlphaRefInt;		/* AlphaRef scaled to an integer */

	/* blending */
	GLboolean BlendEnabled;		/* Blending enabled flag */
	GLenum BlendSrc;		/* Blending source operator */
	GLenum BlendDst;		/* Blending destination operator */
	GLenum BlendEquation;		/* BlendEquation EXTENSION */
	GLfloat BlendColor[4];		/* BlendColor EXTENSION */

	/* logic op */
	GLboolean LogicOpEnabled;	/* Logic op enabled flag */
	GLboolean SWLogicOpEnabled;	/* Do logic ops in software? */
	GLenum LogicOp;			/* Logic operator */

	GLboolean DitherFlag;		/* Dither enable flag */
};


struct gl_current_attrib {
	GLint IntColor[4];		/* Current RGBA color as scaled ints*/
	GLuint Index;			/* Current color index */
	GLfloat Normal[3];		/* Current normal vector */
	GLfloat TexCoord[4];		/* Current texture coordinate */
	GLfloat RasterPos[4];		/* Current raster position */
	GLfloat RasterDistance;		/* Current raster distance */
	GLfloat RasterColor[4];		/* Current raster color */
	GLuint RasterIndex;		/* Current raster index */
	GLfloat RasterTexCoord[4];	/* Current raster texture coord */
	GLboolean RasterPosValid;	/* Raster position valid flag */
	GLboolean EdgeFlag;		/* Current edge flag */
};


struct gl_depthbuffer_attrib {
	GLenum Func;		/* Function for depth buffer compare */
	GLfloat Clear;		/* Value to clear depth buffer to */
	GLboolean Test;		/* Depth buffering enabled flag */
	GLboolean Mask;		/* Depth buffer writable? */
};


struct gl_enable_attrib {
	GLboolean AlphaTest;
	GLboolean AutoNormal;
	GLboolean Blend;
	GLboolean ClipPlane[MAX_CLIP_PLANES];
	GLboolean ColorMaterial;
	GLboolean CullFace;
	GLboolean DepthTest;
	GLboolean Dither;
	GLboolean Fog;
	GLboolean Light[MAX_LIGHTS];
	GLboolean Lighting;
	GLboolean LineSmooth;
	GLboolean LineStipple;
	GLboolean LogicOp;
	GLboolean Map1Color4;
	GLboolean Map1Index;
	GLboolean Map1Normal;
	GLboolean Map1TextureCoord1;
	GLboolean Map1TextureCoord2;
	GLboolean Map1TextureCoord3;
	GLboolean Map1TextureCoord4;
	GLboolean Map1Vertex3;
	GLboolean Map1Vertex4;
	GLboolean Map2Color4;
	GLboolean Map2Index;
	GLboolean Map2Normal;
	GLboolean Map2TextureCoord1;
	GLboolean Map2TextureCoord2;
	GLboolean Map2TextureCoord3;
	GLboolean Map2TextureCoord4;
	GLboolean Map2Vertex3;
	GLboolean Map2Vertex4;
	GLboolean Normalize;
	GLboolean PointSmooth;
	GLboolean PolygonOffset;
	GLboolean PolygonSmooth;
	GLboolean PolygonStipple;
	GLboolean Scissor;
	GLboolean Stencil;
	GLuint Texture;
	GLuint TexGen;
};


struct gl_eval_attrib {
	/* Enable bits */
	GLboolean Map1Color4;
	GLboolean Map1Index;
	GLboolean Map1Normal;
	GLboolean Map1TextureCoord1;
	GLboolean Map1TextureCoord2;
	GLboolean Map1TextureCoord3;
	GLboolean Map1TextureCoord4;
	GLboolean Map1Vertex3;
	GLboolean Map1Vertex4;
	GLboolean Map2Color4;
	GLboolean Map2Index;
	GLboolean Map2Normal;
	GLboolean Map2TextureCoord1;
	GLboolean Map2TextureCoord2;
	GLboolean Map2TextureCoord3;
	GLboolean Map2TextureCoord4;
	GLboolean Map2Vertex3;
	GLboolean Map2Vertex4;
	GLboolean AutoNormal;
	/* Map Grid endpoints and divisions */
	GLuint MapGrid1un;
	GLfloat MapGrid1u1, MapGrid1u2;
	GLuint MapGrid2un, MapGrid2vn;
	GLfloat MapGrid2u1, MapGrid2u2;
	GLfloat MapGrid2v1, MapGrid2v2;
};


struct gl_fog_attrib {
	GLboolean Enabled;		/* Fog enabled flag */
	GLfloat Color[4];		/* Fog color */
	GLfloat Density;		/* Density >= 0.0 */
	GLfloat Start;			/* Start distance in eye coords */
	GLfloat End;			/* End distance in eye coords */
	GLfloat Index;			/* Fog index */
	GLenum Mode;			/* Fog mode */
};


struct gl_hint_attrib {
	/* always one of GL_FASTEST, GL_NICEST, or GL_DONT_CARE */
	GLenum PerspectiveCorrection;
	GLenum PointSmooth;
	GLenum LineSmooth;
	GLenum PolygonSmooth;
	GLenum Fog;
};


struct gl_light_attrib {
	struct gl_light Light[MAX_LIGHTS];	/* Array of lights */
	struct gl_lightmodel Model;		/* Lighting model */
	struct gl_material Material[2];		/* Material 0=front, 1=back */
	GLboolean Enabled;			/* Lighting enabled flag */
	GLenum ShadeModel;			/* GL_FLAT or GL_SMOOTH */
	GLenum ColorMaterialFace;		/* see glColorMaterial */
	GLenum ColorMaterialMode;		/* see glColorMaterial */
	GLboolean ColorMaterialEnabled;		/* see glColorMaterial */

	/* Derived for optimizations: */
	GLint LastEnabled;			/* no. of last enabled light */
	GLboolean Fast;				/* Use fast shader? */
	GLfloat BaseColor[4];
	GLfloat Diffuse[MAX_LIGHTS][3];
	GLfloat Specular[MAX_LIGHTS][3];
};


struct gl_line_attrib {
	GLboolean SmoothFlag;		/* GL_LINE_SMOOTH enabled? */
	GLboolean StippleFlag;		/* GL_LINE_STIPPLE enabled? */
	GLushort StipplePattern;	/* Stipple pattern */
	GLint StippleFactor;		/* Stipple repeat factor */
	GLfloat Width;			/* Line width */
};


struct gl_list_attrib {
	GLuint ListBase;
};


struct gl_pixel_attrib {
	GLenum ReadBuffer;
	GLfloat RedBias, RedScale;	/* Pixel xfer bias & scale values */
	GLfloat GreenBias, GreenScale;
	GLfloat BlueBias, BlueScale;
	GLfloat AlphaBias, AlphaScale;
	GLfloat DepthBias, DepthScale;
	GLint IndexShift;
	GLint IndexOffset;
	GLboolean MapColorFlag;
	GLboolean MapStencilFlag;
	GLfloat ZoomX;			/* Pixel zoom X factor */
	GLfloat ZoomY;			/* Pixel zoom Y factor */
	/* TODO: Do the following belong here??? */
	GLint MapStoSsize;			/* Size of each pixel map */
	GLint MapItoIsize;
	GLint MapItoRsize;
	GLint MapItoGsize;
	GLint MapItoBsize;
	GLint MapItoAsize;
	GLint MapRtoRsize;
	GLint MapGtoGsize;
	GLint MapBtoBsize;
	GLint MapAtoAsize;
	GLint MapStoS[MAX_PIXEL_MAP_TABLE];	/* Pixel map tables */
	GLint MapItoI[MAX_PIXEL_MAP_TABLE];
	GLfloat MapItoR[MAX_PIXEL_MAP_TABLE];
	GLfloat MapItoG[MAX_PIXEL_MAP_TABLE];
	GLfloat MapItoB[MAX_PIXEL_MAP_TABLE];
	GLfloat MapItoA[MAX_PIXEL_MAP_TABLE];
	GLfloat MapRtoR[MAX_PIXEL_MAP_TABLE];
	GLfloat MapGtoG[MAX_PIXEL_MAP_TABLE];
	GLfloat MapBtoB[MAX_PIXEL_MAP_TABLE];
	GLfloat MapAtoA[MAX_PIXEL_MAP_TABLE];
};


struct gl_point_attrib {
	GLboolean SmoothFlag;	/* True if GL_POINT_SMOOTH is enabled */
	GLfloat Size;		/* Point size */
};


struct gl_polygon_attrib {
	GLenum FrontFace;	/* Either GL_CW or GL_CCW */
	GLenum FrontMode;	/* Either GL_POINT, GL_LINE or GL_FILL */
	GLenum BackMode;	/* Either GL_POINT, GL_LINE or GL_FILL */
	GLboolean Unfilled;	/* True if back or front mode is not GL_FILL */
	GLenum CullFaceMode;	/* Culling mode GL_FRONT or GL_BACK */
	GLboolean CullFlag;	/* Culling on/off flag */
        GLuint CullBits;	/* Used for cull testing */
	GLboolean SmoothFlag;	/* True if GL_POLYGON_SMOOTH is enabled */
	GLboolean StippleFlag;	/* True if GL_POLYGON_STIPPLE is enabled */
        GLfloat OffsetFactor;   /* Polygon offset factor */
        GLfloat OffsetBias;     /* Polygon offset bias */
        GLboolean OffsetEnabled;/* Polygon offset enable */
};


struct gl_scissor_attrib {
	GLboolean	Enabled;		/* Scissor test enabled? */
	GLint		X, Y;			/* Lower left corner of box */
	GLsizei		Width, Height;		/* Size of box */
	GLint		Xmin, Xmax, Ymin, Ymax;	/* Bounds in window coords */
};


struct gl_stencil_attrib {
	GLboolean	Enabled;	/* Enabled flag */
	GLenum		Function;	/* Stencil function */
	GLenum		FailFunc;	/* Fail function */
	GLenum		ZPassFunc;	/* Depth buffer pass function */
	GLenum		ZFailFunc;	/* Depth buffer fail function */
	GLstencil	Ref;		/* Reference value */
	GLstencil	ValueMask;	/* Value mask */
	GLstencil	Clear;		/* Clear value */
	GLstencil	WriteMask;	/* Write mask */
};


#define Q_BIT 1
#define R_BIT 2
#define S_BIT 4
#define T_BIT 8


struct gl_texture_attrib {
	GLuint Enabled;			/* Bitwise-OR of 1 and 2 */
	GLenum EnvMode;			/* GL_MODULATE, GL_DECAL, GL_BLEND */
	GLfloat EnvColor[4];
	GLfloat BorderColor[4];		/* TODO: border color 1-D and 2-D */
	GLuint TexGenEnabled;		/* Bitwise-OR of [QRST]_BIT values */
	GLenum GenModeS;		/* Tex coord generation mode, either */
	GLenum GenModeT;		/*	GL_OBJECT_LINEAR, or */
	GLenum GenModeR;		/*	GL_EYE_LINEAR, or    */
	GLenum GenModeQ;		/*      GL_SPHERE_MAP        */
	GLfloat ObjectPlaneS[4];
	GLfloat ObjectPlaneT[4];
	GLfloat ObjectPlaneR[4];
	GLfloat ObjectPlaneQ[4];
	GLfloat EyePlaneS[4];
	GLfloat EyePlaneT[4];
	GLfloat EyePlaneR[4];
	GLfloat EyePlaneQ[4];
	GLenum WrapS1D;			/* GL_CLAMP or GL_REPEAT */
	GLenum WrapT1D;			/* GL_CLAMP or GL_REPEAT */
	GLenum WrapS2D;			/* GL_CLAMP or GL_REPEAT */
	GLenum WrapT2D;			/* GL_CLAMP or GL_REPEAT */
	GLenum MinFilter1D;		/* 1-D minification filter */
	GLenum MagFilter1D;		/* 1-D magnification filter */
	GLenum MinFilter2D;		/* 2-D minification filter */
	GLenum MagFilter2D;		/* 2-D magnification filter */
};


struct gl_transform_attrib {
	GLenum MatrixMode;			/* Matrix mode */
	GLboolean Normalize;			/* Normalize all normals? */
	GLfloat ClipEquation[MAX_CLIP_PLANES][4];
	GLboolean ClipEnabled[MAX_CLIP_PLANES];
	GLboolean AnyClip;			/* Any ClipEnabled[] true? */

};


struct gl_viewport_attrib {
	GLint X, Y;		/* position */
	GLsizei Width, Height;	/* size */
	GLfloat Near, Far;	/* Depth buffer range */
	GLfloat Sx, Sy, Sz;	/* NDC to WinCoord scaling */
	GLfloat Tx, Ty, Tz;	/* NDC to WinCoord translation */
};


/* For the attribute stack: */
struct gl_attrib_node {
	GLbitfield kind;
	void *data;
	struct gl_attrib_node *next;
};


/* Display list info shared by multiple contexts: */
struct gl_list_group {
	union node *List[MAX_DISPLAYLISTS];	/* The display lists */
	GLint RefCount;				/* Reference count */
};




/*
 * Bitmasks to indicate what auxillary information must be interpolated
 * when clipping (CC.ClipMask).
 */
#define CLIP_FCOLOR_BIT		0x01
#define CLIP_BCOLOR_BIT		0x02
#define CLIP_FINDEX_BIT		0x04
#define CLIP_BINDEX_BIT		0x08
#define CLIP_TEXTURE_BIT	0x10



/*
 * Bitmasks to indicate which rasterization options are enabled (CC.RasterMask)
 */
#define ALPHATEST_BIT	0x001	/* Alpha-test pixels? */
#define BLEND_BIT	0x002	/* Blend pixels? */
#define DEPTH_BIT	0x004	/* Depth-test pixels? */
#define FOG_BIT		0x008	/* Per-pixel fog? */
#define LOGIC_OP_BIT	0x010	/* Apply logic op in software? */
#define SCISSOR_BIT	0x020	/* Scissor pixels? */
#define STENCIL_BIT	0x040	/* Stencil pixels? */
#define MASKING_BIT	0x080	/* Need to apply glColorMask()? */
#define ALPHABUF_BIT	0x100	/* Using software alpha buffer? */
#define WINCLIP_BIT	0x200	/* Clip pixels/primitives to window? */



/*
 * Point, line and polygon rasterizer functions:
 */
typedef void (*points_func)( GLuint first, GLuint last );
typedef void (*line_func)( GLuint v1, GLuint v2, GLuint pv );
typedef void (*polygon_func)( GLuint n, GLuint vlist[], GLuint pv );
typedef void (*triangle_func)( GLuint v1, GLuint v2, GLuint v3, GLuint pv );



/*
 * The library context: 
 */

struct gl_context {
	/* Modelview matrix and stack */
	GLfloat ModelViewMatrix[16];
	GLfloat ModelViewInv[16];	/* Inverse of ModelViewMatrix */
	GLboolean ModelViewInvValid;	/* Is the inverse matrix now valid? */
	GLuint ModelViewStackDepth;
	GLfloat ModelViewStack[MAX_MODELVIEW_STACK_DEPTH][16];

	/* Projection matrix and stack */
	GLfloat ProjectionMatrix[16];
	GLuint ProjectionStackDepth;
	GLfloat ProjectionStack[MAX_PROJECTION_STACK_DEPTH][16];

	/* Texture matrix and stack */
	GLfloat TextureMatrix[16];
        GLboolean IdentityTexMat;	/* Is TextureMatrix==Identity? */
	GLuint TextureStackDepth;
	GLfloat TextureStack[MAX_TEXTURE_STACK_DEPTH][16];

	/* Display List stuff */
	GLuint CallDepth;	/* Current recursion calling depth */
	GLboolean ExecuteFlag;	/* Execute GL commands? */
	GLboolean CompileFlag;	/* Compile GL commands into display list? */
	struct gl_list_group *ListGroup;	/* Shared display list group */

	/* Attribute stack */
	GLuint AttribStackDepth;
	struct gl_attrib_node *AttribStack[MAX_ATTRIB_STACK_DEPTH];

	/* Attribute groups */
	struct gl_accum_attrib		Accum;
	struct gl_array_attrib		Array;
	struct gl_colorbuffer_attrib	Color;
	struct gl_current_attrib	Current;
	struct gl_depthbuffer_attrib	Depth;
	struct gl_eval_attrib		Eval;
	struct gl_fog_attrib		Fog;
	struct gl_hint_attrib		Hint;
	struct gl_light_attrib		Light;
	struct gl_line_attrib		Line;
	struct gl_list_attrib		List;
	struct gl_pixel_attrib		Pixel;
	struct gl_point_attrib		Point;
	struct gl_polygon_attrib	Polygon;
	GLuint PolygonStipple[32];
	struct gl_scissor_attrib	Scissor;
	struct gl_stencil_attrib	Stencil;
	struct gl_texture_attrib	Texture;
	struct gl_transform_attrib	Transform;
	struct gl_viewport_attrib	Viewport;

	/* PixelStore parameters */
	GLint PackAlignment;
	GLint PackRowLength;
	GLint PackSkipPixels;
	GLint PackSkipRows;
	GLboolean PackSwapBytes;
	GLboolean PackLSBFirst;
	GLint UnpackAlignment;
	GLint UnpackRowLength;
	GLint UnpackSkipPixels;
	GLint UnpackSkipRows;
	GLboolean UnpackSwapBytes;
	GLboolean UnpackLSBFirst;

	/* Feedback */
	GLenum FeedbackType;
	GLuint FeedbackMask;
	GLfloat *FeedbackBuffer;
	GLuint FeedbackBufferSize;
	GLuint FeedbackCount;

	/* Selection */
	GLuint *SelectBuffer;
	GLuint SelectBufferSize;	/* size of SelectBuffer */
	GLuint SelectBufferCount;	/* number of values in SelectBuffer */
	GLuint SelectHits;		/* number of records in SelectBuffer */
	GLuint NameStackDepth;
	GLuint NameStack[MAX_NAME_STACK_DEPTH];
	GLboolean HitFlag;
	GLfloat HitMinZ, HitMaxZ;

	/* Current 1-D texture map */
	GLuint TextureComponents1D[MAX_TEXTURE_LEVELS];
	GLuint TextureWidth1D[MAX_TEXTURE_LEVELS];  /* includes border */
	GLuint TextureBorder1D[MAX_TEXTURE_LEVELS];
	GLubyte *TextureImage1D[MAX_TEXTURE_LEVELS];
	GLboolean TextureImage1DDeleteFlag[MAX_TEXTURE_LEVELS];

	/* Current 2-D texture map */
	GLuint TextureComponents2D[MAX_TEXTURE_LEVELS];
	GLuint TextureWidth2D[MAX_TEXTURE_LEVELS];  /* includes border */
	GLuint TextureHeight2D[MAX_TEXTURE_LEVELS]; /* includes border */
	GLuint TextureBorder2D[MAX_TEXTURE_LEVELS];
	GLubyte *TextureImage2D[MAX_TEXTURE_LEVELS];
	GLboolean TextureImage2DDeleteFlag[MAX_TEXTURE_LEVELS];

	/* Miscellaneous */
        GLboolean NewState;     /* If GL_TRUE, must call gl_update_state() */
	GLenum RenderMode;	/* either GL_RENDER, GL_SELECT, GL_FEEDBACK */
	GLenum Mode;		/* glBegin primitive or GL_BITMAP */

	GLuint StippleCounter;	/* Line stipple counter */
	GLuint ClipMask;	/* OR of CLIP_* values from above */
	GLuint RasterMask;	/* OR of rasterization flags */
	GLuint LightTwoSide;	/* Compute two-sided lighting? */
	GLfloat PlaneA, PlaneB;	/* Eq. of plane of current polygon */
	GLfloat PlaneC, PlaneD;
        GLuint Orientation;     /* Polygon orientation: 0=front, 1=back face */

	GLboolean NeedNormals;	 /* Are vertex normal vectors needed? */
        GLboolean FastDrawPixels;/* Use optimized glDrawPixels? */
        GLboolean MutablePixels; /* Can rasterization change pixel's color? */
        GLboolean MonoPixels;    /* Are all pixels likely to be same color? */

        GLfloat RasterOffsetX;  /* added to window coords before rasterizing */
        GLfloat RasterOffsetY;  /* added to window coords before rasterizing */

        GLint ColorShift;	/* If flat shading ColorShift=0 */
				/* If smooth shading ColorShift=FIXED_SHIFT */

	GLboolean RGBAflag;	/* Is frame buffer in RGBA mode, not CI? */
	GLboolean DBflag;	/* Is frame buffer double buffered? */
	GLint BufferWidth;	/* Width of frame buffer in pixels */
	GLint BufferHeight;	/* Height of frame buffer in pixels */

	GLfloat RedScale;	/* These values are used to scale color */
	GLfloat GreenScale;	/* components from the range [0,1] to */
	GLfloat BlueScale;	/* integer values.  It should be the case */
	GLfloat AlphaScale;	/* that: scale = 2^bits - 1 where bits is */
				/* the number of bits for the component */
				/* in the frame buffer. */
        GLboolean EightBitColor;/* TRUE if all the above scales are 255.0 */

#ifdef FUTURE
        /* Ancillary buffers */
        GLint DepthBufferBits;		/* really just true/false */
        GLint AccumBufferBits;		/* really just true/false */
        GLint StencilBufferBits;	/* really just true/false */
#endif

	/* Special buffers (BS = BufferWidth * BufferHeight): */
	GLdepth *DepthBuffer;	  /* Zbuffer: array [BS] of depth values */
	GLstencil *StencilBuffer; /* Stencil buffer: array [BS] of ubytes */
	GLaccum *AccumBuffer;	  /* Acc. buffer: array [4*BS] of components */

        /* Sofware alpha planes: */
        GLboolean FrontAlphaEnabled;
        GLboolean BackAlphaEnabled;
        GLubyte *FrontAlphaBuffer;
        GLubyte *BackAlphaBuffer;
        GLubyte *AlphaBuffer;	/* Points to front or back alpha buffer */

	GLenum ErrorValue;

	/* Current Primitive functions */
	void (*VertexFunc)( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
        points_func PointsFunc;
        line_func LineFunc;
#ifdef MONDELLO
        line_func LineFunc2;
#endif
        polygon_func PolygonFunc;
        polygon_func AuxPolygonFunc;
        triangle_func TriangleFunc;

#ifdef PROFILE
        /* Performance measurments */
        GLuint BeginEndCount;	/* number of glBegin/glEnd pairs */
        GLdouble BeginEndTime;	/* seconds spent between glBegin/glEnd */
        GLuint VertexCount;	/* number of vertices processed */
        GLdouble VertexTime;	/* total time in seconds */
        GLuint PointCount;	/* number of points rendered */
        GLdouble PointTime;	/* total time in seconds */
        GLuint LineCount;	/* number of lines rendered */
        GLdouble LineTime;	/* total time in seconds */
        GLuint PolygonCount;	/* number of polygons rendered */
        GLdouble PolygonTime;	/* total time in seconds */
        GLuint ClearCount;	/* number of glClear calls */
        GLdouble ClearTime;	/* seconds spent in glClear */
        GLuint SwapCount;	/* number of swap-buffer calls */
        GLdouble SwapTime;	/* seconds spent in swap-buffers */
#endif
};




extern struct gl_context CC;



/*
 * Context creation, initialization, etc.
 */

extern void gl_initialize_context( struct gl_context *c );

/*
extern struct gl_context *gl_new_context( struct gl_context *shareList );
*/

extern struct gl_context *
gl_new_context( GLboolean rgb_flag,
                GLfloat redscale,
                GLfloat greenscale,
                GLfloat bluescale,
                GLfloat alphascale,
                GLboolean db_flag,
                struct gl_context *shareList );

/*
 * FUTURE CHANGES:
 *   New args for requesting alpha buffer, stencil buffer, and accum buffer.
 */
#ifdef FUTURE
extern struct gl_context *
gl_new_context( GLboolean rgb_flag,
                GLboolean alpha_flag,
                GLboolean db_flag,
                GLint depth_size,
                GLint stencil_size,
                GLint accum_size,
                GLfloat red_scale,
                GLfloat green_scale,
                GLfloat blue_scale,
                GLfloat alpha_scale,
                struct gl_context *share_list );
#endif


extern void gl_destroy_context( struct gl_context *c );

extern void gl_set_context( struct gl_context *c );

extern void gl_copy_context( struct gl_context *src, struct gl_context *dst,
			     GLuint mask );



/*
 * Miscellaneous
 */

extern void gl_error( GLenum error, char *s );


extern void gl_update_state( void );



#ifdef PROFILE
extern GLdouble gl_time( void );
#endif


#endif
