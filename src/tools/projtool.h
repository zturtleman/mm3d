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


#ifndef __PROJTOOL_H
#define __PROJTOOL_H

#include "tool.h"

#include "projtoolwidget.h"

class ProjectionTool : public Tool, public ProjToolWidget::Observer
{
   public:
      ProjectionTool();
      virtual ~ProjectionTool();

      int getToolCount() { return 1; };
      const char * getPath();
      const char * getName( int arg );

      bool isCreation() { return true; };

      void activated( int arg, Model * model, Q3MainWindow * mainwin );
      void deactivated();

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp(   Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );

      const char ** getPixmap();

      // Observer methods
      void setTypeValue( int newValue );

   protected:

      ToolCoordT m_proj;
      double m_orig[3];
      int    m_x;
      int    m_y;
      bool   m_allowX;
      bool   m_allowY;
      int    m_type;
      ProjToolWidget * m_widget;
};

#endif // __PROJTOOL_H
