/* nurbs.c */

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
$Id: nurbs.c,v 1.16 1996/01/26 15:32:08 brianp Exp $

$Log: nurbs.c,v $
 * Revision 1.16  1996/01/26  15:32:08  brianp
 * added Bogdan Sikorski's January 25th and 26th patches
 *
 * Revision 1.15  1995/12/12  21:33:36  brianp
 * fixed a few C+ errors/warnings in castings
 *
 * Revision 1.14  1995/11/03  14:11:50  brianp
 * Bogdan's November 3, 1995 updates
 *
 * Revision 1.13  1995/09/20  18:25:57  brianp
 * removed Bogdan's old email address
 *
 * Revision 1.12  1995/07/31  16:29:49  brianp
 * applied Bogdan's patch from July 31, 1995
 *
 * Revision 1.11  1995/07/28  21:36:36  brianp
 * changed all GLUenum to GLenum
 *
 * Revision 1.10  1995/07/28  14:44:14  brianp
 * incorporated Bogdan's July 27 revisions
 *
 * Revision 1.9  1995/05/30  13:12:39  brianp
 * added gluNurbsCallback() stub
 *
 * Revision 1.8  1995/05/29  20:09:11  brianp
 * added gluGetNurbsProperty()
 *
 * Revision 1.7  1995/05/24  13:44:11  brianp
 * added gluBeginTrim, gluEndTrim, gluPwlCurve stubs
 *
 * Revision 1.6  1995/05/22  16:56:20  brianp
 * Release 1.2
 *
 * Revision 1.5  1995/05/16  19:17:21  brianp
 * minor changes to allow compilation with real OpenGL headers
 *
 * Revision 1.4  1995/04/28  20:06:23  brianp
 * print an error message when trying to use gluNewNurbsRenderer()
 *
 * Revision 1.3  1995/04/28  14:37:32  brianp
 * moved GLUnurbsObj struct from .h to .c file
 *
 * Revision 1.2  1995/03/04  19:39:18  brianp
 * version 1.1 beta
 *
 * Revision 1.1  1995/02/24  15:45:01  brianp
 * Initial revision
 *
 */


/*
 * NURBS implementation written by Bogdan Sikorski (bogdan@cira.it)
 * See README2 for more info.
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "nurbs.h"


void
call_user_error( GLUnurbsObj *nobj, GLenum error )
{
	nobj->error=error;
	if(nobj->error_callback != NULL) {
		(*(nobj->error_callback))(error);
	}
	else {
	   printf("NURBS error %d %s\n", error, gluErrorString(error) );
	}
}



GLUnurbsObj *gluNewNurbsRenderer( void )
{
   GLUnurbsObj *n;
   GLfloat tmp_viewport[4];
   GLint i,j;

   n = (GLUnurbsObj *) malloc( sizeof(GLUnurbsObj) );
   if (n) {
      /* init */
      n->culling=GL_FALSE;
      n->nurbs_type=GLU_NURBS_NONE;
      n->error=GLU_NO_ERROR;
      n->error_callback=NULL;
      n->auto_load_matrix=GL_TRUE;
      n->sampling_tolerance=50.0;
      n->display_mode=GLU_FILL;
      /* in case the user doesn't supply the sampling matrices */
      /* set projection and modelview to identity */
      for(i=0;i<4;i++)
      	for(j=0;j<4;j++)
      		if(i==j)
      		{
				n->sampling_matrices.model[i*4+j]=1.0;
				n->sampling_matrices.proj[i*4+j]=1.0;
			}
			else
      		{
				n->sampling_matrices.model[i*4+j]=0.0;
				n->sampling_matrices.proj[i*4+j]=0.0;
			}
	  /* and set the viewport sampling matrix to current ciewport */
	  glGetFloatv(GL_VIEWPORT,tmp_viewport);
	  for(i=0;i<4;i++)
		  n->sampling_matrices.viewport[i]=tmp_viewport[i];
	  n->trim=NULL;
   }
   return n;
}



void gluDeleteNurbsRenderer( GLUnurbsObj *nobj )
{
   if (nobj) {
      free( nobj );
   }
}



void gluLoadSamplingMatrices( GLUnurbsObj *nobj,
			      const GLfloat modelMatrix[16],
			      const GLfloat projMatrix[16],
			      const GLint viewport[4] )
{
	GLint	i;

	for(i=0;i<16;i++)
	{
		nobj->sampling_matrices.model[i]=modelMatrix[i];
		nobj->sampling_matrices.proj[i]=projMatrix[i];
	}
	for(i=0;i<4;i++)
		nobj->sampling_matrices.viewport[i]=viewport[i];
}


void gluNurbsProperty( GLUnurbsObj *nobj, GLenum property, GLfloat value )
{
   GLenum val;

   switch (property) {
      case GLU_SAMPLING_TOLERANCE:
      	 if(value <= 0.0)
      	 {
      	 	call_user_error(nobj,GLU_INVALID_VALUE);
      	 	return;
		 }
         nobj->sampling_tolerance=value;
         break;
      case GLU_DISPLAY_MODE:
         val=(GLenum)value;
         if(val!=GLU_FILL && val!=GLU_OUTLINE_POLYGON && val!=GLU_OUTLINE_PATCH)
         {
         	call_user_error(nobj,GLU_INVALID_ENUM);
         	return;
		 }
		 if(nobj->nurbs_type==GLU_NURBS_CURVE)
         {
         	call_user_error(nobj,GLU_NURBS_ERROR26);
         	return;
		 }
         nobj->display_mode=val;
if(val==GLU_OUTLINE_PATCH)
	fprintf(stderr,"NURBS, for the moment, can display only in POLYGON mode\n");
         break;
      case GLU_CULLING:
         val=(GLenum)value;
         if(val!=GL_TRUE && val!=GL_FALSE)
         {
         	call_user_error(nobj,GLU_INVALID_ENUM);
         	return;
		 }
         nobj->culling = (GLboolean) value;
         break;
      case GLU_AUTO_LOAD_MATRIX:
         val=(GLenum)value;
         if(val!=GL_TRUE && val!=GL_FALSE)
         {
         	call_user_error(nobj,GLU_INVALID_ENUM);
         	return;
		 }
         nobj->auto_load_matrix = (GLboolean) value;
         break;
      default:
         call_user_error(nobj,GLU_NURBS_ERROR26);
   }
}


void gluGetNurbsProperty( GLUnurbsObj *nobj, GLenum property, GLfloat *value )
{
   switch (property) {
      case GLU_SAMPLING_TOLERANCE:
         *value = nobj->sampling_tolerance;
         break;
      case GLU_DISPLAY_MODE:
         *value = (GLfloat) nobj->display_mode;
         break;
      case GLU_CULLING:
	 *value = nobj->culling ? 1.0 : 0.0;
         break;
      case GLU_AUTO_LOAD_MATRIX:
         *value = nobj->auto_load_matrix ? 1.0 : 0.0;
	 break;
      default:
         call_user_error(nobj,GLU_INVALID_ENUM);
   }
}



void gluBeginCurve( GLUnurbsObj *nobj )
{
	if(nobj->nurbs_type==GLU_NURBS_CURVE)
	{
		call_user_error(nobj,GLU_NURBS_ERROR6);
		return;
	}
	nobj->nurbs_type=GLU_NURBS_CURVE;
	nobj->curve.geom.type=GLU_INVALID_ENUM;
	nobj->curve.color.type=GLU_INVALID_ENUM;
	nobj->curve.texture.type=GLU_INVALID_ENUM;
	nobj->curve.normal.type=GLU_INVALID_ENUM;
}


void gluEndCurve( GLUnurbsObj * nobj )
{
	if(nobj->nurbs_type==GLU_NURBS_NONE)
	{
		call_user_error(nobj,GLU_NURBS_ERROR7);
		return;
	}
	if(nobj->curve.geom.type==GLU_INVALID_ENUM)
	{
		call_user_error(nobj,GLU_NURBS_ERROR8);
		nobj->nurbs_type=GLU_NURBS_NONE;
		return;
	}
	glPushAttrib( (GLbitfield) (GL_EVAL_BIT | GL_ENABLE_BIT) );
	glDisable(GL_MAP1_VERTEX_3);
	glDisable(GL_MAP1_VERTEX_4);
	glDisable(GL_MAP1_INDEX);
	glDisable(GL_MAP1_COLOR_4);
	glDisable(GL_MAP1_NORMAL);
	glDisable(GL_MAP1_TEXTURE_COORD_1);
	glDisable(GL_MAP1_TEXTURE_COORD_2);
	glDisable(GL_MAP1_TEXTURE_COORD_3);
	glDisable(GL_MAP1_TEXTURE_COORD_4);
	glDisable(GL_MAP2_VERTEX_3);
	glDisable(GL_MAP2_VERTEX_4);
	glDisable(GL_MAP2_INDEX);
	glDisable(GL_MAP2_COLOR_4);
	glDisable(GL_MAP2_NORMAL);
	glDisable(GL_MAP2_TEXTURE_COORD_1);
	glDisable(GL_MAP2_TEXTURE_COORD_2);
	glDisable(GL_MAP2_TEXTURE_COORD_3);
	glDisable(GL_MAP2_TEXTURE_COORD_4);
	do_nurbs_curve(nobj);
	glPopAttrib();
	nobj->nurbs_type=GLU_NURBS_NONE;
}


void gluNurbsCurve( GLUnurbsObj *nobj, GLint nknots, GLfloat *knot,
		    GLint stride, GLfloat *ctlarray, GLint order, GLenum type )
{
	if(nobj->nurbs_type==GLU_NURBS_TRIM)
	{
		nurbs_trim *ptr1;
		trim_list *ptr2;

return;
		if(type!=GLU_MAP1_TRIM_2 && type!=GLU_MAP1_TRIM_3)
		{
			call_user_error(nobj,GLU_NURBS_ERROR14);
			return;
		}
		for(ptr1=nobj->trim;ptr1->next;ptr1=ptr1->next);
		if(ptr1->trim_loop)
		{
			for(ptr2=ptr1->trim_loop;ptr2->next;ptr2=ptr2->next);
			if((ptr2->next=(trim_list *)malloc(sizeof(trim_list)))==NULL)
			{
				call_user_error(nobj,GLU_OUT_OF_MEMORY);
				return;
			}
			ptr2=ptr2->next;
		}
		else
		{
			if((ptr2=(trim_list *)malloc(sizeof(trim_list)))==NULL)
			{
				call_user_error(nobj,GLU_OUT_OF_MEMORY);
				return;
			}
			ptr1->trim_loop=ptr2;
		}
		ptr2->trim_type=GLU_TRIM_NURBS;
		ptr2->curve.nurbs_curve.knot_count=nknots;
		ptr2->curve.nurbs_curve.knot=knot;
		ptr2->curve.nurbs_curve.stride=stride;
		ptr2->curve.nurbs_curve.ctrlarray=ctlarray;
		ptr2->curve.nurbs_curve.order=order;
		ptr2->curve.nurbs_curve.dim= (type==GLU_MAP1_TRIM_2 ? 2 : 3 );
		ptr2->curve.nurbs_curve.type=type;
		ptr2->next=NULL;
	}
	else
	{
		if(type==GLU_MAP1_TRIM_2 || type==GLU_MAP1_TRIM_3)
		{
			call_user_error(nobj,GLU_NURBS_ERROR22);
			return;
		}
		if(nobj->nurbs_type!=GLU_NURBS_CURVE)
		{
			call_user_error(nobj,GLU_NURBS_ERROR10);
			return;
		}
		switch(type)
		{
			case GL_MAP1_VERTEX_3:
			case GL_MAP1_VERTEX_4:
				if(nobj->curve.geom.type!=GLU_INVALID_ENUM)
				{
					call_user_error(nobj,GLU_NURBS_ERROR8);
					return;
				}
				nobj->curve.geom.type=type;
				nobj->curve.geom.knot_count=nknots;
				nobj->curve.geom.knot=knot;
				nobj->curve.geom.stride=stride;
				nobj->curve.geom.ctrlarray=ctlarray;
				nobj->curve.geom.order=order;
				break;
			case GL_MAP1_INDEX:
			case GL_MAP1_COLOR_4:
				nobj->curve.color.type=type;
				nobj->curve.color.knot_count=nknots;
				nobj->curve.color.knot=knot;
				nobj->curve.color.stride=stride;
				nobj->curve.color.ctrlarray=ctlarray;
				nobj->curve.color.order=order;
				break;
			case GL_MAP1_NORMAL:
				nobj->curve.normal.type=type;
				nobj->curve.normal.knot_count=nknots;
				nobj->curve.normal.knot=knot;
				nobj->curve.normal.stride=stride;
				nobj->curve.normal.ctrlarray=ctlarray;
				nobj->curve.normal.order=order;
				break;
			case GL_MAP1_TEXTURE_COORD_1:
			case GL_MAP1_TEXTURE_COORD_2:
			case GL_MAP1_TEXTURE_COORD_3:
			case GL_MAP1_TEXTURE_COORD_4:
				nobj->curve.texture.type=type;
				nobj->curve.texture.knot_count=nknots;
				nobj->curve.texture.knot=knot;
				nobj->curve.texture.stride=stride;
				nobj->curve.texture.ctrlarray=ctlarray;
				nobj->curve.texture.order=order;
				break;
			default:
				 call_user_error(nobj,GLU_INVALID_ENUM);
		}
	}
}


void gluBeginSurface( GLUnurbsObj *nobj )
{
	switch(nobj->nurbs_type)
	{
		case GLU_NURBS_NONE:
			nobj->nurbs_type=GLU_NURBS_SURFACE;
			nobj->surface.geom.type=GLU_INVALID_ENUM;
			nobj->surface.color.type=GLU_INVALID_ENUM;
			nobj->surface.texture.type=GLU_INVALID_ENUM;
			nobj->surface.normal.type=GLU_INVALID_ENUM;
			break;
		case GLU_NURBS_TRIM:
			call_user_error(nobj,GLU_NURBS_ERROR16);
			break;
		case GLU_NURBS_SURFACE:
		case GLU_NURBS_NO_TRIM:
		case GLU_NURBS_TRIM_DONE:
			call_user_error(nobj,GLU_NURBS_ERROR27);
			break;
		case GLU_NURBS_CURVE:
			call_user_error(nobj,GLU_NURBS_ERROR6);
			break;
	}
}


void gluEndSurface( GLUnurbsObj * nobj )
{
	switch(nobj->nurbs_type)
	{
		case GLU_NURBS_NONE:
			call_user_error(nobj,GLU_NURBS_ERROR13);
			break;
		case GLU_NURBS_TRIM:
			call_user_error(nobj,GLU_NURBS_ERROR12);
			break;
		case GLU_NURBS_TRIM_DONE:
/*			if(nobj->trim->trim_loop==NULL)
			{
				call_user_error(nobj,GLU_NURBS_ERROR18);
				return;
			}*/
			/* no break - fallthrough */
		case GLU_NURBS_NO_TRIM:
			glPushAttrib( (GLbitfield) (GL_EVAL_BIT | GL_ENABLE_BIT) );
			glDisable(GL_MAP2_VERTEX_3);
			glDisable(GL_MAP2_VERTEX_4);
			glDisable(GL_MAP2_INDEX);
			glDisable(GL_MAP2_COLOR_4);
			glDisable(GL_MAP2_NORMAL);
			glDisable(GL_MAP2_TEXTURE_COORD_1);
			glDisable(GL_MAP2_TEXTURE_COORD_2);
			glDisable(GL_MAP2_TEXTURE_COORD_3);
			glDisable(GL_MAP2_TEXTURE_COORD_4);
/*			glDisable(GL_MAP1_VERTEX_3);
			glDisable(GL_MAP1_VERTEX_4);
			glDisable(GL_MAP1_INDEX);
			glDisable(GL_MAP1_COLOR_4);
			glDisable(GL_MAP1_NORMAL);
			glDisable(GL_MAP1_TEXTURE_COORD_1);
			glDisable(GL_MAP1_TEXTURE_COORD_2);
			glDisable(GL_MAP1_TEXTURE_COORD_3);
			glDisable(GL_MAP1_TEXTURE_COORD_4);*/
			do_nurbs_surface(nobj);
			glPopAttrib();
			break;
		default:
			call_user_error(nobj,GLU_NURBS_ERROR8);
	}
	nobj->nurbs_type=GLU_NURBS_NONE;
}


void gluNurbsSurface( GLUnurbsObj *nobj,
		      GLint sknot_count, GLfloat *sknot,
		      GLint tknot_count, GLfloat *tknot,
		      GLint s_stride, GLint t_stride,
		      GLfloat *ctrlarray,
		      GLint sorder, GLint torder,
		      GLenum type )
{
	if(nobj->nurbs_type==GLU_NURBS_NO_TRIM || nobj->nurbs_type==GLU_NURBS_TRIM ||
		nobj->nurbs_type==GLU_NURBS_TRIM_DONE)
	{
		if(type==GL_MAP2_VERTEX_3 || type==GL_MAP2_VERTEX_4)
		{
			call_user_error(nobj,GLU_NURBS_ERROR8);
			return;
		}
	}
	else
	if(nobj->nurbs_type!=GLU_NURBS_SURFACE)
	{
		call_user_error(nobj,GLU_NURBS_ERROR11);
		return;
	}
	switch(type)
	{
		case GL_MAP2_VERTEX_3:
		case GL_MAP2_VERTEX_4:
			nobj->surface.geom.sknot_count=sknot_count;
			nobj->surface.geom.sknot=sknot;
			nobj->surface.geom.tknot_count=tknot_count;
			nobj->surface.geom.tknot=tknot;
			nobj->surface.geom.s_stride=s_stride;
			nobj->surface.geom.t_stride=t_stride;
			nobj->surface.geom.ctrlarray=ctrlarray;
			nobj->surface.geom.sorder=sorder;
			nobj->surface.geom.torder=torder;
			nobj->surface.geom.type=type;
			nobj->nurbs_type=GLU_NURBS_NO_TRIM;
			break;
		case GL_MAP2_INDEX:
		case GL_MAP2_COLOR_4:
			nobj->surface.color.sknot_count=sknot_count;
			nobj->surface.color.sknot=sknot;
			nobj->surface.color.tknot_count=tknot_count;
			nobj->surface.color.tknot=tknot;
			nobj->surface.color.s_stride=s_stride;
			nobj->surface.color.t_stride=t_stride;
			nobj->surface.color.ctrlarray=ctrlarray;
			nobj->surface.color.sorder=sorder;
			nobj->surface.color.torder=torder;
			nobj->surface.color.type=type;
			break;
		case GL_MAP2_NORMAL:
			nobj->surface.normal.sknot_count=sknot_count;
			nobj->surface.normal.sknot=sknot;
			nobj->surface.normal.tknot_count=tknot_count;
			nobj->surface.normal.tknot=tknot;
			nobj->surface.normal.s_stride=s_stride;
			nobj->surface.normal.t_stride=t_stride;
			nobj->surface.normal.ctrlarray=ctrlarray;
			nobj->surface.normal.sorder=sorder;
			nobj->surface.normal.torder=torder;
			nobj->surface.normal.type=type;
			break;
		case GL_MAP2_TEXTURE_COORD_1:
		case GL_MAP2_TEXTURE_COORD_2:
		case GL_MAP2_TEXTURE_COORD_3:
		case GL_MAP2_TEXTURE_COORD_4:
			nobj->surface.texture.sknot_count=sknot_count;
			nobj->surface.texture.sknot=sknot;
			nobj->surface.texture.tknot_count=tknot_count;
			nobj->surface.texture.tknot=tknot;
			nobj->surface.texture.s_stride=s_stride;
			nobj->surface.texture.t_stride=t_stride;
			nobj->surface.texture.ctrlarray=ctrlarray;
			nobj->surface.texture.sorder=sorder;
			nobj->surface.texture.torder=torder;
			nobj->surface.texture.type=type;
			break;
		default:
			 call_user_error(nobj,GLU_INVALID_ENUM);
	}
}


void
gluNurbsCallback( GLUnurbsObj *nobj, GLenum which, void (*fn)(GLenum))
{
	nobj->error_callback=fn;
	if(which!=GLU_ERROR)
		call_user_error(nobj,GLU_INVALID_ENUM);
}

void
gluBeginTrim( GLUnurbsObj *nobj )
{
	nurbs_trim *ptr;

	if(nobj->nurbs_type!=GLU_NURBS_TRIM_DONE)
		if(nobj->nurbs_type!=GLU_NURBS_NO_TRIM)
		{
			call_user_error(nobj,GLU_NURBS_ERROR15);
			return;
		}
	nobj->nurbs_type=GLU_NURBS_TRIM;
fprintf(stderr,"NURBS - trimming not supported yet\n");
/*	if((ptr=(nurbs_trim *)malloc(sizeof(nurbs_trim)))==NULL)
	{
		call_user_error(nobj,GLU_OUT_OF_MEMORY);
		return;
	}
	if(nobj->trim)
	{
		nurbs_trim *tmp_ptr;

		for(tmp_ptr=nobj->trim;tmp_ptr->next;tmp_ptr=tmp_ptr->next);
		tmp_ptr->next=ptr;
	}
	else
		nobj->trim=ptr;
	ptr->trim_loop=NULL;
	ptr->segments=NULL;
	ptr->next=NULL;*/
}

void
gluPwlCurve( GLUnurbsObj *nobj, GLint count, GLfloat *array, GLint stride,
	GLenum type)
{
	nurbs_trim *ptr1;
	trim_list *ptr2;

	if(nobj->nurbs_type==GLU_NURBS_CURVE)
	{
		call_user_error(nobj,GLU_NURBS_ERROR9);
		return;
	}
	if(nobj->nurbs_type==GLU_NURBS_NONE)
	{
		call_user_error(nobj,GLU_NURBS_ERROR19);
		return;
	}
	if(type!=GLU_MAP1_TRIM_2 && type!=GLU_MAP1_TRIM_3)
	{
		call_user_error(nobj,GLU_NURBS_ERROR14);
		return;
	}
/*	for(ptr1=nobj->trim;ptr1->next;ptr1=ptr1->next);
	if(ptr1->trim_loop)
	{
		for(ptr2=ptr1->trim_loop;ptr2->next;ptr2=ptr2->next);
		if((ptr2->next=(trim_list *)malloc(sizeof(trim_list)))==NULL)
		{
			call_user_error(nobj,GLU_OUT_OF_MEMORY);
			return;
		}
		ptr2=ptr2->next;
	}
	else
	{
		if((ptr2=(trim_list *)malloc(sizeof(trim_list)))==NULL)
		{
			call_user_error(nobj,GLU_OUT_OF_MEMORY);
			return;
		}
		ptr1->trim_loop=ptr2;
	}
	ptr2->trim_type=GLU_TRIM_PWL;
	ptr2->curve.pwl_curve.pt_count=count;
	ptr2->curve.pwl_curve.ctrlarray=array;
	ptr2->curve.pwl_curve.stride=stride;
	ptr2->curve.pwl_curve.dim= (type==GLU_MAP1_TRIM_2 ? 2 : 3 );
	ptr2->curve.pwl_curve.type=type;
	ptr2->next=NULL;*/
}

void
gluEndTrim( GLUnurbsObj *nobj )
{
	if(nobj->nurbs_type!=GLU_NURBS_TRIM)
	{
		call_user_error(nobj,GLU_NURBS_ERROR17);
		return;
	}
	nobj->nurbs_type=GLU_NURBS_TRIM_DONE;
}
