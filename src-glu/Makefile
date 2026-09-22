# Makefile for GLU for Unix

# Mesa 3-D graphics library
# Version:  1.2.3
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


# $Id: Makefile,v 1.16 1996/01/19 19:21:03 brianp Exp $

# $Log: Makefile,v $
# Revision 1.16  1996/01/19  19:21:03  brianp
# use mv to put libraries in LIBDIR
#
# Revision 1.15  1995/09/19  14:17:22  brianp
# bumped version to 1.2.3
#
# Revision 1.14  1995/08/01  20:53:08  brianp
# modified to use $(MAKELIB)
#
# Revision 1.13  1995/06/21  15:55:20  brianp
# renamed source files to 8.3 convention
# added Linux ELF shared library target
# Release 1.2.1
#
# Revision 1.12  1995/05/22  16:57:01  brianp
# Release 1.2
#
# Revision 1.11  1995/05/01  14:50:38  brianp
# added polygon tesselator files
#
# Revision 1.10  1995/04/17  13:50:07  brianp
# use GLU_LIB variable
#
# Revision 1.9  1995/03/08  19:48:53  brianp
# added -Y to makedepend
#
# Revision 1.8  1995/03/08  19:42:50  brianp
# revamped for makedepend
#
# Revision 1.7  1995/03/04  19:40:02  brianp
# updated for Make-config
#
# Revision 1.6  1995/03/01  21:06:44  brianp
# replaced 'make' with $(MAKE)
#
# Revision 1.5  1995/02/28  20:42:26  brianp
# added tess.c module
#
# Revision 1.4  1995/02/24  19:35:57  brianp
# added VPATH for RCS
#
# Revision 1.3  1995/02/24  16:53:16  brianp
# added netbsd target
#
# Revision 1.2  1995/02/24  15:34:27  brianp
# updated for RCS
#
# Revision 1.1  1995/02/24  15:33:30  brianp
# Initial revision
#


##### MACROS #####

VPATH = RCS

INCDIR = ../include
LIBDIR = ../lib

SOURCES = glu.c mipmap.c nurbs.c nurbscrv.c nurbssrf.c nurbsutl.c \
	project.c quadric.c tess.c tesselat.c polytest.c

OBJECTS = $(SOURCES:.c=.o)



##### RULES #####

.c.o:
	$(CC) -c -I$(INCDIR) $(CFLAGS) $<



##### TARGETS #####

default:
	@echo "Specify a target configuration"

clean:
	-rm *.o *~

targets: $(LIBDIR)/$(GLU_LIB)

# Make the library:
$(LIBDIR)/$(GLU_LIB): $(OBJECTS)
	$(MAKELIB) $(GLU_LIB) $(OBJECTS)
	$(RANLIB) $(GLU_LIB)
	mv $(GLU_LIB)* $(LIBDIR)

include ../Make-config

include depend



#
# Run 'make depend' to update the dependencies if you change what's included
# by any source file.
# 
depend: $(SOURCES)
	makedepend -fdepend -Y -I../include $(SOURCES)

