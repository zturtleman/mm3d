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


#ifndef __DRAGVERTEXTOOL_H
#define __DRAGVERTEXTOOL_H

#include "tool.h"
#include "glmath.h"

#include <QtGui/QKeySequence>

#include <list>

class DragVertexTool: public Tool
{
   public:
      DragVertexTool();
      virtual ~DragVertexTool();

      int getToolCount() { return 1; };
      const char * getName( int arg );
      bool getKeyBinding( int arg, int & keyBinding ) { keyBinding = Qt::Key_M; return true; };
 
      void activated( int argc, Model * model, QMainWindow * mainwin );

      bool isManipulation() { return true; };

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y );
      void mouseButtonUp(   Parent * parent, int buttonState, int x, int y );
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y );

      const char ** getPixmap();

   protected:
      Matrix m_view;
      Matrix m_inv;

      int                 m_vertId;
      double              m_vertCoords[3];
      std::list< Vector > m_vertVectors;
};

#endif // __DRAGVERTEXTOOL_H
