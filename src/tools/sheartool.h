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


#ifndef __SHEARTOOL_H
#define __SHEARTOOL_H

#include "tool.h"
#include "model.h"

#include <list>
using std::list;

class ShearTool : public Tool
{
   public:
      ShearTool();
      virtual ~ShearTool();

      int getToolCount() { return 1; };
      const char * getName( int arg );

      bool getKeyBinding( int arg, int & keyBinding );
 
      bool isManipulation() { return true; };

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp( Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );
      const char ** getPixmap();

      double distance( const double & x1, const double & y1, const double & x2, const double & y2 );
      double min( double a, double b );
      double max( double a, double b );

   protected:
      double m_minX;
      double m_maxX;
      double m_maxY;
      double m_minZ;

      int m_axis;

      double m_far;
      double m_orig;

      double m_startLengthX;
      double m_startLengthY;

      ToolCoordList m_positionCoords;
};

#endif // __SHEARTOOL_H
