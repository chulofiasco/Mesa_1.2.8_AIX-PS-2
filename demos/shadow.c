/* shadow.c */

/*
 * Use the stencil buffer to generate shadows.  This demo inspired by
 * an article in IRIS Universe Magazine, No. 18.  This program is in
 * the public domain.
 *
 * Brian Paul
 */



#include <math.h>
#include "gltk.h"



static GLfloat light_pos[4] = { 0.0, 10.0, 0.0, 1.0 };
static GLfloat view_rotx, view_roty;
static GLfloat ground_xrot, ground_yrot, ground_zrot;
static GLfloat ground_y;


/*
 * These triangles cast the shadow.
 */

#define NUM_TRI 4

static GLfloat triangle[NUM_TRI][3][3] = {
    0.5, 4.0, 0.5,
   -0.5, 4.0, 0.5,
    0.0, 4.0, 2.0,

    0.5, 4.0, 0.5,
    0.5, 4.0, -0.5,
    2.0, 4.0, 0.0,

    0.5, 4.0, -0.5,
    -0.5, 4.0, -0.5,
    0.0, 4.0, -2.0,

    -0.5, 4.0, -0.5,
    -0.5, 4.0,  0.5,
    -2.0, 4.0,  0.0
};



static void init( void )
{
   static GLfloat ambient[4] = {0.3, 0.3, 0.3, 1.0};

   view_rotx = 50.0;
   view_roty = 20.0;

   ground_xrot = ground_yrot = ground_zrot = 0.0;
   ground_y = 0.0;

   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LEQUAL );
   glShadeModel( GL_FLAT );
   glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient );
   glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
}



/*
 * Draw the whole scene.  For each frame we'll call this function twice.
 * The first time with just ambient lighting, the second time with ambient
 * and point source lighting.
 */
static void draw_scene( void )
{
   static GLint ground = -1;
   GLint i, j;
   GLfloat x, y, z;

   glPushMatrix();
   glRotatef( ground_xrot, 1.0, 0.0, 0.0 );
   glRotatef( ground_yrot, 0.0, 1.0, 0.0 );
   glRotatef( ground_zrot, 0.0, 0.0, 1.0 );
   glTranslatef( 0.0, ground_y, 0.0 );

   if (ground==-1) {
      /* make the ground checkerboard */
      ground = glGenLists(1);
      glNewList( ground, GL_COMPILE_AND_EXECUTE );
      for (i=0;i<5;i++) {
	 for (j=0;j<5;j++) {
	    if ((i+j)%2) {
	       glColor3f( 0.0, 1.0, 0.0 );
	    }
	    else {
	       glColor3f( 0.0, 0.0, 1.0 );
	    }
	    glNormal3f( 0.0, 1.0, 0.0 );
	    x = i*2.0 - 5.0;
	    y = 0.0;
	    z = j*2.0 - 5.0;
	    glBegin( GL_POLYGON );
	    glVertex3f( x,     y, z+2.0 );
	    glVertex3f( x+2.0, y, z+2.0 );
	    glVertex3f( x+2.0, y, z );
	    glVertex3f( x,     y, z );
	    glEnd();
	 }
      }
      glEndList();
   }
   else {
      /* draw ground checkerboard */
      glCallList( ground );
   }

   glPopMatrix();

   /* draw triangles */
   glColor3f( 1.0, 0.0, 0.0 );
   glNormal3f( 0.0, 1.0, 0.0 );
   glBegin( GL_TRIANGLES );
   for (i=0;i<NUM_TRI;i++) {
      glVertex3fv( triangle[i][0] );
      glVertex3fv( triangle[i][1] );
      glVertex3fv( triangle[i][2] );
   }
   glEnd();
}



/*
 * For each triangle in triangle list:
 *    For each edge of triangle:
 *       Construct a shadow volume boundary polygon by extending an edge away
 *         from the light source for an arbitrary distance.
 *       Draw the boundary polygon into the stencil planes, with invert
 *         stencil op, do depth test but don't update depth buffer.
 * When finished, where stencil buffer = 1 we're in a shadow.
 */
static void draw_shadow_volumes( void )
{
   GLint tri;
   GLint i, j;
   GLfloat v0[3], v1[3];
   GLfloat p0[3], p1[3], p2[3], p3[3];

   for (tri=0;tri<NUM_TRI;tri++) {

      /* loop over triangle edges */
      for (i=0;i<3;i++) {
	 j = (i+1) % 3;
	 /* v0 = vector from light to vertex i */
	 v0[0] = triangle[tri][i][0] - light_pos[0];
	 v0[1] = triangle[tri][i][1] - light_pos[1];
	 v0[2] = triangle[tri][i][2] - light_pos[2];
	 /* v1 = vector from light to vertex i+1 */
	 v1[0] = triangle[tri][j][0] - light_pos[0];
	 v1[1] = triangle[tri][j][1] - light_pos[1];
	 v1[2] = triangle[tri][j][2] - light_pos[2];

	 /* compute vertices of shadow volume boundary side */
	 p0[0] = triangle[tri][i][0];
	 p0[1] = triangle[tri][i][1];
	 p0[2] = triangle[tri][i][2];

	 p1[0] = triangle[tri][j][0];
	 p1[1] = triangle[tri][j][1];
	 p1[2] = triangle[tri][j][2];

	 p2[0] = p1[0] + v1[0] * 10.0;
	 p2[1] = p1[1] + v1[1] * 10.0;
	 p2[2] = p1[2] + v1[2] * 10.0;

	 p3[0] = p0[0] + v0[0] * 10.0;
	 p3[1] = p0[1] + v0[1] * 10.0;
	 p3[2] = p0[2] + v0[2] * 10.0;

	 /* draw the shadow volume boundary polygon */
	 glBegin( GL_POLYGON );
	 glVertex3fv( p0 );
	 glVertex3fv( p1 );
	 glVertex3fv( p2 );
	 glVertex3fv( p3 );
	 glEnd();
      }
   }
}



static void draw( void )
{
   /* clear the buffers */
   glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT
	    | GL_STENCIL_BUFFER_BIT );

   /* transform by the view angle */
   glPushMatrix();
   glRotatef( view_rotx, 1.0, 0.0, 0.0 );
   glRotatef( view_roty, 0.0, 0.0, 1.0 );

   glLightfv( GL_LIGHT0, GL_POSITION, light_pos );

   /* Draw scene with ambient lighting only */
   glEnable( GL_LIGHTING );
   glDisable( GL_LIGHT0 );
   glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT );
   glEnable( GL_COLOR_MATERIAL );
   draw_scene();

   /* Draw shadow volumes */
   glDisable( GL_LIGHTING );
   glStencilFunc( GL_ALWAYS, 0, 0xffffffff );
   glStencilOp( GL_KEEP, GL_KEEP, GL_INVERT );
   glEnable( GL_STENCIL_TEST );
   glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
   glDepthMask( GL_FALSE );
   draw_shadow_volumes();

   /* Draw scene with point sources where stencil==0 only */
   glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
   glDepthMask( GL_TRUE );
   glEnable( GL_LIGHT0 );
   glEnable( GL_LIGHTING );
   glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
   glEnable( GL_COLOR_MATERIAL );
   glStencilFunc( GL_EQUAL, 0, 0xffffffff );   /* draw where stencil==0 */
   glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
   draw_scene();

   /* all done */
   glDisable( GL_STENCIL_TEST );
   glDisable( GL_LIGHT0 );

   glPopMatrix();

   glFlush();
   tkSwapBuffers();
}




static void idle( void )
{
   ground_yrot -= 2.0;
   ground_xrot = sin(ground_yrot*0.1) * 10.0;
   ground_zrot = cos(ground_yrot*0.1) * 10.0;
   ground_y = sin( ground_yrot*0.15) * 2.0;
   draw();
}


/* change view angle, exit upon ESC */
static GLenum key(int k, GLenum mask)
{
   switch (k) {
      case TK_UP:
         view_rotx += 5.0;
	 return GL_TRUE;
      case TK_DOWN:
         view_rotx -= 5.0;
	 return GL_TRUE;
      case TK_LEFT:
         view_roty += 5.0;
	 return GL_TRUE;
      case TK_RIGHT:
         view_roty -= 5.0;
	 return GL_TRUE;
      case TK_ESCAPE:
	 tkQuit();
   }
   return GL_FALSE;
}


/* new window size or exposure */
static void reshape( int width, int height )
{
   glViewport(0, 0, (GLint)width, (GLint)height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 50.0 );
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef( 0.0, 0.0, -35.0 );
}



main()
{
   tkInitPosition(0, 0, 300, 300);
   tkInitDisplayMode( TK_RGB | TK_DEPTH | TK_DOUBLE | TK_STENCIL | TK_DIRECT );

   if (tkInitWindow("Stencil Shadow") == GL_FALSE) {
      tkQuit();
   }

   init();

   tkExposeFunc( reshape );
   tkReshapeFunc( reshape );
   tkKeyDownFunc( key );
   tkIdleFunc( idle );
   tkDisplayFunc( draw );
   tkExec();
}
