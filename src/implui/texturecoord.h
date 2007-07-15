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

#include "qpixmap.h"
#include "texturecoord.base.h"

#include <list>
#include <map>

using std::list;
using std::map;

#include "mq3macro.h"
#include "model.h"

class QAccel;
class TextureWidget;

class TextureCoord : public TextureCoordBase, public Model::Observer
{
   Q_OBJECT
   public:
      TextureCoord( Model * model, QWidget * parent = NULL, const char * name = "" );
      ~TextureCoord();

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

      enum
      {
         HELP_ID = 0,
         UNDO_ID = 1,
         REDO_ID = 2,
         TOOL_SELECT_ID = 3,
         TOOL_MOVE_ID = 4,
         TOOL_ROTATE_ID = 5,
         TOOL_SCALE_ID = 6,
      };

      // Model::Observer methods
      void modelChanged( int changeBits );

   public slots:
      void show();
      void accelEvent( int );
      void setModel( Model * m );
      virtual void mapSchemeChangedEvent(int);
      virtual void mouseToolChangedEvent(int);
      virtual void resetClickedEvent();
      virtual void zoomLevelChangedEvent(QString);
      virtual void zoomChangeEvent();
      virtual void scaleSettingsChangedEvent();

      virtual void updateTextureCoordsEvent();
      virtual void updateDoneEvent();

      void zoomIn();
      void zoomOut();
      void undoEvent();
      void redoEvent();

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

      void mapTriangle();
      void mapQuad();
      void mapGroup( int direction );
      void clearTriangles();

      void useGroupCoordinates();

      int getDefaultDirection();

      void cancelMapChange();

      QAccel  * m_accel;
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
