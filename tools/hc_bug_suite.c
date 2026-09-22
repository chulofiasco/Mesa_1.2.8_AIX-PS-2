/* Unified Compiler Diagnostic Suite */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>

jmp_buf global_env;
void global_segv_handler(int sig) { 
    signal(SIGSEGV, global_segv_handler);
    longjmp(global_env, 1); 
}

/* ================= test_math.c ================= */



void test_inline_math(int slices, int stacks) {
    double drho = 3.14159265358979323846 / (double)stacks;
    double dtheta = 2.0 * 3.14159265358979323846 / (double)slices;
    double sum_x = 0, sum_y = 0, sum_z = 0;
    int i, j, count = 0;
    
    printf("--- Test 1: Inline Math (Original gluSphere) ---\n");
    if (setjmp(global_env) == 0) {
        for (i = 0; i < stacks; i++) {
            double rho = i * drho;
            for (j = 0; j <= slices; j++) {
                double theta = j * dtheta;
                double x = -sin(theta) * sin(rho);
                double y = cos(theta) * sin(rho);
                double z = cos(rho);
                sum_x += x; sum_y += y; sum_z += z;
                count++;
            }
        }
        printf("Status: SUCCESS\n");
        printf("Points calculated: %d\n", count);
        printf("Checksums: X=%f, Y=%f, Z=%f\n\n", sum_x, sum_y, sum_z);
    } else {
        printf("Status: CRASHED WITH SIGSEGV!\n\n");
    }
}

void test_stack_arrays(int slices, int stacks) {
    double drho = 3.14159265358979323846 / (double)stacks;
    double dtheta = 2.0 * 3.14159265358979323846 / (double)slices;
    double sum_x = 0, sum_y = 0, sum_z = 0;
    int i, j, count = 0;
    double s_th[64], c_th[64], s_rho[64], c_rho[64];
    
    printf("--- Test 2: Stack Arrays ---\n");
    if (setjmp(global_env) == 0) {
        for (j = 0; j <= slices; j++) {
            s_th[j] = sin(j * dtheta);
            c_th[j] = cos(j * dtheta);
        }
        for (i = 0; i <= stacks; i++) {
            s_rho[i] = sin(i * drho);
            c_rho[i] = cos(i * drho);
        }

        for (i = 0; i < stacks; i++) {
            for (j = 0; j <= slices; j++) {
                double x = -s_th[j] * s_rho[i];
                double y = c_th[j] * s_rho[i];
                double z = c_rho[i];
                sum_x += x; sum_y += y; sum_z += z;
                count++;
            }
        }
        printf("Status: SUCCESS (Unexpected!)\n");
        printf("Points calculated: %d\n", count);
        printf("Checksums: X=%f, Y=%f, Z=%f\n\n", sum_x, sum_y, sum_z);
    } else {
        printf("Status: CRASHED WITH SIGSEGV!\n\n");
    }
}

void test_malloc_arrays(int slices, int stacks) {
    double drho = 3.14159265358979323846 / (double)stacks;
    double dtheta = 2.0 * 3.14159265358979323846 / (double)slices;
    double sum_x = 0, sum_y = 0, sum_z = 0;
    int i, j, count = 0;
    double *s_th = (double*)malloc((slices + 1) * sizeof(double));
    double *c_th = (double*)malloc((slices + 1) * sizeof(double));
    double *s_rho = (double*)malloc((stacks + 1) * sizeof(double));
    double *c_rho = (double*)malloc((stacks + 1) * sizeof(double));
    
    printf("--- Test 3: Malloc Arrays ---\n");
    if (setjmp(global_env) == 0) {
        for (j = 0; j <= slices; j++) {
            s_th[j] = sin(j * dtheta);
            c_th[j] = cos(j * dtheta);
        }
        for (i = 0; i <= stacks; i++) {
            s_rho[i] = sin(i * drho);
            c_rho[i] = cos(i * drho);
        }

        for (i = 0; i < stacks; i++) {
            for (j = 0; j <= slices; j++) {
                double x = -s_th[j] * s_rho[i];
                double y = c_th[j] * s_rho[i];
                double z = c_rho[i];
                sum_x += x; sum_y += y; sum_z += z;
                count++;
            }
        }
        printf("Status: SUCCESS\n");
        printf("Points calculated: %d\n", count);
        printf("Checksums: X=%f, Y=%f, Z=%f\n\n", sum_x, sum_y, sum_z);
    } else {
        printf("Status: CRASHED WITH SIGSEGV!\n\n");
    }
    free(s_th); free(c_th); free(s_rho); free(c_rho);
}

void test_volatile_loop_counters(int slices, int stacks) {
    double drho = 3.14159265358979323846 / (double)stacks;
    double dtheta = 2.0 * 3.14159265358979323846 / (double)slices;
    double sum_x = 0, sum_y = 0, sum_z = 0;
    volatile int i, j;
    int count = 0;
    
    printf("--- Test 4: Inline Math with VOLATILE loop counters ---\n");
    if (setjmp(global_env) == 0) {
        for (i = 0; i < stacks; i++) {
            double rho = i * drho;
            for (j = 0; j <= slices; j++) {
                double theta = j * dtheta;
                double x = -sin(theta) * sin(rho);
                double y = cos(theta) * sin(rho);
                double z = cos(rho);
                sum_x += x; sum_y += y; sum_z += z;
                count++;
            }
        }
        printf("Status: SUCCESS\n");
        printf("Points calculated: %d\n", count);
        printf("Checksums: X=%f, Y=%f, Z=%f\n\n", sum_x, sum_y, sum_z);
    } else {
        printf("Status: CRASHED WITH SIGSEGV!\n\n");
    }
}

void test_malloc_volatile_loops(int slices, int stacks) {
    double drho = 3.14159265358979323846 / (double)stacks;
    double dtheta = 2.0 * 3.14159265358979323846 / (double)slices;
    double sum_x = 0, sum_y = 0, sum_z = 0;
    volatile int i, j;
    int count = 0;
    double *s_th = (double*)malloc((slices + 1) * sizeof(double));
    double *c_th = (double*)malloc((slices + 1) * sizeof(double));
    double *s_rho = (double*)malloc((stacks + 1) * sizeof(double));
    double *c_rho = (double*)malloc((stacks + 1) * sizeof(double));
    
    printf("--- Test 5: Malloc Arrays with VOLATILE loop counters ---\n");
    if (setjmp(global_env) == 0) {
        for (j = 0; j <= slices; j++) {
            s_th[j] = sin(j * dtheta);
            c_th[j] = cos(j * dtheta);
        }
        for (i = 0; i <= stacks; i++) {
            s_rho[i] = sin(i * drho);
            c_rho[i] = cos(i * drho);
        }

        for (i = 0; i < stacks; i++) {
            for (j = 0; j <= slices; j++) {
                double x = -s_th[j] * s_rho[i];
                double y = c_th[j] * s_rho[i];
                double z = c_rho[i];
                sum_x += x; sum_y += y; sum_z += z;
                count++;
            }
        }
        printf("Status: SUCCESS\n");
        printf("Points calculated: %d\n", count);
        printf("Checksums: X=%f, Y=%f, Z=%f\n\n", sum_x, sum_y, sum_z);
    } else {
        printf("Status: CRASHED WITH SIGSEGV!\n\n");
    }
    free(s_th); free(c_th); free(s_rho); free(c_rho);
}

int test_math_main(int argc, char** argv) {
    signal(SIGSEGV, global_segv_handler);
    
    printf("Beginning High C Mathematical Optimizer Tests...\n\n");
    
    test_inline_math(32, 32);
    test_stack_arrays(32, 32);
    test_malloc_arrays(32, 32);
    test_volatile_loop_counters(32, 32);
    test_malloc_volatile_loops(32, 32);
    
    printf("Tests completed.\n");
    return 0;
}

/* ================= test_args.c ================= */

/* Local definitions */
void test_3_floats_local(float x, float y, float z) {
    printf("  Local  3 floats:   Expected (1.1, 2.2, 3.3) -> Received (%f, %f, %f)\n", x, y, z);
}
void test_4_floats_local(float x, float y, float z, float w) {
    printf("  Local  4 floats:   Expected (1.1, 2.2, 3.3, 4.4) -> Received (%f, %f, %f, %f)\n", x, y, z, w);
}
void test_3_doubles_local(double x, double y, double z) {
    printf("  Local  3 doubles:  Expected (1.1, 2.2, 3.3) -> Received (%f, %f, %f)\n", x, y, z);
}
void test_4_doubles_local(double x, double y, double z, double w) {
    printf("  Local  4 doubles:  Expected (1.1, 2.2, 3.3, 4.4) -> Received (%f, %f, %f, %f)\n", x, y, z, w);
}
void test_6_doubles_local(double a, double b, double c, double d, double e, double f) {
    printf("  Local  6 doubles:  Expected (1.1, 2.2, 3.3, 4.4, 1.5, 20.0) -> Received (%f, %f, %f, %f, %f, %f)\n", a, b, c, d, e, f);
}
void test_volatile_float_expr_local(double a, double b, double c, double d) {
    printf("  Local  Volat expr: Expected (60.0, 1.0, 1.0, 20.0) -> Received (%f, %f, %f, %f)\n", a, b, c, d);
}

/* External definitions from test_args_lib.c */
extern void test_3_floats(float x, float y, float z);
extern void test_4_floats(float x, float y, float z, float w);
extern void test_3_doubles(double x, double y, double z);
extern void test_4_doubles(double x, double y, double z, double w);
extern void test_6_doubles(double a, double b, double c, double d, double e, double f);
extern void test_volatile_float_expr(double a, double b, double c, double d);

int test_args_main(void) {
    int w = 300, h = 300;
    volatile float vx = 1.1f, vy = 2.2f, vz = 3.3f;

    printf("\n--- MetaWare High C Argument Passing Diagnostics ---\n");

    printf("\n1. Testing glTranslatef signature (3 floats)\n");
    test_3_floats_local(vx, vy, vz);
    test_3_floats(vx, vy, vz);

    printf("\n2. Testing gl_save_and_execute_vertex signature (4 floats)\n");
    test_4_floats_local(1.1f, 2.2f, 3.3f, 4.4f);
    test_4_floats(1.1f, 2.2f, 3.3f, 4.4f);

    printf("\n3. Testing glVertex3d / glTranslated signature (3 doubles)\n");
    test_3_doubles_local(1.1, 2.2, 3.3);
    test_3_doubles(1.1, 2.2, 3.3);

    printf("\n4. Testing gluPerspective signature (4 doubles)\n");
    test_4_doubles_local(1.1, 2.2, 3.3, 4.4);
    test_4_doubles(1.1, 2.2, 3.3, 4.4);
    
    printf("\n5. Testing gluPerspective with float casting expression\n");
    test_volatile_float_expr_local(60.0, (float)w / (float)h, 1.0, 20.0);
    test_volatile_float_expr(60.0, (float)w / (float)h, 1.0, 20.0);

    printf("\n6. Testing glFrustum signature (6 doubles)\n");
    test_6_doubles_local(1.1, 2.2, 3.3, 4.4, 1.5, 20.0);
    test_6_doubles(1.1, 2.2, 3.3, 4.4, 1.5, 20.0);

    printf("\n--------------------------------------------------\n\n");
    return 0;
}

/* ================= test_args_lib.c ================= */

void test_3_floats(float x, float y, float z) {
    printf("  Extern 3 floats:   Expected (1.1, 2.2, 3.3) -> Received (%f, %f, %f)\n", x, y, z);
}

void test_4_floats(float x, float y, float z, float w) {
    printf("  Extern 4 floats:   Expected (1.1, 2.2, 3.3, 4.4) -> Received (%f, %f, %f, %f)\n", x, y, z, w);
}

void test_3_doubles(double x, double y, double z) {
    printf("  Extern 3 doubles:  Expected (1.1, 2.2, 3.3) -> Received (%f, %f, %f)\n", x, y, z);
}

void test_4_doubles(double x, double y, double z, double w) {
    printf("  Extern 4 doubles:  Expected (1.1, 2.2, 3.3, 4.4) -> Received (%f, %f, %f, %f)\n", x, y, z, w);
}

void test_6_doubles(double a, double b, double c, double d, double e, double f) {
    printf("  Extern 6 doubles:  Expected (1.1, 2.2, 3.3, 4.4, 1.5, 20.0) -> Received (%f, %f, %f, %f, %f, %f)\n", a, b, c, d, e, f);
}

void test_volatile_float_expr(double a, double b, double c, double d) {
    printf("  Extern Volat expr: Expected (60.0, 1.0, 1.0, 20.0) -> Received (%f, %f, %f, %f)\n", a, b, c, d);
}

/* ================= check_math.c ================= */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int check_math_main() {
    int x = 1;
    char *p = (char *)&x;
    printf("Endianness: %s\n", (p[0] == 1) ? "Little Endian" : "Big Endian");
    printf("sizeof(float): %d\n", sizeof(float));
    printf("sizeof(double): %d\n", sizeof(double));
    printf("sizeof(int): %d\n", sizeof(int));
    
    printf("sin(M_PI / 2): %f\n", sin(M_PI / 2.0));
    printf("cos(M_PI): %f\n", cos(M_PI));
    printf("tan(M_PI / 4): %f\n", tan(M_PI / 4.0));
    return 0;
}

/* ================= test_dodec.c ================= */

typedef double GLdouble;

void diff3(GLdouble *a, GLdouble *b, GLdouble *c)
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}

void crossprod(GLdouble v1[3], GLdouble v2[3], GLdouble prod[3])
{
    GLdouble p[3];
    p[0] = v1[1]*v2[2] - v2[1]*v1[2];
    p[1] = v1[2]*v2[0] - v2[2]*v1[0];
    p[2] = v1[0]*v2[1] - v2[0]*v1[1];
    prod[0] = p[0]; prod[1] = p[1]; prod[2] = p[2];
}

void normalize(GLdouble v[3])
{
    GLdouble d;
    d = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    printf("Debug normalize: d = %f, v = [%f, %f, %f]\n", d, v[0], v[1], v[2]);
    if (d == 0.0) {
        printf("ERROR: normalize: zero length vector\n");
	    v[0] = d = 1.0;
    }
    d = 1/d;
    v[0] *= d; v[1] *= d; v[2] *= d;
}

GLdouble dodec[20][3];

void initdodec(void)
{
    volatile GLdouble alpha, beta;
    volatile GLdouble tmp1, tmp2;

    tmp1 = sqrt(5.0);
    tmp2 = 2.0 / (3.0 + tmp1);
    alpha = sqrt(tmp2);
    
    tmp1 = sqrt(5.0);
    tmp2 = 6.0 / (3.0 + tmp1);
    beta = 1.0 + sqrt(tmp2) - 2.0 + 2.0 * alpha;
    
    printf("Debug initdodec: alpha=%f, beta=%f\n", alpha, beta);
    dodec[0][0] = -alpha; dodec[0][1] = 0; dodec[0][2] = beta;
    dodec[1][0] = alpha; dodec[1][1] = 0; dodec[1][2] = beta;
    dodec[2][0] = -1; dodec[2][1] = -1; dodec[2][2] = -1;
    dodec[3][0] = -1; dodec[3][1] = -1; dodec[3][2] = 1;
    dodec[4][0] = -1; dodec[4][1] = 1; dodec[4][2] = -1;
    dodec[5][0] = -1; dodec[5][1] = 1; dodec[5][2] = 1;
    dodec[6][0] = 1; dodec[6][1] = -1; dodec[6][2] = -1;
    dodec[7][0] = 1; dodec[7][1] = -1; dodec[7][2] = 1;
    dodec[8][0] = 1; dodec[8][1] = 1; dodec[8][2] = -1;
    dodec[9][0] = 1; dodec[9][1] = 1; dodec[9][2] = 1;
    dodec[10][0] = beta; dodec[10][1] = alpha; dodec[10][2] = 0;
    dodec[11][0] = beta; dodec[11][1] = -alpha; dodec[11][2] = 0;
    dodec[12][0] = -beta; dodec[12][1] = alpha; dodec[12][2] = 0;
    dodec[13][0] = -beta; dodec[13][1] = -alpha; dodec[13][2] = 0;
    dodec[14][0] = -alpha; dodec[14][1] = 0; dodec[14][2] = -beta;
    dodec[15][0] = alpha; dodec[15][1] = 0; dodec[15][2] = -beta;
    dodec[16][0] = 0; dodec[16][1] = beta; dodec[16][2] = alpha;
    dodec[17][0] = 0; dodec[17][1] = beta; dodec[17][2] = -alpha;
    dodec[18][0] = 0; dodec[18][1] = -beta; dodec[18][2] = alpha;
    dodec[19][0] = 0; dodec[19][1] = -beta; dodec[19][2] = -alpha;
}

void test_pentagon(int a, int b, int c, int d, int e)
{
    GLdouble n0[3], d1[3], d2[3];

    diff3(&dodec[a][0], &dodec[b][0], d1);
    diff3(&dodec[b][0], &dodec[c][0], d2);
    printf("d1: %f %f %f\n", d1[0], d1[1], d1[2]);
    printf("d2: %f %f %f\n", d2[0], d2[1], d2[2]);
    crossprod(d1, d2, n0);
    printf("n0: %f %f %f\n", n0[0], n0[1], n0[2]);
    normalize(n0);
}

int test_dodec_main()
{
    initdodec();
    /* The first pentagon from dodecahedron() */
    test_pentagon(0, 1, 9, 16, 5);
    return 0;
}

/* ================= hc_bug_test.c ================= */
/*
 * hc_bug_test.c
 *
 * Diagnostic for two suspected MetaWare HC R2.2g code generation bugs
 * that corrupt 3-D cube geometry in Mesa 1.2.8 on AIX 1.3 PS/2.
 *
 * Bug 1 (Section A): caller-side mis-passing of the 4th GLdouble argument
 *   when a function is called with 6 alternating double arguments drawn
 *   from the same two named local variables.
 *
 * Bug 2 (Section B): the RHS of a chained double array assignment to a
 *   given column index silently reuses the RHS of the previous chained
 *   assignment statement that targeted the same column index.
 *
 * Compile without optimisation (matches build_all.sh for src-aux):
 *   cc -Hon=ansi hc_bug_test.c -o hc_bug_test
 *
 * Compile with optimisation (to see whether -O changes behaviour):
 *   cc -Hon=ansi -O hc_bug_test.c -o hc_bug_test_opt
 *
 * Run:
 *   ./hc_bug_test 2>&1 | tee hc_bug_test.out
 */

#include <stdlib.h>   /* malloc, free */
#include <math.h>     /* sin, cos */

/* Use the same typedef as Mesa so the compiler sees exactly the same type. */
typedef double GLdouble;
typedef unsigned int GLuint;

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                          */
/* ------------------------------------------------------------------ */

static int total_checks = 0;
static int failed_checks = 0;

static void check(const char *label, GLdouble got, GLdouble expected)
{
    total_checks++;
    if (got != expected) {
        failed_checks++;
        fprintf(stderr, "  FAIL  %-20s got=%10.6f  expected=%10.6f\n",
                label, got, expected);
    } else {
        fprintf(stderr, "  ok    %-20s %.6f\n", label, got);
    }
}

static void section(const char *tag, const char *desc)
{
    fprintf(stderr, "\n[%s] %s\n", tag, desc);
}

/* ------------------------------------------------------------------ */
/* Section A: Bug 1 -- caller-side GLdouble argument passing           */
/*                                                                      */
/* Each recv_N function just prints its received arguments.  The       */
/* interesting question is whether arg4 arrives as +0.5 or -0.5.       */
/* ------------------------------------------------------------------ */

static void recv2(GLdouble a1, GLdouble a2)
{
    check("a1(exp -0.5)", a1, -0.5);
    check("a2(exp +0.5)", a2,  0.5);
}

static void recv4(GLdouble a1, GLdouble a2, GLdouble a3, GLdouble a4)
{
    check("a1(exp -0.5)", a1, -0.5);
    check("a2(exp +0.5)", a2,  0.5);
    check("a3(exp -0.5)", a3, -0.5);
    check("a4(exp +0.5)", a4,  0.5);   /* Bug 1 candidate */
}

static void recv5(GLdouble a1, GLdouble a2, GLdouble a3, GLdouble a4, GLdouble a5)
{
    check("a1(exp -0.5)", a1, -0.5);
    check("a2(exp +0.5)", a2,  0.5);
    check("a3(exp -0.5)", a3, -0.5);
    check("a4(exp +0.5)", a4,  0.5);   /* Bug 1 candidate */
    check("a5(exp -0.5)", a5, -0.5);
}

static void recv6(GLdouble a1, GLdouble a2, GLdouble a3,
                  GLdouble a4, GLdouble a5, GLdouble a6)
{
    check("a1(exp -0.5)", a1, -0.5);
    check("a2(exp +0.5)", a2,  0.5);
    check("a3(exp -0.5)", a3, -0.5);
    check("a4(exp +0.5)", a4,  0.5);   /* Bug 1 — observed to receive -0.5 */
    check("a5(exp -0.5)", a5, -0.5);
    check("a6(exp +0.5)", a6,  0.5);
}

/* Same receiver but with the expected sign pattern reversed.
 * Used to test if arg4 always gets arg3's value regardless of which
 * variable is which. */
static void recv6_rev(GLdouble a1, GLdouble a2, GLdouble a3,
                      GLdouble a4, GLdouble a5, GLdouble a6)
{
    check("a1(exp +0.5)", a1,  0.5);
    check("a2(exp -0.5)", a2, -0.5);
    check("a3(exp +0.5)", a3,  0.5);
    check("a4(exp -0.5)", a4, -0.5);   /* reversed: bug would make this +0.5 */
    check("a5(exp +0.5)", a5,  0.5);
    check("a6(exp -0.5)", a6, -0.5);
}

/* Receivers with a trailing unsigned int -- replicating the exact
 * drawbox(x0,x1,y0,y1,z0,z1, GLenum) call signature.
 * The hypothesis is that the trailing int is the actual bug trigger:
 * A4 passes (6 doubles, no int), but the real drawbox call fails.
 */
static void recv4i(GLdouble a1, GLdouble a2, GLdouble a3, GLdouble a4,
                   unsigned int type)
{
    check("a1(exp -0.5)", a1, -0.5);
    check("a2(exp +0.5)", a2,  0.5);
    check("a3(exp -0.5)", a3, -0.5);
    check("a4(exp +0.5)", a4,  0.5);   /* Bug 1 candidate */
    type = type;   /* suppress unused-param warning without (void) idiom */
}

static void recv6i(GLdouble a1, GLdouble a2, GLdouble a3,
                   GLdouble a4, GLdouble a5, GLdouble a6, unsigned int type)
{
    check("a1(exp -0.5)", a1, -0.5);
    check("a2(exp +0.5)", a2,  0.5);
    check("a3(exp -0.5)", a3, -0.5);
    check("a4(exp +0.5)", a4,  0.5);   /* Bug 1 -- does trailing int trigger this? */
    check("a5(exp -0.5)", a5, -0.5);
    check("a6(exp +0.5)", a6,  0.5);
    type = type;
}

static void recv6i_rev(GLdouble a1, GLdouble a2, GLdouble a3,
                       GLdouble a4, GLdouble a5, GLdouble a6, unsigned int type)
{
    check("a1(exp +0.5)", a1,  0.5);
    check("a2(exp -0.5)", a2, -0.5);
    check("a3(exp +0.5)", a3,  0.5);
    check("a4(exp -0.5)", a4, -0.5);   /* reversed: bug would give +0.5 here */
    check("a5(exp +0.5)", a5,  0.5);
    check("a6(exp -0.5)", a6, -0.5);
    type = type;
}

static void test_arg_passing(void)
{
    GLdouble neg = -0.5;
    GLdouble pos =  0.5;

    /* A1: 2 alternating args -- baseline, must always pass */
    section("A1", "2 alternating double args from 2 named vars (baseline)");
    recv2(neg, pos);

    /* A2: 4 alternating args -- does the bug first appear here? */
    section("A2", "4 alternating double args from 2 named vars");
    recv4(neg, pos, neg, pos);

    /* A3: 5 alternating args */
    section("A3", "5 alternating double args from 2 named vars");
    recv5(neg, pos, neg, pos, neg);

    /* A4: 6 alternating args from 2 named vars -- THE BUG TRIGGER */
    section("A4", "6 alternating double args from 2 named vars (KNOWN BUG TRIGGER)");
    recv6(neg, pos, neg, pos, neg, pos);

    /* A5: 6 args, 6 DISTINCT named vars -- should always pass */
    section("A5", "6 double args from 6 distinct named vars (control)");
    {
        GLdouble a = -0.5, b = 0.5, c = -0.5, d = 0.5, e = -0.5, f = 0.5;
        recv6(a, b, c, d, e, f);
    }

    /* A6: 6 literal constant args -- does the compiler treat them differently? */
    section("A6", "6 double literal constant args");
    recv6(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

    /* A7: 6 alternating args, reversed sign order (pos, neg, ...).
     * If the bug is "arg4 always receives arg3's value", arg4 should
     * receive +0.5 (pos) instead of the expected -0.5 (neg). */
    section("A7", "6 alternating double args, reversed sign (pos first)");
    recv6_rev(pos, neg, pos, neg, pos, neg);

    /* A8: The call below intentionally passes neg as arg4, which does NOT
     * match recv6's hardcoded expectation of +0.5 for a4.  The single FAIL
     * printed IS EXPECTED and only confirms the compiler pushes what was
     * specified.  It is NOT a compiler bug -- ignore this failure. */
    section("A8", "6 args: neg,pos,neg,NEG,neg,pos -- a4=neg by design (1 FAIL expected)");
    recv6(neg, pos, neg, neg, neg, pos);

    /* ---- Critical narrowing tests: 6 doubles + trailing int ------------ */
    /* The real drawbox signature is: drawbox(d,d,d,d,d,d, GLenum)         */
    /* A4 above PASSES with 6-double-only receivers.  The hypothesis is     */
    /* that the trailing unsigned int (GLenum) is the actual bug trigger.   */

    section("A9", "4 alternating doubles + trailing int");
    recv4i(neg, pos, neg, pos, 2u);

    section("A10", "6 alternating doubles + trailing int (exact drawbox pattern)");
    recv6i(neg, pos, neg, pos, neg, pos, 2u);

    section("A11", "6 alternating doubles + trailing int, reversed sign");
    recv6i_rev(pos, neg, pos, neg, pos, neg, 2u);
}

/* ------------------------------------------------------------------ */
/* Section B: Bug 2 -- chained GLdouble 2-D array assignment           */
/*                                                                      */
/* The drawbox vertex array has 8 rows, 3 columns (indices 0,1,2).    */
/* The original Mesa code uses 6 chained-assignment lines to fill it. */
/* Tests isolate which patterns trigger the wrong-value bug.           */
/* ------------------------------------------------------------------ */

/* Sentinel value: anything other than -0.5 or +0.5 so we can detect
 * a cell that was never written. */
#define SENTINEL 9.9

static void init_v(GLdouble v[8][3])
{
    int i, j;
    for (i = 0; i < 8; i++)
        for (j = 0; j < 3; j++)
            v[i][j] = SENTINEL;
}

/* B helper: print and check a row of cells for subscript k */
static void check_col(GLdouble v[8][3], int k,
                       int i0, int i1, int i2, int i3, /* expect val0 */
                       int i4, int i5, int i6, int i7, /* expect val1 */
                       GLdouble val0, GLdouble val1)
{
    char label[32];
#define CHK(idx, expected) \
    sprintf(label, "v[%d][%d](exp%.1f)", idx, k, (expected)); \
    check(label, v[idx][k], (expected));

    CHK(i0, val0); CHK(i1, val0); CHK(i2, val0); CHK(i3, val0);
    CHK(i4, val1); CHK(i5, val1); CHK(i6, val1); CHK(i7, val1);
#undef CHK
}

static void test_chained_assign(void)
{
    GLdouble v[8][3];
    GLdouble y0 = -0.5, y1 = 0.5;
    GLdouble z0 = -0.5, z1 = 0.5;

    /* -------------------------------------------------------------- */
    /* B1: subscript [0] written twice -- x-case.  Known to work.     */
    /* -------------------------------------------------------------- */
    section("B1", "col [0] written twice consecutively (x-case, baseline)");
    init_v(v);
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = y0;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = y1;
    check_col(v, 0,  0,1,2,3,  4,5,6,7,  y0, y1);

    /* -------------------------------------------------------------- */
    /* B2: subscript [1] written ONCE -- no preceding [1] statement.  */
    /* Does the bug require a prior statement to the same column?      */
    /* -------------------------------------------------------------- */
    section("B2", "col [1] written once, NO prior col [1] statement");
    init_v(v);
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;
    {
        char label[32];
        sprintf(label, "v[2][1](exp%.1f)", y1); check(label, v[2][1], y1);
        sprintf(label, "v[3][1](exp%.1f)", y1); check(label, v[3][1], y1);
        sprintf(label, "v[6][1](exp%.1f)", y1); check(label, v[6][1], y1);
        sprintf(label, "v[7][1](exp%.1f)", y1); check(label, v[7][1], y1);
    }

    /* -------------------------------------------------------------- */
    /* B3: subscript [1] written TWICE consecutively -- y-case.       */
    /* Second statement is expected to assign y1 but receives y0.     */
    /* -------------------------------------------------------------- */
    section("B3", "col [1] written twice consecutively (y-case, KNOWN BUG)");
    init_v(v);
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = y0;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;   /* Bug: gets y0? */
    check_col(v, 1,  0,1,4,5,  2,3,6,7,  y0, y1);

    /* -------------------------------------------------------------- */
    /* B4: subscript [2] written ONCE -- no preceding [2] statement.  */
    /* -------------------------------------------------------------- */
    section("B4", "col [2] written once, NO prior col [2] statement");
    init_v(v);
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = z1;
    {
        char label[32];
        sprintf(label, "v[1][2](exp%.1f)", z1); check(label, v[1][2], z1);
        sprintf(label, "v[2][2](exp%.1f)", z1); check(label, v[2][2], z1);
        sprintf(label, "v[5][2](exp%.1f)", z1); check(label, v[5][2], z1);
        sprintf(label, "v[6][2](exp%.1f)", z1); check(label, v[6][2], z1);
    }

    /* -------------------------------------------------------------- */
    /* B5: subscript [2] written TWICE consecutively -- z-case.       */
    /* -------------------------------------------------------------- */
    section("B5", "col [2] written twice consecutively (z-case, BUG?)");
    init_v(v);
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = z0;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = z1;   /* Bug: gets z0? */
    check_col(v, 2,  0,3,4,7,  1,2,5,6,  z0, z1);

    /* -------------------------------------------------------------- */
    /* B6: subscript [1] written twice, but with an INTERVENING stmt  */
    /* for a different column -- does the intervening stmt break the   */
    /* bug's trigger?                                                  */
    /* -------------------------------------------------------------- */
    section("B6", "col [1] twice with intervening col [2] stmt");
    init_v(v);
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = y0;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = z0;   /* intervening different col */
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;   /* still buggy? */
    {
        char label[32];
        sprintf(label, "v[2][1](exp%.1f)", y1); check(label, v[2][1], y1);
        sprintf(label, "v[7][1](exp%.1f)", y1); check(label, v[7][1], y1);
    }

    /* -------------------------------------------------------------- */
    /* B7: all 6 drawbox chained-assign lines together.               */
    /* This is the exact pattern from shapes.c drawbox().             */
    /* -------------------------------------------------------------- */
    section("B7", "all 6 drawbox chained-assign lines (exact shapes.c pattern)");
    init_v(v);
    {
        GLdouble x0 = -0.5, x1 = 0.5;
        v[0][0] = v[1][0] = v[2][0] = v[3][0] = x0;
        v[4][0] = v[5][0] = v[6][0] = v[7][0] = x1;
        v[0][1] = v[1][1] = v[4][1] = v[5][1] = y0;
        v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;   /* Bug? */
        v[0][2] = v[3][2] = v[4][2] = v[7][2] = z0;
        v[1][2] = v[2][2] = v[5][2] = v[6][2] = z1;   /* Bug? */

        fprintf(stderr, "  Vertex table (row: x y z):\n");
        {
            int i;
            for (i = 0; i < 8; i++)
                fprintf(stderr, "    v[%d] = %7.4f  %7.4f  %7.4f\n",
                        i, v[i][0], v[i][1], v[i][2]);
        }

        /* Check col [0]: x0 for rows 0-3, x1 for rows 4-7 */
        check_col(v, 0,  0,1,2,3,  4,5,6,7,  x0, x1);
        /* Check col [1]: y0 for {0,1,4,5}, y1 for {2,3,6,7} */
        check_col(v, 1,  0,1,4,5,  2,3,6,7,  y0, y1);
        /* Check col [2]: z0 for {0,3,4,7}, z1 for {1,2,5,6} */
        check_col(v, 2,  0,3,4,7,  1,2,5,6,  z0, z1);
    }
}

/* ------------------------------------------------------------------ */
/* Section C: individual assignments -- control, must always pass      */
/* ------------------------------------------------------------------ */

static void test_individual_assign(void)
{
    GLdouble v[8][3];
    GLdouble x0 = -0.5, x1 = 0.5;
    GLdouble y0 = -0.5, y1 = 0.5;
    GLdouble z0 = -0.5, z1 = 0.5;

    section("C1", "individual (non-chained) assignments -- same pattern as B7");
    init_v(v);

    v[0][0] = x0; v[1][0] = x0; v[2][0] = x0; v[3][0] = x0;
    v[4][0] = x1; v[5][0] = x1; v[6][0] = x1; v[7][0] = x1;

    v[0][1] = y0; v[1][1] = y0; v[4][1] = y0; v[5][1] = y0;
    v[2][1] = y1; v[3][1] = y1; v[6][1] = y1; v[7][1] = y1;

    v[0][2] = z0; v[3][2] = z0; v[4][2] = z0; v[7][2] = z0;
    v[1][2] = z1; v[2][2] = z1; v[5][2] = z1; v[6][2] = z1;

    fprintf(stderr, "  Vertex table (row: x y z):\n");
    {
        int i;
        for (i = 0; i < 8; i++)
            fprintf(stderr, "    v[%d] = %7.4f  %7.4f  %7.4f\n",
                    i, v[i][0], v[i][1], v[i][2]);
    }

    check_col(v, 0,  0,1,2,3,  4,5,6,7,  x0, x1);
    check_col(v, 1,  0,1,4,5,  2,3,6,7,  y0, y1);
    check_col(v, 2,  0,3,4,7,  1,2,5,6,  z0, z1);
}

/* ------------------------------------------------------------------ */
/* Section D: simulate auxWireCube calling context                     */
/*                                                                      */
/* All prior tests use literal-initialized neg/pos values in simple    */
/* functions.  In shapes.c, s2 and ms2 are computed from a GLdouble    */
/* parameter after several prior function calls (malloc, findList,     */
/* glNewList(makeModelPtr(...))).  These tests add each ingredient      */
/* incrementally to find the actual trigger.                            */
/* ------------------------------------------------------------------ */

/* D1/D2: just compute from parameter, no prior calls */
static void test_param_derived(GLdouble size)
{
    GLdouble pos = size / 2.0;
    GLdouble neg = -pos;

    section("D1", "6 alt doubles computed from GLdouble PARAMETER (no prior calls)");
    recv6(neg, pos, neg, pos, neg, pos);

    section("D2", "6 alt doubles from parameter + trailing int (no prior calls)");
    recv6i(neg, pos, neg, pos, neg, pos, 2u);
}

/* Simulated findList: compares a GLdouble array -- uses FPU internally */
static GLdouble sim_stored = -999.0;
static GLuint   sim_listid = 0;

static GLuint sim_findList(int type, GLdouble *params, int n)
{
    /* Walk a small array of stored sizes to find a match.
     * Uses GLdouble comparison -- exercises the FPU like the real findList. */
    GLdouble stored[4];
    int i;
    type = type; n = n;
    stored[0] = sim_stored;
    stored[1] = -888.0;
    stored[2] = -777.0;
    stored[3] = -666.0;
    for (i = 0; i < 4; i++) {
        if (stored[i] == params[0])
            return sim_listid;
    }
    return 0;
}

static GLuint sim_makeModelPtr(int type, GLdouble *params, int n)
{
    type = type; n = n;
    sim_stored = params[0];
    sim_listid = 42;
    return sim_listid;
}

static void sim_glNewList(GLuint list, GLuint mode)
{
    list = list; mode = mode;
}

static void sim_glEndList(void) {}
static void sim_glCallList(GLuint list) { list = list; }

/* ------------------------------------------------------------------ */
/* Section F: confirm the fix and verify other call patterns           */
/*                                                                      */
/* sim_drawbox mirrors the real Mesa drawbox() signature exactly.      */
/* F1: old broken s2/ms2 pattern  -- y1 FAILS                         */
/* F2: fixed 6-var pattern        -- all PASS                          */
/* F3: auxWireBox pattern (6 distinct neg/pos vars, each used once)    */
/* F4: gluCylinder pattern (3 doubles, radius repeated)                */
/* F5: doughnut pattern (2 doubles + 2 ints + GLenum)                  */
/* F6: fixed pattern inside full auxWireCube malloc/sizeArray frame    */
/* ------------------------------------------------------------------ */

/* Exact same signature as the real drawbox() */
static void sim_drawbox(GLdouble x0, GLdouble x1, GLdouble y0, GLdouble y1,
                        GLdouble z0, GLdouble z1, unsigned int type)
{
    GLdouble half = 0.5;
    fprintf(stderr, "  params: x0=%.2f x1=%.2f y0=%.2f y1=%.2f z0=%.2f z1=%.2f\n",
            x0, x1, y0, y1, z0, z1);
    check("x0(exp -0.5)", x0, -half);
    check("x1(exp +0.5)", x1,  half);
    check("y0(exp -0.5)", y0, -half);
    check("y1(exp +0.5)", y1,  half);   /* corrupted by Bug 1 in BROKEN pattern */
    check("z0(exp -0.5)", z0, -half);
    check("z1(exp +0.5)", z1,  half);
    type = type;
}

/* Mirrors gluCylinder(quadric*, baseR, topR, height, slices, stacks) */
static void sim_glcylinder(GLdouble base, GLdouble top, GLdouble height,
                            int slices, int stacks)
{
    fprintf(stderr, "  params: base=%.4f top=%.4f height=%.4f slices=%d stacks=%d\n",
            base, top, height, slices, stacks);
    check("base(exp 0.8)",   base,   0.8);
    check("top(exp 0.8)",    top,    0.8);
    check("height(exp 1.5)", height, 1.5);
    slices = slices; stacks = stacks;
}

/* Mirrors doughnut(r, R, nsides, rings, type) */
static void sim_doughnut(GLdouble r, GLdouble R, int nsides, int rings,
                          unsigned int type)
{
    fprintf(stderr, "  params: r=%.4f R=%.4f nsides=%d rings=%d\n",
            r, R, nsides, rings);
    check("r(exp 0.3)", r, 0.3);
    check("R(exp 0.8)", R, 0.8);
    nsides = nsides; rings = rings; type = type;
}

static void test_section_f(void)
{
    /* F1: BROKEN -- original s2/ms2, ms2 used as y1 (Bug 1) */
    section("F1", "sim_drawbox: BROKEN s2/ms2 pattern (y1 expected to FAIL)");
    {
        GLdouble size = 1.0;
        GLdouble s2  = size / 2.0;
        GLdouble ms2 = -s2;
        sim_drawbox(ms2, s2, ms2, s2, ms2, s2, 2u);
    }

    /* F2: FIXED -- 6 distinct vars, each used exactly once */
    section("F2", "sim_drawbox: FIXED 6-var pattern (all expected to PASS)");
    {
        GLdouble size = 1.0;
        GLdouble x0 = -(size/2.0), x1 = size/2.0;
        GLdouble y0 = -(size/2.0), y1 = size/2.0;
        GLdouble z0 = -(size/2.0), z1 = size/2.0;
        sim_drawbox(x0, x1, y0, y1, z0, z1, 2u);
    }

    /* F3: auxWireBox-style -- 6 negation-paired vars, each used ONCE */
    section("F3", "auxWireBox style: 6 distinct neg/pos vars, each used once");
    {
        GLdouble w = 1.0, h = 1.0, d = 1.0;
        GLdouble w2 = w/2.0, mw2 = -w2;
        GLdouble h2 = h/2.0, mh2 = -h2;
        GLdouble d2 = d/2.0, md2 = -d2;
        sim_drawbox(mw2, w2, mh2, h2, md2, d2, 2u);
    }

    /* F4: gluCylinder -- 3 doubles, first arg (radius) repeated */
    section("F4", "gluCylinder pattern: (r, r, h, 12, 2) -- radius used twice");
    {
        GLdouble radius = 0.8;
        GLdouble height = 1.5;
        sim_glcylinder(radius, radius, height, 12, 2);
    }

    /* F5: doughnut -- 2 doubles + 2 ints + GLenum, small count */
    section("F5", "doughnut pattern: (innerR, outerR, 5, 10, GL_LINE_LOOP)");
    {
        GLdouble innerR = 0.3;
        GLdouble outerR = 0.8;
        sim_doughnut(innerR, outerR, 5, 10, 2u);
    }

    /* F6: FIXED pattern inside a full auxWireCube-like frame              */
    /*     (malloc + sizeArray pointer + nested scope, same as real shapes) */
    section("F6", "FIXED 6-var inside auxWireCube frame (malloc+sizeArray+nested scope)");
    {
        GLdouble size = 1.0;
        GLdouble *sizeArray = (GLdouble *) malloc(sizeof(GLdouble));
        if (sizeArray) {
            *sizeArray = size;
            {
                GLdouble x0 = -(size/2.0), x1 = size/2.0;
                GLdouble y0 = -(size/2.0), y1 = size/2.0;
                GLdouble z0 = -(size/2.0), z1 = size/2.0;
                sim_drawbox(x0, x1, y0, y1, z0, z1, 2u);
            }
            free(sizeArray);
        }
    }
}

/* D3: parameter-derived values, WITH prior GLdouble-comparing function calls */
static void test_with_fpu_context(GLdouble size)
{
    GLdouble pos = size / 2.0;
    GLdouble neg = -pos;
    GLdouble dummy;

    /* Simulate the GLdouble-comparison work that findList does */
    dummy = size * 2.0;
    if (dummy == 9999.0)   /* always false -- just exercises FPU */
        fprintf(stderr, "  (never)\n");

    section("D3", "6 alt doubles from parameter after GLdouble arithmetic");
    recv6i(neg, pos, neg, pos, neg, pos, 2u);
}

/* D4: full auxWireCube structural simulation */
static void auxWireCube_sim(GLdouble size)
{
    GLdouble *sizeArray;
    GLuint displayList;

    /* Replicate auxWireCube exactly: malloc, findList check, glNewList(makeModelPtr) */
    sizeArray = (GLdouble *) malloc(sizeof(GLdouble));
    if (!sizeArray) return;
    *sizeArray = size;

    displayList = sim_findList(1, sizeArray, 1);

    if (displayList == 0) {
        sim_glNewList(sim_makeModelPtr(1, sizeArray, 1), 0x1300u);
        /* Nested scope, exactly as in auxWireCube */
        {
            GLdouble s2  = size / 2.0;
            GLdouble ms2 = -s2;
            section("D4", "6 alt doubles + trailing int inside auxWireCube simulation");
            recv6i(ms2, s2, ms2, s2, ms2, s2, 2u);
        }
        sim_glEndList();
    }
    else {
        sim_glCallList(displayList);
        free(sizeArray);
    }
}

/* D5: second call -- tests the glCallList path (displayList != 0) */
static void auxWireCube_sim2(GLdouble size)
{
    GLdouble *sizeArray;
    GLuint displayList;

    sizeArray = (GLdouble *) malloc(sizeof(GLdouble));
    if (!sizeArray) return;
    *sizeArray = size;
    displayList = sim_findList(1, sizeArray, 1);

    if (displayList == 0) {
        /* should not reach here on second call */
        sim_glNewList(sim_makeModelPtr(1, sizeArray, 1), 0x1300u);
        {
            GLdouble s2  = size / 2.0;
            GLdouble ms2 = -s2;
            section("D5a", "D5: first-call path (unexpected)");
            recv6i(ms2, s2, ms2, s2, ms2, s2, 2u);
        }
        sim_glEndList();
    }
    else {
        section("D5", "second call takes glCallList path -- no drawbox involved");
        fprintf(stderr, "  (glCallList path -- no recv6i call)\n");
        sim_glCallList(displayList);
        free(sizeArray);
    }
}

static void test_context_sim(void)
{
    /* D1/D2: parameter-derived values, minimal context */
    test_param_derived(1.0);

    /* D3: add GLdouble arithmetic before the call */
    test_with_fpu_context(1.0);

    /* D4: full auxWireCube structure -- first call (creates list) */
    auxWireCube_sim(1.0);

    /* D5: second call to same size (exercises the glCallList path) */
    auxWireCube_sim2(1.0);
}

/* ------------------------------------------------------------------ */
/* Section E: isolate the negation-relationship trigger                */
/*                                                                      */
/* D1 shows the bug fires when neg = -pos (computed from parameter).   */
/* A4 shows the bug does NOT fire when neg = -0.5 (literal).           */
/* These tests vary the computation form to find the exact trigger.    */
/* ------------------------------------------------------------------ */

static void test_negation_forms(GLdouble size)
{
    GLdouble pos, neg;

    /* E1: unary minus on the variable (same as D1 -- expected to FAIL) */
    section("E1", "neg = -pos  (unary minus -- expected to FAIL)");
    pos = size / 2.0;
    neg = -pos;
    recv6(neg, pos, neg, pos, neg, pos);

    /* E2: subtraction from zero instead of unary minus */
    section("E2", "neg = 0.0 - pos  (subtraction form)");
    pos = size / 2.0;
    neg = 0.0 - pos;
    recv6(neg, pos, neg, pos, neg, pos);

    /* E3: multiply by -1.0 instead of negation */
    section("E3", "neg = pos * (-1.0)  (multiply form)");
    pos = size / 2.0;
    neg = pos * (-1.0);
    recv6(neg, pos, neg, pos, neg, pos);

    /* E4: neg and pos computed INDEPENDENTLY -- no relationship between them */
    section("E4", "neg = -(size/2)  pos = size/2  (independent computations)");
    pos = size / 2.0;
    neg = -(size / 2.0);   /* NOT derived from pos */
    recv6(neg, pos, neg, pos, neg, pos);

    /* E5: 6 fully distinct variables -- proposed fix for shapes.c */
    section("E5", "6 distinct vars: x0,x1,y0,y1,z0,z1 = +/-size/2  (proposed fix)");
    {
        GLdouble x0 = -(size/2.0), x1 = size/2.0;
        GLdouble y0 = -(size/2.0), y1 = size/2.0;
        GLdouble z0 = -(size/2.0), z1 = size/2.0;
        recv6(x0, x1, y0, y1, z0, z1);
    }

    /* E6: 6 distinct vars with trailing int (exact proposed fix for drawbox call) */
    section("E6", "6 distinct vars + trailing int  (proposed fix, full drawbox sig)");
    {
        GLdouble x0 = -(size/2.0), x1 = size/2.0;
        GLdouble y0 = -(size/2.0), y1 = size/2.0;
        GLdouble z0 = -(size/2.0), z1 = size/2.0;
        recv6i(x0, x1, y0, y1, z0, z1, 2u);
    }

    /* E7: neg computed with an intermediate temp to break the direct negation */
    section("E7", "neg via temp: tmp=pos; neg=-tmp; (extra indirection)");
    {
        GLdouble tmp;
        pos = size / 2.0;
        tmp = pos;
        neg = -tmp;
        recv6(neg, pos, neg, pos, neg, pos);
    }
}

/* ------------------------------------------------------------------ */
/* Section G: gluCylinder GLU_FILL math simulation                     */
/* ------------------------------------------------------------------ */

static void sim_glVertex3f(float x, float y, float z) {
    check("glVertex3f x (exp 0.0)", (GLdouble)x, 0.0);
    check("glVertex3f y (exp 0.8)", (GLdouble)y, 0.8);
    check("glVertex3f z (exp 0.0)", (GLdouble)z, 0.0);
}

static void test_section_g(void) {
    section("G1", "gluCylinder GLU_FILL math pattern (x1*r, y1*r)");
    {
        double baseRadius = 0.8;
        int slices = 15;
        double da = 2.0*3.1415926 / slices;
        int i;
        
        for (i = 0; i < slices; i++) {
            int idx = i;
            float x1 = -sin(idx*da);
            float y1 = cos(idx*da);
            double r = baseRadius;
            
            if (i == 0) {
                sim_glVertex3f((float)(x1*r), (float)(y1*r), 0.0f);
            }
        }
    }
}

static void test_section_g2(void) {
    section("G2", "gluCylinder GLU_FILL with isolated math");
    {
        double baseRadius = 0.8;
        int slices = 15;
        double da = 2.0*3.1415926 / slices;
        int i;
        
        for (i = 0; i < slices; i++) {
            int idx = i;
            float x1 = -sin(idx*da);
            float y1 = cos(idx*da);
            double r = baseRadius;
            
            if (i == 0) {
                float vx = (float)(x1*r);
                float vy = (float)(y1*r);
                sim_glVertex3f(vx, vy, 0.0f);
            }
        }
    }
}

static void test_section_g3(void) {
    section("G3", "gluCylinder GLU_FILL with flattened math");
    {
        double baseRadius = 0.8;
        int slices = 15;
        double da = 2.0*3.1415926 / slices;
        int i;
        
        for (i = 0; i < slices; i++) {
            int idx = i;
            volatile double a1 = idx * da;
            volatile double s1 = sin(a1);
            volatile double c1 = cos(a1);
            volatile float x1 = -s1;
            volatile float y1 = c1;
            volatile double r = baseRadius;
            
            if (i == 0) {
                volatile float vx = (float)(x1*r);
                volatile float vy = (float)(y1*r);
                sim_glVertex3f(vx, vy, 0.0f);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Section H: doughnut math loop simulation                            */
/* ------------------------------------------------------------------ */

static void sim_glVertex3dv(const double *v) {
    check("glVertex3dv x (exp 1.1)", v[0], 1.1);
    check("glVertex3dv y (exp 0.0)", v[1], 0.0);
    check("glVertex3dv z (exp 0.0)", v[2], 0.0);
}

static void test_section_h(void) {
    section("H1", "doughnut nested math pattern");
    {
        double r = 0.3;
        double R = 0.8;
        int rings = 10, nsides = 5;
        int i, j;
        double theta, phi;
        double p0[3];
        
        for (i = 0; i < rings; i++) {
            theta = (double)i*2.0*3.1415926/rings;
            for (j = 0; j < nsides; j++) {
                phi = (double)j*2.0*3.1415926/nsides;
                p0[0] = cos(theta)*(R + r*cos(phi));
                p0[1] = -sin(theta)*(R + r*cos(phi));
                p0[2] = r*sin(phi);
                if (i == 0 && j == 0) {
                    sim_glVertex3dv(p0);
                }
            }
        }
    }
}

static void test_section_h2(void) {
    section("H2", "doughnut nested math with accumulator instead of cast");
    {
        double r = 0.3;
        double R = 0.8;
        int rings = 10, nsides = 5;
        int i, j;
        double theta, phi;
        double p0[3];
        
        double dtheta = 2.0*3.1415926/rings;
        double dphi = 2.0*3.1415926/nsides;
        theta = 0.0;
        
        for (i = 0; i < rings; i++) {
            phi = 0.0;
            for (j = 0; j < nsides; j++) {
                p0[0] = cos(theta)*(R + r*cos(phi));
                p0[1] = -sin(theta)*(R + r*cos(phi));
                p0[2] = r*sin(phi);
                if (i == 0 && j == 0) {
                    sim_glVertex3dv(p0);
                }
                phi += dphi;
            }
            theta += dtheta;
        }
    }
}

static void test_section_h3(void) {
    section("H3", "doughnut flattened math");
    {
        double r = 0.3;
        double R = 0.8;
        int rings = 10, nsides = 5;
        int i, j;
        double p0[3];
        
        for (i = 0; i < rings; i++) {
            double theta = (double)i*2.0*3.1415926/rings;
            for (j = 0; j < nsides; j++) {
                volatile double phi = (double)j*2.0*3.1415926/nsides;
                
                volatile double s_th = sin(theta);
                volatile double c_th = cos(theta);
                volatile double s_ph = sin(phi);
                volatile double c_ph = cos(phi);
                
                volatile double R_rcph = R + r*c_ph;
                
                p0[0] = c_th*R_rcph;
                p0[1] = -s_th*R_rcph;
                p0[2] = r*s_ph;
                
                if (i == 0 && j == 0) {
                    sim_glVertex3dv(p0);
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Section I: m_xformpt pointer aliasing simulation                   */
/* ------------------------------------------------------------------ */

static void sim_m_xformpt(double pin[3], double pout[3]) {
    int i;
    for (i = 0; i < 3; i++) {
        pout[i] = pin[i];
    }
}

static void test_section_i(void) {
    section("I1", "m_xformpt pointer aliasing copy");
    {
        double p0[3] = {1.1, 2.2, 3.3};
        sim_m_xformpt(p0, p0);
        check("p0[0] (exp 1.1)", p0[0], 1.1);
        check("p0[1] (exp 2.2)", p0[1], 2.2);
        check("p0[2] (exp 3.3)", p0[2], 3.3);
    }
}

static void test_section_j(void) {
    section("J1", "gluSphere GLU_LINE nested math pattern");
    {
        int i;
        for (i = 0; i < 15; i++) {
            double theta = i * 0.1;
            double rho = 0.5, drho = 0.1;
            double x = -sin(theta) * sin(rho+drho);
            double y = cos(theta) * sin(rho+drho);
            
            if (i == 0) {
                /* theta = 0, sin(0)=0, cos(0)=1 */
                /* rho+drho = 0.6 */
                check("gluSphere GLU_LINE x", x, 0.0);
                check("gluSphere GLU_LINE y", y, sin(0.6));
            }
        }
    }
}

static void test_section_k(void) {
    section("K1", "gluDisk simple nested math pattern");
    {
        int i;
        for (i = 0; i < 15; i++) {
            double angle = i * 0.1;
            double radius = 2.0;
            if (i == 0) {
                double x = radius * sin(angle);
                double y = radius * cos(angle);
                check("gluDisk x", x, 0.0);
                check("gluDisk y", y, 2.0);
            }
        }
    }
}

static void test_section_l(void) {
    section("L1", "gluDisk complex nested math pattern");
    {
        int i;
        for (i = 0; i < 15; i++) {
            double angle = i * 0.1;
            double radius = 1.0, delta_radius = 1.0;
            if (i == 0) {
                double x = (radius+delta_radius)*sin(angle);
                double y = (radius+delta_radius)*cos(angle);
                check("gluDisk outer x", x, 0.0);
                check("gluDisk outer y", y, 2.0);
            }
        }
    }
}

static void test_section_j2(void) {
    section("J2", "gluSphere GLU_LINE flattened math pattern");
    {
        int i;
        for (i = 0; i < 15; i++) {
            double theta = i * 0.1;
            double rho = 0.5, drho = 0.1;
            volatile double s_th = sin(theta);
            volatile double c_th = cos(theta);
            volatile double s_rhodrho = sin(rho+drho);
            volatile double c_rhodrho = cos(rho+drho);
            double x = -s_th * s_rhodrho;
            double y = c_th * s_rhodrho;
            
            if (i == 0) {
                check("gluSphere GLU_LINE x", x, 0.0);
                check("gluSphere GLU_LINE y", y, sin(0.6));
            }
        }
    }
}

static void test_section_k2(void) {
    section("K2", "gluDisk simple flattened math pattern");
    {
        int i;
        for (i = 0; i < 15; i++) {
            double angle = i * 0.1;
            double radius = 2.0;
            if (i == 0) {
                volatile double s_ang = sin(angle);
                volatile double c_ang = cos(angle);
                double x = radius * s_ang;
                double y = radius * c_ang;
                check("gluDisk x", x, 0.0);
                check("gluDisk y", y, 2.0);
            }
        }
    }
}

static void test_section_l2(void) {
    section("L2", "gluDisk complex flattened math pattern");
    {
        int i;
        for (i = 0; i < 15; i++) {
            double angle = i * 0.1;
            double radius = 1.0, delta_radius = 1.0;
            if (i == 0) {
                volatile double s_ang = sin(angle);
                volatile double c_ang = cos(angle);
                double x = (radius+delta_radius)*s_ang;
                double y = (radius+delta_radius)*c_ang;
                check("gluDisk outer x", x, 0.0);
                check("gluDisk outer y", y, 2.0);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int hc_bug_test_main(void)
{
    fprintf(stderr, "MetaWare HC R2.2g double bug diagnostic\n");
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "(All output to stderr so it is not buffered by pipes)\n");

    test_arg_passing();
    test_chained_assign();
    test_individual_assign();
    test_context_sim();
    test_negation_forms(1.0);
    test_section_f();
    test_section_g();
    test_section_g2();
    test_section_g3();
    test_section_h();
    test_section_h2();
    test_section_h3();
    test_section_i();
    test_section_j();
    test_section_j2();
    test_section_k();
    test_section_k2();
    test_section_l();
    test_section_l2();

    fprintf(stderr, "\n========================================\n");
    if (failed_checks == 0) {
        fprintf(stderr, "ALL %d checks PASSED -- no bug triggered\n", total_checks);
    } else {
        fprintf(stderr, "%d of %d checks FAILED\n", failed_checks, total_checks);
    }

    return (failed_checks > 0) ? 1 : 0;
}

/* ================= test_math_cast.c ================= */
int test_math_cast_main(void) {
    unsigned short us_val = 95;
    double d_val = 95.0;

    printf("--- Testing log() ---\n");
    printf("log(2.0) = %f\n", log(2.0));
    printf("log((double)95) = %f\n", log(d_val));
    printf("log((unsigned short)95) = %f\n", log(us_val));

    printf("\n--- Testing divisions ---\n");
    double div1 = log(d_val) / log(2.0);
    double div2 = log(us_val) / log(2.0);
    printf("log((double)95) / log(2.0) = %f\n", div1);
    printf("log((unsigned short)95) / log(2.0) = %f\n", div2);

    printf("\n--- Testing cast to int ---\n");
    int i_val1 = (int)div1;
    int i_val2 = (int)div2;
    printf("(int)( %f ) = %d\n", div1, i_val1);
    printf("(int)( %f ) = %d\n", div2, i_val2);

    printf("\n--- Testing pow() ---\n");
    printf("pow(2.0, (double)%d) = %f\n", i_val1, pow(2.0, (double)i_val1));
    printf("pow(2.0, (float)%d)  = %f\n", i_val1, pow(2.0, (float)i_val1));
    printf("pow(2.0, (int)%d)    = %f\n", i_val1, pow(2.0, i_val1)); /* Implicit promotion vs stack corruption! */

    printf("\n--- Full Expression Evaluated ---\n");
    int final_correct = (int)pow(2.0, (double)((int)(log((double)us_val)/log(2.0))));
    int final_original = (int)pow(2.0, (float)((int)(log(us_val)/log(2.0))));
    printf("Corrected Expression: %d\n", final_correct);
    printf("Original Expression:  %d\n", final_original);

    return 0;
}


/* ================= MAIN ================= */
int main(int argc, char** argv) {
    printf("\n==============================================\n");
    printf("   UNIFIED COMPILER DIAGNOSTIC SUITE START\n");
    printf("==============================================\n\n");
    printf("\n--- Running %s ---\n", "test_math.c");
    test_math_main(argc, argv);
    printf("\n--- Running %s ---\n", "test_args.c");
    test_args_main();
    printf("\n--- Running %s ---\n", "test_args_lib.c");
    printf("\n--- Running %s ---\n", "check_math.c");
    check_math_main();
    printf("\n--- Running %s ---\n", "test_dodec.c");
    test_dodec_main();
    printf("\n--- Running %s ---\n", "hc_bug_test.c");
    hc_bug_test_main();
    printf("\n--- Running %s ---\n", "test_math_cast.c");
    test_math_cast_main();
    printf("\n==============================================\n");
    printf("   UNIFIED COMPILER DIAGNOSTIC SUITE END\n");
    printf("==============================================\n\n");
    return 0;
}
