/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2007 Kevin Worcester
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 *
 *  See the COPYING file for full license text.
 */


#include "model.h"
#include "log.h"
#include "texture.h"

#include <math.h>

static void _defaultMaterial()
{
   float fval[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
   glMaterialfv( GL_FRONT, GL_AMBIENT,
         fval );
   fval[0] = fval[1] = fval[2] = 0.8f;
   glMaterialfv( GL_FRONT, GL_DIFFUSE, fval );
   fval[0] = fval[1] = fval[2] = 0.0f;
   glMaterialfv( GL_FRONT, GL_SPECULAR, fval );
   glMaterialfv( GL_FRONT, GL_EMISSION, fval );
   glMaterialf( GL_FRONT, GL_SHININESS, 0.0f );
}

static void _drawPointOrientation( bool selected, float scale, 
        const Matrix & m )
{
    float color = (selected) ? 0.9f : 0.7f;

    Vector v1;
    Vector v2;

    glBegin( GL_LINES );

    glColor3f( color, 0.0f, 0.0f );

    v1.setAll( 0.0, 0.0, 0.0 );
    v2.setAll( scale, 0.0, scale );

    v1.transform( m );
    v2.transform( m );

    glVertex3dv( v1.getVector() );
    glVertex3dv( v2.getVector() );

    v1.setAll( scale, 0.0, scale );
    v2.setAll( 0.0, 0.0, scale * 2 );

    v1.transform( m );
    v2.transform( m );

    glVertex3dv( v1.getVector() );
    glVertex3dv( v2.getVector() );

    glColor3f( 0.0f, color, 0.0f );

    v1.setAll( 0.0, 0.0, 0.0 );
    v2.setAll( 0.0, scale, scale );

    v1.transform( m );
    v2.transform( m );

    glVertex3dv( v1.getVector() );
    glVertex3dv( v2.getVector() );

    v1.transform( m );
    v2.transform( m );

    v1.setAll( 0.0, scale, scale );
    v2.setAll( 0.0, 0.0, scale * 2 );

    v1.transform( m );
    v2.transform( m );

    glVertex3dv( v1.getVector() );
    glVertex3dv( v2.getVector() );

    glColor3f( 0.0f, 0.0f, color );

    v1.setAll( 0.0, 0.0, 0.0 );
    v2.setAll( 0.0, 0.0, scale * 3 );

    v1.transform( m );
    v2.transform( m );

    glVertex3dv( v1.getVector() );
    glVertex3dv( v2.getVector() );

    glEnd(); // GL_LINES
}

static void _drawPointOrientation( bool selected, float scale, 
        const double * trans, const double * rot )
{
    Matrix m;
    m.setTranslation( trans );
    m.setRotation( rot );

    _drawPointOrientation( selected, scale, m );
}

static const int CYL_VERT_COUNT = 8;
static const int CYL_SEAM_VERT  = 2;

static double _cylinderVertices[ CYL_VERT_COUNT ][ 3 ] =
{
   {  0.30,  1.00,  0.00 },
   {  0.21,  1.00,  0.21 },
   {  0.00,  1.00,  0.30 },
   { -0.21,  1.00,  0.21 },
   { -0.30,  1.00,  0.00 },
   { -0.21,  1.00, -0.21 },
   {  0.00,  1.00, -0.30 },
   {  0.21,  1.00, -0.21 },
};

static void _drawProjectionCylinder( bool selected, float scale, 
        const Vector & pos, const Vector & up, const Vector & seam )
{
    float color = (selected) ? 0.75f : 0.55f;

    double topVerts[ CYL_VERT_COUNT ][ 3 ];
    double botVerts[ CYL_VERT_COUNT ][ 3 ];

    int i;
    int v;

    double left[3] = {0,0,0};
    double orig[3] = {0,0,0};
    calculate_normal( left, orig, (double *) up.getVector(), (double *) seam.getVector() );

    double len = mag3( up.getVector() );

    Vector seamVec = seam;
    seamVec.normalize3();
    seamVec.scale( len );

    for ( i = 0; i < 3; i++ )
    {
       left[i] *= len;
    }

    Matrix m;
    for ( i = 0; i < 3; i++ )
    {
       m.set( 0, i, left[i] );
       m.set( 1, i, up.get(i) );
       m.set( 2, i, seamVec.get(i) );
    }

    for ( v = 0; v < CYL_VERT_COUNT; v++ )
    {
       topVerts[ v ][ 0 ] =  _cylinderVertices[ v ][ 0 ];
       topVerts[ v ][ 1 ] =  _cylinderVertices[ v ][ 1 ];
       topVerts[ v ][ 2 ] =  _cylinderVertices[ v ][ 2 ];

       botVerts[ v ][ 0 ] =  _cylinderVertices[ v ][ 0 ];
       botVerts[ v ][ 1 ] = -_cylinderVertices[ v ][ 1 ];
       botVerts[ v ][ 2 ] =  _cylinderVertices[ v ][ 2 ];

       m.apply( topVerts[ v ] );
       m.apply( botVerts[ v ] );

       topVerts[v][ 0 ] += pos[ 0 ];
       topVerts[v][ 1 ] += pos[ 1 ];
       topVerts[v][ 2 ] += pos[ 2 ];

       botVerts[v][ 0 ] += pos[ 0 ];
       botVerts[v][ 1 ] += pos[ 1 ];
       botVerts[v][ 2 ] += pos[ 2 ];
    }

    glLineStipple( 1, 0xf1f1 );

    glEnable( GL_LINE_STIPPLE );
    glBegin( GL_LINES );

    glColor3f( 0.0f, color, 0.0f );

    for ( v = 0; v < CYL_VERT_COUNT; v++ )
    {
       int v2 = (v + 1) % CYL_VERT_COUNT;

       glVertex3dv( topVerts[v] );
       glVertex3dv( topVerts[v2] );

       glVertex3dv( botVerts[v] );
       glVertex3dv( botVerts[v2] );

       if ( v == CYL_SEAM_VERT )
       {
          glEnd();
          glLineWidth( 3.0 );
          glBegin( GL_LINES );
       }
       glVertex3dv( topVerts[v] );
       glVertex3dv( botVerts[v] );
       if ( v == CYL_SEAM_VERT )
       {
          glEnd();
          glLineWidth( 1.0 );
          glBegin( GL_LINES );
       }
    }

    glEnd(); // GL_LINES
    glDisable( GL_LINE_STIPPLE );
    glLineWidth( 1.0 );

    glBegin( GL_POINTS );
    glVertex3dv( topVerts[ CYL_SEAM_VERT ] );
    glVertex3dv( botVerts[ CYL_SEAM_VERT ] );
    glEnd();
}

static const int SPH_VERT_COUNT = 8;
static const int SPH_SEAM_VERT_TOP = 0;
static const int SPH_SEAM_VERT_BOT = 4;

static double _sphereXVertices[ SPH_VERT_COUNT ][ 3 ] =
{
   {  0.00,  1.00,  0.00 },
   {  0.00,  0.71,  0.71 },
   {  0.00,  0.00,  1.00 },
   {  0.00, -0.71,  0.71 },
   {  0.00, -1.00,  0.00 },
   {  0.00, -0.71, -0.71 },
   {  0.00,  0.00, -1.00 },
   {  0.00,  0.71, -0.71 },
};

static double _sphereYVertices[ SPH_VERT_COUNT ][ 3 ] =
{
   {  1.00,  0.00,  0.00 },
   {  0.71,  0.00,  0.71 },
   {  0.00,  0.00,  1.00 },
   { -0.71,  0.00,  0.71 },
   { -1.00,  0.00,  0.00 },
   { -0.71,  0.00, -0.71 },
   {  0.00,  0.00, -1.00 },
   {  0.71,  0.00, -0.71 },
};

static double _sphereZVertices[ SPH_VERT_COUNT ][ 3 ] =
{
   {  1.00,  0.00,  0.00 },
   {  0.71,  0.71,  0.00 },
   {  0.00,  1.00,  0.00 },
   { -0.71,  0.71,  0.00 },
   { -1.00,  0.00,  0.00 },
   { -0.71, -0.71,  0.00 },
   {  0.00, -1.00,  0.00 },
   {  0.71, -0.71,  0.00 },
};

static void _drawProjectionSphere( bool selected, float scale, 
        const Vector & pos, const Vector & up, const Vector & seam )
{
    float color = (selected) ? 0.75f : 0.55f;

    double xVerts[ SPH_VERT_COUNT ][ 3 ];
    double yVerts[ SPH_VERT_COUNT ][ 3 ];
    double zVerts[ SPH_VERT_COUNT ][ 3 ];

    int i;
    int v;

    double left[3] = {0,0,0};
    double orig[3] = {0,0,0};
    calculate_normal( left, orig, (double *) up.getVector(), (double *) seam.getVector() );

    Vector seamVec = seam;

    seamVec.normalize3();

    double len = mag3( (double *) up.getVector() );
    seamVec.scale( len );

    for ( i = 0; i < 3; i++ )
    {
       left[i] *= len;
    }

    Matrix m;

    for ( i = 0; i < 3; i++ )
    {
       m.set( 0, i, left[i] );
       m.set( 1, i, up.get(i) );
       m.set( 2, i, seamVec.get(i) );
    }

    for ( v = 0; v < SPH_VERT_COUNT; v++ )
    {
       xVerts[ v ][ 0 ] =  _sphereXVertices[ v ][ 0 ];
       xVerts[ v ][ 1 ] =  _sphereXVertices[ v ][ 1 ];
       xVerts[ v ][ 2 ] =  _sphereXVertices[ v ][ 2 ];

       yVerts[ v ][ 0 ] =  _sphereYVertices[ v ][ 0 ];
       yVerts[ v ][ 1 ] =  _sphereYVertices[ v ][ 1 ];
       yVerts[ v ][ 2 ] =  _sphereYVertices[ v ][ 2 ];

       zVerts[ v ][ 0 ] =  _sphereZVertices[ v ][ 0 ];
       zVerts[ v ][ 1 ] =  _sphereZVertices[ v ][ 1 ];
       zVerts[ v ][ 2 ] =  _sphereZVertices[ v ][ 2 ];

       m.apply( xVerts[ v ] );
       m.apply( yVerts[ v ] );
       m.apply( zVerts[ v ] );

       xVerts[v][ 0 ] += pos[ 0 ];
       xVerts[v][ 1 ] += pos[ 1 ];
       xVerts[v][ 2 ] += pos[ 2 ];

       yVerts[v][ 0 ] += pos[ 0 ];
       yVerts[v][ 1 ] += pos[ 1 ];
       yVerts[v][ 2 ] += pos[ 2 ];

       zVerts[v][ 0 ] += pos[ 0 ];
       zVerts[v][ 1 ] += pos[ 1 ];
       zVerts[v][ 2 ] += pos[ 2 ];
    }

    glLineStipple( 1, 0xf1f1 );

    glEnable( GL_LINE_STIPPLE );
    glBegin( GL_LINES );

    glColor3f( 0.0f, color, 0.0f );

    bool thick = false;

    for ( v = 0; v < SPH_VERT_COUNT; v++ )
    {
       int v2 = (v + 1) % SPH_VERT_COUNT;

       if ( v >= SPH_SEAM_VERT_TOP && v < SPH_SEAM_VERT_BOT )
       {
          glEnd();
          glLineWidth( 3.0 );
          glBegin( GL_LINES );
          thick = true;
       }
       glVertex3dv( xVerts[v] );
       glVertex3dv( xVerts[v2] );
       if ( thick )
       {
          glEnd();
          glLineWidth( 1.0 );
          glBegin( GL_LINES );
          thick = false;
       }

       glVertex3dv( yVerts[v] );
       glVertex3dv( yVerts[v2] );

       glVertex3dv( zVerts[v] );
       glVertex3dv( zVerts[v2] );
    }

    glEnd(); // GL_LINES
    glDisable( GL_LINE_STIPPLE );

    glBegin( GL_POINTS );
    glVertex3dv( xVerts[ SPH_SEAM_VERT_TOP ] );
    glVertex3dv( xVerts[ SPH_SEAM_VERT_BOT ] );
    glEnd();
}

static const int PLN_VERT_COUNT = 4;

static double _planeVertices[ PLN_VERT_COUNT ][ 3 ] =
{
   { -1.00, -1.00,  0.00 },
   {  1.00, -1.00,  0.00 },
   {  1.00,  1.00,  0.00 },
   { -1.00,  1.00,  0.00 },
};

static void _drawProjectionPlane( bool selected, float scale, 
        const Vector & pos, const Vector & up, const Vector & seam )
{
    float color = (selected) ? 0.75f : 0.55f;

    double verts[ PLN_VERT_COUNT ][ 3 ];

    int i;
    int v;

    double left[3] = {0,0,0};
    double orig[3] = {0,0,0};
    calculate_normal( left, orig, (double *) up.getVector(), (double *) seam.getVector() );

    Vector seamVec = seam;

    seamVec.normalize3();

    double len = mag3( (double *) up.getVector() );
    seamVec.scale( len );

    for ( i = 0; i < 3; i++ )
    {
       left[i] *= len;
    }

    Matrix m;

    for ( i = 0; i < 3; i++ )
    {
       m.set( 0, i, left[i] );
       m.set( 1, i, up.get(i) );
       m.set( 2, i, seamVec.get(i) );
    }

    for ( v = 0; v < PLN_VERT_COUNT; v++ )
    {
       verts[ v ][ 0 ] =  _planeVertices[ v ][ 0 ];
       verts[ v ][ 1 ] =  _planeVertices[ v ][ 1 ];
       verts[ v ][ 2 ] =  _planeVertices[ v ][ 2 ];

       m.apply( verts[ v ] );

       verts[v][ 0 ] += pos[ 0 ];
       verts[v][ 1 ] += pos[ 1 ];
       verts[v][ 2 ] += pos[ 2 ];
    }

    glLineStipple( 1, 0xf1f1 );

    glLineWidth( 3.0 );

    glEnable( GL_LINE_STIPPLE );
    glBegin( GL_LINES );

    glColor3f( 0.0f, color, 0.0f );

    for ( v = 0; v < PLN_VERT_COUNT; v++ )
    {
       int v2 = (v + 1) % PLN_VERT_COUNT;

       glVertex3dv( verts[v] );
       glVertex3dv( verts[v2] );
    }

    glEnd(); // GL_LINES
    glDisable( GL_LINE_STIPPLE );

    glLineWidth( 1.0 );
}

void Model::draw( unsigned drawOptions, ContextT context, float * viewPoint )
{
   bool colorSelected = false;

   if ( m_initialized )
   {
      if ( drawOptions & DO_WIREFRAME )
      {
         glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      }
      else
      {
#ifdef MM3D_EDIT
         if ( drawOptions & DO_BACKFACECULL )
         {
            glEnable( GL_CULL_FACE );
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
         }
         else
         {
            glDisable( GL_CULL_FACE );
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
         }
#else
         glPolygonMode( GL_FRONT, GL_FILL );
#endif
      }

      if ( !m_validNormals )
      {
         calculateNormals();
      }

      DrawingContext * drawContext = NULL;
      if ( context )
      {
         drawContext = getDrawingContext( context );

         if ( !drawContext->m_valid )
         {
            loadTextures( context );
            log_debug( "loaded textures for %p\n", context );
         }
      }

      if ( drawOptions & DO_ALPHA )
      {
         glEnable( GL_BLEND );
         if ( !m_validBspTree )
         {
            calculateBspTree();
         }
      }
      else
      {
         glDisable( GL_BLEND );
      }

      for ( unsigned t = 0; t < m_triangles.size(); t++ )
      {
         m_triangles[t]->m_marked = false;
      }

      glEnable( GL_LIGHT0 );
      glDisable( GL_LIGHT1 );
      _defaultMaterial();
      glColor3f( 0.9f, 0.9f, 0.9f );

      if ( m_animationMode )
      {
         glDisable( GL_BLEND ); // TODO: alpha blending on animations is not implemented
         if ( m_animationMode == ANIMMODE_SKELETAL && m_currentAnim < m_skelAnims.size() )
         {
            for ( unsigned m = 0; m < m_groups.size(); m++ )
            {
               Group * grp = m_groups[m];

               if ( drawOptions & DO_TEXTURE )
               {
                  glColor3f( 1.0f, 1.0f, 1.0f );
                  if ( grp->m_materialIndex >= 0 )
                  {
                     int index = grp->m_materialIndex;

                     glMaterialfv( GL_FRONT, GL_AMBIENT,
                           m_materials[ index ]->m_ambient );
                     glMaterialfv( GL_FRONT, GL_DIFFUSE,
                           m_materials[ index ]->m_diffuse );
                     glMaterialfv( GL_FRONT, GL_SPECULAR,
                           m_materials[ index ]->m_specular );
                     glMaterialfv( GL_FRONT, GL_EMISSION,
                           m_materials[ index ]->m_emissive );
                     glMaterialf( GL_FRONT, GL_SHININESS,
                           m_materials[ index ]->m_shininess );

                     if ( m_materials[ index ]->m_type == Model::Material::MATTYPE_TEXTURE
                           && (!m_materials[ index ]->m_textureData->m_isBad || (drawOptions & DO_BADTEX) ) )
                     {
                        if ( drawContext )
                        {
                           glBindTexture( GL_TEXTURE_2D,
                                 drawContext->m_matTextures[ grp->m_materialIndex ] );
                        }
                        else
                        {
                           glBindTexture( GL_TEXTURE_2D,
                                 m_materials[ grp->m_materialIndex ]->m_texture );
                        }

                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
                              (m_materials[ grp->m_materialIndex ]->m_sClamp ? GL_CLAMP : GL_REPEAT) );
                        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 
                              (m_materials[ grp->m_materialIndex ]->m_tClamp ? GL_CLAMP : GL_REPEAT) );

                        glEnable( GL_TEXTURE_2D );
                     }
                     else
                     {
                        glDisable( GL_TEXTURE_2D );
                     }
                  }
                  else
                  {
                     _defaultMaterial();
                     glDisable( GL_TEXTURE_2D );
                  }
               }
               else
               {
                  _defaultMaterial();
                  glDisable( GL_TEXTURE_2D );
                  glColor3f( 0.9f, 0.9f, 0.9f );
               }

               colorSelected = false;

               glBegin( GL_TRIANGLES );
               for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
                     it != grp->m_triangleIndices.end();
                     ++it )
               {
                  unsigned triIndex = *it;
                  Triangle * triangle = m_triangles[ triIndex ];
                  triangle->m_marked = true;

                  if ( triangle->m_visible )
                  {
                     if ( triangle->m_selected )
                     {
                        if ( colorSelected == false )
                        {
                           if ( !(drawOptions & DO_TEXTURE) )
                           {
                              glColor3f( 1.0, 0.0, 0.0 );
                           }
                        }
                        colorSelected = true;
                     }
                     else
                     {
                        if ( colorSelected == true )
                        {
                           if ( !(drawOptions & DO_TEXTURE) )
                           {
                              glColor3f( 0.9, 0.9, 0.9 );
                           }
                        }
                        colorSelected = false;
                     }

                     for ( int v = 0; v < 3; v++ )
                     {
                        Vertex * vertex = (m_vertices[ triangle->m_vertexIndices[v] ]);

                        glTexCoord2f( triangle->m_s[ v ], triangle->m_t[ v ] );

                        if ( drawOptions & DO_SMOOTHING )
                        {
                           glNormal3dv( triangle->m_normalSource[v] );
                        }
                        else
                        {
                           glNormal3dv( triangle->m_flatSource );
                        }

                        glVertex3dv( vertex->m_drawSource );
                     }
                  }
               }
               glEnd();
            }

            // draw ungrouped
            {
               _defaultMaterial();
               glDisable( GL_TEXTURE_2D );
               glBegin( GL_TRIANGLES );
               for ( unsigned t = 0; t < m_triangles.size(); t++ )
               {
                  if ( m_triangles[t]->m_marked == false  )
                  {
                     Triangle * triangle = m_triangles[ t ];
                     triangle->m_marked = true;

                     if ( triangle->m_visible )
                     {
                        if ( triangle->m_selected )
                        {
                           if ( colorSelected == false )
                           {
                              glColor3f( 1.0, 0.0, 0.0 );
                           }
                           colorSelected = true;
                        }
                        else
                        {
                           if ( colorSelected == true )
                           {
                              glColor3f( 0.9, 0.9, 0.9 );
                           }
                           colorSelected = false;
                        }

                        for ( int v = 0; v < 3; v++ )
                        {
                           Vertex * vertex = (m_vertices[ triangle->m_vertexIndices[v] ]);

                           glTexCoord2f( triangle->m_s[ v ], triangle->m_t[ v ] );

                           if ( drawOptions & DO_SMOOTHING )
                           {
                              glNormal3dv( triangle->m_normalSource[v] );
                           }
                           else
                           {
                              glNormal3dv( triangle->m_flatSource );
                           }

                           glVertex3dv( vertex->m_drawSource );
                        }
                     }
                  }
               }
               glEnd();
            }
         }
         else if ( m_animationMode == ANIMMODE_FRAME && m_currentAnim < m_frameAnims.size() )
         {
            //log_debug( "drawing animation '%s' frame %d\n", m_frameAnims[m_currentAnim]->m_name.c_str(), m_currentFrame );

            if ( m_currentFrame < m_frameAnims[m_currentAnim]->m_frameData.size() && m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices->size() > 0 )
            {
               for ( unsigned m = 0; m < m_groups.size(); m++ )
               {
                  Group * grp = m_groups[m];
                  if ( drawOptions & DO_TEXTURE )
                  {
                     glColor3f( 1.0, 1.0, 1.0 );

                     if ( grp->m_materialIndex >= 0 )
                     {
                        int index = grp->m_materialIndex;

                        glMaterialfv( GL_FRONT, GL_AMBIENT,
                              m_materials[ index ]->m_ambient );
                        glMaterialfv( GL_FRONT, GL_DIFFUSE,
                              m_materials[ index ]->m_diffuse );
                        glMaterialfv( GL_FRONT, GL_SPECULAR,
                              m_materials[ index ]->m_specular );
                        glMaterialfv( GL_FRONT, GL_EMISSION,
                              m_materials[ index ]->m_emissive );
                        glMaterialf( GL_FRONT, GL_SHININESS,
                              m_materials[ index ]->m_shininess );

                        if ( m_materials[ index ]->m_type == Model::Material::MATTYPE_TEXTURE
                              && (!m_materials[ index ]->m_textureData->m_isBad || (drawOptions & DO_BADTEX) ) )
                        {
                           if ( drawContext )
                           {
                              glBindTexture( GL_TEXTURE_2D,
                                    drawContext->m_matTextures[ grp->m_materialIndex ] );
                           }
                           else
                           {
                              glBindTexture( GL_TEXTURE_2D,
                                    m_materials[ grp->m_materialIndex ]->m_texture );
                           }

                           glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
                                 (m_materials[ grp->m_materialIndex ]->m_sClamp ? GL_CLAMP : GL_REPEAT) );
                           glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 
                                 (m_materials[ grp->m_materialIndex ]->m_tClamp ? GL_CLAMP : GL_REPEAT) );

                           glEnable( GL_TEXTURE_2D );
                        }
                        else
                        {
                           glDisable( GL_TEXTURE_2D );
                        }
                     }
                     else
                     {
                        _defaultMaterial();
                        glDisable( GL_TEXTURE_2D );
                     }
                  }
                  else
                  {
                     _defaultMaterial();
                     glColor3f( 0.9, 0.9, 0.9 );
                     glDisable( GL_TEXTURE_2D );
                  }

                  colorSelected = false;

                  glBegin( GL_TRIANGLES );
                  for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
                        it != grp->m_triangleIndices.end();
                        ++it )
                  {
                     Triangle * triangle = m_triangles[ *it ];
                     triangle->m_marked = true;

                     if ( triangle->m_visible )
                     {
                        if ( triangle->m_selected )
                        {
                           if ( colorSelected == false )
                           {
                              if ( !(drawOptions & DO_TEXTURE) )
                              {
                                 glColor3f( 1.0, 0.0, 0.0 );
                              }
                           }
                           colorSelected = true;
                        }
                        else
                        {
                           if ( colorSelected == true )
                           {
                              if ( !(drawOptions & DO_TEXTURE) )
                              {
                                 glColor3f( 0.9, 0.9, 0.9 );
                              }
                           }
                           colorSelected = false;
                        }

                        for ( int v = 0; v < 3; v++ )
                        {
                           FrameAnimVertex * vertex = ((*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[ triangle->m_vertexIndices[v] ]);

                           glTexCoord2f( triangle->m_s[ v ], triangle->m_t[ v ] );
                           glNormal3f( 
                                 vertex->m_normal[0], 
                                 vertex->m_normal[1], 
                                 vertex->m_normal[2] );
                           glVertex3f( 
                                 vertex->m_coord[0],
                                 vertex->m_coord[1],
                                 vertex->m_coord[2]
                                 );
                        }
                     }
                  }
                  glEnd();
               }

               // draw ungrouped
               {
                  _defaultMaterial();
                  glDisable( GL_TEXTURE_2D );
                  glBegin( GL_TRIANGLES );
                  for ( unsigned t = 0; t < m_triangles.size(); t++ )
                  {
                     if ( m_triangles[t]->m_marked == false  )
                     {
                        Triangle * triangle = m_triangles[ t ];
                        triangle->m_marked = true;

                        if ( triangle->m_visible )
                        {
                           if ( triangle->m_selected )
                           {
                              if ( colorSelected == false )
                              {
                                 glColor3f( 1.0, 0.0, 0.0 );
                              }
                              colorSelected = true;
                           }
                           else
                           {
                              if ( colorSelected == true )
                              {
                                 glColor3f( 0.9, 0.9, 0.9 );
                              }
                              colorSelected = false;
                           }

                           for ( int v = 0; v < 3; v++ )
                           {
                              FrameAnimVertex * vertex = ((*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[ triangle->m_vertexIndices[v] ]);

                              glNormal3f( 
                                    vertex->m_normal[0], 
                                    vertex->m_normal[1], 
                                    vertex->m_normal[2] );
                              glVertex3f( 
                                    vertex->m_coord[0],
                                    vertex->m_coord[1],
                                    vertex->m_coord[2]
                                    );
                           }
                        }
                     }
                  }
                  glEnd();
               }
            }
            else
            {
               log_error( "No frame, (or no vertices) for this animation frame.\n" );
            }
         }
      }
      else
      {
         bool skipAlphaGroup = false;
         for ( unsigned m = 0; m < m_groups.size(); m++ )
         {
            Group * grp = m_groups[m];
            skipAlphaGroup = false;

            if ( drawOptions & DO_TEXTURE )
            {
               glColor3f( 1.0, 1.0, 1.0 );
               if ( grp->m_materialIndex >= 0 )
               {
                  int index = grp->m_materialIndex;

                  if ( (drawOptions & DO_ALPHA) 
                        && m_materials[ index ]->m_type == Model::Material::MATTYPE_TEXTURE
                        && m_materials[ index ]->m_textureData->m_format == Texture::FORMAT_RGBA )
                  {
                     skipAlphaGroup = true;
                  }

                  glMaterialfv( GL_FRONT, GL_AMBIENT,
                        m_materials[ index ]->m_ambient );
                  glMaterialfv( GL_FRONT, GL_DIFFUSE,
                        m_materials[ index ]->m_diffuse );
                  glMaterialfv( GL_FRONT, GL_SPECULAR,
                        m_materials[ index ]->m_specular );
                  glMaterialfv( GL_FRONT, GL_EMISSION,
                        m_materials[ index ]->m_emissive );
                  glMaterialf( GL_FRONT, GL_SHININESS,
                        m_materials[ index ]->m_shininess );

                  if ( m_materials[ index ]->m_type == Model::Material::MATTYPE_TEXTURE
                        && (!m_materials[ index ]->m_textureData->m_isBad || (drawOptions & DO_BADTEX) ) )
                  {
                     if ( drawContext )
                     {
                        glBindTexture( GL_TEXTURE_2D,
                              drawContext->m_matTextures[ grp->m_materialIndex ] );
                     }
                     else
                     {
                        glBindTexture( GL_TEXTURE_2D,
                              m_materials[ grp->m_materialIndex ]->m_texture );
                     }

                     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
                           (m_materials[ grp->m_materialIndex ]->m_sClamp ? GL_CLAMP : GL_REPEAT) );
                     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 
                           (m_materials[ grp->m_materialIndex ]->m_tClamp ? GL_CLAMP : GL_REPEAT) );

                     glEnable( GL_TEXTURE_2D );
                  }
                  else
                  {
                     glDisable( GL_TEXTURE_2D );
                  }
               }
               else
               {
                  _defaultMaterial();

                  glDisable( GL_TEXTURE_2D );
                  glColor3f( 0.9, 0.9, 0.9 );
               }
            }
            else
            {
               _defaultMaterial();
            }

            colorSelected = false;

            if ( skipAlphaGroup == false )
            {
               glBegin( GL_TRIANGLES );
               for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
                     it != grp->m_triangleIndices.end();
                     ++it )
               {
                  Triangle * triangle = m_triangles[ *it ];
                  triangle->m_marked = true;

                  if ( triangle->m_visible )
                  {
                     if ( triangle->m_selected )
                     {
                        if ( colorSelected == false )
                        {
                           if ( !(drawOptions & DO_TEXTURE) )
                           {
                              glColor3f( 1.0, 0.0, 0.0 );
                           }
                           glEnd();
                           glDisable( GL_LIGHT0 );
                           glEnable( GL_LIGHT1 );
                           glBegin( GL_TRIANGLES );
                        }
                        colorSelected = true;
                     }
                     else
                     {
                        if ( colorSelected == true )
                        {
                           if ( !(drawOptions & DO_TEXTURE) )
                           {
                              glColor3f( 0.9, 0.9, 0.9 );
                           }
                        }
                        glEnd();
                        glDisable( GL_LIGHT1 );
                        glEnable( GL_LIGHT0 );
                        glBegin( GL_TRIANGLES );
                        colorSelected = false;
                     }

                     for ( int v = 0; v < 3; v++ )
                     {
                        Vertex * vertex = (m_vertices[ triangle->m_vertexIndices[v] ]);

                        glTexCoord2f( triangle->m_s[ v ], triangle->m_t[ v ] );
                        if ( (drawOptions & DO_SMOOTHING) )
                        {
                           glNormal3dv( triangle->m_finalNormals[v] );
                        }
                        else
                        {
                           glNormal3dv( triangle->m_flatNormals );
                        }
                        glVertex3dv( vertex->m_coord );
                     }
                  }
               }
               glEnd();
            }
            else
            {
               for ( std::set<int>::const_iterator it = grp->m_triangleIndices.begin();
                     it != grp->m_triangleIndices.end();
                     ++it )
               {
                  m_triangles[ *it ]->m_marked = true;
               }
            }
         }

         // Draw ungrouped triangles
         {
            _defaultMaterial();
            glDisable( GL_TEXTURE_2D );
            glBegin( GL_TRIANGLES );
            for ( unsigned t = 0; t < m_triangles.size(); t++ )
            {
               if ( m_triangles[t]->m_marked == false  )
               {
                  Triangle * triangle = m_triangles[ t ];
                  triangle->m_marked = true;

                  if ( triangle->m_visible )
                  {
                     if ( triangle->m_selected )
                     {
                        if ( colorSelected == false )
                        {
                           glColor3f( 1.0, 0.0, 0.0 );
                        }
                        glEnd();
                        glDisable( GL_LIGHT0 );
                        glEnable( GL_LIGHT1 );
                        glBegin( GL_TRIANGLES );
                        colorSelected = true;
                     }
                     else
                     {
                        if ( colorSelected == true )
                        {
                           glColor3f( 0.9, 0.9, 0.9 );
                        }
                        glEnd();
                        glDisable( GL_LIGHT1 );
                        glEnable( GL_LIGHT0 );
                        glBegin( GL_TRIANGLES );
                        colorSelected = false;
                     }

                     for ( int v = 0; v < 3; v++ )
                     {
                        Vertex * vertex = (m_vertices[ triangle->m_vertexIndices[v] ]);

                        if ( drawOptions & DO_SMOOTHING )
                        {
                           glNormal3f( 
                                 triangle->m_vertexNormals[v][0], 
                                 triangle->m_vertexNormals[v][1], 
                                 triangle->m_vertexNormals[v][2] );
                        }
                        else
                        {
                           glNormal3f( 
                                 triangle->m_flatNormals[0], 
                                 triangle->m_flatNormals[1], 
                                 triangle->m_flatNormals[2] );
                        }
                        glVertex3f( 
                              vertex->m_coord[0],
                              vertex->m_coord[1],
                              vertex->m_coord[2]
                              );
                     }
                  }
               }
            }
            glEnd();
         }

         // Draw depth-sorted alpha blended polys last
         if ( (drawOptions & DO_ALPHA) && viewPoint )
         {
            m_bspTree.render( viewPoint, drawContext );
         }

      }

      glDisable( GL_TEXTURE_2D );
   }
}

void Model::drawLines() // Used for orthographic projections
{
   bool colorSelected = false;

   glDisable( GL_TEXTURE_2D );

   if ( m_initialized )
   {
      if ( m_animationMode )
      {
         if ( m_animationMode == ANIMMODE_SKELETAL && m_currentAnim < m_skelAnims.size() )
         {
            glBegin( GL_LINES );
            glColor3f( 1.0, 1.0, 1.0 );
            for ( unsigned t = 0; t < m_triangles.size(); t++ )
            {
               Triangle * triangle = m_triangles[ t ];

               if ( triangle->m_visible )
               {
                  if ( triangle->m_selected )
                  {
                     if ( colorSelected == false )
                     {
                        glColor3f( 1.0, 0.0, 0.0 );
                     }
                     colorSelected = true;
                  }
                  else
                  {
                     if ( colorSelected == true )
                     {
                        glColor3f( 1.0, 1.0, 1.0 );
                     }
                     colorSelected = false;
                  }

                  double vertices[3][3];

                  for ( int v = 0; v < 3; v++ )
                  {
                     Vertex * vertex = (m_vertices[ triangle->m_vertexIndices[v] ]);

                     vertices[v][0] = vertex->m_drawSource[0];
                     vertices[v][1] = vertex->m_drawSource[1];
                     vertices[v][2] = vertex->m_drawSource[2];
                  }

                  glVertex3dv( vertices[0] );
                  glVertex3dv( vertices[1] );

                  glVertex3dv( vertices[1] );
                  glVertex3dv( vertices[2] );

                  glVertex3dv( vertices[2] );
                  glVertex3dv( vertices[0] );
               }
            }
            glEnd();
         }
         else if ( m_animationMode == ANIMMODE_FRAME && m_currentAnim < m_frameAnims.size() )
         {
            //log_debug( "drawing animation '%s' frame %d\n", m_frameAnims[m_currentAnim]->m_name.c_str(), m_currentFrame );

            if ( m_currentFrame < m_frameAnims[m_currentAnim]->m_frameData.size() && m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices->size() > 0 )
            {
               glBegin( GL_LINES );
               glColor3f( 1.0, 1.0, 1.0 );
               for ( unsigned t = 0; t < m_triangles.size(); t++ )
               {
                  Triangle * triangle = m_triangles[ t ];
                  triangle->m_marked = true;

                  if ( triangle->m_visible )
                  {
                     if ( triangle->m_selected )
                     {
                        if ( colorSelected == false )
                        {
                           glColor3f( 1.0, 0.0, 0.0 );
                        }
                        colorSelected = true;
                     }
                     else
                     {
                        if ( colorSelected == true )
                        {
                           glColor3f( 1.0, 1.0, 1.0 );
                        }
                        colorSelected = false;
                     }

                     double vertices[3][3];

                     for ( int v = 0; v < 3; v++ )
                     {
                        FrameAnimVertex * vertex = ((*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[ triangle->m_vertexIndices[v] ]);

                        vertices[v][0] = vertex->m_coord[0];
                        vertices[v][1] = vertex->m_coord[1];
                        vertices[v][2] = vertex->m_coord[2];
                     }

                     glVertex3dv( vertices[0] );
                     glVertex3dv( vertices[1] );

                     glVertex3dv( vertices[1] );
                     glVertex3dv( vertices[2] );

                     glVertex3dv( vertices[2] );
                     glVertex3dv( vertices[0] );
                  }
               }
               glEnd();
            }
            else
            {
               log_error( "No frame, (or no vertices) for this animation frame.\n" );
            }
         }
      }
      else
      {
         glLineWidth( 1.0 );
         glBegin( GL_LINES );
         glColor3f( 1.0, 1.0, 1.0 );
         for ( unsigned t = 0; t < m_triangles.size(); t++ )
         {
            Triangle * triangle = m_triangles[ t ];
            triangle->m_marked = true;

            if ( triangle->m_visible )
            {
               if ( triangle->m_selected )
               {
                  if ( colorSelected == false )
                  {
                     glEnd();
                     glLineWidth( 1.6 );
                     glBegin( GL_LINES );
                     glColor3f( 1.0, 0.0, 0.0 );
                  }
                  colorSelected = true;
               }
               else
               {
                  if ( colorSelected == true )
                  {
                     glEnd();
                     glLineWidth( 1.0 );
                     glBegin( GL_LINES );
                     glColor3f( 1.0, 1.0, 1.0 );
                  }
                  colorSelected = false;
               }

               double vertices[3][3];

               for ( int v = 0; v < 3; v++ )
               {
                  Vertex * vertex = (m_vertices[ triangle->m_vertexIndices[v] ]);

                  vertices[v][0] = vertex->m_coord[0];
                  vertices[v][1] = vertex->m_coord[1];
                  vertices[v][2] = vertex->m_coord[2];
               }

               glVertex3dv( vertices[0] );
               glVertex3dv( vertices[1] );

               glVertex3dv( vertices[1] );
               glVertex3dv( vertices[2] );

               glVertex3dv( vertices[2] );
               glVertex3dv( vertices[0] );
            }
         }
         glEnd();
      }
   }

   glLineWidth( 1.0 );
}

void Model::drawVertices()
{
   if ( m_initialized ) // && m_selectionMode == SelectVertices )
   {
      if ( m_animationMode )
      {
         // Note, in animation mode, we don't draw vertices for
         // skeletal animation, only for frame animations
         if ( m_animationMode == ANIMMODE_FRAME && m_currentAnim < m_frameAnims.size() )
         {
            if ( m_currentFrame < m_frameAnims[m_currentAnim]->m_frameData.size() && m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices->size() > 0 )
            {
               glPointSize( 3.0 );

               glBegin( GL_POINTS );

               glColor3f( 1.0, 1.0, 1.0 );
               for ( unsigned t = 0; t < m_vertices.size(); t++ )
               {
                  if ( m_vertices[t]->m_visible )
                  {
                     FrameAnimVertex * vertex = (*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[t];

                     if ( m_vertices[t]->m_selected == false )
                     {
                        glVertex3f( 
                              vertex->m_coord[0],
                              vertex->m_coord[1],
                              vertex->m_coord[2]
                              );
                     }
                  }
               }
               glEnd();

               glPointSize( 4.0 );

               glBegin( GL_POINTS );
               glColor3f( 1.0, 0.0, 0.0 );
               for ( unsigned t = 0; t < m_vertices.size(); t++ )
               {
                  if ( m_vertices[t]->m_visible )
                  {
                     FrameAnimVertex * vertex = (*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_frameVertices)[t];

                     if ( m_vertices[t]->m_selected )
                     {
                        glVertex3f( 
                              vertex->m_coord[0],
                              vertex->m_coord[1],
                              vertex->m_coord[2]
                              );
                     }
                  }
               }
               glEnd();
            }
         }
      }
      else
      {
         glPointSize( 3.0 );

         glBegin( GL_POINTS );

         glColor3f( 1.0, 1.0, 1.0 );
         for ( unsigned t = 0; t < m_vertices.size(); t++ )
         {
            if ( m_vertices[t]->m_visible )
            {
               Vertex * vertex = (m_vertices[ t ]);

               if ( vertex->m_selected == false )
               {
                  glVertex3f( 
                        vertex->m_coord[0],
                        vertex->m_coord[1],
                        vertex->m_coord[2]
                        );
               }
            }
         }
         glEnd();

         glPointSize( 4.0 );

         glBegin( GL_POINTS );
         glColor3f( 1.0, 0.0, 0.0 );
         for ( unsigned t = 0; t < m_vertices.size(); t++ )
         {
            if ( m_vertices[t]->m_visible )
            {
               Vertex * vertex = (m_vertices[ t ]);

               if ( vertex->m_selected )
               {
                  glVertex3f( 
                        vertex->m_coord[0],
                        vertex->m_coord[1],
                        vertex->m_coord[2]
                        );
               }
            }
         }
         glEnd();
      }
   }
}

#ifdef MM3D_EDIT

void Model::drawJoints()
{
   if ( m_drawJoints != JOINTMODE_NONE )
   {
      if ( m_initialized )
      {
         if ( m_animationMode == ANIMMODE_NONE || m_animationMode == ANIMMODE_SKELETAL )
         {
            glPointSize( 3.0 );

            glBegin( GL_LINES );
            for ( unsigned j = 0; j < m_joints.size(); j++ )
            {
               if ( m_joints[j]->m_visible && m_joints[j]->m_parent >= 0
                     && m_joints[ m_joints[j]->m_parent ]->m_visible )
               {
                  if ( m_animationMode && parentJointSelected( j ) )
                  {
                     glColor3f( 1.0, 0.0, 1.0 );
                  }
                  else
                  {
                     glColor3f( 0.0, 0.0, 1.0 );
                  }
                  Joint * joint  = m_joints[ j ];
                  Joint * parent = m_joints[ joint->m_parent ];

                  Vector pvec;
                  pvec.set( 0, parent->m_final.get(3, 0) );
                  pvec.set( 1, parent->m_final.get(3, 1) );
                  pvec.set( 2, parent->m_final.get(3, 2) );

                  Vector jvec;
                  jvec.set( 0, joint->m_final.get(3, 0) );
                  jvec.set( 1, joint->m_final.get(3, 1) );
                  jvec.set( 2, joint->m_final.get(3, 2) );

                  glVertex3f( 
                        pvec.get( 0 ),
                        pvec.get( 1 ),
                        pvec.get( 2 )
                        );
                  glVertex3f( 
                        jvec.get( 0 ),
                        jvec.get( 1 ),
                        jvec.get( 2 )
                        );

                  Vector face( 0.0, 1.0, 0.0 );
                  Vector point( jvec.get(0) - pvec.get(0), jvec.get(1) - pvec.get(1), jvec.get(2) - pvec.get(2) );

                  double length = distance( pvec, jvec );

                  Quaternion qrot;
                  qrot.setRotationToPoint( face, point );

                  Vector v1(  0.07,  0.10,  0.07 );
                  Vector v2( -0.07,  0.10,  0.07 );
                  Vector v3( -0.07,  0.10, -0.07 );
                  Vector v4(  0.07,  0.10, -0.07 );

                  v1 = v1 * length;
                  v2 = v2 * length;
                  v3 = v3 * length;
                  v4 = v4 * length;

                  Matrix m;
                  m.setRotationQuaternion( qrot );
                  v1 = v1 * m;
                  v2 = v2 * m;
                  v3 = v3 * m;
                  v4 = v4 * m;

                  if ( m_drawJoints == JOINTMODE_BONES )
                  {
                     glVertex3f( pvec.get( 0 ), pvec.get( 1 ), pvec.get( 2 ));
                     glVertex3f( pvec.get(0) + v1.get(0), pvec.get(1) + v1.get(1), pvec.get(2) + v1.get(2) );

                     glVertex3f( pvec.get( 0 ), pvec.get( 1 ), pvec.get( 2 ));
                     glVertex3f( pvec.get(0) + v2.get(0), pvec.get(1) + v2.get(1), pvec.get(2) + v2.get(2) );

                     glVertex3f( pvec.get( 0 ), pvec.get( 1 ), pvec.get( 2 ));
                     glVertex3f( pvec.get(0) + v3.get(0), pvec.get(1) + v3.get(1), pvec.get(2) + v3.get(2) );

                     glVertex3f( pvec.get( 0 ), pvec.get( 1 ), pvec.get( 2 ));
                     glVertex3f( pvec.get(0) + v4.get(0), pvec.get(1) + v4.get(1), pvec.get(2) + v4.get(2) );

                     glVertex3f( pvec.get(0) + v1.get(0), pvec.get(1) + v1.get(1), pvec.get(2) + v1.get(2) );
                     glVertex3f( pvec.get(0) + v2.get(0), pvec.get(1) + v2.get(1), pvec.get(2) + v2.get(2) );

                     glVertex3f( pvec.get(0) + v2.get(0), pvec.get(1) + v2.get(1), pvec.get(2) + v2.get(2) );
                     glVertex3f( pvec.get(0) + v3.get(0), pvec.get(1) + v3.get(1), pvec.get(2) + v3.get(2) );

                     glVertex3f( pvec.get(0) + v3.get(0), pvec.get(1) + v3.get(1), pvec.get(2) + v3.get(2) );
                     glVertex3f( pvec.get(0) + v4.get(0), pvec.get(1) + v4.get(1), pvec.get(2) + v4.get(2) );

                     glVertex3f( pvec.get(0) + v4.get(0), pvec.get(1) + v4.get(1), pvec.get(2) + v4.get(2) );
                     glVertex3f( pvec.get(0) + v1.get(0), pvec.get(1) + v1.get(1), pvec.get(2) + v1.get(2) );

                     glVertex3f( jvec.get( 0 ), jvec.get( 1 ), jvec.get( 2 ));
                     glVertex3f( pvec.get(0) + v1.get(0), pvec.get(1) + v1.get(1), pvec.get(2) + v1.get(2) );

                     glVertex3f( jvec.get( 0 ), jvec.get( 1 ), jvec.get( 2 ));
                     glVertex3f( pvec.get(0) + v2.get(0), pvec.get(1) + v2.get(1), pvec.get(2) + v2.get(2) );

                     glVertex3f( jvec.get( 0 ), jvec.get( 1 ), jvec.get( 2 ));
                     glVertex3f( pvec.get(0) + v3.get(0), pvec.get(1) + v3.get(1), pvec.get(2) + v3.get(2) );

                     glVertex3f( jvec.get( 0 ), jvec.get( 1 ), jvec.get( 2 ));
                     glVertex3f( pvec.get(0) + v4.get(0), pvec.get(1) + v4.get(1), pvec.get(2) + v4.get(2) );

                  }
               }
            }
            glEnd();

            glBegin( GL_POINTS );
            for ( unsigned j = 0; j < m_joints.size(); j++ )
            {
               if ( m_joints[j]->m_visible )
               {
                  if ( m_joints[j]->m_selected )
                  {
                     glColor3f( 1.0, 0.0, 1.0 );
                  }
                  else if ( (m_animationMode && hasSkelAnimKeyframe( m_currentAnim, m_currentFrame, j, true ))  
                        || (m_animationMode && hasSkelAnimKeyframe( m_currentAnim, m_currentFrame, j, false )) )
                  {
                     glColor3f( 0.0, 1.0, 0.0 );
                  }
                  else
                  {
                     glColor3f( 0.0, 0.0, 1.0 );
                  }
                  Joint * joint = (m_joints[ j ]);

                  glVertex3f( 
                        joint->m_final.get(3, 0),
                        joint->m_final.get(3, 1),
                        joint->m_final.get(3, 2)
                        );
               }
            }
            glEnd();
         }
      }
   }
}

void Model::drawPoints()
{
   float scale = 2.0f;
   if ( m_initialized )
   {
      switch ( m_animationMode )
      {
         case ANIMMODE_NONE:
            for ( unsigned p = 0; p < m_points.size(); p++ )
            {
               if ( m_points[p]->m_visible )
               {
                  Point * point = (m_points[ p ]);

                  _drawPointOrientation( point->m_selected, scale, 
                        point->m_trans, point->m_rot );

                  if ( point->m_selected )
                  {
                     glColor3f( 0.7, 1.0, 0.0 );
                  }
                  else
                  {
                     glColor3f( 0.0, 0.5, 0.0 );
                  }

                  glBegin( GL_POINTS );
                  glVertex3f( 
                        point->m_trans[0],
                        point->m_trans[1],
                        point->m_trans[2]
                        );
                  glEnd();
               }
            }
            break;

         case ANIMMODE_SKELETAL:
            for ( unsigned p = 0; p < m_points.size(); p++ )
            {
               if ( m_points[p]->m_visible )
               {
                  /*
                  Matrix mat;
                  Point * point = (m_points[ p ]);

                  mat.setTranslation( point->m_localTranslation );
                  mat.setRotation( point->m_localRotation );

                  int j = point->m_boneId;
                  if ( j >= 0 )
                  {
                     mat = mat * m_joints[j]->m_final;
                  }
                  */

                  Matrix mat;
                  Point * point = (m_points[ p ]);

                  mat.setTranslation( point->m_kfTrans );
                  mat.setRotation( point->m_kfRot );

                  _drawPointOrientation( false, scale, mat );

                  glColor3f( 0.0, 0.5, 0.0 );
                  glBegin( GL_POINTS );
                  glVertex3d(
                        mat.get(3, 0),
                        mat.get(3, 1),
                        mat.get(3, 2)
                        );
                  glEnd();
               }
            }
            break;

         case ANIMMODE_FRAME:
            if ( m_currentAnim < m_frameAnims.size() && m_currentFrame < m_frameAnims[m_currentAnim ]->m_frameData.size() )
            {
               for ( unsigned p = 0; p < m_points.size(); p++ )
               {
                  if ( m_points[p]->m_visible )
                  {
                     if ( p < m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_framePoints->size() )
                     {
                        FrameAnimPoint * point = 
                           ((*m_frameAnims[m_currentAnim]->m_frameData[m_currentFrame]->m_framePoints)[ p ]);

                        _drawPointOrientation( m_points[p]->m_selected, 1.0f,
                              point->m_trans, point->m_rot );

                        if ( m_points[p]->m_selected )
                        {
                           glColor3f( 0.7, 1.0, 0.0 );
                        }
                        else
                        {
                           glColor3f( 0.0, 0.5, 0.0 );
                        }

                        glBegin( GL_POINTS );
                        glVertex3f( 
                              point->m_trans[0],
                              point->m_trans[1],
                              point->m_trans[2]
                              );
                        glEnd();
                     }
                  }
               }
            }
            break;

         default:
            log_error( "unknown draw mode in drawPoints()\n" );
            break;
      }
   }
}

void Model::drawProjections()
{
   if ( m_drawProjections )
   {
      if ( m_initialized )
      {
         if ( m_animationMode == ANIMMODE_NONE )
         {
            for ( unsigned p = 0; p < m_projections.size(); p++ )
            {
               TextureProjection * proj = (m_projections[ p ]);

               float scale = mag3( proj->m_upVec );

               Matrix m;
               m.set( 0, 0, proj->m_upVec[0] );
               m.set( 1, 1, proj->m_upVec[1] );
               m.set( 2, 2, proj->m_upVec[2] );
               m.setTranslation( proj->m_pos[0], proj->m_pos[1], proj->m_pos[2] );

               switch ( proj->m_type )
               {
                  case Model::TPT_Sphere:
                     _drawProjectionSphere( proj->m_selected, scale, Vector(proj->m_pos), Vector(proj->m_upVec), Vector(proj->m_seamVec) );
                     break;
                  case Model::TPT_Cylinder:
                     _drawProjectionCylinder( proj->m_selected, scale, Vector(proj->m_pos), Vector(proj->m_upVec), Vector(proj->m_seamVec) );
                     break;
                  case Model::TPT_Plane:
                  default:
                     _drawProjectionPlane( proj->m_selected, scale, Vector(proj->m_pos), Vector(proj->m_upVec), Vector(proj->m_seamVec) );
                     break;
               }

               if ( proj->m_selected )
               {
                  glColor3f( 0.7, 1.0, 0.0 );
               }
               else
               {
                  glColor3f( 0.0, 0.5, 0.0 );
               }

               glBegin( GL_POINTS );
               glVertex3f( 
                     proj->m_pos[0],
                     proj->m_pos[1],
                     proj->m_pos[2]
                     );
               glEnd();
            }

         }
      }
   }
}

#endif // MM3D_EDIT


