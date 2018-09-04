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


#ifndef __TORUSTOOL_H
#define __TORUSTOOL_H

#include "tool.h"
#include "torustoolwidget.h"
#include "toolpoly.h"

#include <list>

class TorusTool : public Tool, public TorusToolWidget::Observer
{
   public:
      TorusTool();
      virtual ~TorusTool();

      int getToolCount() { return 1; };
      const char * getName( int arg );

      void activated( int arg, Model * model, QMainWindow * mainwin );
      void deactivated();

      bool isCreation() { return true; };

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp( Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );

      const char ** getPixmap();

      // TorusToolWidget::Observer
      void setSegmentsValue( int newValue );
      void setSidesValue( int newValue );
      void setWidthValue( int newValue );
      void setCircleValue( bool newValue );
      void setCenterValue( bool o );

   protected:

      void updateDimensions( Tool::Parent * parent, 
            double xdiff, double ydiff, double zdiff );

      TorusToolWidget * m_widget;

      bool m_inverted;
      ToolCoordList m_vertices;

      unsigned m_segments;
      unsigned m_sides;
      unsigned m_width;

      bool   m_circle;
      bool   m_center;
      double m_startX;
      double m_startY;
      double m_diameter;
};

#endif // __TORUSTOOL_H
