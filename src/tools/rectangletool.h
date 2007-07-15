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


#ifndef __RECTANGLETOOL_H
#define __RECTANGLETOOL_H

#include "tool.h"

class RectangleTool : public Tool
{
   public:
      RectangleTool();
      virtual ~RectangleTool();

      int getToolCount() { return 1; };
      const char * getPath();
      const char * getName( int arg );
 
      bool isCreation() { return true; };

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp(   Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );

      const char ** getPixmap();

   protected:
      bool m_tracking;
      Parent * m_parent;

      ToolCoordT m_v1;
      ToolCoordT m_v2;
      ToolCoordT m_v3;
      ToolCoordT m_v4;

      double m_x1;
      double m_y1;
};

#endif // __RECTANGLETOOL_H
