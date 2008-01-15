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


#ifndef __SCALETOOL_H
#define __SCALETOOL_H

#include "tool.h"
#include "model.h"

#include "scaletoolwidget.h"

#include <list>
using std::list;

class ScaleTool : public Tool, public ScaleToolWidget::Observer
{
   public:
      ScaleTool();
      virtual ~ScaleTool();

      int getToolCount() { return 1; };
      const char * getName( int arg );

      bool getKeyBinding( int arg, int & keyBinding );
 
      bool isManipulation() { return true; };

      void activated( int arg, Model * model, Q3MainWindow * mainwin );
      void deactivated();

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp( Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );
      const char ** getPixmap();

      double distance( double x1, double y1, double x2, double y2 );
      double min( double a, double b );
      double max( double a, double b );

      // Observer methods
      void setProportionValue( int newValue );
      void setPointValue( int newValue );

   protected:
      double m_x;
      double m_y;

      double m_minX;
      double m_maxX;
      double m_minY;
      double m_maxY;
      double m_minZ;
      double m_maxZ;

      double m_farX;
      double m_farY;
      double m_farZ;

      double m_centerX;
      double m_centerY;
      double m_centerZ;

      double m_startLengthX;
      double m_startLengthY;
      double m_startLengthZ;

      bool   m_allowX;
      bool   m_allowY;

      double m_projScale;
      std::list<int> m_projList;

      ToolCoordList m_positionCoords;

      int m_proportion;
      int m_point;
      ScaleToolWidget * m_widget;
};

#endif // __SCALETOOL_H
