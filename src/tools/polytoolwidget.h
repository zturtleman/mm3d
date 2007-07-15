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

#include "mq3macro.h"
#include "mq3compat.h"

class QMainWindow;

class QVBoxLayout;
class QHBoxLayout;

#ifdef HAVE_QT4
#define QGroupBox Q3GroupBox
#endif
class QGroupBox;
class QSpinBox;
class QCheckBox;
class QLabel;

class PolyToolWidget : public QDockWindow
{
   Q_OBJECT

   public:
      class Observer
      {
         public:
            virtual ~Observer() {};
            virtual void setFanValue( bool newValue )  = 0;
      };

      PolyToolWidget( Observer * observer, QWidget * parent );
      virtual ~PolyToolWidget();

   public slots:
      void fanValueChanged( bool cube );

   protected:
      Observer    * m_observer;

      QBoxLayout * m_layout;

      QGroupBox   * m_groupBox;
      QLabel      * m_fanLabel;
      QCheckBox   * m_fanValue;
      QLabel      * m_segmentLabel;
      QSpinBox    * m_segmentValue;
};

#endif // __POLYTOOLWIDGET_H
