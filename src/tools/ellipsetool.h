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


#ifndef __ELLIPSOIDTOOL_H
#define __ELLIPSOIDTOOL_H

#include "tool.h"
#include "ellipsetoolwidget.h"

#include <list>

class EllipsoidTool : public Tool, public EllipsoidToolWidget::Observer
{
   public:
      EllipsoidTool();
      virtual ~EllipsoidTool();

      int getToolCount() { return 1; };
      const char * getName( int arg );

      void activated( int arg, Model * model, QMainWindow * mainwin );
      void deactivated();

      bool isCreation() { return true; };

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp( Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );

      const char ** getPixmap();

      // EllipsoidToolWidget::Observer
      void setSmoothnessValue( int newValue );
      void setSphere( bool o );
      void setCenter( bool o );

   protected:

      void updateVertexCoords( Tool::Parent *, double x, double y, double z, 
            double xrad, double yrad, double zrad );

      EllipsoidToolWidget * m_widget;
      unsigned m_smoothness;
      bool     m_isSphere;
      bool     m_fromCenter;

      ToolCoordList m_vertices;

      double m_startX;
      double m_startY;
};

#endif // __ELLIPSOIDTOOL_H
