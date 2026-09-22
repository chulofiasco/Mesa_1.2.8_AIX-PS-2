# Makefile for MetaWare High C compiler bug diagnostics and tests

CC = cc
CFLAGS = -Hon=ansi -I../include -I../GL
LDFLAGS = -L../lib
LIBS = -lMesaaux -lMesatk -lMesaGLU -lMesaGL -lXext -lX11 -lm

TARGETS = hc_bug_suite test_gallery ostest mesatest x11_renderbench

all: $(TARGETS)

hc_bug_suite: hc_bug_suite.c
	$(CC) $(CFLAGS) hc_bug_suite.c -lm -o hc_bug_suite

test_gallery: test_gallery.c
	# Note: test_gallery depends on Mesa libraries to be built first in ../lib
	$(CC) $(CFLAGS) test_gallery.c $(LDFLAGS) $(LIBS) -o test_gallery

x11_renderbench: x11_renderbench.c
	$(CC) $(CFLAGS) x11_renderbench.c $(LDFLAGS) -lMesaaux -lMesatk -lMesaGLU -lMesaGL -lXm -lXt -lXext -lX11 -lm -o x11_renderbench

ostest: ostest.c
	# Note: ostest depends on Mesa libraries to be built first in ../lib
	$(CC) $(CFLAGS) ostest.c $(LDFLAGS) $(LIBS) -o ostest

mesatest: mesatest.c
	$(CC) $(CFLAGS) mesatest.c $(LDFLAGS) $(LIBS) -o mesatest


clean:
	rm -f *.o $(TARGETS)
