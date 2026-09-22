# Top-level makefile for Mesa

# Mesa 3-D graphics library
# Version:  1.2.8
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


# $Id: Makefile,v 1.32 1996/05/22 17:51:04 brianp Exp $

# $Log: Makefile,v $
# Revision 1.32  1996/05/22  17:51:04  brianp
# added nt/ directory to tar files
#
# Revision 1.31  1996/05/22  14:11:37  brianp
# compile src-tk for NeXT
#
# Revision 1.30  1996/05/15  18:28:06  brianp
# added bsdos, qnx, linux-mondello, and several irix6 targets
#
# Revision 1.29  1996/03/05  19:19:25  brianp
# removed solaris-gcc config
#
# Revision 1.28  1996/02/27  15:17:21  brianp
# added next configuration from Pascal Thibaudeau
#
# Revision 1.27  1996/01/22  16:16:44  brianp
# added amiwin, next-x11, new machten and solaris targets
#
# Revision 1.26  1995/11/30  17:16:32  brianp
# updated version to 1.2.5
#
# Revision 1.25  1995/11/22  18:55:30  brianp
# added sunos5-x11r6-gcc-sl target from Oleg Krivosheev
#
# Revision 1.24  1995/11/16  20:52:10  brianp
# added a number of new Sun targets per Frederic Devernay
#
# Revision 1.23  1995/11/13  21:54:01  brianp
# added unicos target
#
# Revision 1.22  1995/10/31  19:33:40  brianp
# added aix-sl and hpux-gcc targets
# updated version to 1.2.4
#
# Revision 1.21  1995/09/29  16:04:48  brianp
# renamed HOME to MESA_HOME and NAME to MESA_NAME
#
# Revision 1.20  1995/09/22  19:57:24  brianp
# added solaris-gcc config from Steve Stevenson
#
# Revision 1.19  1995/09/15  13:05:42  brianp
# added dgux, hpux-sl and ultrix-gcc configs
# updated version to 1.2.3
#
# Revision 1.18  1995/08/01  21:03:27  brianp
# added machten target, updates for 1.2.2
#
# Revision 1.17  1995/06/21  16:25:23  brianp
# added irix5-dso, linux-elf and unixware targets
# Release 1.2.1
#
# Revision 1.16  1995/05/22  16:53:59  brianp
# Release 1.2
#
# Revision 1.15  1995/05/16  14:12:23  brianp
# added amix target
# added contrib tree to tar file
#
# Revision 1.14  1995/04/20  17:35:52  brianp
# made SCO a separate target
#
# Revision 1.13  1995/04/20  13:32:34  brianp
# added SCO to Linux config
#
# Revision 1.12  1995/04/17  14:50:24  brianp
# 1.1.4 beta release
#
# Revision 1.11  1995/03/31  17:05:29  brianp
# 1.1.3 beta release
#
# Revision 1.10  1995/03/14  14:34:02  brianp
# 1.1.2 beta release
#
# Revision 1.9  1995/03/09  14:51:39  brianp
# added osf1 target per Joseph Canedo
#
# Revision 1.8  1995/03/08  19:50:30  brianp
# added depend files to TAR_FILES
#
# Revision 1.7  1995/03/07  14:38:33  brianp
# changed NAME to Mesa-1.1.1beta
#
# Revision 1.6  1995/03/07  14:31:31  brianp
# new hpux CFLAGS per Jan Springer's suggestion
#
# Revision 1.5  1995/03/04  20:00:57  brianp
# changed tar NAME from Mesa1.1beta to Mesa-1.1beta
#
# Revision 1.4  1995/03/04  19:47:13  brianp
# 1.1 beta revision
#
# Revision 1.3  1995/03/01  21:11:53  brianp
# added GLUT to tar target
#
# Revision 1.2  1995/02/24  16:20:26  brianp
# added netbsd target
#
# Revision 1.1  1995/02/24  16:19:25  brianp
# Initial revision
#


# To add a new configuration for your system add it to the list below
# then update the Make-config file.



default:
	@echo "Type one of the following:"
	@echo "  make aix                  for IBM RS/6000 with AIX"
	@echo "  make aix-sl               for IBM RS/6000, make shared libs"
	@echo "  make amiga                for Amigas"
	@echo "  make amiwin               for Amiga with SAS/C and AmiWin"
	@echo "  make amix                 for Amiga 3000 UX  SVR4 v2.1 systems"
	@echo "  make bsdos                for BSD/OS from BSDI using GCC"
	@echo "  make dgux                 for Data General"
	@echo "  make freebsd              for FreeBSD systems with GCC"
	@echo "  make gcc                  for a generic system with GCC"
	@echo "  make hpux                 for HP systems with HPUX"
	@echo "  make hpux-gcc             for HP systems with HPUX using GCC"
	@echo "  make hpux-sl              for HP systems with HPUX, make shared libs"
	@echo "  make irix4                for SGI systems with IRIX 4.x"
	@echo "  make irix5                for SGI systems with IRIX 5.x"
	@echo "  make irix5-dso            for SGI systems with IRIX 5.x, make DSOs"
	@echo "  make irix6-32             for SGI systems with IRIX 6.x, make 32-bit libs"
	@echo "  make irix6-n32            for SGI systems with IRIX 6.x, make n32-bit libs"
	@echo "  make irix6-64             for SGI systems with IRIX 6.x, make 64-bit libs"
	@echo "  make linux                for Linux systems with GCC"
	@echo "  make linux-elf            for Linux systems, make ELF shared libs"
	@echo "  make linux-mondello       for Linux with prototype Cirrus Mondello card"
	@echo "  make mswindows            for Microsoft Windows"
	@echo "  make macintosh            for Macintosh"
	@echo "  make machten-2.2          for Macs w/ MachTen 2.2 (68k w/ FPU)"
	@echo "  make machten-4.0          for Macs w/ MachTen 4.0.1 or newer with GNU make"
	@echo "  make netbsd               for NetBSD 1.0 systems with GCC"
	@echo "  make next                 for NeXT systems with NEXTSTEP 3.3"
	@echo "  make next-x11             for NeXT with X11"
	@echo "  make osf1                 for DEC Alpha systems with OSF/1"
	@echo "  make qnx                  for QNX V4 systems with Watcom compiler"
	@echo "  make sco                  for SCO Unix systems with ODT"
	@echo "  make solaris-x86          for PCs with Solaris"
	@echo "  make solaris-x86-gcc      for PCs with Solaris using GCC"
#	@echo "  make solaris-gcc          for Solaris 2 systems with GCC"
	@echo "  make sunos4               for Suns with SunOS 4.x"
	@echo "  make sunos4-sl            for Suns with SunOS 4.x, make shared libs"
	@echo "  make sunos4-gcc           for Suns with SunOS 4.x and GCC"
	@echo "  make sunos4-gcc-sl        for Suns with SunOS 4.x, GCC, make shared libs"
	@echo "  make sunos5               for Suns with SunOS 5.x"
	@echo "  make sunos5-sl            for Suns with SunOS 5.x, make shared libs"
	@echo "  make sunos5-gcc           for Suns with SunOS 5.x and GCC"
	@echo "  make sunos5-gcc-sl        for Suns with SunOS 5.x, GCC, make shared libs"
	@echo "  make sunos5-x11r6-gcc-sl  for Suns with X11R6, GCC, make shared libs"
	@echo "  make ultrix-gcc           for DEC systems with Ultrix and GCC"
	@echo "  make unicos               for Cray C90 (and other?) systems"
	@echo "  make unixware             for PCs running UnixWare"
	@echo "  make vistra               for Stardent Vistra systems"
	@echo "  make clean"



aix aix-sl amix bsdos debug dgux freebsd gcc hpux hpux-gcc hpux-sl irix4 irix5 irix5-dso irix6-32 irix6-n32 irix6-64 linux linux-elf machten-2.2 machten-4.0 netbsd next-x11 osf1 qnx sco solaris-x86 solaris-x86-gcc sunos4 sunos4-sl sunos4-gcc sunos4-gcc-sl sunos5 sunos5-sl sunos5-gcc sunos5-gcc-sl sunos5-x11r6-gcc-sl ultrix-gcc unicos unixware vistra:
	-mkdir lib
	touch src/depend
	touch src-glu/depend
	cd src ; $(MAKE) $@
	cd src-glu ; $(MAKE) $@
	cd src-tk ; $(MAKE) $@
	cd src-aux ; $(MAKE) $@
	cd demos ; $(MAKE) $@
	cd samples ; $(MAKE) $@
	cd book ; $(MAKE) $@

amiga:
	@echo "See the README.AMIGA file for installation information"

amiwin:
	mklib.amiwin

linux-mondello:
	-mkdir lib
	touch src/depend
	touch src-glu/depend
	cd src ; $(MAKE) $@
	cd mondello ; $(MAKE) $@
	cd src-glu ; $(MAKE) $@
	cd src-tk2 ; $(MAKE) $@
	cd src-aux ; $(MAKE) $@
	cd demos ; $(MAKE) $@
	cd samples ; $(MAKE) $@
	cd book ; $(MAKE) $@

macintosh:
	@echo "See the README file for Macintosh intallation information"

mswindows:
	@echo "See the windows/README.WIN for installation information"

next:
	-mkdir lib
	cd src ; $(MAKE) -f Makefile.NeXT $@
	cd src-glu ; $(MAKE) -f Makefile.NeXT $@
	cd src-tk ; $(MAKE) -f Makefile.NeXT $@
	cd src-aux ; $(MAKE) -f Makefile.NeXT $@
	cd NeXT ; $(MAKE) -f Makefile.NeXT $@




# Remove .o files, emacs backup files, etc.
clean:
	-rm -f include/*~
	-rm -f include/GL/*~
	-rm -f src/*.o src/*~ src/*.a
	-rm -f src-aux/*.o src-aux/*~ src-aux/*.a
	-rm -f src-glu/*.o src-glu/*~ src-glu/*.a
	-rm -f src-tk/*.o src-tk/*~ src-tk/*.a
	-rm -f src-tk2/*.o src-tk2/*~ src-tk2/*.a
	-rm -f book/*.o book/*~
	-rm -f demos/*.o demos/*~
	-rm -f samples/*.o samples/*~
	-rm -f mondello/*.o mondello/*~ mondello/*.a

# Remove everthing that can be remade
realclean: clean
	-rm -f lib/*.a lib/*.so*
	cd book ; $(MAKE) realclean
	cd demos ; $(MAKE) realclean
	cd samples ; $(MAKE) realclean
	cd mondello; $(MAKE) realclean
	-rm -f lib/*.a lib/*.so*



MESA_HOME = Mesa-1.2.8
MESA_NAME = Mesa-1.2.8

TAR_FILES =	\
	$(MESA_HOME)/README			\
	$(MESA_HOME)/README.AMIWIN		\
	$(MESA_HOME)/README.AMIGA		\
	$(MESA_HOME)/README.GLUT		\
	$(MESA_HOME)/IAFA-PACKAGE		\
	$(MESA_HOME)/LICENSE			\
	$(MESA_HOME)/Makefile			\
	$(MESA_HOME)/Make-config		\
	$(MESA_HOME)/mklib.*			\
	$(MESA_HOME)/include/*.h		\
	$(MESA_HOME)/include/GL/*.h		\
	$(MESA_HOME)/include/mondello/*.h	\
	$(MESA_HOME)/src*/README[12]		\
	$(MESA_HOME)/src*/Makefile		\
	$(MESA_HOME)/src*/Makefile.NeXT		\
	$(MESA_HOME)/src*/Smakefile		\
	$(MESA_HOME)/src*/depend		\
	$(MESA_HOME)/src*/*.[ch]		\
	$(MESA_HOME)/src/gl.FD			\
	$(MESA_HOME)/demos/Makefile		\
	$(MESA_HOME)/demos/Smakefile		\
	$(MESA_HOME)/demos/*.[cf]		\
	$(MESA_HOME)/demos/isosurf.dat		\
	$(MESA_HOME)/book/Makefile		\
	$(MESA_HOME)/book/Smakefile		\
	$(MESA_HOME)/book/NOTES			\
	$(MESA_HOME)/book/README		\
	$(MESA_HOME)/book/*.[ch]		\
	$(MESA_HOME)/samples/Makefile		\
	$(MESA_HOME)/samples/Smakefile		\
	$(MESA_HOME)/samples/NOTES		\
	$(MESA_HOME)/samples/README		\
	$(MESA_HOME)/samples/*.rgb		\
	$(MESA_HOME)/samples/*.c		\
	$(MESA_HOME)/widgets			\
	$(MESA_HOME)/windows			\
	$(MESA_HOME)/NeXT			\
	$(MESA_HOME)/mondello			\
	$(MESA_HOME)/amiga			\
	$(MESA_HOME)/nt


tar:
	cd .. ; \
	tar -cvf $(MESA_NAME).tar $(TAR_FILES) ; \
	gzip $(MESA_NAME).tar ; \
	mv $(MESA_NAME).tar.gz $(MESA_HOME)



SRC_FILES = src/Makefile src/*.[ch]

srctar:
	tar -cvf src.tar $(SRC_FILES) ; \
	gzip src.tar
