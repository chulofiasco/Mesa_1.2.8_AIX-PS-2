# Makefile for demo programs

# Mesa 3-D graphics library
# Version:  1.2
# Copyright (C) 1995-1996  Brian Paul  (brianp@ssec.wisc.edu)
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


# $Id: Makefile,v 1.12 1996/05/15 18:32:06 brianp Exp $

# $Log: Makefile,v $
# Revision 1.12  1996/05/15  18:32:06  brianp
# added reflect and winpos programs
#
# Revision 1.11  1995/10/27  20:58:04  brianp
# added polygon offset demo
#
# Revision 1.10  1995/10/19  18:39:07  brianp
# added gamma demo
#
# Revision 1.9  1995/10/12  18:32:00  brianp
# added osdemo target
#
# Revision 1.8  1995/08/01  21:00:14  brianp
# use $(TK_LIB) and $(AUX_LIB), added gears and shadow demos
#
# Revision 1.7  1995/05/01  15:25:20  brianp
# clean up and reorganize
#
# Revision 1.6  1995/04/20  13:39:05  brianp
# added glxdemo, glxpixmap
# removed accum, prim, stencil
#
# Revision 1.5  1995/03/07  14:27:01  brianp
# added realclean target
#
# Revision 1.4  1995/03/01  21:08:51  brianp
# replaced 'make' with $(MAKE)
#
# Revision 1.3  1995/02/24  16:48:44  brianp
# fixed netbsd CFLAGS
#
# Revision 1.2  1995/02/24  16:26:26  brianp
# added netbsd target
#
# Revision 1.1  1995/02/24  16:23:53  brianp
# Initial revision
#


##### MACROS #####

INCDIR = ../include

GL_LIBS = -L../lib -lMesaaux -lMesatk -lMesaGLU -lMesaGL -lm $(XLIBS)

LIB_DEP = ../lib/$(GL_LIB) ../lib/$(GLU_LIB) ../lib/$(TK_LIB) ../lib/$(AUX_LIB)

PROGS = bounce gamma gears glxdemo glxpixmap isosurf offset osdemo shadow \
	spin test0 wave xdemo reflect winpos



##### RULES #####

.SUFFIXES:
.SUFFIXES: .c

.c: $(LIB_DEP)
	$(CC) -I$(INCDIR) $(CFLAGS) $< $(GL_LIBS) -o $@



##### TARGETS #####

default:
	@echo "Specify a target configuration"

clean:
	-rm *.o *~

realclean:
	-rm $(PROGS)
	-rm *.o *~

targets: $(PROGS)

include ../Make-config

