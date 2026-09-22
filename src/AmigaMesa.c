/* $Id: AmigaMesa.c,v 1.7 Exp $ */

/*
 * Mesa 3-D graphics library
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
$Log: AmigaMesa.c,v1.6 $
 * Revision 1.7  1996/05/21  23:08:42  StefanZ
 * A few bug and enforcer fixes
 *
 * Revision 1.6  1996/04/29  22:14:31  StefanZ
 * BugFixes reported by by Daniel Jönsson
 *
 * Revision 1.5  1996/03/14  23:54:33  StefanZ
 * Doublebuffer & Tmprastport seams to work (big speed improvment)
 * a fastpolydraw is also implemented
 *
 * Revision 1.4  1996/03/07  16:55:04  StefanZ
 * Much of the code works now (RGB mode is simulated) Doublebuffers... (didn't work)
 *
 * Revision 1.3  1996/02/29  02:12:45  StefanZ
 * First sight of colors (even the right ones) maglight.c works
 *
 * Revision 1.2  1996/02/25  13:11:16  StefanZ
 * First working version. Draws everything with the same color
 * (Colormaping is now urgent needed)
 *
 * Revision 1.1  1996/02/23  22:01:15  StefanZ
 * Made changes to match latest version of ddsample 1.5
 *
 * Revision 1.0  1996/02/21  11:01:15  StefanZ
 * File created from ddsample.c ver 1.3 and amesa.c ver 1.5
 * in a brave atempt to rebuild the amiga version
 *
 */

/*
TODO:
Have two colortables in dubbelduffering so that it dont get deallocated when showing. (memory saving)
Dynamic allocate the vectorbuffer for polydrawing. (Speed improvment)
Draw lines insted off pixles in Write_monocolor_span.(speed improvment)
Use writepixelarray in sted of WritePixel in write_index_span & write_color_span. (speed improvment)

Check make_temp_Rport and dont use areafill if it fail's.

IDEAS:
 Make drawing on a Cyber-GFX in full color. (If someone gave me a 
 gfx-card I would speed up on this :)
 Make the gl a sharedlibrary. (Have ben started)


*/


/*
 * Note that you'll usually have to flip Y coordinates since Mesa's
 * window coordinates start at the bottom and increase upward.  Most
 * window system's Y-axis increases downward
 *
 * See dd.h for more device driver info.
 * See the other device driver implementations for ideas.
 *
 * The Makefile should compile this module along with the rest of
 * the core Mesa library.
 */


#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>




#include <stdlib.h>
#include <stdio.h>
#include "gl/AmigaMesa.h"
#include "context.h"
#include "dd.h"
#include "xform.h"
#include "vb.h"

#define MAX_POLYGON 300




/**********************************************************************/
/*****              Internal Data                                 *****/
/**********************************************************************/
struct amigamesa_context * Current = NULL;


/**********************************************************************/
/*****              Some Usefull code                             *****/
/**********************************************************************/



static struct RastPort *make_rastport( int width, int height, int depth );
static void destroy_rastport( struct RastPort *rp );
static BOOL make_temp_raster( struct RastPort *rp );
static void destroy_temp_raster( struct RastPort *rp );
static void AllocOneLine(struct amigamesa_context *AmigaMesaCreateContext);
static void FreeOneLine(struct amigamesa_context *AmigaMesaCreateContext);

#define c8to32(x) (((((((x)<<8)|(x))<<8)|(x))<<8)|(x))

__inline int RGBA(GLubyte r,GLubyte g,GLubyte b,GLubyte a)
	{
	int pen;
	
/*printf("RGBA(%d,%d,%d,%d)",r,g,b,a);getchar();*/
	pen=ObtainBestPen(Current->window->WScreen->ViewPort.ColorMap,
			c8to32((ULONG)r),c8to32((ULONG)g),c8to32((ULONG)b),
			OBP_Precision,PRECISION_GUI,TAG_DONE);
	if(!(pen==-1))
		{
		Current->mypen[pen]+=1;
		}
	else
		{
		pen=FindColor(Current->window->WScreen->ViewPort.ColorMap,
									c8to32((ULONG)r),c8to32((ULONG)g),c8to32((ULONG)b),-1); 
									/* If all pen is no sharable  take one */
/*	kprintf("(no sharable pen)\n",pen);*/

		}
/*printf("=%d\n",pen);*/
	return pen;
	}
	

__inline GLint FIXx(GLint x)
	{
	return(Current->left + x);
	}

__inline GLint FIXy(GLint y)
	{
	return(Current->bottom - y);
	}


/**********************************************************************/
/*****              Miscellaneous device driver funcs             *****/
/**********************************************************************/

static void afinish( void )
{
   /* implements glFinish if possible */

}



static void aflush( void )
{
   /* implements glFlush if possible */

}



static void aclear_index( GLuint index )
{
   /* implement glClearIndex */
   /* usually just save the value in the context struct */
/*printf("aclear_index=glClearIndex=%d\n",index);*/
	Current->clearpixel=Current->penconv[index];
}



static void aclear_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   /* implement glClearColor */
   /* color components are floats in [0,1] */
   /* usually just save the value in the context struct */
/*printf("aclear_color=glClearColor(%d,%d,%d,%d)\n",r,g,b,a);*/
	/* @@@ TODO FREE COLOR IF NOT USED */
	Current->clearpixel=RGBA(r,g,b,a);
}



static void aclear( GLboolean all, GLint x, GLint y, GLint width, GLint height )
{
/*
 * Clear the specified region of the color buffer using the clear color
 * or index as specified by one of the two functions above.
 * If all==GL_TRUE, clear whole buffer
 */
/*printf("aclear(%d,%d,%d,%d,%d)\n",all,x,y,width,height);*/
	SetAPen(Current->rp,Current->clearpixel);
	if(all)
		{

		RectFill(Current->rp,Current->left,Current->window->BorderTop,Current->left+Current->width-1,Current->bottom);

		if (Current->rgb_flag)
			{
			int I;
			for(I=0;I<=255;I++)  /* Dealocate pens is in RGB mode */
				{
				while (Current->mypen[I]!=0)			/* TODO This may free some others pen also */
					{
					Current->mypen[I]-=1;
					ReleasePen(Current->window->WScreen->ViewPort.ColorMap,I);
					}
				}
			}
		}
	else
		{
		if(Current->rp!=0)
			{
/*			printf("RectFill(0x%x,%d,%d,%d,%d)\n",Current->rp,FIXx(x),FIXy(y)-height,width,FIXy(y));*/
			RectFill(Current->rp,FIXx(x),FIXy(y)-height,width,FIXy(y));
			}
		else
			printf("Serius error Current->rp=0 detected in aclear() in file amigamesa.c\n");
		}
	
}


static void aset_index( GLuint index )
{
   /* Set the current color index. */
/*printf("aset_index(%d)\n",index);*/
	Current->pixel = Current->penconv[index];
}



static void aset_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
/*printf("aset_color(%d,%d,%d,%d)\n",r,g,b,a);*/

		/* Set the current RGBA color. */
		/* r is in 0..CC.RedScale */
		/* g is in 0..CC.GreenScale */
		/* b is in 0..CC.BlueScale */
		/* a is in 0..CC.AlphaScale */
	Current->pixel = RGBA(r,g,b,a);
	/*(a << 24) | (r << 16) | (g << 8) | b;*/
}



static GLboolean aindex_mask( GLuint mask )
{
   /* implement glIndexMask if possible, else return GL_FALSE */
/*printf("aindex_mask(0x%x)\n",mask);*/
	Current->rp->Mask = (UBYTE) mask;

	return(GL_TRUE);
}



static GLboolean acolor_mask( GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask)
{
   /* implement glColorMask if possible, else return GL_FALSE */
	return(GL_FALSE);
	
}



static GLboolean alogicop( GLenum op )
{
   /*
    * Implements glLogicOp if possible.  Return GL_TRUE if the device driver
    * can perform the operation, otherwise return GL_FALSE.  If GL_FALSE
    * is returned, the logic op will be done in software by Mesa.
    */
	return(GL_FALSE);
}


static void adither( GLboolean enable )
{
   /* enable/disable dithering if applicable */
}



static GLboolean aset_buffer( GLenum mode )
{
   /* set the current drawing/reading buffer, return GL_TRUE or GL_FALSE */
   /* for success/failure */
	
/*	amiga_setup_DD_pointers(); in ddsample is this right?????*/
/*	printf("aset_buffer TODO\n");*/
	

/* TODO implemed a set of buffers */
	if (mode==GL_FRONT)
		{
		return(GL_TRUE);
		}
	else if (mode==GL_BACK)
		{
		return(GL_TRUE);
		}
	else
		{
		return(GL_FALSE);
		}
}



static void abuffer_size( GLuint *width, GLuint *height, GLuint *depth )
{
   /* return the width, height and depth (bits/pixel) of the current buffer */
   /* if anything special has to been done when the buffer/window is        */
   /* resized, do it now                                                    */
/*	printf("abuffer_size\n");*/


	if(!((*width=Current->width) ==(Current->window->Width - Current->window->BorderLeft - Current->window->BorderRight)&&
	 ((*height=Current->height) ==(Current->window->Height - Current->window->BorderTop - Current->window->BorderBottom))))
		{
		FreeOneLine(Current);
		Current->width =Current->window->Width - Current->window->BorderLeft - Current->window->BorderRight;
		Current->height=Current->window->Height - Current->window->BorderTop - Current->window->BorderBottom;
		Current->left = Current->window->BorderLeft;
		Current->bottom = Current->window->Height - Current->window->BorderBottom - 1;

		destroy_temp_raster( Current->rp); /* deallocate temp raster */


		if (Current->db_flag)
			{
			if (Current->back_rp)							/* Fix dubbel buffer */
				{
				destroy_rastport(Current->back_rp);
				}
			if((Current->back_rp = make_rastport(Current->window->Width,Current->window->Height,Current->depth))==NULL)
				{
				Current->rp = Current->front_rp;
				printf("To little mem free. Couldn't allocate Dubblebuffer in this size.\n");
				}
			else
				{
				Current->rp=Current->back_rp;
				}
			}
			
		if (Current->rgb_flag)
			{
	      /* RGBA color buffers */
			Current->gl_ctx->RGBAflag = GL_TRUE;
			*depth= 24; /* TODO */
			Current->depth =Current->depth = Current->window->RPort->BitMap->Depth;
			}
		else
	   	{
	      /* CI color buffers */
			Current->gl_ctx->RGBAflag = GL_FALSE;
			*depth=Current->depth = Current->window->RPort->BitMap->Depth;

			}

		if(!make_temp_raster( Current->rp ))
			printf("Error allocating TmpRasterPort\n");
	
		AllocOneLine(Current);
	}

}	


/**********************************************************************/
/*****           Accelerated point, line, polygon rendering       *****/
/**********************************************************************/


static void afast_points_function( GLuint first, GLuint last )
{

   /* Render a number of points by some hardware/OS accerated method */

	int i,col;
/* printf("afast_points_function\n");*/
	

   if (VB.MonoColor) {
      /* draw all points using the current color (set_color) */
			SetAPen(Current->rp,Current->pixel);
/*			printf("VB.MonoColor\n");*/
      for (i=first;i<=last;i++) {
         if (VB.Unclipped[i]) {
            /* compute window coordinate */
            int x, y;
            x =FIXx((GLint) (VB.Win[i][0]));
            y =FIXy((GLint) (VB.Win[i][1]));
						WritePixel(Current->rp,x,y);
/*						printf("WritePixel(%d,%d)\n",x,y);*/
         }
      }
   }
   else {
      /* each point is a different color */
/*			printf("!VB.MonoColor\n");*/

      for (i=first;i<=last;i++) {
         /*if (VB.Unclipped[i])*/ {
            int x, y;
            x =FIXx((GLint) (VB.Win[i][0]));
          	y =FIXy((GLint) (VB.Win[i][1])); 
          	col=*VB.Color[i];
          	SetAPen(Current->rp,Current->penconv[col]);
						WritePixel(Current->rp,x,y);
/*						printf("WritePixel(%d,%d)\n",x,y);*/
         }
      }
   }
}



static points_func achoose_points_function( void )
{
/*printf("achoose_points_function\n");*/
   /* Examine the current rendering state and return a pointer to a */
   /* fast point-rendering function if possible. */
   if (CC.Point.Size==1.0 && !CC.Point.SmoothFlag && CC.RasterMask==0
       && !CC.Texture.Enabled /*&&  ETC, ETC */) {
      return afast_points_function;
   }
   else {
      return afast_points_function;
/*      return NULL;*/
   }
}



static void afast_line_function( GLuint v0, GLuint v1, GLuint pv )
{

  										 /* Render a line by some hardware/OS accerated method */
	int x0, y0, x1, y1;
/*	printf("afast_line_function\n");*/

	if (VB.MonoColor)
		{
		SetAPen(Current->rp,Current->pixel);
		}
	else
		{
		SetAPen(Current->rp,Current->penconv[*VB.Color[pv]]);
		}
	x0 = FIXx((int) (VB.Win[v0][0]));
	y0 = FIXy((int) (VB.Win[v0][1]));
	x1 = FIXx((int) (VB.Win[v1][0]));
	y1 = FIXy((int) (VB.Win[v1][1]));

	Move(Current->rp,x0,y0);
	Draw(Current->rp,x1,y1);
}



static line_func achoose_line_function( void )
{
	/*printf("achoose_line_function\n");*/

   /* Examine the current rendering state and return a pointer to a */
   /* fast line-rendering function if possible. */

   if (CC.Line.Width==1.0 && !CC.Line.SmoothFlag && !CC.Line.StippleFlag
       && CC.Light.ShadeModel==GL_FLAT && CC.RasterMask==0
       && !CC.Texture.Enabled /*&&  ETC, ETC */ ) 
		{
		return afast_line_function;
		}
	else 
		{

		return NULL;
		}
}

/*
 * Draw a filled polygon of a single color.  If there is hardware/OS support
 * for polygon drawing use that here.  Otherwise, call a function in
 * polygon.c to do the drawing.
 */

static void afast_polygon_function( GLuint n, GLuint vlist[], GLuint pv )
{
	int i,j;

   /* Render a line by some hardware/OS accerated method */
/*	printf("afast_polygon_function\n");*/

	if (VB.MonoColor)
		{
		SetAPen(Current->rp,Current->pixel);
		}
	else
		{
		SetAPen(Current->rp,Current->penconv[*VB.Color[pv]]);
		}


	AreaMove(Current->rp, FIXx((int) VB.Win[0][0]), FIXy( (int) VB.Win[0][1]));


	for (i=1;i<n;i++)
		{
		j=vlist[i];
		AreaDraw( Current->rp, FIXx((int) VB.Win[j][0]), FIXy( (int) VB.Win[j][1]));

		}
	AreaEnd( Current->rp );

}



static polygon_func achoose_polygon_function( void )
{
	/*	printf("achoose_polygon_function\n");*/

   /* Examine the current rendering state and return a pointer to a */
   /* fast polygon-rendering function if possible. */
	if (!CC.Polygon.SmoothFlag && !CC.Polygon.StippleFlag
       && CC.Light.ShadeModel==GL_FLAT && CC.RasterMask==0
       && !CC.Texture.Enabled /*&&  ETC, ETC */ )
		{
    return afast_polygon_function; 
/*		return NULL;*/
		}
	else 
		{
		return NULL;
		}
}



/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/


static void awrite_index_span( GLuint n, GLint x, GLint y,
                              const GLuint index[],
                              const GLubyte mask[] )
{
   int 	i,ant;
   UBYTE *dp;
 /*	printf("awrite_index_span(%d,%d,%d)\n",n,x,y);getchar();	*/

	y=FIXy(y);
	x=FIXx(x);
	if((dp = Current->imageline) && Current->tmpras)
		{                /* if imageline allocated then use fastversion */

		ant=0;
   	for (i=0;i<n;i++)          	   /* draw pixel (x[i],y[i]) using index[i] */
   		{
      	if (mask[i])
      		{
      		ant++;
      		x++;
				*dp++ = Current->penconv[index[i]];
				}
			else
				{
				if(ant)
					WritePixelLine8(Current->rp,x,y,ant,Current->imageline,Current->tmpras);
				dp=Current->imageline;                 
				ant=0;
				x++;
				}
			}
		if(ant)
			WritePixelLine8(Current->rp,x,y,ant,Current->imageline,Current->tmpras);

		}
	else
		{          /* Slower */
   	for (i=0;i<n;i++,x++)
   		{
      	if (mask[i])
      		{
      	   /* draw pixel (x[i],y[i]) using index[i] */
				SetAPen(Current->rp,Current->penconv[index[i]]);
				WritePixel(Current->rp,x,y);
				}

			}
		
		}
}



static void awrite_monoindex_span(GLuint n,GLint x,GLint y,const GLubyte mask[])
{
   int i;
/*	printf("awrite_monoindex_span(%d,%d,%d)<\n",n,x,y);getchar();	*/
	

	SetAPen(Current->rp,Current->pixel);
	y=FIXy(y);
	x=FIXx(x);

	for (i=0;i<n;i++,x++)
	{
	if (mask[i])
		{
         /* draw pixel (x[i],y[i]) using current color index */
		WritePixel(Current->rp,x,y);
    }
	}
}



static void awrite_color_span( GLuint n, GLint x, GLint y,
                              const GLubyte red[], const GLubyte green[],
                              const GLubyte blue[], const GLubyte alpha[],
                              const GLubyte mask[] )
{
   int i;
/*	printf("awrite_color_span(%d,%d,%d)<\n",n,x,y);	getchar();	*/
	

	y=FIXy(y);
	x=FIXx(x);
	if (mask)
		{
      /* draw some pixels */
		for (i=0; i<n; i++, x++)
			{
			if (mask[i])
				{
            /* draw pixel x,y using color red[i]/green[i]/blue[i]/alpha[i] */
				SetAPen(Current->rp,RGBA(red[i],green[i],blue[i],alpha[i]));
				WritePixel(Current->rp,x,y);
				}
			}
		}
	else
		{
      /* draw all pixels */
		for (i=0; i<n; i++, x++)
			{
         /* draw pixel x,y using color red[i]/green[i]/blue[i]/alpha[i] */
			SetAPen(Current->rp,RGBA(red[i],green[i],blue[i],alpha[i]));
			WritePixel(Current->rp,x,y);
			}
		}
}



static void awrite_monocolor_span( GLuint n, GLint x, GLint y,
                                  const GLubyte mask[])
{
   int i;
	/*printf("awrite_monocolor_span(%d,%d,%d)<<<<<<<\n",n,x,y);	getchar();	*/
	

	y=FIXy(y);
	x=FIXx(x);
	SetAPen(Current->rp,Current->pixel);

   for (i=0; i<n; i++, x++) {
      if (mask[i]) {
					WritePixel(Current->rp,x,y);
      }
   }
}



/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/


static void aread_index_span( GLuint n, GLint x, GLint y, GLuint index[])
{
   int i;
 	/*printf("aread_index_span>>\n");*/
	

 	y=FIXy(y);
	x=FIXx(x);
	for (i=0; i<n; i++,x++)
		{
		index[i] = ReadPixel(Current->rp,x,y);
		}
}



static void aread_color_span( GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
	int i,col;
	ULONG ColTab[3];

 	/*printf("aread_color_span>>\n");*/
 		

 	y=FIXy(y);
	x=FIXx(x);
	for (i=0; i<n; i++, x++)
		{
		col=ReadPixel(Current->rp,x,y);
		GetRGB32(Current->window->WScreen->ViewPort.ColorMap,col,1,ColTab);

		red[i]   = ColTab[0]>>24;
		green[i] = ColTab[1]>>24;
		blue[i]  = ColTab[2]>>24;
		alpha[i]=255;

   }
}



/**********************************************************************/
/*****              Write arrays of pixels                        *****/
/**********************************************************************/


static void awrite_index_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLuint index[], const GLubyte mask[] )
{
   int i;
 	/*printf("awrite_index_pixels<\n");*/
	

   for (i=0; i<n; i++) {
      if (mask[i]) {
/*         plot pixel x[i], y[i] using index[i] */
		SetAPen(Current->rp,Current->penconv[index[i]]);
		WritePixel(Current->rp,FIXx(x[i]),FIXy(y[i]));

      }
   }
}



static void awrite_monoindex_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
 	/*printf("awrite_monoindex_pixels<\n");*/
 		

		SetAPen(Current->rp,Current->pixel);

   for (i=0; i<n; i++) {
      if (mask[i]) {
/*         write pixel x[i], y[i] using current index  */
		WritePixel(Current->rp,FIXx(x[i]),FIXy(y[i]));

      }
   }
}



static void awrite_color_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLubyte r[], const GLubyte g[],
                                const GLubyte b[], const GLubyte a[],
                                const GLubyte mask[] )
{
   int i;
/* 	printf("awrite_color_pixels<\n");*/
	

   for (i=0; i<n; i++) {
      if (mask[i]) {
/*         write pixel x[i], y[i] using red[i],green[i],blue[i],alpha[i] */
		SetAPen(Current->rp,RGBA(r[i],g[i],b[i],a[i]));
		WritePixel(Current->rp,FIXx(x[i]),FIXy(y[i]));
      }
   }
}



static void awrite_monocolor_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
/* 	printf("awrite_monocolor_pixels<\n");*/
 		

		SetAPen(Current->rp,Current->pixel);

   for (i=0; i<n; i++) {
      if (mask[i]) {
/*         write pixel x[i], y[i] using current color*/
		WritePixel(Current->rp,FIXx(x[i]),FIXy(y[i]));
      }
   }
}




/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

/* Read an array of color index pixels. */
static void aread_index_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLuint index[], const GLubyte mask[] )
{
  int i;
/*	printf("aread_index_pixels-\n");*/
	

  for (i=0; i<n; i++) {
     if (mask[i]) {
/*        index[i] = read_pixel x[i], y[i] */
		index[i] = ReadPixel(Current->rp,FIXx(x[i]),FIXy(y[i]));
     }
  }
}



static void aread_color_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLubyte red[], GLubyte green[],
                               GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
	int i,col;
	ULONG ColTab[3];

/*	printf("aread_color_pixels-\n");*/
		

	for (i=0; i<n; i++)
		{
		if (mask[i])
			{
			col=ReadPixel(Current->rp,FIXx(x[i]),FIXy(y[i]));

			GetRGB32(Current->window->WScreen->ViewPort.ColorMap,col,1,ColTab);

			red[i]   = ColTab[0]>>24;
			green[i] = ColTab[1]>>24;
			blue[i]  = ColTab[2]>>24;
			alpha[i]=255;
      }
   }
}




/**********************************************************************/
/**********************************************************************/


/* org:static void setup_DD_pointers( void ) */

void amiga_setup_DD_pointers( void )
{
   /* Initialize all the pointers in the DD struct.  Do this whenever */
   /* a new context is made current or we change buffers via set_buffer! */

   DD.finish = afinish;
   DD.flush = aflush;

   DD.clear_index = aclear_index;
   DD.clear_color = aclear_color;
   DD.clear = aclear;

   DD.index = aset_index;
   DD.color = aset_color;
   DD.index_mask = aindex_mask;
   DD.color_mask = acolor_mask;

   DD.logicop = alogicop;
   DD.dither = adither;

   DD.set_buffer = aset_buffer;
   DD.buffer_size = abuffer_size;

   DD.get_points_func = achoose_points_function;
   DD.get_line_func = achoose_line_function;
   DD.get_polygon_func = achoose_polygon_function;

   /* Pixel/span writing functions: */
   DD.write_color_span       = awrite_color_span;
   DD.write_monocolor_span   = awrite_monocolor_span;
   DD.write_color_pixels     = awrite_color_pixels;
   DD.write_monocolor_pixels = awrite_monocolor_pixels;
   DD.write_index_span       = awrite_index_span;
   DD.write_monoindex_span   = awrite_monoindex_span;
   DD.write_index_pixels     = awrite_index_pixels;
   DD.write_monoindex_pixels = awrite_monoindex_pixels;

   /* Pixel/span reading functions: */
   DD.read_index_span = aread_index_span;
   DD.read_color_span = aread_color_span;
   DD.read_index_pixels = aread_index_pixels;
   DD.read_color_pixels = aread_color_pixels;
}



/**********************************************************************/
/*****               Amiga/Mesa Private Functions                 *****/
/**********************************************************************/


/*
 * Create a new rastport to use as a back buffer.
 * Input:  width, height - size in pixels
 *         depth - number of bitplanes
 */
struct RastPort *make_rastport( int width, int height, int depth )
{
	struct RastPort *rp;
	struct BitMap *bm;

	if (bm=AllocBitMap(width,height,depth,BMF_CLEAR|BMF_INTERLEAVED,0))
		{
		if (rp = (struct RastPort *) malloc( sizeof(struct RastPort)))
			{
			InitRastPort( rp );
			rp->BitMap = bm;
			return rp;
			}
		else
			{
			FreeBitMap(bm);
			return 0;
			}
		}
	else
		return 0;
}



/*
 * Deallocate a rastport.
 */

void destroy_rastport( struct RastPort *rp )
{
	WaitBlit();
	FreeBitMap( rp->BitMap );
	free( rp );
}



/*
 * Construct a temporary raster for use by the given rasterport.
 * Temp rasters are used for polygon drawing.
 */



BOOL static make_temp_raster( struct RastPort *rp )
{
	BOOL	OK;
	unsigned long width, height;
   PLANEPTR p;
   struct TmpRas *tmpras;

	OK=TRUE;
	if(rp==0)
		{
		printf("Zero rp\n");
		return(FALSE);
		}
   width = rp->BitMap->BytesPerRow*8;
   height = rp->BitMap->Rows;

	/* allocate structures */
   p = AllocRaster( width, height );
	if(p!=0)
		{
   	tmpras = (struct TmpRas *) AllocVec( sizeof(struct TmpRas),MEMF_ANY);
		if(tmpras!=0)
			{
  			if(InitTmpRas( tmpras, p, ((width+15)/16)*height ))
				{
/*				Current->tmpras=tmpras;*/
				rp->TmpRas = tmpras;
				}
	   	else
   			OK=FALSE;
   		}
		else
			OK=FALSE;
		}
	else
		return(FALSE);
	
	if (OK)
		return(TRUE);
	else
		{
		printf("Error when allocationg TmpRas\n");
		if (tmpras)
			FreeVec(tmpras);
		if (p)
			FreeRaster(p,width, height);
		return(FALSE);
   	}


}

BOOL allocarea(struct RastPort *rp )
	{
	BOOL	OK;
	struct AreaInfo *areainfo;
   UWORD *pattern;
   APTR vbuffer;

	OK=TRUE;
   
  	areainfo = (struct AreaInfo *) AllocVec( sizeof(struct AreaInfo),MEMF_ANY );
	if(areainfo!=0)
		{
		pattern = (UWORD *) AllocVec( sizeof(UWORD),MEMF_ANY);
		if(pattern!=0)
			{
	 		*pattern = 0xffff;		/*@@@ org: 0xffffffff*/
			vbuffer = (APTR) AllocVec( MAX_POLYGON * 5 * sizeof(WORD),MEMF_ANY);
			if(vbuffer!=0)
   			{
				/* initialize */
				InitArea( areainfo, vbuffer, MAX_POLYGON );
  				/* bind to rastport */
  				rp->AreaPtrn = pattern;
  				rp->AreaInfo = areainfo;
  				rp->AreaPtSz = 0;
     			}
     		else
  		  		OK=FALSE;
  	  		}
   	else
   	  	OK=FALSE;
   	}
   else	
   	  	OK=FALSE;   
	/*	return(FALSE);*/
		
	if (OK)
		return (OK);
	else
		{
		printf("Error when allocationg AreaBuffers\n");
		if (vbuffer)
			FreeVec(vbuffer);
		if (pattern)
			FreeVec(pattern);
		if (areainfo)
			FreeVec(areainfo);
		return(OK);
   	}
			
	}

void freearea(struct RastPort *rp)
	{
	if (rp->AreaInfo)
		{
		if (rp->AreaInfo->VctrTbl)
			FreeVec(rp->AreaInfo->VctrTbl);
		if (rp->AreaPtrn)
			{
			FreeVec(rp->AreaPtrn);
			rp->AreaPtrn=NULL;
			}
		FreeVec(rp->AreaInfo);
		rp->AreaInfo=NULL;
		}
	}
/*
 * Destroy a temp raster.
 */

static void destroy_temp_raster( struct RastPort *rp )
{
   /* bitmap */
	
	unsigned long width, height;
	
   width = rp->BitMap->BytesPerRow*8;
   height = rp->BitMap->Rows;

	if (rp->TmpRas)
		{
		if(rp->TmpRas->RasPtr)
			FreeRaster( rp->TmpRas->RasPtr,width,height );
		FreeVec( rp->TmpRas );
		rp->TmpRas=NULL;
/*		Current->tmpras = NULL;*/
		
		}
}

void AllocOneLine( struct amigamesa_context *c)
	{
	if(c->imageline)
		FreeVec(c->imageline);
	c->imageline = AllocVec((c->width+15) & 0xfffffff0,MEMF_ANY);  /* One Line */
	}
	
void FreeOneLine( struct amigamesa_context *c)
	{
	if(c->imageline)
		{
		FreeVec(c->imageline);
		c->imageline=NULL;
		}
		
	}

BOOL InitLibrary(void)
	{
	if(!GfxBase)
		{
/*		printf("Opening graphics.library\n");*/
		if (!(GfxBase = OpenLibrary("graphics.library",39)))
			{
			printf("MesaOpenGL Error\nCouldent open graphics.library v39\n");
			return(FALSE);
			}
		}
/*	printf("GfxBase=0x%x\n",GfxBase);*/
	return(TRUE);
	}

void DisposeLibrary(void)
	{
	if (GfxBase)   /* @@@ TODO Open and close gfxlibrary when open and flush lib.*/
		CloseLibrary(GfxBase);
	GfxBase=NULL;
	}

/**********************************************************************/
/*****               Amiga/Mesa API Functions                     *****/
/**********************************************************************/


/*
 * Implement the client-visible Amiga/Mesa interface functions defined
 * in Mesa/include/GL/Amigamesa.h
 */

__asm __saveds struct amigamesa_context *AmigaMesaCreateContext(register __a1 struct Window *window,register __d0 GLboolean rgb_flag,register __d1 GLboolean db_flag )
{
   /* Create a new Amiga/Mesa context */
   /* Be sure to initialize the following in the core Mesa context: */
   /* RedScale, GreenScale, BlueScale, AlphaScale */
   /* DrawBuffer, ReadBuffer */  /* @@@ IMPLEMENTERA ???*/
   /* RGBAflag, DBflag */
   
	GLfloat redscale,greenscale,bluescale,alphascale;
	int			I;
	struct amigamesa_context *c;

/*	printf("Hello and welcome to a Libfunction\n");*/

      /* allocate amigamesa_context struct initialized to zeros */
	c = (struct amigamesa_context *) AllocVec(sizeof(struct amigamesa_context),MEMF_PUBLIC|MEMF_CLEAR);
	if (!c)
		{
		printf("Not able to create mesa_context. You are VERY low on memory.\n");
		return(NULL);
		}
	

	c->sharelist=NULL;     														/* shared viewarea */
	redscale=greenscale=bluescale=alphascale=255.0;


	c->gl_ctx = gl_new_context( rgb_flag,
                               redscale, greenscale, bluescale, alphascale,
                               db_flag, NULL );  /*c->sharelist->gl_ctx ); @@@ tillfälligt */
	c->rgb_flag = rgb_flag;
	c->db_flag = db_flag;

	c->front_rp =window->RPort;
		c->back_rp=NULL;
		c->rp = c->front_rp;

	c->window = window;

	c->width = window->Width - window->BorderLeft - window->BorderRight;
	c->height = window->Height - window->BorderTop - window->BorderBottom;
	c->left = window->BorderLeft;
	c->bottom = window->Height - window->BorderBottom - 1;

	c->depth = c->rp->BitMap->Depth;

	c->gl_ctx->BufferWidth = c->width;
	c->gl_ctx->BufferHeight = c->height;
		
	if (rgb_flag)
		{
      /* RGBA color buffers */
		c->gl_ctx->RGBAflag = GL_TRUE;
		c->pixel = 0;
		}
	else
   	{
      /* CI color buffers */
		c->gl_ctx->RGBAflag = GL_FALSE;
		c->pixel = 0;
		}

	if (db_flag)
		{
		if((c->back_rp = make_rastport(c->window->Width,c->window->Height,c->depth))!=NULL)
			{
	/*		printf("make_rastport OK\n");*/
			c->gl_ctx->Color.DrawBuffer = GL_BACK;
			c->rp = c->back_rp;
			}
		else
			{
			printf("make_rastport Faild\n");
			c->gl_ctx->Color.DrawBuffer = GL_FRONT;
			}
		}
	else
		{
		c->gl_ctx->Color.DrawBuffer = GL_FRONT;
		}

	
	AllocOneLine(c); /* A linebuffer for WritePixelLine */


	if (!make_temp_raster( c->rp ))
		printf("Error allocating TmpRastPort\n");

	allocarea(c->rp);
		
/*
   printf("win width = %d\n", (int) window->Width );
   printf("win height = %d\n", (int) window->Height );
   printf("left = %d\n", (int) window->BorderLeft );
   printf("rightt = %d\n", (int) window->BorderRight );
   printf("top = %d\n", (int) window->BorderTop );
   printf("bottom = %d\n", (int) c->bottom );
*/
	for(I=0;I<=255;I++)
		{
		c->penconv[I]=I;    /* Fill it with ordenery pen nr */
		}
/*   printf("Createcontext finishied\n");*/
	return c;
}


__asm __saveds void AmigaMesaDestroyContext(register __a0 struct amigamesa_context *c )
{
   /* destroy a Amiga/Mesa context */

	int	I;

	if (c==Current)
		Current=NULL;

	FreeOneLine(c);
	freearea(c->rp);

	destroy_temp_raster( c->rp);

	gl_destroy_context( c->gl_ctx );


	if (c->rgb_flag)
		{
		if (c->rgb_buffer)
			{
			printf("free(c->rgb_buffer)\n");
			free( c->rgb_buffer );
			}
		}

	if (c->back_rp)
		{
		destroy_rastport( c->back_rp );
		c->back_rp=NULL;
		}

	for(I=0;I<=255;I++)
		{
/*		printf("%d=%d\n",I,c->mypen[I]);*/
		
		while (c->mypen[I]!=0)			/* TODO This may free some others pen also */
			{
			c->mypen[I]-=1;
			ReleasePen(c->window->WScreen->ViewPort.ColorMap,I);
			}
		}
	FreeVec( c );
}


__asm __saveds void AmigaMesaMakeCurrent(register __a0 struct amigamesa_context *c )
{
   /* Make the specified context the current one */
   /* the order of operations here is very important! */
   gl_set_context( c->gl_ctx );
   Current = c;

   amiga_setup_DD_pointers();

   if (Current->gl_ctx->Viewport.Width==0) {
      /* initialize viewport to window size */
      gl_viewport( 0, 0, Current->width, Current->height );
   }
}


__asm __saveds void AmigaMesaSwapBuffers(void)
	{
									 /* copy/swap back buffer to front if applicable */
	if (Current->back_rp)
		{
		UBYTE minterm = 0xc0;
		int x = Current->window->BorderLeft;
		int y = Current->window->BorderTop;
		

		ClipBlit( Current->back_rp, x, y,   /* from */
							Current->front_rp, x, y,  /* to */
							Current->width, Current->height,  /* size */
							minterm );
		}

	}
