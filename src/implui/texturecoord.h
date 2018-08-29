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


#ifndef __TEXTURECOORD_H
#define __TEXTURECOORD_H

#include "texturecoord.base.h"

#include "model.h"

#include <QtWidgets/QDialog>

#include <list>
#include <map>

using std::list;
using std::map;

class TextureWidget;

class TextureCoord : public QDialog, public Ui::TextureCoordBase, public Model::Observer
{
   Q_OBJECT
   public:
      TextureCoord( Model * model, QWidget * parent = NULL );
      virtual ~TextureCoord();

      enum MapSchemeTypes
      {
         MapSchemeTriangle = 0,
         MapSchemeQuad     = 1,
         MapSchemeGroup    = 2
      };

      enum ToolTypes
      {
         ToolSelect = 0,
         ToolMove   = 1,
         ToolScale  = 2
      };

      // Model::Observer methods
      void modelChanged( int changeBits );

   public slots:
      void show();
      void closeEvent( QCloseEvent * e );
      void helpNowEvent();
      void toolSelectEvent();
      void toolMoveEvent();
      void toolRotateEvent();
      void toolScaleEvent();
      void setModel( Model * m );
      virtual void resetClickedEvent();
      virtual void zoomLevelChangedEvent(QString);
      virtual void zoomChangeEvent();
      virtual void scaleSettingsChangedEvent();

      virtual void updateTextureCoordsEvent();
      virtual void updateSelectionDoneEvent();
      virtual void updateDoneEvent();

      void rotateCcwEvent();
      void rotateCwEvent();
      void vFlipEvent();
      void hFlipEvent();

      void selectionColorChangedEvent(int);
      void linesColorChangedEvent(int);

      void zoomIn();
      void zoomOut();
      void undoEvent();
      void redoEvent();

      void mapTriangle();
      void mapQuad();
      void mapGroupEvent();
      void mapGroup( int direction );

      void close();

   protected:
      struct _TextureTriangle_t
      {
         int m_triangleNum;
         int m_vertexNum[3];
         
      };
      typedef struct _TextureTriangle_t TextureTriangleT;

      void initWindow();
      void operationComplete( const char * opname );

      void clearTriangles();

      void useGroupCoordinates();

      int getDefaultDirection();

      void cancelMapChange();

      TextureWidget * m_textureWidget;
      Model * m_model;
      int     m_undoCount;
      int     m_redoCount;
      bool    m_inUndo;
      bool    m_ignoreChange;
      int     m_currentDirection;
      int     m_currentMapScheme;
      list<int> m_triangles;
      map<int,TextureTriangleT> m_textureTriangles;
      map<int,int> m_textureVertices;
};

#endif //  __TEXTURECOORD_H
