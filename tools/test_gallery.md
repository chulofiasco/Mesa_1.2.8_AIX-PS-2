# Mesa Test Gallery (`test_gallery.c`)

The `test_gallery` is a unified testing application designed to demonstrate and validate various capabilities of the Mesa 3D graphics library. It consolidates multiple smaller tests into a single, interactive, multi-panel gallery that you can cycle through using the keyboard.

## Compiling and Running

To build the test gallery, ensure the Mesa libraries are built, navigate to the `tools/` directory, and run:
```bash
make test_gallery
```
To run the gallery, simply execute:
```bash
./test_gallery
```

## Global Controls

These controls work across all scenes in the gallery:

*   **`SPACE`**: Cycle to the next scene.
*   **`p`** / **`P`**: Pause or resume global auto-rotation and animations.
*   **`r`** / **`R`**: Instantly jump the animation state (rotations and waves) forward by 15 degrees/steps.

## Scenes & Scene-Specific Controls

The gallery is divided into 5 distinct scenes, each validating a different feature set of the Mesa rendering pipeline.

### Scene 0: Basic Geometry (`aux` primitives)
Displays a perfectly balanced 3x3 grid of `aux` wireframe and solid primitive shapes (Spheres, Cones, Cylinders, Cubes, and a Torus).
*   **`i`** / **`I`**: Invert colors (toggles between a dark theme and a high-contrast white theme).

### Scene 1: GLU Quadrics
Displays a 3x3 grid of GLU quadric objects (Disks, Partial Disks, Spheres, Cylinders, and Cones) drawn using various styles (Fill, Line, Point).
*   **`i`** / **`I`**: Invert colors.

### Scene 2: Texturing, Lighting, and Material State
Demonstrates texture mapping (using a procedurally generated checkerboard), lighting, and material state.
*   **`c`** / **`C`**: Toggle the shape between a Sphere and a Torus.
*   **`w`** / **`W`**: Toggle wireframe mode (polygon fill vs line).
*   **`t`** / **`T`**: Toggle texture mapping on/off.
*   **`l`** / **`L`**: Toggle lighting on/off.
*   **`x`** / **`X`**: Manually bump the tumble on the X-axis.
*   **`y`** / **`Y`**: Manually bump the tumble on the Y-axis.

### Scene 3: Accumulation Buffer Antialiasing & Line Smooth
Validates anti-aliasing techniques by spinning a flat grid of lines and a wireframe sphere.
*   **`a`** / **`A`**: Toggle Accumulation Buffer anti-aliasing.
*   **`s`** / **`S`**: Toggle native `GL_LINE_SMOOTH` anti-aliasing.

### Scene 4: Color Gamut and Vertex Deformation (Wave)
A smooth-shaded quadrant grid displaying deep color interpolation meeting at a bright center, featuring a togglable vertex manipulation effect.
*   **`v`** / **`V`**: Toggle the 3D "Wave" ripple effect. When activated, the color mesh dynamically tilts backward into perspective and perfectly undulating waves ripple across its vertices.

## Architecture

The gallery utilizes a modular `scene_X_display` and `scene_X_init` pattern, managed by a `master_display` loop that isolates state leakage (like active textures and lighting) between the different scenes, ensuring each test renders in a clean state.
