/*  Maverick Model 3D
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


#include "texturecoord.h"

#include "textureframe.h"
#include "texwidget.h"
#include "model.h"
#include "mapdirection.h"
#include "log.h"
#include "decalmgr.h"
#include "helpwin.h"

#include "keycfg.h"
#include "3dmprefs.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLineEdit>
#include <QtGui/QPixmap>
#include <QtWidgets/QShortcut>

#include <math.h>

TextureCoord::TextureCoord( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_undoCount( 0 ),
     m_redoCount( 0 ),
     m_inUndo( false ),
     m_ignoreChange( false ),
     m_currentDirection( 0 ),
     m_currentMapScheme( 2 ) // if you change this, change the setChecked line below also
{
   setupUi( this );

   // TODO handle undo of select/unselect?
   // Can't do this until after constructor is done because of observer interface
   //setModel( m_model );
   m_textureFrame->setModel( model );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   QShortcut * undo = new QShortcut( QKeySequence( tr("CTRL+Z", "Undo shortcut")), this );
   connect( undo, SIGNAL(activated()), this, SLOT(undoEvent()) );
   QShortcut * redo = new QShortcut( QKeySequence( tr("CTRL+Y", "Redo shortcut")), this );
   connect( redo, SIGNAL(activated()), this, SLOT(redoEvent()) );

   QShortcut * select = new QShortcut( g_keyConfig.getKey( "tool_select_vertices" ), this );
   connect( select, SIGNAL(activated()), this, SLOT(toolSelectEvent()) );
   QShortcut * move = new QShortcut( g_keyConfig.getKey( "tool_move" ), this );
   connect( move, SIGNAL(activated()), this, SLOT(toolMoveEvent()) );
   QShortcut * rotate = new QShortcut( g_keyConfig.getKey( "tool_rotate" ), this );
   connect( rotate, SIGNAL(activated()), this, SLOT(toolRotateEvent()) );
   QShortcut * scale = new QShortcut( g_keyConfig.getKey( "tool_scale" ), this );
   connect( scale, SIGNAL(activated()), this, SLOT(toolScaleEvent()) );

   m_textureWidget = m_textureFrame->getTextureWidget();
   m_textureWidget->setInteractive( true );
   m_textureWidget->setDrawBorder( true );

   g_prefs.setDefault( "ui_texcoord_scale_aspect", 0 );
   g_prefs.setDefault( "ui_texcoord_scale_center", 1 );

   bool aspect = ( g_prefs( "ui_texcoord_scale_aspect" ).intValue() != 0 );
   bool center = ( g_prefs( "ui_texcoord_scale_center" ).intValue() != 0 );

   m_scaleAspect->setChecked( aspect );
   m_scaleCenter->setChecked( center );
   
   m_textureWidget->setScaleKeepAspect( m_scaleAspect->isChecked() );
   m_textureWidget->setScaleFromCenter( m_scaleCenter->isChecked() );

   connect( m_textureWidget, SIGNAL(updateCoordinatesSignal()), this, SLOT(updateTextureCoordsEvent()));
   connect( m_textureWidget, SIGNAL(updateSelectionDoneSignal()), this, SLOT(updateSelectionDoneEvent()));
   connect( m_textureWidget, SIGNAL(updateCoordinatesDoneSignal()), this, SLOT(updateDoneEvent()));
   connect( m_textureWidget, SIGNAL(zoomLevelChanged(QString)), this, SLOT(zoomLevelChangedEvent(QString)) );

   m_selectButton->setChecked( true );
   m_textureWidget->setMouseOperation( TextureWidget::MouseSelect );

   m_groupButton->setChecked( true );  // if you change this, change m_currentMapScheme also

   g_prefs.setDefault( "ui_texcoord_lines_color", 0xffffff );
   g_prefs.setDefault( "ui_texcoord_selection_color", 0xff0000 );

   uint32_t linesColor = g_prefs( "ui_texcoord_lines_color" ).intValue();
   uint32_t selectionColor = g_prefs( "ui_texcoord_selection_color" ).intValue();

   m_textureWidget->setLinesColor( linesColor );
   m_textureWidget->setSelectionColor( selectionColor );

   int linesIndex = 0;
   linesIndex |= (linesColor & 0x800000) ? 4 : 0 ;
   linesIndex |= (linesColor & 0x008000) ? 2 : 0 ;
   linesIndex |= (linesColor & 0x000080) ? 1 : 0 ;
   m_linesColor->setCurrentIndex( linesIndex );

   int selectionIndex = 0;
   selectionIndex |= (selectionColor & 0x800000) ? 4 : 0 ;
   selectionIndex |= (selectionColor & 0x008000) ? 2 : 0 ;
   selectionIndex |= (selectionColor & 0x000080) ? 1 : 0 ;
   m_selectionColor->setCurrentIndex( selectionIndex );
}

TextureCoord::~TextureCoord()
{
}

void TextureCoord::undoEvent()
{
   if ( m_undoCount > 0 )
   {
      m_inUndo = true;
      m_model->undo();
      m_undoCount--;
      m_inUndo = false;
      m_textureWidget->restoreSelectedUv();
      m_textureWidget->update();
   }
}

void TextureCoord::redoEvent()
{
   if ( m_undoCount < m_redoCount )
   {
      m_inUndo = true;
      m_model->redo();
      m_undoCount++;
      m_inUndo = false;
      m_textureWidget->update();
   }
}

void TextureCoord::operationComplete( const char * opname )
{
   m_undoCount++;
   m_redoCount = m_undoCount;

   m_ignoreChange = true;
   m_model->operationComplete( opname );
   m_ignoreChange = false;
}

void TextureCoord::show()
{
   setModel( m_model );
   
   if ( !isVisible() )
   {
      // If we are visible, setModel already did this
      initWindow();
   }
   QDialog::show();
}

void TextureCoord::initWindow()
{
   m_textureWidget->clearCoordinates();

   bool foundTexture = false;

   list<int> trilist;
   m_model->getSelectedTriangles( trilist );

   list<int>::iterator it;
   for ( it = trilist.begin(); !foundTexture && it != trilist.end(); it++ )
   {
      int g = m_model->getTriangleGroup( trilist.front() );
      int m = m_model->getGroupTextureId( g );
      if ( m >= 0 )
      {
         // FIXME cache current texture value and don't change it if we 
         // don't have to (to prevent resetting zoom and center)
         m_textureFrame->textureChangedEvent( m + 1 );
         foundTexture = true;
      }
   }

   if ( !foundTexture )
   {
      log_error( "no group selected\n" );
   }

   useGroupCoordinates();
   DecalManager::getInstance()->modelUpdated( m_model );

   if ( m_inUndo )
      m_textureWidget->restoreSelectedUv();
   else
      m_textureWidget->saveSelectedUv();
   m_textureWidget->update();
}

void TextureCoord::setModel( Model * model )
{
   m_undoCount = 0;
   m_redoCount = 0;

   if ( model != m_model )
   {
      model->addObserver( this );
   }

   m_model = model;
   m_textureFrame->setModel( model );

   if ( isVisible() )
   {
      initWindow();
   }
}

void TextureCoord::closeEvent( QCloseEvent * e )
{
   e->ignore();
   hide();
}

void TextureCoord::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_texturecoordwin.html", true );
   win->show();
}

void TextureCoord::toolSelectEvent()
{
   m_selectButton->setChecked( true );
   m_textureWidget->setMouseOperation( TextureWidget::MouseSelect );
   m_textureWidget->update();
}

void TextureCoord::toolMoveEvent()
{
   m_moveButton->setChecked( true );
   m_textureWidget->setMouseOperation( TextureWidget::MouseMove );
   m_textureWidget->update();
}

void TextureCoord::toolRotateEvent()
{
   m_rotateButton->setChecked( true );
   m_textureWidget->setMouseOperation( TextureWidget::MouseRotate );
   m_textureWidget->update();
}

void TextureCoord::toolScaleEvent()
{
   m_scaleButton->setChecked( true );
   m_textureWidget->setMouseOperation( TextureWidget::MouseScale );
   m_textureWidget->update();
}

void TextureCoord::mapGroupEvent()
{
   if ( m_currentMapScheme == MapSchemeGroup )
   {
      return;
   }

   MapDirection dir(this);
   dir.setMapDirection( getDefaultDirection() );
   if ( dir.exec() )
   {
      mapGroup( dir.getMapDirection() );
   }
   else
   {
      cancelMapChange();
      return;
   }
   m_textureWidget->update();
}

void TextureCoord::resetClickedEvent()
{
   if ( m_currentMapScheme == MapSchemeGroup )
   {
      MapDirection dir(this);

      if ( dir.exec() )
      {
         mapGroup( dir.getMapDirection() );
         m_textureWidget->update();
         log_debug( "reset texture coordinates and map from direction %d\n", dir.getMapDirection() );
      }
   }
   else
   {
      if ( QMessageBox::Ok == QMessageBox::warning( this, tr("Reset coordinates?", "window title"), 
            tr("Are you sure you want to reset texture coordinates for this group?"),
            QMessageBox::Ok, QMessageBox::Cancel ) )
      {
         log_debug( "reset texture coordinates\n" );
         switch ( m_currentMapScheme )
         {
            case MapSchemeTriangle:
               mapTriangle();
               break;
            case MapSchemeQuad:
               mapQuad();
               break;
            default:
               break;
         }
      }
   }
}

void TextureCoord::updateDoneEvent()
{
   operationComplete( tr("Move texture coordinates").toUtf8() );
}

void TextureCoord::updateSelectionDoneEvent()
{
   m_textureWidget->saveSelectedUv();
   operationComplete( tr("Select texture coordinates").toUtf8() );
}

void TextureCoord::updateTextureCoordsEvent()
{
   list<int> trilist;
   m_model->getSelectedTriangles( trilist );
   if ( trilist.size() > 0 )
   {
      float s[3];
      float t[3];

      if ( m_currentMapScheme == 2 )
      {
         list<int>::iterator it;
         for( it = trilist.begin(); it != trilist.end(); it++ )
         {
            if ( m_model->isTriangleSelected( *it ) )
            {
               m_textureWidget->getCoordinates( 
                     m_textureTriangles[ *it ].m_triangleNum, s, t );

               for ( int v = 0; v < 3; v++ )
               {
                  m_model->setTextureCoords( *it, v, s[v], t[v] );
               }
            }
         }
      }
      else
      {
         list<int>::iterator it;
         list<int>::iterator texIt = m_triangles.begin();
         for( it = trilist.begin(); it != trilist.end(); it++ )
         {
            if ( m_model->isTriangleSelected( *it ) )
            {
               if ( texIt == m_triangles.end() )
               {
                  texIt = m_triangles.begin();
               }

               m_textureWidget->getCoordinates( *texIt, s, t );
               for ( int v = 0; v < 3; v++ )
               {
                  m_model->setTextureCoords( *it, v, s[v], t[v] );
               }

               texIt++;
            }
         }
      }

      DecalManager::getInstance()->modelUpdated( m_model );
   }
   else
   {
      log_error( "no group selected\n" );
   }
}

void TextureCoord::scaleSettingsChangedEvent()
{
   m_textureWidget->setScaleKeepAspect( m_scaleAspect->isChecked() );
   m_textureWidget->setScaleFromCenter( m_scaleCenter->isChecked() );

   g_prefs( "ui_texcoord_scale_aspect" ) = m_scaleAspect->isChecked() ? 1 : 0;
   g_prefs( "ui_texcoord_scale_center" ) = m_scaleCenter->isChecked() ? 1 : 0;
}

void TextureCoord::rotateCcwEvent()
{
   m_textureWidget->rotateCoordinatesCcw();
   updateTextureCoordsEvent();
   updateDoneEvent();
}

void TextureCoord::rotateCwEvent()
{
   m_textureWidget->rotateCoordinatesCw();
   updateTextureCoordsEvent();
   updateDoneEvent();
}

void TextureCoord::vFlipEvent()
{
   m_textureWidget->vFlipCoordinates();
   updateTextureCoordsEvent();
   updateDoneEvent();
}

void TextureCoord::hFlipEvent()
{
   m_textureWidget->hFlipCoordinates();
   updateTextureCoordsEvent();
   updateDoneEvent();
}

void TextureCoord::selectionColorChangedEvent( int newColor )
{
   uint32_t rgb = 0;
   rgb |= (newColor & 1) ? 0x0000ff : 0;
   rgb |= (newColor & 2) ? 0x00ff00 : 0;
   rgb |= (newColor & 4) ? 0xff0000 : 0;
   m_textureWidget->setSelectionColor( rgb );
   m_textureWidget->update();
   g_prefs( "ui_texcoord_selection_color" ) = (int) rgb;
}

void TextureCoord::linesColorChangedEvent( int newColor )
{
   uint32_t rgb = 0;
   rgb |= (newColor & 1) ? 0x0000ff : 0;
   rgb |= (newColor & 2) ? 0x00ff00 : 0;
   rgb |= (newColor & 4) ? 0xff0000 : 0;
   m_textureWidget->setLinesColor( rgb );
   m_textureWidget->update();
   g_prefs( "ui_texcoord_lines_color" ) = (int) rgb;
}

void TextureCoord::zoomIn()
{
   m_textureWidget->zoomIn();
}

void TextureCoord::zoomOut()
{
   m_textureWidget->zoomOut();
}

void TextureCoord::zoomChangeEvent()
{
   double zoom = m_zoomInput->text().toDouble();
   if ( zoom < 0.00001 )
   {
      zoom = 1;
   }
   m_textureWidget->setZoomLevel( zoom );
}

void TextureCoord::zoomLevelChangedEvent( QString zoomStr )
{
   m_zoomInput->setText( zoomStr );
}

void TextureCoord::mapTriangle()
{
   if ( MapSchemeTriangle == m_currentMapScheme )
   {
      return;
   }

   m_currentMapScheme = MapSchemeTriangle;

   m_textureWidget->clearCoordinates();
   clearTriangles();

   double min = 0.0; //m_textureWidget->getMinViewCoord();
   double max = 1.0; //m_textureWidget->getMaxViewCoord();

   int v1 = m_textureWidget->addVertex( min, max );
   int v2 = m_textureWidget->addVertex( min, min );
   int v3 = m_textureWidget->addVertex( max, min );

   m_triangles.push_back( m_textureWidget->addTriangle( v1, v2, v3 ) );
   updateTextureCoordsEvent();
   updateDoneEvent();
   m_textureWidget->update();
}

void TextureCoord::mapQuad()
{
   if ( MapSchemeQuad == m_currentMapScheme )
   {
      return;
   }

   m_currentMapScheme = MapSchemeQuad;

   m_textureWidget->clearCoordinates();
   clearTriangles();

   double min = 0.0; //m_textureWidget->getMinViewCoord();
   double max = 1.0; //m_textureWidget->getMaxViewCoord();

   int v1 = m_textureWidget->addVertex( min, max );
   int v2 = m_textureWidget->addVertex( max, max );
   int v3 = m_textureWidget->addVertex( min, min );
   int v4 = m_textureWidget->addVertex( max, min );

   m_triangles.push_back( m_textureWidget->addTriangle( v4, v2, v1 ) );
   m_triangles.push_back( m_textureWidget->addTriangle( v1, v3, v4 ) );
   updateTextureCoordsEvent();
   updateDoneEvent();
   m_textureWidget->update();
}

void TextureCoord::mapGroup( int direction )
{
   log_debug( "mapGroup( %d )\n", direction );

   m_currentMapScheme = MapSchemeGroup;

   double range = 1.0; //m_textureWidget->getMaxViewCoord() - m_textureWidget->getMinViewCoord();
   double xOff  = 0.0; //m_textureWidget->getMinViewCoord();
   double yOff  = xOff;

   m_textureWidget->clearCoordinates();

   clearTriangles();

   m_textureTriangles.clear();
   m_textureVertices.clear();

   bool setBounds = true;

   double xMin = 0.0;
   double yMin = 0.0;
   double xMax = 0.0;
   double yMax = 0.0;

   double coord[2];

   list<int> trilist;
   m_model->getSelectedTriangles( trilist );
   if ( trilist.size() > 0 )
   {
      list<int>::iterator it;

      for( it = trilist.begin(); it != trilist.end(); it++ )
      {
         if ( m_model->isTriangleSelected( *it ) )
         {
            for ( int t = 0; t < 3; t++ )
            {
               int v = m_model->getTriangleVertex( *it, t );
               m_model->getVertexCoords2d( v, 
                     static_cast<Model::ProjectionDirectionE> (direction+1), coord );

               if ( setBounds )
               {
                  xMin = xMax = coord[0];
                  yMin = yMax = coord[1];
                  setBounds = false;
               }
               else
               {
                  if ( coord[0] < xMin )
                  {
                     xMin = coord[0];
                  }
                  else if ( coord[0] > xMax )
                  {
                     xMax = coord[0];
                  }

                  if ( coord[1] < yMin )
                  {
                     yMin = coord[1];
                  }
                  else if ( coord[1] > yMax )
                  {
                     yMax = coord[1];
                  }
               }
            }
         }
      }

      log_debug( "Bounds = (%f,%f) - (%f,%f)\n", xMin, yMin, xMax, yMax );

      for( it = trilist.begin(); it != trilist.end(); it++ )
      {
         if ( m_model->isTriangleSelected( *it ) )
         {
            int t;
            int vert[3];
            for ( t = 0; t < 3; t++ )
            {
               int v = m_model->getTriangleVertex( *it, t );
               m_model->getVertexCoords2d( v, 
                     static_cast<Model::ProjectionDirectionE> (direction+1), coord );

               if ( m_textureVertices.find( v ) != m_textureVertices.end() )
               {
                  vert[t] = m_textureVertices[ v ];
               }
               else
               {
                  vert[t] = m_textureWidget->addVertex( 
                        (((coord[0] - xMin) / (xMax - xMin)) * range) + xOff, 
                        (((coord[1] - yMin) / (yMax - yMin)) * range) + yOff);
                  m_textureVertices[ v ] = vert[t];
               }
            }

            int tri = m_textureWidget->addTriangle( vert[0], vert[1], vert[2] );

            TextureTriangleT texTri;
            texTri.m_triangleNum = tri;
            for ( t = 0; t < 3; t++ )
            {
               texTri.m_vertexNum[t] = vert[t];
            }
            m_textureTriangles[ *it ] = texTri;

            m_triangles.push_back( tri );
         }
      }
   }
   else
   {
      log_error( "no triangles selected\n" );
   }

   updateTextureCoordsEvent();
   updateDoneEvent();
   m_textureWidget->update();
}

void TextureCoord::clearTriangles()
{
   while ( m_triangles.size() )
   {
      m_triangles.pop_front();
   }
}

void TextureCoord::close()
{
   hide();
}

void TextureCoord::useGroupCoordinates()
{
   m_textureTriangles.clear();
   m_textureVertices.clear();

   float m = 0.0;

   list<int> trilist;
   m_model->getSelectedTriangles( trilist );
   if ( trilist.size() > 0 )
   {
      list<int>::iterator it;

      for( it = trilist.begin(); it != trilist.end(); it++ )
      {
         if ( m_model->isTriangleSelected( *it ) )
         {
            float tCoord;
            float sCoord;

            int vert[3];
            int t;

            for ( t = 0; t < 3; t++ )
            {
               m_model->getTextureCoords( *it, t, sCoord, tCoord );

               vert[t] = m_textureWidget->addVertex( sCoord, tCoord );

               sCoord = fabs( sCoord );
               tCoord = fabs( tCoord );

               sCoord = ( sCoord > tCoord ) ? sCoord : tCoord;

               if ( sCoord > m )
               {
                  m = sCoord;
               }
            }
            int tri = m_textureWidget->addTriangle( vert[0], vert[1], vert[2] );


            TextureTriangleT texTri;
            texTri.m_triangleNum = tri;
            for ( t = 0; t < 3; t++ )
            {
               texTri.m_vertexNum[t] = vert[t];
            }
            m_textureTriangles[ *it ] = texTri;

            m_triangles.push_back( tri );
         }
      }

      log_debug( "max texture coordinate value is %f\n", m );
   }
   else
   {
      log_error( "no group selected\n" );
   }
}

int TextureCoord::getDefaultDirection()
{
   list<int> trilist;
   m_model->getSelectedTriangles( trilist );
   if ( trilist.size() > 0 )
   {
      list<int>::iterator it;

      float total[3];
      float normal[3];
      int t;

      for ( t = 0; t < 3; t++ )
      {
         total[t] = 0;
      }

      for( it = trilist.begin(); it != trilist.end(); it++ )
      {
         if ( m_model->isTriangleSelected( *it ) )
         {
            for ( t = 0; t < 3; t++ )
            {
               m_model->getNormal( *it, t, normal );
               for ( int v = 0; v < 3; v++ )
               {
                  total[v] += normal[v];
               }
            }
         }
      }

      int index = 0;
      for ( t = 1; t < 3; t++ )
      {
         if ( fabs(total[t]) > fabs(total[index]) )
         {
            index = t;
         }
      }

      switch( index )
      {
         case 0:
            if ( total[index] >= 0.0 )
            {
               return 3;
            }
            else
            {
               return 2;
            }
            break;
         case 1:
            if ( total[index] >= 0.0 )
            {
               return 4;
            }
            else
            {
               return 5;
            }
            break;
         case 2:
            if ( total[index] >= 0.0 )
            {
               return 0;
            }
            else
            {
               return 1;
            }
            break;
         default:
            log_error( "bad normal index: %d\n", index );
            return 0;
      }
   }
   else
   {
      log_error( "No group selected\n" );
   }

   return 0;
}

void TextureCoord::cancelMapChange()
{
   switch ( m_currentMapScheme )
   {
      case MapSchemeTriangle:
         m_triangleButton->setChecked( true );
         break;
      case MapSchemeQuad:
         m_quadButton->setChecked( true );
         break;
      case MapSchemeGroup:
         m_groupButton->setChecked( true );
         break;
      default:
         break;
   }
}

void TextureCoord::modelChanged( int changeBits )
{
   if ( !m_ignoreChange )
   {
      if ( !m_inUndo )
      {
         m_undoCount = 0;
         m_redoCount = 0;
      }

      if ( isVisible() )
      {
         initWindow();
      }
   }
}

