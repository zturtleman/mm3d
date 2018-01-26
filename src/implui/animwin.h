/*  Misfit Model 3D
 * 
 *  Copyright (c) 2008 Kevin Worcester
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


#ifndef ANIMWIN_H_INC__
#define ANIMWIN_H_INC__

#include <QtWidgets/QDockWidget>

#include "animwidget.h"

class Model;

class AnimWindow : public QDockWidget
{
   Q_OBJECT

   public:
      AnimWindow( Model * model, bool isUndo, QWidget * parent = NULL );
      virtual ~AnimWindow();

      AnimWidget * getAnimWidget() { return m_animWidget; }

   signals:
      void animWindowClosed();

   public slots:
      void close();

   protected:
      void closeEvent( QCloseEvent * e );

   private:
      AnimWidget * m_animWidget;
};

#endif  // ANIMWIN_H_INC__
