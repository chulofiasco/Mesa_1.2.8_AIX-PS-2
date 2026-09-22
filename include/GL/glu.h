/* $Id: glu.h,v 1.16 1996/05/15 15:38:35 brianp Exp $ */

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
$Log: glu.h,v $
 * Revision 1.16  1996/05/15  15:38:35  brianp
 * changed return types from int to GLint
 *
 * Revision 1.15  1995/08/03  22:00:57  brianp
 * explictly assigned values to GLU_ symbols to match OpenGL
 *
 * Revision 1.14  1995/08/03  19:35:29  brianp
 * replaced all GLUenum with GLenum
 *
 * Revision 1.13  1995/07/28  15:00:44  brianp
 * added comments for each NURBS error code
 * fixed a couple prototypes
 *
 * Revision 1.12  1995/07/18  20:22:15  brianp
 * added GLU_INCOMPATIBLE_GL_VERSION
 * start GLUenum values at 10000
 *
 * Revision 1.11  1995/05/30  13:13:03  brianp
 * added GLU_TRUE, GLU_FALSE, GLU_NURBS_ERRORxx, etc.
 *
 * Revision 1.10  1995/05/29  20:08:38  brianp
 * added gluGetNurbsProperty() prototype
 *
 * Revision 1.9  1995/05/24  13:43:30  brianp
 * added gluBeginTrim, gluEndTrim, gluPwlCurve
 *
 * Revision 1.8  1995/05/22  17:03:21  brianp
 * Release 1.2
 *
 * Revision 1.7  1995/05/16  18:03:22  brianp
 * renamed GLU_EDGEFLAG to GLU_EDGE_FLAG
 * renamed quadric, triangulator and nurbs structs
 *
 * Revision 1.6  1995/04/28  20:04:57  brianp
 * added stuff for Bogdan Sikorski's polygon tesselator
 *
 * Revision 1.5  1995/04/28  14:50:50  brianp
 * moved structs to their respective .c files
 *
 * Revision 1.4  1995/04/18  15:50:19  brianp
 * changed GLenum arguments to GLUenum, added ErrorFunc to quadric object
 *
 * Revision 1.3  1995/04/17  14:41:26  brianp
 * added GLU version 1.1 function: gluGetString
 *
 * Revision 1.2  1995/03/04  19:45:47  brianp
 * 1.1 beta revision
 *
 * Revision 1.1  1995/02/28  21:21:03  brianp
 * Initial revision
 *
 */


#ifndef GLU_H
#define GLU_H


#ifdef __cplusplus
extern "C" {
#endif


#include "GL/gl.h"


#define GLU_VERSION_1_1		1


#define GLU_TRUE   GL_TRUE
#define GLU_FALSE  GL_FALSE


/* Normal vectors */
#define GLU_SMOOTH	100000UL
#define GLU_FLAT	100001UL
#define GLU_NONE	100002UL

/* Quadric draw styles */
#define GLU_POINT	100010UL
#define GLU_LINE	100011UL
#define GLU_FILL	100012UL
#define GLU_SILHOUETTE	100013UL

/* Quadric orientation */
#define GLU_OUTSIDE	100020UL
#define GLU_INSIDE	100021UL

/* Tesselator */
#define GLU_BEGIN	100100UL
#define GLU_VERTEX	100101UL
#define GLU_END		100102UL
#define GLU_ERROR	100103UL
#define GLU_EDGE_FLAG	100104UL

/* Contour types */
#define GLU_CW		100120UL
#define GLU_CCW		100121UL
#define GLU_INTERIOR	100122UL
#define GLU_EXTERIOR	100123UL
#define GLU_UNKNOWN	100124UL

/* Tesselation errors */
#define GLU_TESS_ERROR1	100151UL  /* missing gluEndPolygon */
#define GLU_TESS_ERROR2 100152UL  /* missing gluBeginPolygon */
#define GLU_TESS_ERROR3 100153UL  /* misoriented contour */
#define GLU_TESS_ERROR4 100154UL  /* vertex/edge intersection */
#define GLU_TESS_ERROR5 100155UL  /* misoriented or self-intersecting loops */
#define GLU_TESS_ERROR6 100156UL  /* coincident vertices */
#define GLU_TESS_ERROR7 100157UL  /* all vertices collinear */
#define GLU_TESS_ERROR8 100158UL  /* intersecting edges */
#define GLU_TESS_ERROR9 100159UL  /* not coplanar contours */

/* NURBS */
#define GLU_AUTO_LOAD_MATRIX	100200UL
#define GLU_CULLING		100201UL
#define GLU_PARAMETRIC_TOLERANCE 100202UL
#define GLU_SAMPLING_TOLERANCE	100203UL
#define GLU_DISPLAY_MODE	100204UL
#define GLU_SAMPLING_METHOD	100205UL
#define GLU_U_STEP		100206UL
#define GLU_V_STEP		100207UL

#define GLU_PATH_LENGTH		100215UL
#define GLU_PARAMETRIC_ERROR	100216UL
#define GLU_DOMAIN_DISTANCE	100217UL

#define GLU_MAP1_TRIM_2		100210UL
#define GLU_MAP1_TRIM_3		100211UL

#define GLU_OUTLINE_POLYGON	100240UL
#define GLU_OUTLINE_PATCH	100241UL

#define GLU_NURBS_ERROR1  100251UL   /* spline order un-supported */
#define GLU_NURBS_ERROR2  100252UL   /* too few knots */
#define GLU_NURBS_ERROR3  100253UL   /* valid knot range is empty */
#define GLU_NURBS_ERROR4  100254UL   /* decreasing knot sequence */
#define GLU_NURBS_ERROR5  100255UL   /* knot multiplicity > spline order */
#define GLU_NURBS_ERROR6  100256UL   /* endcurve() must follow bgncurve() */
#define GLU_NURBS_ERROR7  100257UL   /* bgncurve() must precede endcurve() */
#define GLU_NURBS_ERROR8  100258UL   /* ctrlarray or knot vector is NULL */
#define GLU_NURBS_ERROR9  100259UL   /* can't draw pwlcurves */
#define GLU_NURBS_ERROR10 100260UL   /* missing gluNurbsCurve() */
#define GLU_NURBS_ERROR11 100261UL   /* missing gluNurbsSurface() */
#define GLU_NURBS_ERROR12 100262UL   /* endtrim() must precede endsurface() */
#define GLU_NURBS_ERROR13 100263UL   /* bgnsurface() must precede endsurface() */
#define GLU_NURBS_ERROR14 100264UL   /* curve of improper type passed as trim curve */
#define GLU_NURBS_ERROR15 100265UL   /* bgnsurface() must precede bgntrim() */
#define GLU_NURBS_ERROR16 100266UL   /* endtrim() must follow bgntrim() */
#define GLU_NURBS_ERROR17 100267UL   /* bgntrim() must precede endtrim()*/
#define GLU_NURBS_ERROR18 100268UL   /* invalid or missing trim curve*/
#define GLU_NURBS_ERROR19 100269UL   /* bgntrim() must precede pwlcurve() */
#define GLU_NURBS_ERROR20 100270UL   /* pwlcurve referenced twice*/
#define GLU_NURBS_ERROR21 100271UL   /* pwlcurve and nurbscurve mixed */
#define GLU_NURBS_ERROR22 100272UL   /* improper usage of trim data type */
#define GLU_NURBS_ERROR23 100273UL   /* nurbscurve referenced twice */
#define GLU_NURBS_ERROR24 100274UL   /* nurbscurve and pwlcurve mixed */
#define GLU_NURBS_ERROR25 100275UL   /* nurbssurface referenced twice */
#define GLU_NURBS_ERROR26 100276UL   /* invalid property */
#define GLU_NURBS_ERROR27 100277UL   /* endsurface() must follow bgnsurface() */
#define GLU_NURBS_ERROR28 100278UL   /* intersecting or misoriented trim curves */
#define GLU_NURBS_ERROR29 100279UL   /* intersecting trim curves */
#define GLU_NURBS_ERROR30 100280UL   /* UNUSED */
#define GLU_NURBS_ERROR31 100281UL   /* unconnected trim curves */
#define GLU_NURBS_ERROR32 100282UL   /* unknown knot error */
#define GLU_NURBS_ERROR33 100283UL   /* negative vertex count encountered */
#define GLU_NURBS_ERROR34 100284UL   /* negative byte-stride */
#define GLU_NURBS_ERROR35 100285UL   /* unknown type descriptor */
#define GLU_NURBS_ERROR36 100286UL   /* null control point reference */
#define GLU_NURBS_ERROR37 100287UL   /* duplicate point on pwlcurve */

/* Errors */
#define GLU_INVALID_ENUM		100900UL
#define GLU_INVALID_VALUE		100901UL
#define GLU_OUT_OF_MEMORY		100902UL
#define GLU_INCOMPATIBLE_GL_VERSION	100903UL

/* New in GLU 1.1 */
#define GLU_VERSION	100800UL
#define GLU_EXTENSIONS	100801UL


typedef struct GLUquadricObj GLUquadricObj;

typedef struct GLUtriangulatorObj GLUtriangulatorObj;

typedef struct GLUnurbsObj GLUnurbsObj;



/*
 *
 * Miscellaneous functions
 *
 */

extern void gluLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez,
		       GLdouble centerx, GLdouble centery, GLdouble centerz,
		       GLdouble upx, GLdouble upy, GLdouble upz );


extern void gluOrtho2D( GLdouble left, GLdouble right,
		        GLdouble bottom, GLdouble top );


extern void gluPerspective( GLdouble fovy, GLdouble aspect,
			    GLdouble zNear, GLdouble zFar );


extern void gluPickMatrix( GLdouble x, GLdouble y,
			   GLdouble width, GLdouble height,
			   GLint viewport[4] );

extern GLint gluProject( GLdouble objx, GLdouble objy, GLdouble objz,
                         const GLdouble modelMatrix[16],
                         const GLdouble projMatrix[16],
                         const GLint viewport[4],
                         GLdouble *winx, GLdouble *winy, GLdouble *winz );

extern GLint gluUnProject( GLdouble winx, GLdouble winy, GLdouble winz,
                           const GLdouble modelMatrix[16],
                           const GLdouble projMatrix[16],
                           const GLint viewport[4],
                           GLdouble *objx, GLdouble *objy, GLdouble *objz );

extern const GLubyte* gluErrorString( GLenum errorCode );



/*
 *
 * Mipmapping and image scaling
 *
 */

extern GLint gluScaleImage( GLenum format,
                            GLint widthin, GLint heightin,
                            GLenum typein, const void *datain,
                            GLint widthout, GLint heightout,
                            GLenum typeout, void *dataout );

extern GLint gluBuild1DMipmaps( GLenum target, GLint components,
			        GLint width, GLenum format,
			        GLenum type, const void *data );

extern GLint gluBuild2DMipmaps( GLenum target, GLint components,
                                GLint width, GLint height, GLenum format,
                                GLenum type, const void *data );



/*
 *
 * Quadrics
 *
 */

extern GLUquadricObj *gluNewQuadric( void );

extern void gluDeleteQuadric( GLUquadricObj *state );

extern void gluQuadricDrawStyle( GLUquadricObj *quadObject,
				 GLenum drawStyle );

extern void gluQuadricOrientation( GLUquadricObj *quadObject,
				   GLenum orientation );

extern void gluQuadricNormals( GLUquadricObj *quadObject, GLenum normals );

extern void gluQuadricTexture( GLUquadricObj *quadObject,
			       GLboolean textureCoords );

extern void gluQuadricCallback( GLUquadricObj *qobj,
			        GLenum which, void (*fn)(GLenum) );

extern void gluCylinder( GLUquadricObj *qobj,
			 GLdouble baseRadius,
			 GLdouble topRadius,
			 GLdouble height,
			 GLint slices, GLint stacks );

extern void gluSphere( GLUquadricObj *qobj,
		       GLdouble radius, GLint slices, GLint stacks );

extern void gluDisk( GLUquadricObj *qobj,
		     GLdouble innerRadius, GLdouble outerRadius,
		     GLint slices, GLint loops );

extern void gluPartialDisk( GLUquadricObj *qobj, GLdouble innerRadius,
			    GLdouble outerRadius, GLint slices, GLint loops,
			    GLdouble startAngle, GLdouble sweepAngle );



/*
 *
 * Nurbs
 *
 */

extern GLUnurbsObj *gluNewNurbsRenderer( void );

extern void gluDeleteNurbsRenderer( GLUnurbsObj *nobj );

extern void gluLoadSamplingMatrices( GLUnurbsObj *nobj,
				     const GLfloat modelMatrix[16],
				     const GLfloat projMatrix[16],
				     const GLint viewport[4] );

extern void gluNurbsProperty( GLUnurbsObj *nobj, GLenum property,
			      GLfloat value );

extern void gluGetNurbsProperty( GLUnurbsObj *nobj, GLenum property,
				 GLfloat *value );

extern void gluBeginCurve( GLUnurbsObj *nobj );

extern void gluEndCurve( GLUnurbsObj * nobj );

extern void gluNurbsCurve( GLUnurbsObj *nobj, GLint nknots, GLfloat *knot,
			   GLint stride, GLfloat *ctlarray, GLint order,
			   GLenum type );

extern void gluBeginSurface( GLUnurbsObj *nobj );

extern void gluEndSurface( GLUnurbsObj * nobj );

extern void gluNurbsSurface( GLUnurbsObj *nobj,
			     GLint sknot_count, GLfloat *sknot,
			     GLint tknot_count, GLfloat *tknot,
			     GLint s_stride, GLint t_stride,
			     GLfloat *ctlarray,
			     GLint sorder, GLint torder,
        	             GLenum type );

extern void gluBeginTrim( GLUnurbsObj *nobj );

extern void gluEndTrim( GLUnurbsObj *nobj );

extern void gluPwlCurve( GLUnurbsObj *nobj, GLint count, GLfloat *array,
			 GLint stride, GLenum type );

extern void gluNurbsCallback( GLUnurbsObj *nobj, GLenum which, void (*fn)(GLenum) );



/*
 *
 * Polygon tesselation
 *
 */

extern GLUtriangulatorObj* gluNewTess( void );

extern void gluTessCallback( GLUtriangulatorObj *tobj, GLenum which,
			      void (*fn)() );

extern void gluDeleteTess( GLUtriangulatorObj *tobj );

extern void gluBeginPolygon( GLUtriangulatorObj *tobj );

extern void gluEndPolygon( GLUtriangulatorObj *tobj );

extern void gluNextContour( GLUtriangulatorObj *tobj, GLenum type );

extern void gluTessVertex( GLUtriangulatorObj *tobj, GLdouble v[3],
			   void *data );



/*
 *
 * New functions in GLU 1.1
 *
 */

extern const GLubyte* gluGetString( GLenum name );



#ifdef __cplusplus
}
#endif


#endif
