/* wmesa.c */

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
 *
 * Windows driver by: Mark E. Peterson (markp@ic.mankato.mn.us)
 */

/*
$Id: wmesa.c,v 1.11 1995/12/19 22:18:41 brianp Exp $

$Log: wmesa.c,v $
 * Revision 1.11  1995/12/19  22:18:41  brianp
 * removed 0.5 window coordinate offsets thanks to CC.RasterOffsetX/Y
 *
 * Revision 1.10  1995/11/20  20:11:08  brianp
 * incorporated Mark's November 17th updates
 *
 * Revision 1.9  1995/10/30  15:30:00  brianp
 * pass mask array to read_[index|color]_pixels()
 *
 * Revision 1.8  1995/10/19  15:44:42  brianp
 * updated arguments to set_color, clear_color and gl_new_context
 *
 * Revision 1.7  1995/10/17  21:28:01  brianp
 * updated for new device driver structure in Mesa 1.2.4
 *
 * Revision 1.6  1995/09/15  18:52:55  brianp
 * cleaned up some code
 * modified write_color_span to take NULL mask
 *
 * Revision 1.5  1995/07/24  18:56:00  brianp
 * added dd_finish()
 *
 * Revision 1.4  1995/06/21  15:39:26  brianp
 * removed #include "wmesaP.h"
 *
 * Revision 1.3  1995/06/21  15:38:10  brianp
 * added struct_wmesa_context, don't need wmesaP.h anymore
 *
 * Revision 1.2  1995/06/09  18:53:39  brianp
 * removed compute_mult_and_shift()
 * dd_read/write_color_span/pixels() take GLubyte arrays instead of GLints
 *
 * Revision 1.1  1995/06/09  18:52:29  brianp
 * Initial revision
 *
 */


#include <stdlib.h>
#include <windows.h>
#include "gl\wmesa.h"
#include "context.h"
#include "dd.h"
#include "xform.h"
#include "vb.h"
#include "wing.h"


/* Bit's used for dest: */
#define FRONT_PIXMAP	1
#define BACK_PIXMAP	2
#define BACK_XIMAGE	4

struct wmesa_context
{
  struct gl_context *gl_ctx;	/* the main library context */
  HWND Window;
  HDC Compat_DC;                /* Display context for double buffering. */
  HBITMAP Old_Compat_BM,Compat_BM;            /* Bitmap for double buffering */
  GLuint width, height,ScanWidth;
  GLboolean db_flag;	/* double buffered? */
  GLboolean rgb_flag;	/* RGB mode? */
  GLuint depth;		/* bits per pixel (1, 8, 24, etc) */
  unsigned long pixel;	/* current color index or RGBA pixel value */
  unsigned long clearpixel; /* pixel for clearing the color buffers */
  char *ScreenMem; // WinG memory
  BITMAPINFO *IndexFormat;
  HPALETTE hPal; // Current Palette
};


#ifdef NDEBUG
  #define assert(ignore)	((void) 0)
#else
  void Mesa_Assert(void *Cond,void *File,unsigned Line)
  {
    char Msg[512];
    sprintf(Msg,"%s %s %d",Cond,File,Line);
    MessageBox(NULL,Msg,"Assertion failed.",MB_OK);
    exit(1);
  }
  #define assert(e)	if (!e) Mesa_Assert(#e,__FILE__,__LINE__);
#endif
#define DD_GETDC ((Current->db_flag) ? Current->Compat_DC : GetDC(Current->Window))
#define DD_RELEASEDC if (!Current->db_flag) ReleaseDC(Current->Window,DC)
static WMesaContext Current = NULL;
#ifdef __SYMANTEC_BUGS
  struct dd_function_table DD;
#endif
#define FLIP(Y)  (Current->height-(Y)-1)





/* Finish all pending operations and synchronize. */
static void finish(void)
{
   /* no op */
}



static void flush(void)
{
   /* no op */
}



/*
 * Set the color index used to clear the color buffer.
 */
static void clear_index(GLuint index)
{
  Current->clearpixel = index;
}



/*
 * Set the color used to clear the color buffer.
 */
static void clear_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
  Current->clearpixel=RGB(r, g, b );
}



/*
 * Clear the specified region of the color buffer using the clear color
 * or index as specified by one of the two functions above.
 */
static void clear(GLboolean all,GLint x, GLint y, GLint width, GLint height )
{
  if (all)
  {
    x=y=0;
    width=Current->width;
    height=Current->height;
  }
  if (Current->rgb_flag==GL_TRUE)
  {
    HDC DC=DD_GETDC;
    HPEN Pen=CreatePen(PS_SOLID,1,Current->clearpixel);
    HBRUSH Brush=CreateSolidBrush(Current->clearpixel);
    HPEN Old_Pen=SelectObject(DC,Pen);
    HBRUSH Old_Brush=SelectObject(DC,Brush);
    Rectangle(DC,x,y,x+width,y+height);
    SelectObject(DC,Old_Pen);
    SelectObject(DC,Old_Brush);
    DeleteObject(Pen);
    DeleteObject(Brush);
    DD_RELEASEDC;
  }
  else
  {
    int i;
    char *Mem=Current->ScreenMem+y*Current->ScanWidth+x;
    for (i=0; i<height; i++)
    {
      memset(Mem,Current->clearpixel,width);
      Mem+=width;
    }
  }
}



/* Set the current color index. */
static void set_index(GLuint index)
{
  Current->pixel=index;
}



/* Set the current RGBA color. */
static void set_color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
  Current->pixel = RGB( r, g, b );
}



/* Set the index mode bitplane mask. */
static GLboolean index_mask(GLuint mask)
{
   /* can't implement */
   return GL_FALSE;
}



/* Set the RGBA drawing mask. */
static GLboolean color_mask( GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask)
{
   /* can't implement */
   return GL_FALSE;
}



/*
 * Set the pixel logic operation.  Return GL_TRUE if the device driver
 * can perform the operation, otherwise return GL_FALSE.  If GL_FALSE
 * is returned, the logic op will be done in software by Mesa.
 */
GLboolean logicop( GLenum op )
{
   /* can't implement */
   return GL_FALSE;
}


static void dither( GLboolean enable )
{
   /* No op */
}



static GLboolean set_buffer( GLenum mode )
{
   /* TODO: this could be better */
   if (mode==GL_FRONT || mode==GL_BACK) {
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}



/* Return characteristics of the output buffer. */
static void buffer_size( GLuint *width, GLuint *height, GLuint *depth )
{
  int New_Size;
  RECT CR;
  GetClientRect(Current->Window,&CR);
  *width=CR.right;
  *height=CR.bottom;
  *depth = Current->depth;
  New_Size=((*width)!=Current->width) || ((*height)!=Current->height);
  if (New_Size)
  {
    Current->width=*width;
    Current->ScanWidth=Current->width;
    if ((Current->ScanWidth%sizeof(long))!=0)
      Current->ScanWidth+=(sizeof(long)-(Current->ScanWidth%sizeof(long)));
    Current->height=*height;
    if (Current->db_flag)
    {
      if (Current->rgb_flag==GL_TRUE)
        Current->Compat_BM=CreateCompatibleBitmap(Current->Compat_DC,Current->width,Current->height);
      else
      {
        Current->IndexFormat->bmiHeader.biWidth=Current->width;
        if (Current->IndexFormat->bmiHeader.biHeight<0)
          Current->IndexFormat->bmiHeader.biHeight=-Current->height;
        else
          Current->IndexFormat->bmiHeader.biHeight=Current->height;
        Current->Compat_BM=WinGCreateBitmap(Current->Compat_DC,Current->IndexFormat,&((void *) Current->ScreenMem));
      }
      DeleteObject(SelectObject(Current->Compat_DC,Current->Compat_BM));
    }
  }
}



/**********************************************************************/
/*****           Accelerated point, line, polygon rendering       *****/
/**********************************************************************/


static void fast_rgb_points( GLuint first, GLuint last )
{
   int i;
   HDC DC=DD_GETDC;
   if (VB.MonoColor) {
      /* all drawn with current color */
      for (i=first;i<=last;i++) {
         if (VB.Unclipped[i]) {
            int x, y;
            x =       (GLint) VB.Win[i][0];
            y = FLIP( (GLint) VB.Win[i][1] );
            SetPixel(DC,x,y,Current->pixel);
         }
      }
   }
   else {
      /* draw points of different colors */
      HPEN Pen=CreatePen(PS_SOLID,1,Current->pixel);
      HPEN Old_Pen=SelectObject(DC,Pen);
      for (i=first;i<=last;i++) {
         if (VB.Unclipped[i]) {
            int x, y;
            unsigned long pixel=RGB(VB.Color[i][0]*255.0,
                                    VB.Color[i][1]*255.0,
                                    VB.Color[i][2]*255.0);
            x =       (GLint) VB.Win[i][0];
            y = FLIP( (GLint) VB.Win[i][1] );
            SetPixel(DC,x,y,pixel);
         }
      }
      SelectObject(DC,Old_Pen);
      DeleteObject(Pen);
   }
   DD_RELEASEDC;
}



/* Return pointer to accerated points function */
static points_func choose_points_function( void )
{
   if (CC.Point.Size==1.0 && !CC.Point.SmoothFlag && CC.RasterMask==0
       && !CC.Texture.Enabled  && Current->rgb_flag) {
      return fast_rgb_points;
   }
   else {
      return NULL;
   }
}



/* Draw a line using the color specified by VB.Color[pv] */
static void fast_flat_rgb_line( GLuint v0, GLuint v1, GLuint pv )
{
   int x0, y0, x1, y1;
   unsigned long pixel;
   HDC DC=DD_GETDC;
   HPEN Pen;
   HPEN Old_Pen;

   if (VB.MonoColor) {
      pixel = Current->pixel;  /* use current color */
   }
   else {
      pixel = RGB(VB.Color[pv][0]*255.0, VB.Color[pv][1]*255.0, VB.Color[pv][2]*255.0);
   }

   x0 =       (int) VB.Win[v0][0];
   y0 = FLIP( (int) VB.Win[v0][1] );
   x1 =       (int) VB.Win[v1][0];
   y1 = FLIP( (int) VB.Win[v1][1] );

   Pen=CreatePen(PS_SOLID,1,pixel);
   Old_Pen=SelectObject(DC,Pen);
   MoveToEx(DC,x0,y0,NULL);
   LineTo(DC,x1,y1);
   SelectObject(DC,Old_Pen);
   DeleteObject(Pen);
   DD_RELEASEDC;
}



/* Return pointer to accerated line function */
static line_func choose_line_function( void )
{
   if (CC.Line.Width==1.0 && !CC.Line.SmoothFlag && !CC.Line.StippleFlag
       && CC.Light.ShadeModel==GL_FLAT && CC.RasterMask==0
       && !CC.Texture.Enabled && Current->rgb_flag) {
      return fast_flat_rgb_line;
   }
   else {
      return NULL;
   }
}


/* Draw a convex polygon using color VB.Color[pv] */
static void fast_flat_rgb_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
   POINT *Pts=(POINT *) malloc(n*sizeof(POINT));
   HDC DC=DD_GETDC;
   HPEN Pen;
   HBRUSH Brush;
   HPEN Old_Pen;
   HBRUSH Old_Brush;
   GLint pixel;
   int i;

   if (VB.MonoColor) {
      pixel = Current->pixel;  /* use current color */
   }
   else {
      pixel = RGB(VB.Color[pv][0]*255.0, VB.Color[pv][1]*255.0, VB.Color[pv][2]*255.0);
   }

   Pen=CreatePen(PS_SOLID,1,pixel);
   Brush=CreateSolidBrush(pixel);
   Old_Pen=SelectObject(DC,Pen);
   Old_Brush=SelectObject(DC,Brush);

   for (i=0; i<n; i++) {
      int j = vlist[i];
      Pts[i].x =       (int) VB.Win[j][0];
      Pts[i].y = FLIP( (int) VB.Win[j][1] );
   }
   Polygon(DC,Pts,n);
   SelectObject(DC,Old_Pen);
   SelectObject(DC,Old_Brush);
   DeleteObject(Pen);
   DeleteObject(Brush);
   DD_RELEASEDC;
   free(Pts);
}



/* Return pointer to accerated polygon function */
static polygon_func choose_polygon_function( void )
{
   if (!CC.Polygon.SmoothFlag && !CC.Polygon.StippleFlag
       && CC.Light.ShadeModel==GL_FLAT && CC.RasterMask==0
       && !CC.Texture.Enabled && Current->rgb_flag==GL_TRUE) {
      return fast_flat_rgb_polygon;
   }
   else {
      return NULL;
   }
}



/**********************************************************************/
/*****                 Span-based pixel drawing                   *****/
/**********************************************************************/


/* Write a horizontal span of color-index pixels with a boolean mask. */
static void write_index_span( GLuint n, GLint x, GLint y,
                              const GLuint index[],
                              const GLubyte mask[] )
{
  int i;
  char *Mem=Current->ScreenMem+y*Current->ScanWidth+x;
  assert(Current->rgb_flag==GL_FALSE);
  for (i=0; i<n; i++)
    if (mask[i])
      Mem[i]=index[i];
}



/*
 * Write a horizontal span of pixels with a boolean mask.  The current
 * color index is used for all pixels.
 */
static void write_monoindex_span(GLuint n,GLint x,GLint y,const GLubyte mask[])
{
  int i;
  char *Mem=Current->ScreenMem+y*Current->ScanWidth+x;
  assert(Current->rgb_flag==GL_FALSE);
  for (i=0; i<n; i++)
    if (mask[i])
      Mem[i]=Current->pixel;
}



/* Write a horizontal span of color pixels with a boolean mask. */
static void write_color_span( GLuint n, GLint x, GLint y,
			  const GLubyte red[], const GLubyte green[],
			  const GLubyte blue[], const GLubyte alpha[],
			  const GLubyte mask[] )
{
  if (Current->rgb_flag==GL_TRUE)
  {
    int i;
    HDC DC=DD_GETDC;
    y=FLIP(y);
    if (mask) {
       for (i=0; i<n; i++)
         if (mask[i])
           SetPixel(DC,x+i,y,RGB(red[i],green[i],blue[i]));
    }
    else {
       for (i=0; i<n; i++)
         SetPixel(DC,x+i,y,RGB(red[i],green[i],blue[i]));
    }
    DD_RELEASEDC;
  }
  else
  {
    int i;
    char *Mem=Current->ScreenMem+y*Current->ScanWidth+x;
    if (mask) {
       for (i=0; i<n; i++)
         if (mask[i])
           Mem[i]=GetNearestPaletteIndex(Current->hPal,RGB(red[i],green[i],blue[i]));
    }
    else {
       for (i=0; i<n; i++)
         Mem[i]=GetNearestPaletteIndex(Current->hPal,RGB(red[i],green[i],blue[i]));
    }
  }
}



/*
 * Write a horizontal span of pixels with a boolean mask.  The current color
 * is used for all pixels.
 */
static void write_monocolor_span( GLuint n, GLint x, GLint y,const GLubyte mask[])
{
  int i;
  HDC DC=DD_GETDC;
  assert(Current->rgb_flag==GL_TRUE);
  y=FLIP(y);
  for (i=0; i<n; i++)
    if (mask[i])
      SetPixel(DC,x+i,y,Current->pixel);
  DD_RELEASEDC;
}



/**********************************************************************/
/*****                   Array-based pixel drawing                *****/
/**********************************************************************/


/* Write an array of pixels with a boolean mask. */
static void write_index_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLuint index[], const GLubyte mask[] )
{
   int i;
   assert(Current->rgb_flag==GL_FALSE);
   for (i=0; i<n; i++) {
      if (mask[i]) {
         char *Mem=Current->ScreenMem+y[i]*Current->ScanWidth+x[i];
         *Mem = index[i];
      }
   }
}



/*
 * Write an array of pixels with a boolean mask.  The current color
 * index is used for all pixels.
 */
static void write_monoindex_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
   assert(Current->rgb_flag==GL_FALSE);
   for (i=0; i<n; i++) {
      if (mask[i]) {
         char *Mem=Current->ScreenMem+y[i]*Current->ScanWidth+x[i];
         *Mem = Current->pixel;
      }
   }
}



/* Write an array of pixels with a boolean mask. */
static void write_color_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLubyte r[], const GLubyte g[],
                                const GLubyte b[], const GLubyte a[],
                                const GLubyte mask[] )
{
  int i;
  HDC DC=DD_GETDC;
  assert(Current->rgb_flag==GL_TRUE);
  for (i=0; i<n; i++)
    if (mask[i])
      SetPixel(DC,x[i],FLIP(y[i]),RGB(r[i],g[i],b[i]));
  DD_RELEASEDC;
}



/*
 * Write an array of pixels with a boolean mask.  The current color
 * is used for all pixels.
 */
static void write_monocolor_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
  int i;
  HDC DC=DD_GETDC;
  assert(Current->rgb_flag==GL_TRUE);
  for (i=0; i<n; i++)
    if (mask[i])
      SetPixel(DC,x[i],FLIP(y[i]),Current->pixel);
  DD_RELEASEDC;
}



/**********************************************************************/
/*****            Read spans/arrays of pixels                     *****/
/**********************************************************************/


/* Read a horizontal span of color-index pixels. */
static void read_index_span( GLuint n, GLint x, GLint y, GLuint index[])
{
  int i;
  char *Mem=Current->ScreenMem+y*Current->ScanWidth+x;
  assert(Current->rgb_flag==GL_FALSE);
  for (i=0; i<n; i++)
    index[i]=Mem[i];
}




/* Read an array of color index pixels. */
static void read_index_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLuint indx[], const GLubyte mask[] )
{
  int i;
  assert(Current->rgb_flag==GL_FALSE);
  for (i=0; i<n; i++) {
     if (mask[i]) {
        indx[i]=*(Current->ScreenMem+y[i]*Current->ScanWidth+x[i]);
     }
  }
}



/* Read a horizontal span of color pixels. */
static void read_color_span( GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
  int i;
  COLORREF Color;
  HDC DC=DD_GETDC;
  assert(Current->rgb_flag==GL_TRUE);
  y=FLIP(y);
  for (i=0; i<n; i++)
  {
    Color=GetPixel(DC,x+i,y);
    red[i]=GetRValue(Color);
    green[i]=GetGValue(Color);
    blue[i]=GetBValue(Color);
    alpha[i]=255;
  }
  DD_RELEASEDC;
  memset(alpha,0,n*sizeof(GLint));
}


/* Read an array of color pixels. */
static void read_color_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLubyte red[], GLubyte green[],
                               GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
  int i;
  COLORREF Color;
  HDC DC=DD_GETDC;
  assert(Current->rgb_flag==GL_TRUE);
  for (i=0; i<n; i++) {
     if (mask[i]) {
        Color=GetPixel(DC,x[i],FLIP(y[i]));
        red[i]=GetRValue(Color);
        green[i]=GetGValue(Color);
        blue[i]=GetBValue(Color);
        alpha[i]=255;
     }
  }
  DD_RELEASEDC;
  memset(alpha,0,n*sizeof(GLint));
}



/**********************************************************************/
/**********************************************************************/



static void setup_DD_pointers( void )
{
   DD.finish = finish;
   DD.flush = flush;

   DD.clear_index = clear_index;
   DD.clear_color = clear_color;
   DD.clear = clear;

   DD.index = set_index;
   DD.color = set_color;
   DD.index_mask = index_mask;
   DD.color_mask = color_mask;

   DD.logicop = logicop;
   DD.dither = dither;

   DD.set_buffer = set_buffer;
   DD.buffer_size = buffer_size;

   DD.get_points_func = choose_points_function;
   DD.get_line_func = choose_line_function;
   DD.get_polygon_func = choose_polygon_function;

   /* Pixel/span writing functions: */
   DD.write_color_span       = write_color_span;
   DD.write_monocolor_span   = write_monocolor_span;
   DD.write_color_pixels     = write_color_pixels;
   DD.write_monocolor_pixels = write_monocolor_pixels;
   DD.write_index_span       = write_index_span;
   DD.write_monoindex_span   = write_monoindex_span;
   DD.write_index_pixels     = write_index_pixels;
   DD.write_monoindex_pixels = write_monoindex_pixels;

   /* Pixel/span reading functions: */
   DD.read_index_span = read_index_span;
   DD.read_color_span = read_color_span;
   DD.read_index_pixels = read_index_pixels;
   DD.read_color_pixels = read_color_pixels;
}



/**********************************************************************/
/*****                  WMesa API Functions                       *****/
/**********************************************************************/



#define PAL_SIZE 256
static void GetPalette(HPALETTE Pal,RGBQUAD *aRGB)
{
	int i;
	HDC hdc;
	struct
	{
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY aEntries[PAL_SIZE];
	} Palette =
	{
		0x300,
		PAL_SIZE
	};
	hdc=GetDC(NULL);
	if (Pal!=NULL)
    GetPaletteEntries(Pal,0,PAL_SIZE,Palette.aEntries);
  else
    GetSystemPaletteEntries(hdc,0,PAL_SIZE,Palette.aEntries);
	if (GetSystemPaletteUse(hdc) == SYSPAL_NOSTATIC)
	{
		for(i = 0; i <PAL_SIZE; i++)
			Palette.aEntries[i].peFlags = PC_RESERVED;
		Palette.aEntries[255].peRed = 255;
		Palette.aEntries[255].peGreen = 255;
		Palette.aEntries[255].peBlue = 255;
		Palette.aEntries[255].peFlags = 0;
		Palette.aEntries[0].peRed = 0;
		Palette.aEntries[0].peGreen = 0;
		Palette.aEntries[0].peBlue = 0;
		Palette.aEntries[0].peFlags = 0;
	}
	else
	{
		int nStaticColors;
		int nUsableColors;
		nStaticColors = GetDeviceCaps(hdc, NUMCOLORS)/2;
		for (i=0; i<nStaticColors; i++)
			Palette.aEntries[i].peFlags = 0;
		nUsableColors = PAL_SIZE-nStaticColors;
		for (; i<nUsableColors; i++)
			Palette.aEntries[i].peFlags = PC_RESERVED;
		for (; i<PAL_SIZE-nStaticColors; i++)
			Palette.aEntries[i].peFlags = PC_RESERVED;
		for (i=PAL_SIZE-nStaticColors; i<PAL_SIZE; i++)
			Palette.aEntries[i].peFlags = 0;
	}
	ReleaseDC(NULL,hdc);
  for (i=0; i<PAL_SIZE; i++)
  {
    aRGB[i].rgbRed=Palette.aEntries[i].peRed;
    aRGB[i].rgbGreen=Palette.aEntries[i].peGreen;
    aRGB[i].rgbBlue=Palette.aEntries[i].peBlue;
    aRGB[i].rgbReserved=Palette.aEntries[i].peFlags;
  }
}



WMesaContext WMesaCreateContext( HWND hWnd, HPALETTE Pal, GLboolean rgb_flag,
                                 GLboolean db_flag )
{
  BITMAPINFO *Rec;
  HDC DC;
  RECT CR;
  WMesaContext c;

  c = (struct wmesa_context *) calloc(1,sizeof(struct wmesa_context));
  if (!c)
    return NULL;

  c->Window=hWnd;
  if (rgb_flag==GL_FALSE)
  {
    c->rgb_flag = GL_FALSE;
    c->pixel = 1;
    db_flag=GL_TRUE; // WinG requires double buffering
    //c->gl_ctx->BufferDepth = windepth;
  }
  else
  {
    c->rgb_flag = GL_TRUE;
    c->pixel = 0;
  }
  GetClientRect(c->Window,&CR);
  c->width=CR.right;
  c->height=CR.bottom;
  if (db_flag)
  {
    c->db_flag = 1;
    /* Double buffered */
    if (c->rgb_flag==GL_TRUE)
    {
      DC=GetDC(c->Window);
      c->Compat_DC=CreateCompatibleDC(DC);
      c->Compat_BM=CreateCompatibleBitmap(DC,c->width,c->height);
      ReleaseDC(c->Window,DC);
      c->Old_Compat_BM=SelectObject(c->Compat_DC,c->Compat_BM);
    }
    else
    {
      c->Compat_DC=WinGCreateDC();
      Rec=(BITMAPINFO *) malloc(sizeof(BITMAPINFO)+(PAL_SIZE-1)*sizeof(RGBQUAD));
      c->hPal=Pal;
      GetPalette(Pal,Rec->bmiColors);
      WinGRecommendDIBFormat(Rec);
      Rec->bmiHeader.biWidth=c->width;
      Rec->bmiHeader.biHeight*=c->height;
      Rec->bmiHeader.biClrUsed=PAL_SIZE;
      if (Rec->bmiHeader.biPlanes!=1 || Rec->bmiHeader.biBitCount!=8)
      {
        MessageBox(NULL,"Error.","This code presumes a 256 color, single plane, WinG Device.\n",MB_OK);
        exit(1);
      }
      c->Compat_BM=WinGCreateBitmap(c->Compat_DC,Rec,&((void *) c->ScreenMem));
      c->Old_Compat_BM=SelectObject(c->Compat_DC,c->Compat_BM);
      WinGSetDIBColorTable(c->Compat_DC,0,PAL_SIZE,Rec->bmiColors);
      c->IndexFormat=Rec;
      c->ScanWidth=c->width;
      if ((c->ScanWidth%sizeof(long))!=0)
        c->ScanWidth+=(sizeof(long)-(c->ScanWidth%sizeof(long)));
    }
  }
  else
  {
    /* Single Buffered */
    c->db_flag = 0;
  }

  /* allocate a new Mesa context */
  c->gl_ctx = gl_new_context( rgb_flag,
                              255.0, 255.0, 255.0, 255.0,
                              db_flag, NULL);

  setup_DD_pointers();

  return c;
}



void WMesaDestroyContext( WMesaContext c )
{
   gl_destroy_context( c->gl_ctx );
   if (c->db_flag)
   {
     SelectObject(c->Compat_DC,c->Old_Compat_BM);
     DeleteDC(c->Compat_DC);
     DeleteObject(c->Compat_BM);
   }
   free( (void *) c );
}



void WMesaMakeCurrent( WMesaContext c )
{
   gl_set_context( c->gl_ctx );
   Current = c;
   setup_DD_pointers();
   if (Current->gl_ctx->Viewport.Width==0) {
      /* initialize viewport to window size */
      gl_viewport( 0, 0, Current->width, Current->height );
   }
}



void WMesaSwapBuffers( void )
{
  // *** Perhaps the DC should be saved in WMesaContext?
  HDC DC;
  if (Current->db_flag)
  {
    DC=GetDC(Current->Window);
    if (Current->rgb_flag)
      BitBlt(DC,0,0,Current->width,Current->height,Current->Compat_DC,0,0,SRCCOPY);
    else
      WinGBitBlt(DC,0,0,Current->width,Current->height,Current->Compat_DC,0,0);
    ReleaseDC(Current->Window,DC);
  }
}



void WMesaPaletteChange(HPALETTE Pal)
{
  if (Current && Current->rgb_flag==GL_FALSE)
  {
    Current->hPal=Pal;
    GetPalette(Pal,Current->IndexFormat->bmiColors);
    WinGSetDIBColorTable(Current->Compat_DC,0,PAL_SIZE,Current->IndexFormat->bmiColors);
  }
}

