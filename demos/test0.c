/* test0.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include "glaux.h"

static float rotAngle = 0.0;
static int use_rgb = 0;

static void Init( void )
{
   if (use_rgb) {
      glClearColor( 0.0, 0.0, 0.0, 1.0 );
   } else {
      /* Define our palette for index mode */
      auxSetOneColor(0, 0.0, 0.0, 0.0); /* Background */
      auxSetOneColor(1, 1.0, 0.0, 0.0); /* Red */
      auxSetOneColor(2, 0.0, 1.0, 0.0); /* Green */
      auxSetOneColor(3, 0.0, 0.0, 1.0); /* Blue */
      glClearIndex( 0.0 );
   }
   glShadeModel( GL_SMOOTH );
}

static void Reshape( int width, int height )
{
   glViewport(0, 0, (GLint)width, (GLint)height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho( -1.0, 1.0, -1.0, 1.0, -1.0, 1.0 );
   glMatrixMode(GL_MODELVIEW);
}

static void display( void )
{
   glClear( GL_COLOR_BUFFER_BIT );

   glPushMatrix();
   glRotatef( rotAngle, 0.0, 0.0, 1.0 );

   glBegin( GL_TRIANGLES );
   if (use_rgb) glColor3f( 1.0, 0.0, 0.0 );
   else glIndexi( 1 );
   glVertex2f( -0.5, -0.5 );

   if (use_rgb) glColor3f( 0.0, 1.0, 0.0 );
   else glIndexi( 2 );
   glVertex2f(  0.5, -0.5 );

   if (use_rgb) glColor3f( 0.0, 0.0, 1.0 );
   else glIndexi( 3 );
   glVertex2f(  0.0,  0.5 );
   glEnd();

   glPopMatrix();

   glFlush();
   auxSwapBuffers();

   rotAngle += 2.0;
   if (rotAngle >= 360.0) rotAngle -= 360.0;
}

int main( int argc, char **argv )
{
   int i;
   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-rgb") == 0) {
         use_rgb = 1;
      }
   }

   if (use_rgb) {
      auxInitDisplayMode( AUX_RGB | AUX_DOUBLE );
   } else {
      auxInitDisplayMode( AUX_INDEX | AUX_DOUBLE );
   }

   auxInitPosition( 50, 50, 400, 300 );

   if (auxInitWindow("test0") == GL_FALSE) {
      auxQuit();
   }

   Init();
   auxExposeFunc(Reshape);
   auxReshapeFunc(Reshape);

   auxMainLoop( display );
   return 0;
}
