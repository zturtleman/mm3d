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


#ifndef __MVIEWPORT_H
#define __MVIEWPORT_H

#include <QtCore/QtGlobal>
#include <QtOpenGL/QGLWidget>
#include <QtGui/QWheelEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QImage>

#include "tool.h"
#include "decal.h"
#include "texture.h"

#include <list>
#include <string>

using std::list;

class Model;
class Decal;
class Toolbox;
class QTimer;

typedef list<Decal *> DecalList;

class ModelViewport : public QGLWidget, public Tool::Parent
{
   Q_OBJECT
   public:
      enum
      {
         MAX_VERTICAL_UNITS = 32767,
         MAX_HORIZONTAL_UNITS = 32767
      };

      enum _ViewOptions_e
      {
         ViewWireframe,
         ViewFlat,
         ViewSmooth,
         ViewTexture,
         ViewAlpha
      };
      typedef enum _ViewOptions_e ViewOptionsE;

      enum _MouseOperation_e
      {
         MO_None,
         MO_Tool,
         MO_Pan,
         MO_PanButton,
         MO_Rotate,
         MO_RotateButton,
      };
      typedef enum _MouseOperation_e MouseOperationE;

      enum _ScrollButton_e
      {
         ScrollButtonPan,
         ScrollButtonLeft,
         ScrollButtonRight,
         ScrollButtonUp,
         ScrollButtonDown,
         ScrollButtonMAX
      };
      typedef enum _ScrollButton_e ScrollButtonE;

      struct _ViewState_t
      {
         ViewDirectionE direction;
         double rotation[3];
         double translation[3];
         double zoom;
      };
      typedef struct _ViewState_t ViewStateT;


      ModelViewport( QWidget * parent = NULL );
      virtual ~ModelViewport();

      void freeTextures();
      double getZoomLevel() { return m_zoomLevel; };

      void frameArea( double x1, double y1, double z1, double x2, double y2, double z2 );
      void zoomIn();
      void zoomOut();

      void scrollUp();
      void scrollDown();
      void scrollLeft();
      void scrollRight();

      void rotateUp();
      void rotateDown();
      void rotateLeft();
      void rotateRight();
      void rotateClockwise();
      void rotateCounterClockwise();

      void rotateViewport( double x, double y, double z = 0.0 );

      void setModel( Model * model ) { m_model = model; };
      void setToolbox( Toolbox * toolbox ) { m_toolbox = toolbox; };
      
      int constructButtonState( QMouseEvent * e );

      // Tool::Parent methods

      Model * getModel() { return m_model; };
      ViewDirectionE getViewDirection() { return m_viewDirection; };
      void updateView();
      void update3dView();
      void updateAllViews() { emit modelUpdated(); };

      void getParentXYValue( int x, int y, double & xval, double & yval, bool selected );
      void getRawParentXYValue( int x, int y, double & xval, double & yval );
      const Matrix & getParentViewMatrix() const { return m_viewMatrix; };
      const Matrix & getParentViewInverseMatrix() const { return m_invMatrix; };

      bool getXValue( int x, int y, double * val );
      bool getYValue( int x, int y, double * val );
      bool getZValue( int x, int y, double * val );

      void addDecal( Decal * decal );
      void removeDecal( Decal * decal );

      void copyContentsToTexture( Texture * tex );
      void updateCaptureGL();
      
   signals:
      void zoomLevelChanged( const QString & zoomStr );
      void viewDirectionChanged( int dir );
      void modelUpdated();

      void viewportSaveState( int slotNumber, const ModelViewport::ViewStateT & viewState );
      void viewportRecallState( int slotNumber );

   public slots:

      void viewChangeEvent( int dir );
      void setZoomLevel( double zoomLevel );
      void setViewState( const ModelViewport::ViewStateT & viewState );

      void scrollTimeout();

   protected slots:
      void wheelEvent( QWheelEvent * e ) override;
      void mouseMoveEvent( QMouseEvent * e ) override;
      void mousePressEvent( QMouseEvent * e ) override;
      void mouseReleaseEvent( QMouseEvent * e ) override;
      void keyPressEvent( QKeyEvent * e ) override;
      void focusInEvent( QFocusEvent * e ) override;
      void focusOutEvent( QFocusEvent * e ) override;
      //void dragEnterEvent( QDragMoveEvent * e ) override;

   protected:
#if QT_VERSION < QT_VERSION_CHECK( 5, 6, 0 )
      qreal devicePixelRatioF() { return devicePixelRatio(); }
#endif

      void initializeGL() override;
      void paintGL() override;
      void resizeGL( int w, int h ) override;

      void checkGlErrors();

      void updateBackground();

      void adjustViewport();
      void setViewportDraw();
      void setViewportOverlay();

      void drawGridLines();
      void drawOrigin();
      void drawBackground();
      void drawOverlay();

      void makeTextureFromImage( const QImage & i, GLuint & t );

      double getUnitWidth();

      Model * m_model;

      MouseOperationE m_operation;
      int m_activeButton;

      ViewDirectionE m_viewDirection;

      Matrix m_viewMatrix;
      Matrix m_invMatrix;

      double m_centerX;
      double m_centerY;
      double m_centerZ;
      double m_arcballPoint[3];
      double m_rotX;
      double m_rotY;
      double m_rotZ;
      double m_width;
      double m_height;
      double m_zoomLevel;
      double m_unitWidth;
      double m_far;
      double m_near;
      double m_farOrtho;
      double m_nearOrtho;

      int m_viewportWidth;
      int m_viewportHeight;

      GLuint    m_backgroundTexture;
      Texture * m_texture;
      std::string m_backgroundFile;

      QTimer        * m_scrollTimer;
      bool            m_inOverlay;
      ScrollButtonE m_overlayButton;
      GLuint m_scrollTextures[2];

      bool m_capture;
      bool m_texturesLoaded;
      ViewOptionsE m_viewOptions;

      QPoint m_scrollStartPosition;
      QColor m_backColor;

      DecalList m_decals;

      Toolbox * m_toolbox;
};

#endif // __MVIEWPORT_H
