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


#ifndef TOOLWIDGET_H_INC__
#define TOOLWIDGET_H_INC__

#include <QDockWidget>
#include <QMainWindow>
#include <QBoxLayout>

class ToolWidget : public QDockWidget
{
   Q_OBJECT

   public:
      ToolWidget( QMainWindow * window );
      virtual ~ToolWidget();

   protected:
      QBoxLayout * boxLayout() { return m_layout; }
      QWidget * mainWidget() { return m_mainWidget; }

   private:
      QWidget * m_mainWidget;
      QBoxLayout * m_layout;
};

#endif  // TOOLWIDGET_H_INC__

