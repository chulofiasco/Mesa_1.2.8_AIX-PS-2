# Mesa 1.2.8 Unified Diagnostic & Test Tools

This directory contains the consolidated diagnostic programs and graphical testing utilities used to port Mesa 1.2.8 and verify its correctness against the MetaWare High C R2.2g compiler on AIX 1.3.

During the porting effort, over a dozen standalone test files were refactored and merged into four core tools.

## Build Instructions

To build all the core tools in this directory, simply run:
```bash
make
```
*Note: Graphical tests (`test_gallery`, `ostest`, `mesatest`) depend on `libMesaGL`, `libMesaGLU`, and `libMesaaux` being built first in the `../lib` directory.*

---

## 1. Unified Compiler Diagnostic Suite (`hc_bug_suite.c`)

**Purpose:** Verifies that the MetaWare High C compiler is generating correct x86 assembly for complex mathematical and parameter-passing scenarios.
**Execution:** `./hc_bug_suite` (Headless)

This suite is a headless diagnostic tool that consolidates several legacy tests (`test_math.c`, `test_args.c`, `test_math_cast.c`, `test_dodec.c`, and `hc_bug_test.c`). It tests for over 260 known compiler failure states and isolates severe optimizer bugs regarding FPU stack management. It specifically checks for:
*   FPU stack leaking during nested trigonometric loops (e.g. `sin()`/`cos()` in `gluSphere`).
*   Argument shifting and sign dropping when passing implicit `double` arguments.
*   Multidimensional array pointer truncation and chained assignment corruption.
*   Implicit FPU type promotion bugs when casting `unsigned short` to `double` in `<math.h>` functions.

## 2. Interactive Visual Gallery (`test_gallery.c`)

**Purpose:** Provides a visual, interactive verification of all fundamental OpenGL drawing routines via the GLAUX windowing library.
**Execution:** `./test_gallery` (X11 / Motif required)

Instead of compiling numerous different binaries for various visual tests, the gallery merges all legacy visual tests (formerly `test_prims.c`, `test_quadrics.c`, `test_antialias.c`, `tess_test.c`, etc.) into a single interactive 3x3 grid slideshow.
*   Press **SPACEBAR** to cycle through the different testing scenes.
*   It includes verifications for wireframe polyhedra, smooth shading quadrics, complex antialiasing, polygon tessellation, and environment mapping.
*   Press **'r'** to toggle rotation/animation where applicable.
*   This serves as the primary visual proof that shapes render correctly without starbursting or exploding coordinates due to compiler math bugs.

## 3. Expanded Headless Tester (`ostest.c`)

**Purpose:** A fully offline "golden master" image generator utilizing the OSMesa (Off-Screen Mesa) software rendering engine.
**Execution:** `./ostest [output.ppm]` (Headless)

`ostest` initializes a hidden, off-screen RGBA buffer and executes a massive array of advanced graphics tests. It renders 2D textures, alpha blending, GLU quadrics, display lists, and scissor clipping onto a 16-part grid. 
*   The final buffer is written directly to disk as a binary `.ppm` image file.
*   This tool allows for automated integration testing of the entire rendering pipeline without requiring an active X server.
*   It also absorbed tests like `test_blend.c` to verify the mathematical correctness of `GL_LOGIC_OP` blending equations headlessly.

## 4. General Functionality Test (`mesatest.c`)

**Purpose:** A comprehensive GLX functionality test that exercises various internal Mesa rendering paths, transformations, and drawing commands.
**Execution:** `./mesatest` (X11 / GLX required)

`mesatest` operates as a broad-spectrum stress test for the Mesa rendering engine. It opens a standard window and rapidly tests the underlying rasterization loops, vertex transformations, and state machine integrity. It ensures that the core rendering pipeline is stable over long operational periods and that state changes do not leak across frames.

---
*Note: The original, unmerged legacy test files can be found archived in the `legacy/` subdirectory for historical reference.*
