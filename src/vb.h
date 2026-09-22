/* $Id: vb.h,v 1.14 1996/05/08 16:48:04 brianp Exp $ */

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
$Log: vb.h,v $
 * Revision 1.14  1996/05/08  16:48:04  brianp
 * added Win2 field for Mondello driver per Peter McDermott
 *
 * Revision 1.13  1996/04/25  20:42:33  brianp
 * increased VB_MAX to 480
 *
 * Revision 1.12  1995/12/30  01:02:15  brianp
 * changed vertex colors from floating point to fixed point integers
 *
 * Revision 1.11  1995/12/20  15:25:50  brianp
 * changed VB color indexes to GLuint
 *
 * Revision 1.10  1995/10/17  21:43:56  brianp
 * updated a few comments because of new device driver
 *
 * Revision 1.9  1995/09/28  19:38:44  brianp
 * replaced ClipFlag[] with Unclipped[]
 *
 * Revision 1.8  1995/09/25  19:24:33  brianp
 * added per-vertex glMaterial data structures
 *
 * Revision 1.7  1995/09/22  16:48:55  brianp
 * added AnyClipped and prototype Unclipped[[] and MonoColor fields to VB
 *
 * Revision 1.6  1995/09/13  14:48:18  brianp
 * updated VB struct for "vectorized" draw.c
 *
 * Revision 1.5  1995/06/20  16:19:54  brianp
 * unconditionally include integer win coord arrays
 *
 * Revision 1.4  1995/06/15  19:37:28  brianp
 * added integer window coords arrays, an experiment
 *
 * Revision 1.3  1995/06/02  13:55:34  brianp
 * changed from MAX_VERTICES to VB_MAX and VB_SIZE
 *
 * Revision 1.2  1995/05/22  20:59:34  brianp
 * Release 1.2
 *
 * Revision 1.1  1995/03/24  15:32:17  brianp
 * Initial revision
 *
 */


/*
 * Vertex buffer:  vertices from glVertex* are accumulated here until
 * the buffer is full or glEnd() is called.  Then the buffer is flushed
 * (rendered) and reset.
 */


#ifndef VB_H
#define VB_H


#include "context.h"



/*
 * Used in the bitmask for recording glMaterial calls between glBegin/glEnd:
 */
#define FRONT_AMBIENT_BIT     0x1
#define BACK_AMBIENT_BIT      0x2
#define FRONT_DIFFUSE_BIT     0x4
#define BACK_DIFFUSE_BIT      0x8
#define FRONT_SPECULAR_BIT   0x10
#define BACK_SPECULAR_BIT    0x20
#define FRONT_EMISSION_BIT   0x40
#define BACK_EMISSION_BIT    0x80
#define FRONT_SHININESS_BIT 0x100
#define BACK_SHININESS_BIT  0x200
#define FRONT_INDEXES_BIT   0x400
#define BACK_INDEXES_BIT    0x800



/* Flush VB when this number of vertices is accumulated:  (a multiple of 12) */
#define VB_MAX 480

/* Arrays must also accomodate new vertices from clipping: */
#define VB_SIZE  (VB_MAX + 2 * (6 + MAX_CLIP_PLANES))


/*
 * Vertex buffer (not saved/restored on context switches)
 */
struct vertex_buffer {
        GLfloat Obj[VB_SIZE][4];        /* Object coords */
	GLfloat Eye[VB_SIZE][4];	/* Eye coords */
	GLfloat Clip[VB_SIZE][4];	/* Clip coords */
	GLfloat Win[VB_SIZE][3];	/* Window coords */
#ifdef MONDELLO
        GLint   Win2[VB_SIZE][3];       /* Integer window coords -PFM */
                                        /* NOTE: on mondello uses correctly */
#endif

        GLfloat Normal[VB_SIZE][3];     /* Normal vectors */

        /* Colors are values in [0..RedScale], [0..GreenScale], [0,BlueScale],
         * [0,AlphScale] and stored as integers if flat shading or as fixed
         * point numbers if smooth shading.
         */
	GLfixed Fcolor[VB_SIZE][4];	/* Front colors */
	GLfixed Bcolor[VB_SIZE][4];	/* Back colors */
	GLfixed (*Color)[4];		/* == Fcolor or Bcolor */

	GLuint Findex[VB_SIZE];         /* Front color indexes */
	GLuint Bindex[VB_SIZE];         /* Back color indexes */
	GLuint *Index;			/* == Findex or Bindex */

	GLboolean Edgeflag[VB_SIZE];	/* Polygon edge flag */

        GLfloat TexCoord[VB_SIZE][4];   /* Texture coords */

        GLubyte Unclipped[VB_SIZE];	/* 0=clipped, 1=not clipped */
        GLboolean AnyClipped;		/* Were any vertices clipped? */

	GLuint Start;			/* First vertex to process */
	GLuint Count;			/* Number of vertexes in buffer */
	GLuint Free;			/* Next empty position for clipping */

        /* to handle glMaterial calls inside glBegin/glEnd: */
	GLboolean MaterialChanges;	/* True if any glMaterial was called */
        GLuint MaterialMask[VB_SIZE];	/* What material values to change */
	struct gl_material Material[VB_SIZE][2]; /* New material settings */

        GLboolean MonoColor;		/* Do all vertices have same color? */

#ifdef LEAVEOUT
	/* NEW for 1.2.? */
	GLint WinX[VB_SIZE];	/* Scaled integer window coords */
	GLint WinY[VB_SIZE];
	GLint WinZ[VB_SIZE];
#endif
};




extern struct vertex_buffer VB;


extern void gl_init_vb( void );


#endif

