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


#ifndef __SELECTFACETOOL_H
#define __SELECTFACETOOL_H

#include "tool.h"
#include "model.h"
#include "selectfacetoolwidget.h"

class BoundingBox;

class SelectFaceTool : public Tool, public SelectFaceToolWidget::Observer
{
   public:
      SelectFaceTool();
      virtual ~SelectFaceTool();

      int getToolCount() { return 1; };
      const char * getName( int arg );
      const char * getPath();

      bool getKeyBinding( int arg, int & keyBinding );
 
      bool isManipulation() { return true; };

      void activated( int arg, Model * model, Q3MainWindow * mainwin );
      void deactivated();

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp( Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );
      const char ** getPixmap();
      
      // Observer methods
      void setBackfacingValue( bool newValue );

   protected:
      BoundingBox * m_boundingBox;

      bool m_tracking;
      bool m_unselect;
      bool m_includeBackfacing;

      int m_startX;
      int m_startY;

      double m_x1;
      double m_y1;

      Model::SelectionModeE   m_selectionMode;

      Matrix m_mat;

      SelectFaceToolWidget * m_widget;
};

#endif // __SELECTFACETOOL_H
