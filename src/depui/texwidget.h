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


#ifndef __TEXWIDGET_H
#define __TEXWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>

#include <vector>
#include <list>

class Texture;
class Model;
class QTimer;

class TextureWidget : public QGLWidget
{
   Q_OBJECT

   public:

      enum _DrawMode_e
      {
         DM_Edit,
         DM_Edges,
         DM_Filled,
         DM_FilledEdges,
         DM_MAX
      };
      typedef enum _DrawMode_e DrawModeE;

      enum _MouseOperation_e
      {
         MouseMove,
         MouseSelect,
         MouseScale,
         MouseRotate,
         MouseRange
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

      struct _TextureVertex_t
      {
         double s;
         double t;
         bool   selected;
      };

      struct _RotateVertex_t
      {
         unsigned v;
         double x;
         double y;
      };

      typedef struct _TextureVertex_t TextureVertexT;
      typedef std::vector< TextureVertexT * > TextureVertexVector;

      typedef struct _RotateVertex_t RotateVertexT;
      typedef std::vector< RotateVertexT * > RotateVertexVector;

      struct _TextureTriangle_t
      {
         int vertex[3];
      };

      typedef struct _TextureTriangle_t TextureTriangleT;
      typedef std::vector< TextureTriangleT * > TextureTriangleVector;

      TextureWidget( QWidget * parent = NULL );
      virtual ~TextureWidget();

      void setModel( Model * model );
      void setTexture( int materialId, Texture * texture );

      void setInteractive( bool o );
      void set3d( bool o );

      void setTextureCount( unsigned c );
      void setSClamp( bool o ) { m_sClamp = o; setTexture( m_materialId, m_texture ); };
      void setTClamp( bool o ) { m_tClamp = o; setTexture( m_materialId, m_texture ); };

      void setDrawMode( DrawModeE dm );
      void setDrawVertices( bool dv ) { m_drawVertices = dv; };
      void setDrawBorder( bool db ) { m_drawBorder = db; };
      void setMouseOperation( MouseOperationE op );

      void setScaleKeepAspect( bool o ) { m_scaleKeepAspect = o; };
      void setScaleFromCenter( bool o ) { m_scaleFromCenter = o; };

      void setSolidBackground( bool o ) { m_solidBackground = o; };

      void vFlipCoordinates();
      void hFlipCoordinates();
      void rotateCoordinatesCcw();
      void rotateCoordinatesCw();

      int  addVertex( double t, double s );
      int  addTriangle( int v1, int v2, int v3 );

      void clearCoordinates();
      void getCoordinates( int tri, float * s, float * t );

      // This is min/max of the current viewport, not of the
      // vertices in the viewport
      double getMinViewCoord() { return m_xMin; };
      double getMaxViewCoord() { return m_xMax; };

      void setRange( double xMin, double yMin, double xMax, double yMax );
      void getRange( double & xMin, double & yMin, double & xMax, double & yMax );

      // paint my scene on another OpenGL widget
      void paintOnGlWidget( QGLWidget * w );

   public slots:
      void animationTimeout();

      void scrollUp();
      void scrollDown();
      void scrollLeft();
      void scrollRight();

      void zoomIn();
      void zoomOut();

      void scrollTimeout();

      void setZoomLevel( double zoom );

   signals:
      void updateCoordinatesSignal();
      void updateCoordinatesDoneSignal();
      void updateRangeSignal();
      void updateRangeDoneSignal();
      void updateSeamSignal( double xDiff, double yDiff );
      void updateSeamDoneSignal();
      void zoomLevelChanged( QString zoomStr );

   protected:
      virtual void mousePressEvent( QMouseEvent * e );
      virtual void mouseReleaseEvent( QMouseEvent * e );
      virtual void mouseMoveEvent( QMouseEvent * e );
      virtual void wheelEvent( QWheelEvent * e );
      virtual void keyPressEvent( QKeyEvent * e );

      void moveSelectedVertices( double x, double y );
      void updateSelectRegion( double x, double y );
      void startScale( double x, double y );
      void rotateSelectedVertices( double angle );
      void scaleSelectedVertices( double x, double y );

      void setViewportDraw();
      void setViewportOverlay();
      void drawTriangles();
      void drawOverlay();

      void makeTextureFromImage( const QImage & i, GLuint & t );

      void selectDone();
      void drawSelectBox();
      void drawRangeBox();
      void drawRotationPoint();
      void clearSelected();

      double getWindowXCoord( int x );
      double getWindowYCoord( int y );

      void updateCursorShape( int x, int y );
      void getDragDirections( double x, double y, 
            bool & all, bool & top, bool & bottom, bool & left, bool & right );
      void setDragCursor( bool all, bool top, bool bottom, bool left, bool right );

      void initializeGL();
      void paintGL();
      void resizeGL( int w, int h );

      void paintInternal();

      void updateViewport();

      void freeRotateVertices();

      double distance( const double &, const double &, const double &, const double & );
      double max( const double &, const double & );

      int m_viewportWidth;
      int m_viewportHeight;

      bool m_sClamp;
      bool m_tClamp;

      double m_zoom;

      double m_xCenter;
      double m_yCenter;

      int m_lastXPos;
      int m_lastYPos;

      Model * m_model;

      int       m_materialId;
      Texture * m_texture;
      GLuint m_glTexture;

      QTimer        * m_scrollTimer;
      ScrollButtonE m_overlayButton;
      GLuint m_scrollTextures[2];

      TextureVertexVector   m_vertices;
      TextureTriangleVector m_triangles;
      RotateVertexVector    m_rotateVertices;

      DrawModeE       m_drawMode;
      bool            m_drawVertices;
      bool            m_drawBorder;
      bool            m_solidBackground;
      MouseOperationE m_operation;

      bool m_scaleKeepAspect;
      bool m_scaleFromCenter;

      bool   m_selecting;
      bool   m_drawBounding;
      bool   m_drawRange;
      bool   m_interactive;
      bool   m_3d;

      int    m_button;
      
      // For 3d view
      QTimer * m_animTimer;

      double m_xMin;
      double m_xMax;
      double m_yMin;
      double m_yMax;

      // For select
      double m_xSel1;
      double m_ySel1;
      double m_xSel2;
      double m_ySel2;
      
      // For rotation
      double m_xRotPoint;
      double m_yRotPoint;
      double m_xRotStart;
      double m_yRotStart;
      double m_startAngle;

      // For projection range
      double m_xRangeMin;
      double m_yRangeMin;
      double m_xRangeMax;
      double m_yRangeMax;

      // For projection move/resize
      bool   m_dragAll;
      bool   m_dragTop;
      bool   m_dragBottom;
      bool   m_dragLeft;
      bool   m_dragRight;
      
      // For scale
      typedef struct _ScaleVertices_t
      {
         unsigned index;
         double x;
         double y;
      } ScaleVerticesT;

      typedef std::list<ScaleVerticesT> ScaleVerticesList;

      double m_farX;
      double m_farY;
      double m_centerX;
      double m_centerY;
      double m_startLengthX;
      double m_startLengthY;

      ScaleVerticesList m_scaleList;
};

#endif // __TEXWIDGET_H
