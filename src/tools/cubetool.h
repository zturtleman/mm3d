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


#ifndef __CUBETOOL_H
#define __CUBETOOL_H

#include "tool.h"
#include "cubetoolwidget.h"

#include <vector>

class CubeTool : public ::Tool, public CubeToolWidget::Observer
{
   public:
      CubeTool();
      virtual ~CubeTool();

      int getToolCount() { return 1; };
      const char * getName( int arg );
 
      bool isCreation() { return true; };

      void activated( int arg, Model * model, QMainWindow * mainwin );
      void deactivated();

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp(   Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );

      const char ** getPixmap();

      // Observer methods
      void setCubeValue( bool newValue );
      void setSegmentValue( int newValue );

   protected:
      bool m_tracking;
      Parent * m_parent;
      CubeToolWidget * m_widget;

      bool     m_isCube;
      unsigned m_segments;

      void updateVertexCoords( Parent *, double x1, double y1, double z1, 
            double x2, double y2, double z2 );

      ToolCoordList                 m_vertices;
      std::vector<int>              m_triangles;

      double m_x1;
      double m_y1;
      double m_z1;

      bool m_invertedNormals;
};

#endif // __CUBETOOL_H
