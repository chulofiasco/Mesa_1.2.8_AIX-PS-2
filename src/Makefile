# Makefile for core library

# Mesa 3-D graphics library
# Version:  1.2.8
# Copyright (C) 1995  Brian Paul  (brianp@ssec.wisc.edu)
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


# $Id: Makefile,v 1.29 1996/05/15 18:21:35 brianp Exp $

# $Log: Makefile,v $
# Revision 1.29  1996/05/15  18:21:35  brianp
# divided SOURCES into CORE_SOURCES and DRIVER_SOURCES
#
# Revision 1.28  1996/02/27  15:16:30  brianp
# added alphabuf.c
#
# Revision 1.27  1996/02/15  16:03:33  brianp
# added varray.c and removed gamma.c
#
# Revision 1.26  1996/01/19  19:21:28  brianp
# use mv to put libraries in LIBDIR
#
# Revision 1.25  1996/01/16  17:07:53  brianp
# added svgamesa.c to SOURCES
#
# Revision 1.24  1996/01/10  22:44:23  brianp
# changed make-lib commands per Robert Prus
#
# Revision 1.23  1995/11/13  21:34:00  brianp
# added xmesa3.c file
#
# Revision 1.22  1995/10/19  15:55:02  brianp
# added gamma.c to SOURCES
#
# Revision 1.21  1995/10/17  21:44:28  brianp
# changed SOURCES for new device driver and masking.c files
#
# Revision 1.20  1995/08/01  20:58:09  brianp
# modified to use $(MAKELIB)
#
# Revision 1.19  1995/06/21  15:54:36  brianp
# renamed source files to 8.3 convention
# added Linux ELF shared library target
# Release 1.2.1
#
# Revision 1.18  1995/05/22  21:05:42  brianp
# Release 1.2
#
# Revision 1.17  1995/04/17  14:40:37  brianp
# use GL_LIB variable
#
# Revision 1.16  1995/03/24  19:30:25  brianp
# added vb.c to SOURCES
#
# Revision 1.15  1995/03/13  15:58:15  brianp
# added xfonts.c to SOURCES
#
# Revision 1.14  1995/03/10  16:26:16  brianp
# use eval2.c instead of eval.c
#
# Revision 1.13  1995/03/08  19:49:05  brianp
# added -Y to makedepend
#
# Revision 1.12  1995/03/08  19:42:36  brianp
# revamped for makedepend
#
# Revision 1.11  1995/03/08  15:13:10  brianp
# dependency changes for dd_logicop, dd_clear_index, dd_clear_color
#
# Revision 1.10  1995/03/07  19:10:54  brianp
# made changes for pb w/ blending, alpha test, logic ops
#
# Revision 1.9  1995/03/07  14:15:25  brianp
# use eval.c instead of eval2.c because of FPE on Linux
#
# Revision 1.8  1995/03/04  19:18:40  brianp
# changed for Make-config
#
# Revision 1.7  1995/03/01  21:05:20  brianp
# replaced 'make' with $(MAKE)
#
# Revision 1.6  1995/03/01  17:44:40  brianp
# added stenciling for PB
#
# Revision 1.5  1995/02/28  21:23:16  brianp
# added glx support
#
# Revision 1.4  1995/02/27  22:49:15  brianp
# modified for PB
#
# Revision 1.3  1995/02/24  19:33:36  brianp
# added VPATH for RCS
#
# Revision 1.2  1995/02/24  16:50:54  brianp
# added netbsd target
#
# Revision 1.1  1995/02/24  14:32:10  brianp
# Initial revision
#


##### MACROS #####

VPATH = RCS

INCDIR = ../include
LIBDIR = ../lib

CORE_SOURCES = accum.c alpha.c alphabuf.c attrib.c bitmap.c blend.c \
	bresenhm.c clip.c context.c copypix.c dd.c depth.c draw.c \
	drawpix.c enable.c eval2.c fog.c feedback.c fortran.c get.c \
	interp.c light.c lines.c list.c logic.c masking.c misc.c osmesa.c \
	pb.c pixel.c points.c polygons.c readpix.c scissor.c span.c \
	stencil.c texture.c varray.c vb.c vertex.c winpos.c xform.c

DRIVER_SOURCES = glx.c xfonts.c xmesa1.c xmesa2.c xmesa3.c svgamesa.c cmesa.c

SOURCES = $(CORE_SOURCES) $(DRIVER_SOURCES)



OBJECTS = $(SOURCES:.c=.o)



##### RULES #####

.c.o:
	$(CC) -c -I$(INCDIR) $(CFLAGS) $<



##### TARGETS #####

default:
	@echo "Specify a target configuration"

clean:
	-rm *.o *~

targets: $(LIBDIR)/$(GL_LIB)

# Make the library
$(LIBDIR)/$(GL_LIB): $(OBJECTS)
	$(MAKELIB) $(GL_LIB) $(OBJECTS)
	$(RANLIB) $(GL_LIB)
	mv $(GL_LIB)* $(LIBDIR)

include ../Make-config

include depend



#
# Run 'make depend' to update the dependencies if you change what's included
# by any source file.
# 
dep: $(SOURCES)
	makedepend -fdepend -Y -I../include $(SOURCES)

