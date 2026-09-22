# Mesa 3D Graphics Library (v1.2.8)

**Mesa 1.2.8** is a historical release of the Mesa 3D graphics library, originally developed by Brian Paul in 1995. Mesa is an open-source 3D graphics library that provides a generic OpenGL implementation for rendering 3D graphics. This specific version represents the early days of open-source 3D rendering, offering a software rasterizer and a foundational X11/GLX implementation (`xmesa`) before the advent of widespread hardware acceleration.

## Overview

This repository contains the source code for Mesa 1.2.8, along with a historical porting effort to bring the library to **IBM AIX 1.3** running on the **IBM PS/2** architecture (i386).

The porting effort resolves numerous compatibility issues with the era-specific **MetaWare High C 2.2g** compiler, standardizes missing system behaviors, and ensures that the core 3D rasterization pipeline, X11 GLX integration, and GLU libraries function flawlessly on this legacy UNIX environment.

### Included Components

* **`src/`**: The core Mesa 3D software rendering pipeline (OpenGL 1.0 implementation).
* **`src-glu/`**: The standard OpenGL Utility Library (GLU), including NURBS, tessellation, and quadrics support.
* **`widgets/`**: Motif and standard X11/Xt widgets (`libMesaGLw.a`) for integrating OpenGL contexts into X11 UI applications.
* **`demos/`, `book/`, `samples/`**: Over 80 historical 3D demo applications and code samples from the OpenGL Red Book.

## AIX 1.3 Porting Details

For comprehensive developer documentation on the fixes applied to compile this on AIX 1.3 (including compiler diagnostic audits, infinite loop fixes in the GLU tessellator, and `install-sh` fallback bug fixes), please consult the included porting guide:

* [PORTING-MESA-AIX13.md](./PORTING-MESA-AIX13.md)

## Building on AIX 1.3

To build the core library and install it on an AIX 1.3 PS/2 system:

```sh
# Build the base libraries for AIX PS/2
make aix-ps2

# Install libraries to /usr/local/lib and headers to /usr/local/include/GL
make install
```

Optional demo applications can be built and installed using:
```sh
make install-demos
make install-samples
make install-book
```
Build time on a Model 90 with a Pentium 66 Complex:

```real    49:50.95
user     8:58.91
sys     16:56.23
```
