/* tess.c */

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
$Id: tess.h,v 1.6 1995/07/28 21:36:26 brianp Exp $

$Log: tess.h,v $
 * Revision 1.6  1995/07/28  21:36:26  brianp
 * changed all GLUenum to GLenum
 *
 * Revision 1.5  1995/05/26  19:06:52  brianp
 * replaced GLenum with GLUenum in tesselator error callback function
 *
 * Revision 1.4  1995/05/22  16:56:20  brianp
 * Release 1.2
 *
 * Revision 1.3  1995/05/16  19:17:21  brianp
 * minor changes to allow compilation with real OpenGL headers
 *
 * Revision 1.2  1995/04/28  20:07:08  brianp
 * renamed struct GLUtriangulatorObj to struct glu_triangulator_obj
 *
 * Revision 1.1  1995/04/28  16:19:34  brianp
 * Initial revision
 *
 */


/*
 * This file is part of the polygon tesselation code contributed by
 * Bogdan Sikorski
 */


#include "gluP.h"

#define EPSILON 1e-06 /* epsilon for double precision compares */

typedef enum
{
	OXY,
	OYZ,
	OXZ
} projection_type;

typedef struct callbacks_str
{
	void (*begin)( GLenum mode );
	void (*edgeFlag)( GLboolean flag );
	void (*vertex)( GLvoid *v );
	void (*end)( void );
	void (*error)( GLenum err );
} tess_callbacks;

typedef struct vertex_str
{
	void				*data;
	GLdouble			location[3];
	GLdouble			x,y;
	GLboolean			edge_flag;
	struct vertex_str	*shadow_vertex;
	struct vertex_str	*next,*previous;
} tess_vertex;

typedef struct contour_str
{
	GLenum				type;
	GLuint				vertex_cnt;
	GLdouble			area;
	GLenum				orientation;
	struct vertex_str	*vertices,*last_vertex;
	struct contour_str	*next,*previous;
} tess_contour;

typedef struct polygon_str
{
	GLuint				vertex_cnt;
	GLdouble			A,B,C,D;
	GLdouble			area;
	GLenum				orientation;
	struct vertex_str	*vertices,*last_vertex;
} tess_polygon;

struct GLUtriangulatorObj
{
	tess_contour		*contours,*last_contour;
	GLuint				contour_cnt;
	tess_callbacks		callbacks;
	tess_polygon		*current_polygon;
	GLenum				error;
	GLdouble			A,B,C,D;
	projection_type		projection;
};


extern void tess_call_user_error(GLUtriangulatorObj *,GLenum);

