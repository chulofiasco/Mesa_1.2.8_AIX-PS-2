# X11 RenderBench Documentation (Version 2)

`x11_renderbench` is a custom real-time Motif/X11 3D rendering harness built to comprehensively test the Mesa 1.2.8 OpenGL implementation on AIX. It provides an interactive UI to stress test geometric shapes, material properties, lighting constraints, procedural texturing algorithms, and complex UI layouts.

## Compilation and Execution

To compile `x11_renderbench` on AIX 1.3:
```sh
cd tools
make x11_renderbench
```
Execute the binary directly from an X11 environment:
```sh
./x11_renderbench
```

## Core Features and Tools

### 1. Geometric Rendering
The bench loads and renders all primary Mesa GLU and Aux shapes:
*   Standard primitives (Sphere, Cone, Cylinder, Torus, Teapot)
*   Complex polyhedra (Icosahedron, Dodecahedron, Octahedron, Tetrahedron)
*   Polygonal rendering modes (Points, Lines, Filled)
*   Shading modes (Flat vs Smooth Gouraud shading)

### 2. Texture Mapping and Procedural Generation
The suite allows interactive loading of external `.rgb` files (e.g., `nebula.rgb`) for texture mapping, along with dynamic procedural textures.
*   **Procedural Generative Textures:** Added on-the-fly texture generation using Hermite-smoothed 3D Value Noise for organic textures like Clouds, Wood Grain, Marble Swirl, Voronoi Cells, Psych Rainbow, Circuit Board, and Camouflage.
*   **Native UVs:** Supports mathematically accurate texture coordinates generation via `emit_tex_coord()`, wrapping textures perfectly around complex geometry.
*   **Linear Projection:** Automatic generation of linear texture coordinates mapped directly to object space.
*   **Sphere Map:** Fully supports OpenGL `GL_SPHERE_MAP` environment mapping, simulating reflective surfaces.

### 3. Advanced Lighting and Material Diagnostics
Provides real-time sliders and material selectors to modulate lighting and test material tracking:
*   **Global Ambient Light (0.0 - 30.0):** Extended scale to heavily over-saturate scene ambiance.
*   **Light Translation:** X, Y, Z translation sliders to orbit and position the light source around the model.
*   **Material Presets:** Fast toggles for Brass, Chrome, Emerald, Ruby, Sapphire, Rubber, and Normal materials.
*   **Dynamic RGB Overrides:** Manual R, G, B sliders to mix custom diffuse material colors on the fly.
*   **Live Preview Dual-Panes:** Implements a mathematically aligned Motif Layout (shrink-wrapped) with two dedicated GL context rendering previews showing the pure light color and pure material color separately.

### 4. Background Context Environments
Features a sleek Motif button row allowing developers to hot-swap the OpenGL clear color (`glClearColor`) to test material contrast profiles:
*   **Black** (`0.0`)
*   **50% Grey** (`0.5`)
*   **75% Grey** (`0.75`)
*   **White** (`1.0`)

### 5. Architectural Strictness (RGB Only)
`x11_renderbench` strictly enforces an RGB rendering pipeline (`GL_TRUE`). The legacy `-ci` (Color Index) mode has been fully removed from the source to prevent catastrophic visual failures, ensuring that the application always provisions a TrueColor 24-bit X11 Visual or an emulated PseudoColor fallback without compromising color fidelity.

### Reset Application Defaults
The **Reset Defaults** button acts as a global panic switch, safely restoring all toggles, rotations, active textures, and the background clear color to their initial factory state, immediately flushing the state reset to the main rendering context.
