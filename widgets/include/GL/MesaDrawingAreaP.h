/* MesaDrawingAreaP.h -- Private header file for the Mesa widget
   Copyright (C) 1995 Thorsten.Ohl @ Physik.TH-Darmstadt.de

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   $Id: MesaDrawingAreaP.h,v 1.7 1995/05/19 20:30:59 ohl Exp $
 */

#ifndef _MesaDrawingAreaP_h
#define _MesaDrawingAreaP_h

#include <GL/GLwDrawAP.h>
#include <GL/MesaDrawingArea.h>

typedef struct
  {
    char *RCS_Id;
    Widget lists_root;
  }
MesaDrawingAreaClassPart;

typedef struct _MesaDrawingAreaClassRec
  {
    CoreClassPart core_class;
    GLwDrawingAreaClassPart glwDrawingArea_class;
    MesaDrawingAreaClassPart mesaDrawingArea_class;
  }
MesaDrawingAreaClassRec;
extern MesaDrawingAreaClassRec mesaDrawingAreaClassRec;

typedef struct
  {
    Boolean ximage;
    XMesaContext context;
    Boolean share_lists;
    Widget share_lists_with;
  }
MesaDrawingAreaPart;

typedef struct _MesaDrawingAreaRec
  {
    CorePart core;
    GLwDrawingAreaPart glwDrawingArea;
    MesaDrawingAreaPart mesaDrawingArea;
  }
MesaDrawingAreaRec;

#define MesaRGBA(_widget) \
   (((MesaDrawingAreaWidget)_widget)->glwDrawingArea.rgba)
#define MesaDoublebuffer(_widget) \
   (((MesaDrawingAreaWidget)_widget)->glwDrawingArea.doublebuffer)
#define MesaContext(_widget) \
   (((MesaDrawingAreaWidget)_widget)->mesaDrawingArea.context)
#define MesaXImage(_widget) \
   (((MesaDrawingAreaWidget)_widget)->mesaDrawingArea.ximage)
#define MesaShareLists(_widget) \
   (((MesaDrawingAreaWidget)_widget)->mesaDrawingArea.share_lists)
#define MesaShareListsWith(_widget) \
   (((MesaDrawingAreaWidget)_widget)->mesaDrawingArea.share_lists_with)

#define MesaAlphaSize(_widget) \
   (((MesaDrawingAreaWidget)_widget)->glwDrawingArea.alphaSize)
#define MesaDepthSize(_widget) \
   (((MesaDrawingAreaWidget)_widget)->glwDrawingArea.depthSize)
#define MesaStencilSize(_widget) \
   (((MesaDrawingAreaWidget)_widget)->glwDrawingArea.stencilSize)
#define MesaAccumRedSize(_widget) \
   (((MesaDrawingAreaWidget)_widget)->glwDrawingArea.accumRedSize)


#define MesaListsRoot \
   (mesaDrawingAreaClassRec.mesaDrawingArea_class.lists_root)

#endif /* _MesaDrawingAreaP_h */
