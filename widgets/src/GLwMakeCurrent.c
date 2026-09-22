/* GLwMakeCurrent.c -- Implementation file for the Mesa widget
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

   $Id: GLwMakeCurrent.c,v 1.1 1995/04/22 15:29:54 ohl Exp $
 */

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <GL/MesaDrawingAreaP.h>

void
GLwMakeCurrent (Widget w)
{
  if (XtIsSubclass (w, mesaDrawingAreaWidgetClass))
    XMesaMakeCurrent (MesaContext (w));
  else
    XtAppError (XtWidgetToApplicationContext (w),
		"Not a MesaDrawingArea widget!");
}

/* The End. */

