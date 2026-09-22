/* $Id: varray.c,v 1.4 1996/04/30 12:18:21 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  1.2
 * Copyright (C) 1995-1996  Brian Paul  (brianp@ssec.wisc.edu)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
$Log: varray.c,v $
 * Revision 1.4  1996/04/30  12:18:21  brianp
 * typo: replaced sizeof(GLshort) with sizeof(GLint)
 *
 * Revision 1.3  1996/03/02  21:58:45  brianp
 * only use edge flags if GL_TRIANGLES, GL_QUADS, or GL_POLYGON
 *
 * Revision 1.2  1996/02/20  19:48:59  brianp
 * added glVertex3fv() array optimization
 *
 * Revision 1.1  1996/02/15  16:06:05  brianp
 * Initial revision
 *
 */



/*
 * NOTE:  At this time, only three vertex array configurations are optimized:
 *  1.  glVertex3fv(), zero stride
 *  2.  glNormal3fv() with glVertex3fv(), zero stride
 *  3.  glNormal3fv() with glVertex4fv(), zero stride
 *
 * More optimized array configurations can be added.
 */




#include <string.h>
#include "draw.h"
#include "context.h"
#include "macros.h"
#include "vb.h"
#include "xform.h"




void glVertexPointerEXT( GLint size, GLenum type, GLsizei stride,
                         GLsizei count, const void *ptr )
{
   if (size<2 || size>4) {
      gl_error( GL_INVALID_VALUE, "glVertexPointerEXT(size)" );
      return;
   }
   if (stride<0) {
      gl_error( GL_INVALID_VALUE, "glVertexPointerEXT(stride)" );
      return;
   }
   switch (type) {
      case GL_SHORT:
         CC.Array.VertexStrideB = stride ? stride : size*sizeof(GLshort);
         break;
      case GL_INT:
         CC.Array.VertexStrideB = stride ? stride : size*sizeof(GLint);
         break;
      case GL_FLOAT:
         CC.Array.VertexStrideB = stride ? stride : size*sizeof(GLfloat);
         break;
      case GL_DOUBLE_EXT:
         CC.Array.VertexStrideB = stride ? stride : size*sizeof(GLdouble);
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glVertexPointerEXT(type)" );
         return;
   }
   CC.Array.VertexSize = size;
   CC.Array.VertexType = type;
   CC.Array.VertexStride = stride;
   CC.Array.VertexCount = count;
   CC.Array.VertexPtr = (void *) ptr;
}




void glNormalPointerEXT( GLenum type, GLsizei stride,
                         GLsizei count, const void *ptr )
{
   if (stride<0) {
      gl_error( GL_INVALID_VALUE, "glNormalPointerEXT(stride)" );
      return;
   }
   switch (type) {
      case GL_BYTE:
         CC.Array.NormalStrideB = stride ? stride : 3*sizeof(GLbyte);
         break;
      case GL_SHORT:
         CC.Array.NormalStrideB = stride ? stride : 3*sizeof(GLshort);
         break;
      case GL_INT:
         CC.Array.NormalStrideB = stride ? stride : 3*sizeof(GLint);
         break;
      case GL_FLOAT:
         CC.Array.NormalStrideB = stride ? stride : 3*sizeof(GLfloat);
         break;
      case GL_DOUBLE_EXT:
         CC.Array.NormalStrideB = stride ? stride : 3*sizeof(GLdouble);
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glNormalPointerEXT(type)" );
         return;
   }
   CC.Array.NormalType = type;
   CC.Array.NormalStride = stride;
   CC.Array.NormalCount = count;
   CC.Array.NormalPtr = (void *) ptr;
}



void glColorPointerEXT( GLint size, GLenum type, GLsizei stride,
                        GLsizei count, const void *ptr )
{
   if (size<3 || size>4) {
      gl_error( GL_INVALID_VALUE, "glColorPointerEXT(size)" );
      return;
   }
   if (stride<0) {
      gl_error( GL_INVALID_VALUE, "glColorPointerEXT(stride)" );
      return;
   }
   switch (type) {
      case GL_BYTE:
         CC.Array.ColorStrideB = stride ? stride : size*sizeof(GLbyte);
         break;
      case GL_UNSIGNED_BYTE:
         CC.Array.ColorStrideB = stride ? stride : size*sizeof(GLubyte);
         break;
      case GL_SHORT:
         CC.Array.ColorStrideB = stride ? stride : size*sizeof(GLshort);
         break;
      case GL_UNSIGNED_SHORT:
         CC.Array.ColorStrideB = stride ? stride : size*sizeof(GLushort);
         break;
      case GL_INT:
         CC.Array.ColorStrideB = stride ? stride : size*sizeof(GLint);
         break;
      case GL_UNSIGNED_INT:
         CC.Array.ColorStrideB = stride ? stride : size*sizeof(GLuint);
         break;
      case GL_FLOAT:
         CC.Array.ColorStrideB = stride ? stride : size*sizeof(GLfloat);
         break;
      case GL_DOUBLE_EXT:
         CC.Array.ColorStrideB = stride ? stride : size*sizeof(GLdouble);
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glColorPointerEXT(type)" );
         return;
   }
   CC.Array.ColorSize = size;
   CC.Array.ColorType = type;
   CC.Array.ColorStride = stride;
   CC.Array.ColorCount = count;
   CC.Array.ColorPtr = (void *) ptr;
}



void glIndexPointerEXT( GLenum type, GLsizei stride, GLsizei count,
                        const void *ptr )
{
   if (stride<0) {
      gl_error( GL_INVALID_VALUE, "glIndexPointerEXT(stride)" );
      return;
   }
   switch (type) {
      case GL_SHORT:
         CC.Array.IndexStrideB = stride ? stride : sizeof(GLbyte);
         break;
      case GL_INT:
         CC.Array.IndexStrideB = stride ? stride : sizeof(GLint);
         break;
      case GL_FLOAT:
         CC.Array.IndexStrideB = stride ? stride : sizeof(GLfloat);
         break;
      case GL_DOUBLE_EXT:
         CC.Array.IndexStrideB = stride ? stride : sizeof(GLdouble);
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glIndexPointerEXT(type)" );
         return;
   }
   CC.Array.IndexType = type;
   CC.Array.IndexStride = stride;
   CC.Array.IndexCount = count;
   CC.Array.IndexPtr = (void *) ptr;
}



void glTexCoordPointerEXT( GLint size, GLenum type, GLsizei stride,
                           GLsizei count, const void *ptr )
{
   if (size<1 || size>4) {
      gl_error( GL_INVALID_VALUE, "glTexCoordPointerEXT(size)" );
      return;
   }
   switch (type) {
      case GL_SHORT:
         CC.Array.TexCoordStrideB = stride ? stride : size*sizeof(GLshort);
         break;
      case GL_INT:
         CC.Array.TexCoordStrideB = stride ? stride : size*sizeof(GLint);
         break;
      case GL_FLOAT:
         CC.Array.TexCoordStrideB = stride ? stride : size*sizeof(GLfloat);
         break;
      case GL_DOUBLE_EXT:
         CC.Array.TexCoordStrideB = stride ? stride : size*sizeof(GLdouble);
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glTexCoordPointerEXT(type)" );
         return;
   }
   if (stride<0) {
      gl_error( GL_INVALID_VALUE, "glTexCoordPointerEXT(stride)" );
      return;
   }
   CC.Array.TexCoordSize = size;
   CC.Array.TexCoordType = type;
   CC.Array.TexCoordStride = stride;
   CC.Array.TexCoordCount = count;
   CC.Array.TexCoordPtr = (void *) ptr;
}



void glEdgeFlagPointerEXT( GLsizei stride, GLsizei count,
                           const GLboolean *ptr )
{
   if (stride<0) {
      gl_error( GL_INVALID_VALUE, "glEdgeFlagPointerEXT(stride)" );
      return;
   }
   CC.Array.EdgeFlagStride = stride;
   CC.Array.EdgeFlagStrideB = stride ? stride : sizeof(GLboolean);
   CC.Array.EdgeFlagCount = count;
   CC.Array.EdgeFlagPtr = (GLboolean *) ptr;
}



/*
 * Eventually, this may be moved into get.c
 */
void glGetPointervEXT( GLenum pname, void **params )
{
   switch (pname) {
      case GL_VERTEX_ARRAY_POINTER_EXT:
         *params = CC.Array.VertexPtr;
         break;
      case GL_NORMAL_ARRAY_POINTER_EXT:
         *params = CC.Array.NormalPtr;
         break;
      case GL_COLOR_ARRAY_POINTER_EXT:
         *params = CC.Array.ColorPtr;
         break;
      case GL_INDEX_ARRAY_POINTER_EXT:
         *params = CC.Array.IndexPtr;
         break;
      case GL_TEXTURE_COORD_ARRAY_POINTER_EXT:
         *params = CC.Array.TexCoordPtr;
         break;
      case GL_EDGE_FLAG_ARRAY_POINTER_EXT:
         *params = CC.Array.EdgeFlagPtr;
         break;
      default:
         gl_error( GL_INVALID_ENUM, "glGetPointervEXT" );
         return;
   }
}


void glArrayElementEXT( GLint i )
{
   if (CC.Array.NormalEnabled) {
      GLbyte *p = (GLbyte*) CC.Array.NormalPtr + i * CC.Array.NormalStrideB;
      switch (CC.Array.NormalType) {
         case GL_BYTE:
            glNormal3bv( (GLbyte*) p );
            break;
         case GL_SHORT:
            glNormal3sv( (GLshort*) p );
            break;
         case GL_INT:
            glNormal3iv( (GLint*) p );
            break;
         case GL_FLOAT:
            glNormal3fv( (GLfloat*) p );
            break;
         case GL_DOUBLE_EXT:
            glNormal3dv( (GLdouble*) p );
            break;
      }
   }

   if (CC.Array.ColorEnabled) {
      GLbyte *p = (GLbyte*) CC.Array.ColorPtr + i * CC.Array.ColorStrideB;
      switch (CC.Array.ColorType) {
         case GL_BYTE:
            switch (CC.Array.ColorSize) {
               case 3:   glColor3bv( (GLbyte*) p );   break;
               case 4:   glColor4bv( (GLbyte*) p );   break;
            }
            break;
         case GL_UNSIGNED_BYTE:
            switch (CC.Array.ColorSize) {
               case 3:   glColor3ubv( (GLubyte*) p );   break;
               case 4:   glColor4ubv( (GLubyte*) p );   break;
            }
            break;
         case GL_SHORT:
            switch (CC.Array.ColorSize) {
               case 3:   glColor3sv( (GLshort*) p );   break;
               case 4:   glColor4sv( (GLshort*) p );   break;
            }
            break;
         case GL_UNSIGNED_SHORT:
            switch (CC.Array.ColorSize) {
               case 3:   glColor3usv( (GLushort*) p );   break;
               case 4:   glColor4usv( (GLushort*) p );   break;
            }
            break;
         case GL_INT:
            switch (CC.Array.ColorSize) {
               case 3:   glColor3iv( (GLint*) p );   break;
               case 4:   glColor4iv( (GLint*) p );   break;
            }
            break;
         case GL_UNSIGNED_INT:
            switch (CC.Array.ColorSize) {
               case 3:   glColor3uiv( (GLuint*) p );   break;
               case 4:   glColor4uiv( (GLuint*) p );   break;
            }
            break;
         case GL_FLOAT:
            switch (CC.Array.ColorSize) {
               case 3:   glColor3fv( (GLfloat*) p );   break;
               case 4:   glColor4fv( (GLfloat*) p );   break;
            }
            break;
         case GL_DOUBLE_EXT:
            switch (CC.Array.ColorSize) {
               case 3:   glColor3dv( (GLdouble*) p );   break;
               case 4:   glColor4dv( (GLdouble*) p );   break;
            }
            break;
      }
   }

   if (CC.Array.IndexEnabled) {
      GLbyte *p = (GLbyte*) CC.Array.IndexPtr + i * CC.Array.IndexStrideB;
      switch (CC.Array.IndexType) {
         case GL_SHORT:
            glIndexsv( (GLshort*) p );
            break;
         case GL_INT:
            glIndexiv( (GLint*) p );
            break;
         case GL_FLOAT:
            glIndexfv( (GLfloat*) p );
            break;
         case GL_DOUBLE_EXT:
            glIndexdv( (GLdouble*) p );
            break;
      }
   }

   if (CC.Array.TexCoordEnabled) {
      GLbyte *p = (GLbyte*) CC.Array.TexCoordPtr + i * CC.Array.TexCoordStrideB;
      switch (CC.Array.TexCoordType) {
         case GL_SHORT:
            switch (CC.Array.TexCoordSize) {
               case 1:   glTexCoord1sv( (GLshort*) p );   break;
               case 2:   glTexCoord2sv( (GLshort*) p );   break;
               case 3:   glTexCoord3sv( (GLshort*) p );   break;
               case 4:   glTexCoord4sv( (GLshort*) p );   break;
            }
            break;
         case GL_INT:
            switch (CC.Array.TexCoordSize) {
               case 1:   glTexCoord1iv( (GLint*) p );   break;
               case 2:   glTexCoord2iv( (GLint*) p );   break;
               case 3:   glTexCoord3iv( (GLint*) p );   break;
               case 4:   glTexCoord4iv( (GLint*) p );   break;
            }
            break;
         case GL_FLOAT:
            switch (CC.Array.TexCoordSize) {
               case 1:   glTexCoord1fv( (GLfloat*) p );   break;
               case 2:   glTexCoord2fv( (GLfloat*) p );   break;
               case 3:   glTexCoord3fv( (GLfloat*) p );   break;
               case 4:   glTexCoord4fv( (GLfloat*) p );   break;
            }
            break;
         case GL_DOUBLE_EXT:
            switch (CC.Array.TexCoordSize) {
               case 1:   glTexCoord1dv( (GLdouble*) p );   break;
               case 2:   glTexCoord2dv( (GLdouble*) p );   break;
               case 3:   glTexCoord3dv( (GLdouble*) p );   break;
               case 4:   glTexCoord4dv( (GLdouble*) p );   break;
            }
            break;
      }
   }

   if (CC.Array.EdgeFlagEnabled) {
      GLbyte *b = (GLbyte*) CC.Array.EdgeFlagPtr + i * CC.Array.EdgeFlagStrideB;
      glEdgeFlagv( (GLboolean*) b );
   }

   if (CC.Array.VertexEnabled) {
      GLbyte *b = (GLbyte*) CC.Array.VertexPtr + i * CC.Array.VertexStrideB;
      switch (CC.Array.VertexType) {
         case GL_SHORT:
            switch (CC.Array.VertexSize) {
               case 2:   glVertex2sv( (GLshort*) b );   break;
               case 3:   glVertex3sv( (GLshort*) b );   break;
               case 4:   glVertex4sv( (GLshort*) b );   break;
            }
            break;
         case GL_INT:
            switch (CC.Array.VertexSize) {
               case 2:   glVertex2iv( (GLint*) b );   break;
               case 3:   glVertex3iv( (GLint*) b );   break;
               case 4:   glVertex4iv( (GLint*) b );   break;
            }
            break;
         case GL_FLOAT:
            switch (CC.Array.VertexSize) {
               case 2:   glVertex2fv( (GLfloat*) b );   break;
               case 3:   glVertex3fv( (GLfloat*) b );   break;
               case 4:   glVertex4fv( (GLfloat*) b );   break;
            }
            break;
         case GL_DOUBLE_EXT:
            switch (CC.Array.VertexSize) {
               case 2:   glVertex2dv( (GLdouble*) b );   break;
               case 3:   glVertex3dv( (GLdouble*) b );   break;
               case 4:   glVertex4dv( (GLdouble*) b );   break;
            }
            break;
      }
   }
}



void glDrawArraysEXT( GLenum mode, GLint first, GLsizei count )
{
   GLint i;
   GLboolean need_edges;

   if (INSIDE_BEGIN_END) {
      gl_error( GL_INVALID_OPERATION, "glDrawArraysEXT" );
      return;
   }
   if (count<0) {
      gl_error( GL_INVALID_VALUE, "glDrawArraysEXT(count)" );
      return;
   }

   if (CC.Mode==GL_TRIANGLES || CC.Mode==GL_QUADS || CC.Mode==GL_POLYGON) {
      need_edges = GL_TRUE;
   }
   else {
      need_edges = GL_FALSE;
   }

   if (!CC.CompileFlag
       && !CC.Light.Enabled
       && !CC.Texture.Enabled
       && CC.Array.VertexEnabled && CC.Array.VertexType==GL_FLOAT
       && CC.Array.VertexStride==0 && CC.Array.VertexSize==3
       && !CC.Array.NormalEnabled
       && !CC.Array.ColorEnabled
       && !CC.Array.IndexEnabled
       && !CC.Array.TexCoordEnabled
       && !CC.Array.EdgeFlagEnabled) {
      /*
       * SPECIAL CASE:  glVertex3fv() with no lighting
       */
      GLfloat (*vptr)[3];
      GLint remaining;

      glBegin( mode );

      remaining = count;
      vptr = (GLfloat (*)[3]) CC.Array.VertexPtr + 3 * first;
      while (remaining>0) {
         GLint vbspace, n;

         vbspace = VB_MAX - VB.Start;
         n = MIN2( vbspace, remaining );

         gl_xform_points_3fv( n, VB.Eye+VB.Start, CC.ModelViewMatrix, vptr );

         /* assign vertex colors */
         {
            GLint r = CC.Current.IntColor[0] << CC.ColorShift;
            GLint g = CC.Current.IntColor[1] << CC.ColorShift;
            GLint b = CC.Current.IntColor[2] << CC.ColorShift;
            GLint a = CC.Current.IntColor[3] << CC.ColorShift;
            GLint i, start = VB.Start;
            for (i=0;i<n;i++) {
               ASSIGN_4V( VB.Fcolor[start+i], r, g, b, a );
            }
         }

         /* assign polygon edgeflags */
         if (need_edges) {
            GLint i;
            for (i=0;i<n;i++) {
               VB.Edgeflag[VB.Start+i] = CC.Current.EdgeFlag;
            }
         }

         remaining -= n;

         VB.Count = VB.Start + n;
         gl_transform_vb_part2( remaining==0 ? GL_TRUE : GL_FALSE );

         vptr += n;
      }

      glEnd();
   }
   else if (!CC.CompileFlag
       && CC.Light.Enabled
       && !CC.Texture.Enabled
       && CC.Array.VertexEnabled && CC.Array.VertexType==GL_FLOAT
       && CC.Array.VertexStride==0 && CC.Array.VertexSize==4
       && CC.Array.NormalEnabled && CC.Array.NormalType==GL_FLOAT
       && CC.Array.NormalStride==0
       && !CC.Array.ColorEnabled
       && !CC.Array.IndexEnabled
       && !CC.Array.TexCoordEnabled
       && !CC.Array.EdgeFlagEnabled) {
      /*
       * SPECIAL CASE:  glNormal3fv();  glVertex4fv();  with lighting
       */
      GLfloat (*vptr)[4], (*nptr)[3];
      GLint remaining;

      glBegin( mode );

      remaining = count;
      vptr = (GLfloat (*)[4]) CC.Array.VertexPtr + 4 * first;
      nptr = (GLfloat (*)[3]) CC.Array.NormalPtr + 3 * first;
      while (remaining>0) {
         GLint vbspace, n;

         vbspace = VB_MAX - VB.Start;
         n = MIN2( vbspace, remaining );

         gl_xform_points_4fv( n, VB.Eye+VB.Start, CC.ModelViewMatrix, vptr );
         gl_xform_normals_3fv( n, VB.Normal+VB.Start, CC.ModelViewInv, nptr,
                               CC.Transform.Normalize );

         /* assign polygon edgeflags */
         if (need_edges) {
            GLint i;
            for (i=0;i<n;i++) {
               VB.Edgeflag[VB.Start+i] = CC.Current.EdgeFlag;
            }
         }

         remaining -= n;

         VB.Count = VB.Start + n;
         gl_transform_vb_part2( remaining==0 ? GL_TRUE : GL_FALSE );

         vptr += n;
         nptr += n;
      }

      glEnd();
   }
   else if (!CC.CompileFlag
       && CC.Light.Enabled
       && !CC.Texture.Enabled
       && CC.Array.VertexEnabled && CC.Array.VertexType==GL_FLOAT
       && CC.Array.VertexStride==0 && CC.Array.VertexSize==3
       && CC.Array.NormalEnabled && CC.Array.NormalType==GL_FLOAT
       && CC.Array.NormalStride==0
       && !CC.Array.ColorEnabled
       && !CC.Array.IndexEnabled
       && !CC.Array.TexCoordEnabled
       && !CC.Array.EdgeFlagEnabled) {
      /*
       * SPECIAL CASE:  glNormal3fv();  glVertex3fv();  with lighting
       */
      GLfloat (*vptr)[3], (*nptr)[3];
      GLint remaining;

      glBegin( mode );

      remaining = count;
      vptr = (GLfloat (*)[3]) CC.Array.VertexPtr + 3 * first;
      nptr = (GLfloat (*)[3]) CC.Array.NormalPtr + 3 * first;
      while (remaining>0) {
         GLint vbspace, n;

         vbspace = VB_MAX - VB.Start;
         n = MIN2( vbspace, remaining );

         gl_xform_points_3fv( n, VB.Eye+VB.Start, CC.ModelViewMatrix, vptr );
         gl_xform_normals_3fv( n, VB.Normal+VB.Start, CC.ModelViewInv, nptr,
                               CC.Transform.Normalize );

         /* assign polygon edgeflags */
         if (need_edges) {
            GLint i;
            for (i=0;i<n;i++) {
               VB.Edgeflag[VB.Start+i] = CC.Current.EdgeFlag;
            }
         }

         remaining -= n;

         VB.Count = VB.Start + n;
         gl_transform_vb_part2( remaining==0 ? GL_TRUE : GL_FALSE );

         vptr += n;
         nptr += n;
      }

      glEnd();
   }
   else {
      /*
       * GENERAL CASE:
       */
      glBegin( mode );
      for (i=0;i<count;i++) {
         glArrayElementEXT( first+i );
      }
      glEnd();
   }
}


