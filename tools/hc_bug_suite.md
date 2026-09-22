# Unified Compiler Diagnostic Suite (`hc_bug_suite.c`)

The `hc_bug_suite` is a consolidated test harness specifically engineered to diagnose, document, and reproduce floating-point (FPU) code generation and optimization bugs found in the **MetaWare High C Compiler (R2.2g)**. It merges several independent legacy tests into a single, automated execution flow to validate compiler behavior across different variable passing and memory management strategies.

## Overview

Historically, the MetaWare compiler exhibited catastrophic failures (such as `SIGSEGV` or severe numerical corruption) when handling tight math loops, deeply chained double-precision assignments, and inline geometry generation algorithms (like those used heavily in OpenGL and Mesa). This suite isolates those failures.

## Compiling and Running

Ensure you are in the `tools/` directory and run:
```bash
make hc_bug_suite
./hc_bug_suite
```
The suite runs automatically with no user interaction required. It traps its own Segfaults using `setjmp`/`longjmp` wrappers to ensure the full test gauntlet executes without terminating the binary.

---

## MetaWare Compiler Bugs (The "Torus Explosion" & "Starburst")

During the porting of Mesa 1.2.8 and the GLU/AUX primitives, severe rendering corruption was encountered (starburst patterns in cylinders/cones, explosive coordinate generation in toruses, and flattened cubes). Two major optimizer bugs were identified related to x87 FPU stack management:

### Bug 1: FPU Stack Leaks in Complex Expressions (The "Torus Explosion")
**The Issue:** When evaluating complex expressions with function calls (`sin()`, `cos()`), the optimizer may fail to emit `fstp` (pop) instructions. The x87 FPU stack only has 8 registers. In tight loops (like Torus/`doughnut` generation), this overflows the stack. Once crashed, every subsequent floating-point operation evaluates to `-QNaN`.
**The Fix:** Forcefully decouple function calls from arithmetic, and use `volatile` local variables to force the compiler to pop the stack to memory immediately.

### Bug 2: Argument Corruption in Repeated Expressions (The "Starburst")
**The Issue:** When calling a function with multiple arguments that have inline arithmetic (e.g., `glVertex3f((float)(x1*r), (float)(y1*r), 0.0f);`), the compiler erroneously reuses the result of the first argument's evaluation for the second argument.
**The Trigger:** In `gluCylinder`, this resulted in the `y` coordinate being passed identically to the `x` coordinate (both evaluating to 0.0), collapsing all vertices to the central axis and creating a "starburst" effect.
**The Fix:** Variables passed to functions must be pre-calculated and stored in distinct local variables before the function call.

---

## Detailed Test Modules & Expected Failures

The suite executes the following embedded sub-modules in sequence. Here is the breakdown of each test, whether it is expected to fail, and exactly *why*.

### 1. `test_math.c` (FPU Math Optimization)
Tests the compiler's handling of `sin()`/`cos()` and tight inner loops during the generation of procedural spherical geometry (like `gluSphere`).
*   **Test 1: Inline Math**: Standard stack-variable math. **[EXPECTED: PASS]** - Survived by avoiding excessive intermediate array writes.
*   **Test 2: Stack Arrays**: Storing trig evaluations into local stack arrays before multiplication. **[EXPECTED: CRASH (SIGSEGV)]** - The High C optimizer misaligns or corrupts the stack pointers when allocating large local double arrays, leading to an immediate segmentation fault during the loop bounds.
*   **Test 3: Malloc Arrays**: Storing trig evaluations in heap-allocated arrays. **[EXPECTED: CRASH (SIGSEGV)]** - Similar to Test 2, the optimizer corrupts pointer arithmetic on the heap variables during tight inner FPU loops.
*   **Test 4: Volatile Loops**: Forcing loop indices `i` and `j` to be `volatile`. **[EXPECTED: PASS]** - This completely neutralizes the compiler's loop unrolling and register-hoisting optimization passes, preventing the FPU starvation that causes the SIGSEGV.
*   **Test 5: Malloc Arrays with Volatile Loops**: **[EXPECTED: PASS]** - Again, forcing `volatile` prevents the optimizer from corrupting the pointer logic.

### 2. `test_args.c` & `test_args_lib.c` (Stack Alignment)
Validates that multiple double-precision arguments pushed to the call stack are aligned correctly when interacting with externally linked standard libraries.
*   **[EXPECTED: PASS (All Checks)]** - MetaWare handles `float` and `double` promotion properly when calling external varargs or linked library functions (like `glTranslatef` or `gluPerspective`).

### 3. `check_math.c` (Basic Arithmetic Sanity)
Runs isolated addition, subtraction, multiplication, and division checksums on hardcoded floating-point values to verify the FPU is functioning on a basic level.
*   **[EXPECTED: PASS]** - Basic FPU operations outside of loops or deep function calls function correctly.

### 4. `test_dodec.c` (Complex Array Initialization)
Tests whether the compiler can correctly load and process large, static, multi-dimensional floating-point arrays (such as the vertex data required to draw a Dodecahedron).
*   **[EXPECTED: PASS]** - Static multi-dimensional array initialization and addressing compiles flawlessly.

### 5. `hc_bug_test.c` (Deep Chaining and Negation)
A direct probe into specific optimizer logic flaws involving FPU register exhaustion and sign-bit flipping. This suite contains 255 checks, and specifically expects **14 failures**.

*   **[Section A8, D1-D4] Alternating Negatives**: Tests `pos, neg, pos, NEG, pos`. **[EXPECTED: FAIL]** - The 4th or 5th alternating parameter in a tight function call consistently drops its negative sign (returning `0.5` instead of `-0.5`). The FPU stack gets out of sync and drops the unary minus instruction during pipelining.
*   **[Section B3] Chained Assignment (y-case)**: Tests `v[0][1] = v[1][1] = -0.5`. **[EXPECTED: FAIL]** - The compiler attempts to hold intermediate chained array assignments in floating-point registers but loses track of the index, writing garbage or dropping signs.
*   **[Section E1-E7] Unary Negation Forms**: Tests `-x` vs `0.0 - x` vs `x * (-1.0)`. **[EXPECTED: FAIL]** - The optimizer actively drops the negative sign during variable assignment when the negation is executed directly on an independent local variable. The FPU command `FCHS` (Change Sign) gets optimized out if the variable isn't immediately passed to a function.
*   **[Section F1] sim_drawbox Pattern**: **[EXPECTED: FAIL]** - Expected to fail on `y1` due to a combination of chained assignments and unary minus bugs.
*   **[Section G3] gluCylinder Flattened Math**: **[EXPECTED: FAIL]** - Expected failure in `glVertex3f y` calculation where precision drops and fails the strict `0.800000` float assertion due to rounding truncation within the FPU when the math is flattened heavily.

### 6. `test_math_cast.c` (Implicit FPU Promotion)
Tests implicit type casting to `double` in standard library `<math.h>` functions (`log`, `pow`).
*   **[EXPECTED: PASS]** - Unless the compiler fails to properly load and convert `unsigned short` to `double` during the evaluation of compound math expressions, which is a known FPU stack vulnerability in High C.

---

## Output Parsing

All diagnostic results are pumped directly to `stdout`/`stderr` without buffering. Segfaults are caught and reported as `CRASHED WITH SIGSEGV!`. If a test completes but returns corrupt variables, it will report the final calculated checksums vs the expected checksums so you can see exactly where the FPU dropped the sign bit or mangled the data.
