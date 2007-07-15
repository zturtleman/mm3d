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


#ifndef __TRANSFORMWIN_H
#define __TRANSFORMWIN_H

#include "transformwin.base.h"

#include "mq3macro.h"
#include "glmath.h"

class Model;
class QAccel;
class RgbaWin;

class TransformWindow : public TransformWindowBase
{
   Q_OBJECT

   public:
      TransformWindow( Model * model, QWidget * parent = NULL, const char * name = "" );
      virtual ~TransformWindow();

      bool matrixIsUndoable( const Matrix & m );
      bool warnNoUndo( bool undoable );

      // Transform window events
      void translateEvent();
      void rotateEulerEvent();
      void rotateQuaternionEvent();
      void scaleEvent();
      void matrixEvent();

   public slots:
      void setModel( Model * m );
      void helpNowEvent( int );

      void close();

   protected:

      QAccel * m_accel;
      Model  * m_model;
      bool     m_undoable;
};

#endif // __TRANSFORMWIN_H
