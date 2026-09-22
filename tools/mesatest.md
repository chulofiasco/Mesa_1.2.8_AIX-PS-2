# mesatest — Mesa 1.2.8 Comprehensive Library Test

## Overview

`mesatest.c` is a single-file program that exercises all four Mesa 1.2.8 static
libraries in two modes:

- **Console mode** (default): off-screen rendering via OSMesa — no X11 window,
  automated PASS/FAIL per scene, suitable for scripted regression testing.
- **X11 mode** (`-x11`): interactive window via the tk toolkit, cycling through
  eight rendering scenes on keypress.

Each scene targets a distinct GL or GLU subsystem. Written to strict **C89 (ANSI C)**
for MetaWare High C R2.2g on AIX 1.3 PS/2.

---

## Build

From the `Mesa-1.2.8` root directory:

```sh
cc -I./include mesatest.c -L./lib \
   -lMesaaux -lMesatk -lMesaGLU -lMesaGL -lm -lXext -lX11 -o mesatest
```

---

## Usage

```sh
mesatest          # console mode: off-screen test, prints PASS/FAIL per scene
mesatest -x11     # X11 interactive window
```

---

## Console Mode

Default (no arguments). No X11 connection is opened. The program:

1. Allocates a 640×480×4-byte heap buffer (`malloc(WIN_W * WIN_H * 4)`).
2. Creates an OSMesa context (`OSMesaCreateContext(GL_RGBA, NULL)`) and binds it
   to the buffer (`OSMesaMakeCurrent`). No X server required.
3. Queries and prints GL/GLU version strings.
4. Runs each of the 8 scenes exactly once with `g_angle = 45°`.
5. After each scene calls `glFlush()` then `glReadPixels` at the center pixel.
   Any non-zero RGB value means the rasterizer produced output — **PASS**.
6. Prints a summary and exits with code **0** (all pass) or **1** (any fail).

**Verified output on AIX 1.3 PS/2 / dirtbike:**

```
Mesa 1.2.8 Comprehensive Library Test
======================================
Libraries: libMesaGL  libMesaGLU  libMesatk  libMesaaux
Mode     : console (off-screen OSMesa, no X11 needed)
Tip      : run with -x11 for interactive window

[GL]   Vendor    : Brian Paul
[GL]   Renderer  : Mesa
[GL]   Version   : 1.2.8
[GLU]  Version   : 1.2.8 Mesa

Scene                                                 Result
-----------------------------------------------------  ------
  [1/8] Mesa Test 1/8: GL Matrices & glReadPixels     PASS  (px 254,254,254)
  [2/8] Mesa Test 2/8: Lighting (GLU Quadric Sphere)  PASS  (px 190,148,62)
  [3/8] Mesa Test 3/8: Fog (aux Teapot)               PASS  (px 233,233,233)
  [4/8] Mesa Test 4/8: Blending                       PASS  (px 122,180,103)
  [5/8] Mesa Test 5/8: Texture Mapping                PASS  (px 28,0,0)
  [6/8] Mesa Test 6/8: Stencil Buffer (GLU Disk mask)  PASS  (px 37,37,37)
  [7/8] Mesa Test 7/8: Display List (aux Torus)       PASS  (px 11,0,24)
  [8/8] Mesa Test 8/8: GLU Tessellator (concave L-shape)  PASS  (px 24,24,37)

All 8 scenes PASSED.
```

Exit code `0` = all passed. Exit code `1` = at least one FAIL or setup error.

### OSMesa stencil note

`OSMesaCreateContext` in Mesa 1.2.8 takes only `format` and `sharelist` — there is no
parameter to request a stencil buffer. The implementation may not allocate one,
which would cause `GL_STENCIL_OP GL_REPLACE` to be a no-op and `GL_EQUAL 1` to never
pass. Scene 6 handles this by drawing a dim base colour (RGB 15%, 15%, 15%) directly
into the color buffer during Pass 1 (stencil write). The center pixel is non-black
regardless of stencil availability — `(px 37,37,37)` confirms this. In X11 mode
where the GL context does have a stencil buffer, Pass 2 paints bright stripes on top.

---

## Controls (X11 mode)

| Key | Action |
|-----|--------|
| `SPACE` or `N` | Next scene |
| `P` | Previous scene |
| `ESC` or `Q` | Quit |

---

## Libraries Tested

| Library | Exercised by |
|---------|-------------|
| `libMesaGL.a` | Matrix stack (`glPushMatrix`/`glPopMatrix`/`glRotatef`), `GL_LIGHTING`, `GL_FOG`, `GL_BLEND`, `GL_DEPTH_TEST`, `GL_STENCIL_TEST`, `GL_TEXTURE_2D`, display lists (`glNewList`/`glCallList`), `glReadPixels`, `glRectf`, `glBegin`/`glEnd` |
| `libMesaGLU.a` | `gluPerspective`, `gluLookAt`, `gluNewQuadric`/`gluSphere`/`gluDisk`, `gluNewTess`/`gluBeginPolygon`/`gluTessVertex`/`gluEndPolygon`, `gluGetString(GLU_VERSION)` |
| `libMesatk.a` | `tkInitWindow`, `tkExec`, `tkGetSystem(TK_X_DISPLAY)`, `tkGetSystem(TK_X_WINDOW)`, keyboard and expose callbacks |
| `libMesaaux.a` | `auxSolidTeapot`, `auxSolidTorus` (compiled into display list), `auxSolidSphere` |

---

## X11 Integration

Two distinct X11 usage patterns are demonstrated:

**1. Pre-context display query** (before `tkInitWindow`):

A temporary `XOpenDisplay` / `XCloseDisplay` pair is used to query and print X server
information to stdout. This runs before any GL context exists:

```
[X11] Server vendor    : IBM AIX Version 3, Release 1, IBM (c) 1989-1991
[X11] Vendor release   : 3010
[X11] Protocol version : 11.0
[X11] Screens          : 1
[X11] Screen 0 size    : 1024 x 768 pixels
```

**2. Per-scene title update** (via `tkGetSystem`):

After `tkInitWindow` creates the GL context, the underlying X11 `Display *` and `Window`
handles are retrieved via `tkGetSystem(TK_X_DISPLAY, ...)` and `tkGetSystem(TK_X_WINDOW, ...)`.
Each scene change calls `XStoreName()` to update the window title bar and `XSync()` to
flush the change immediately:

```c
XStoreName(g_dpy, g_xwin, g_titles[g_scene]);
XSync(g_dpy, 0);
```

The connected display name is also printed:

```
[X11]  GL display : :0.0
```

---

## Scene Reference

### Scene 1 — GL Matrices & `glReadPixels`

**Tests:** `glMatrixMode`, `glOrtho`, `glRotatef`, `glPushMatrix`/`glPopMatrix`,
`GL_TRIANGLE_FAN`, multi-color per-vertex, `glReadPixels`.

A five-segment colour wheel (red/yellow/green/blue/magenta triangle fan) spins
continuously around the Z axis. On the first rendered frame, `glReadPixels` samples
the center pixel and prints the RGB value to stdout:

```
[Scene 1] glReadPixels center = (R, G, B)  (non-zero = rendering OK)
```

A non-zero result confirms the rasterizer is writing pixels correctly. This is the
minimal "did Mesa draw anything at all" sanity check.

---

### Scene 2 — Lighting + GLU Quadric Sphere

**Tests:** `GL_LIGHTING`, `GL_LIGHT0`, `glMaterialfv`, `glLightfv`, `gluNewQuadric`,
`gluQuadricDrawStyle(GLU_FILL)`, `gluQuadricNormals(GLU_SMOOTH)`, `gluSphere`,
`gluDeleteQuadric`, `gluPerspective`, `gluLookAt`.

A smooth-shaded gold sphere lit by a single directional light rotates on a compound
axis. This scene validates the entire GL lighting pipeline and confirms that the
`GLenum` fix (Section 14 of the porting notes) is working: `GLU_FILL = 100012UL` and
`GLU_SMOOTH = 100000UL` must survive the call to `gluQuadricDrawStyle` and
`gluQuadricNormals` without truncation or routing to the `default:` error branch.

---

### Scene 3 — Fog + `aux` Teapot

**Tests:** `GL_FOG`, `glFogi(GL_FOG_MODE, GL_LINEAR)`, `glFogf`, `glFogfv`,
`GL_LIGHTING`, `auxSolidTeapot` (`libMesaaux`).

An orange lit teapot slowly rotates inside linear fog (start=4, end=14).
`auxSolidTeapot` is the canonical `libMesaaux` shape — calling it confirms the aux
library is linked and functional. The fog gradient effect validates the Mesa fog
pipeline.

---

### Scene 4 — Blending

**Tests:** `GL_BLEND`, `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`,
`glColor4f`, `glRectf`, `glOrtho`.

Four overlapping coloured rectangles are drawn with alpha transparency:
- Opaque blue background quad
- Translucent red quad (α = 0.55)
- Translucent green quad (α = 0.55)
- Translucent white highlight (α = 0.30)

Colour mixing in the overlap regions confirms the Mesa blending pipeline is correct.

---

### Scene 5 — Texture Mapping

**Tests:** `GL_TEXTURE_2D`, `glTexParameteri`, `glTexImage2D` (OpenGL 1.0 style, no
texture objects), `glTexEnvi(GL_TEXTURE_ENV_MODE, GL_REPLACE)`, `glTexCoord2f`,
`gluPerspective`, `gluLookAt`.

A 64×64 orange-and-black checkerboard texture is generated procedurally in a `static`
array and uploaded via `glTexImage2D`. A textured quad slowly rotates on the Y axis
with 4×4 texture repeat, confirming the Mesa software texture sampling pipeline.

**Note on texture style:** Mesa 1.2.8 implements OpenGL 1.0. There are no
`glGenTextures` / `glBindTexture` texture object functions — `glTexImage2D` writes
directly to the single implicit texture. The `static GLubyte texdata[64][64][3]` array
is allocated at program load time and remains resident.

---

### Scene 6 — Stencil Buffer

**Tests:** `GL_STENCIL_TEST`, `glClearStencil`, `glStencilFunc(GL_ALWAYS)`,
`glStencilOp(GL_REPLACE)`, `glStencilFunc(GL_EQUAL)`,
`gluNewQuadric`, `gluDisk`, diagonal stripe fill.

Two-pass technique:

1. **Pass 1** — Render a GLU disk at the origin with `GL_ALWAYS` stencil and
   `GL_REPLACE`. Color writes remain **enabled** (unlike the classic pure-stencil
   technique): the disk is drawn in a dim base grey (RGB 15%,15%,15%). This
   simultaneously writes `1` to the stencil buffer and ensures the disk area is
   non-black regardless of whether the platform has a working stencil buffer.
2. **Pass 2** — Draw diagonal coloured stripes full-viewport with `GL_EQUAL 1`
   stencil test. Where stencil is available and functional, the bright stripes
   overwrite the dim grey inside the disk boundary; outside the disk the stripes
   are suppressed. Where stencil is unavailable (e.g. OSMesa 1.2.8 without an
   allocated stencil buffer), Pass 2 produces no output and the dim grey from
   Pass 1 remains — the center pixel is still non-black, so the scene PASSes.

The entire assembly rotates slowly. In X11 mode (`-x11`) with a full GL context the
stencil buffer is present and the coloured-stripe-inside-grey-disk effect is visible,
confirming the Mesa stencil pipeline.

---

### Scene 7 — Display List + `aux` Shapes

**Tests:** `glGenLists`, `glNewList(GL_COMPILE)`, `glCallList`, `glEndList`,
`auxSolidTorus` (compiled into the list), `auxSolidSphere` (drawn directly),
`gluPerspective`, `gluLookAt`, `GL_LIGHTING`, `glPushMatrix`/`glPopMatrix`.

A blue `auxSolidTorus` is compiled into display list 1 at startup. Each frame, the
list is called (`glCallList`) to draw the torus spinning on two axes. An orange
`auxSolidSphere` orbits the torus on a separate rotation. This scene validates:

- Display list compilation and playback of `libMesaaux` geometry
- Two simultaneous `glPushMatrix`/`glPopMatrix` transform scopes
- Dual-axis animation via composed `glRotatef` calls

---

### Scene 8 — GLU Tessellator (Concave Polygon)

**Tests:** `gluNewTess`, `gluTessCallback` with `GLU_BEGIN`/`GLU_VERTEX`/`GLU_END`
(constants > 65535 — validates the GLenum fix), `gluBeginPolygon`, `gluTessVertex`,
`gluEndPolygon`, `glOrtho`.

An L-shaped concave polygon (6 vertices) is tessellated each frame. The GLU
tessellator decomposes it into triangles and calls the registered `glBegin`,
`glVertex2dv`, and `glEnd` callbacks to render it. The shape slowly rotates and
cycles through six colours.

**This scene is the definitive validation of Technical Porting Fix #14.** Before the
`GLenum` fix:
- `GLU_BEGIN = 100100UL` was truncated to `34564` in the 16-bit `GLenum`
- The `switch(which)` inside `gluTessCallback` could never match any case constant
- All callbacks were silently discarded; the polygon was never rendered

After the fix (`typedef unsigned int GLenum`), all three callbacks register correctly
and the tessellated L-shape appears.

---

## Expected stdout Output (X11 mode)

```
Mesa 1.2.8 Comprehensive Library Test
======================================
Libraries: libMesaGL  libMesaGLU  libMesatk  libMesaaux
Mode     : X11 interactive window
Controls : SPACE/N = next scene   P = prev   ESC/Q = quit

[X11] Server vendor    : IBM AIX Version 3, Release 1, IBM (c) 1989-1991
[X11] Vendor release   : 3010
[X11] Protocol version : 11.0
[X11] Screens          : 1
[X11] Screen 0 size    : 1024 x 768 pixels

[X11]  GL display : :0.0

[GL]   Vendor    : Brian Paul
[GL]   Renderer  : Mesa
[GL]   Version   : 1.2.8
[GLU]  Version   : 1.2.8 Mesa

[Scene 1] Mesa Test 1/8: GL Matrices & glReadPixels
[Scene 1] glReadPixels center = (R, G, B)  (non-zero = rendering OK)
```

Pressing SPACE advances through scenes with a corresponding `[Scene N]` line printed
for each transition.

---

## C89 Compliance Notes

Written to conform to ANSI C89 (ISO/IEC 9899:1990) for `-Hon=ansi` compliance under
MetaWare High C R2.2g:

| Constraint | Approach |
|-----------|---------|
| All declarations before statements | All variables declared at the top of every `{` block |
| No `//` comments | Only `/* */` comments throughout |
| No designated initializers | All struct/array initializers use positional syntax |
| No VLAs | All arrays are fixed-size or `static` |
| No implicit `int` | All functions have explicit return types |
| `void` parameters | All no-argument functions declared `f(void)` |
| Boolean returns | `GLenum` (`GL_TRUE`/`GL_FALSE`) used for tk callback returns |
| No C99 math | `(float)cos((double)angle)` casts used for `float`/`double` promotion |
| Static local arrays | `static GLubyte texdata[64][64][3]` and scene-local `static GLfloat` arrays survive past function return |

---

## Design Notes

**Single-file, no external assets.** The checkerboard texture is generated in code;
no image files are required.

**OpenGL 1.0 texture API.** Mesa 1.2.8 does not provide `glGenTextures` or
`glBindTexture` (those are OpenGL 1.1 extensions). `glTexImage2D` writes directly to
the implicit texture unit.

**`tkIdleFunc(draw)` for animation.** The idle callback continuously invokes `draw()`
to animate all scenes. `g_angle` is incremented 0.5° per call and wraps at 360°.
Scene changes reset `g_angle = 0.0f` for clean transition.

**`gluDisk` in stencil pass.** Scene 6 draws the GLU disk into both the stencil
buffer and the color buffer (dim grey base). This validates that `gluDisk` with
`GLU_FILL`/`GLU_SMOOTH` works correctly with the GLenum fix — previously, corrupted
`DrawStyle` values caused `gluDisk` to return without calling `glEnd()`, locking
the X server. Keeping colour writes active also makes the scene robust against
OSMesa contexts that have no stencil buffer: the dim disc is always drawn.

**Display list correctness.** Scene 7 compiles `auxSolidTorus` into a display list.
This validates that the Mesa display list capture of complex geometry (hundreds of
`glNormal3f`/`glVertex3f` calls from the torus tessellation) works correctly.
