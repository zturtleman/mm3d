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


#ifndef __POLYTOOLWIDGET_H
#define __POLYTOOLWIDGET_H

class QMainWindow;

class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;

class QGroupBox;
class QSpinBox;
class QComboBox;
class QLabel;

#include <QDockWidget>

class PolyToolWidget : public QDockWidget
{
   Q_OBJECT

   public:
      class Observer
      {
         public:
            virtual ~Observer() {};
            virtual void setTypeValue( int type )  = 0;
      };

      PolyToolWidget( Observer * observer, QMainWindow * parent );
      virtual ~PolyToolWidget();

   public slots:
      void typeValueChanged( int type );

   protected:
      Observer    * m_observer;

      QBoxLayout * m_layout;

      QGroupBox   * m_groupBox;
      QLabel      * m_typeLabel;
      QComboBox   * m_typeValue;
      QLabel      * m_segmentLabel;
      QSpinBox    * m_segmentValue;
};

#endif // __POLYTOOLWIDGET_H
