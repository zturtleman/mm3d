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


#ifndef __ELLIPSETOOLWIDGET_H
#define __ELLIPSETOOLWIDGET_H

class QMainWindow;

class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;

class QGroupBox;
class QSpinBox;
class QLabel;
class QCheckBox;

#include "toolwidget.h"

class EllipsoidToolWidget : public ToolWidget
{
   Q_OBJECT

   public:
      class Observer
      {
         public:
            virtual ~Observer() {};
            virtual void setSmoothnessValue( int newValue ) = 0;
            virtual void setSphere( bool o ) = 0;
            virtual void setCenter( bool o ) = 0;
      };

      EllipsoidToolWidget( Observer * observer, QMainWindow * parent );
      virtual ~EllipsoidToolWidget();

   public slots:
      void smoothnessValueChanged( int newValue );
      void sphereCheckBoxValueChanged( bool o );
      void centerCheckBoxValueChanged( bool o );

   protected:
      Observer    * m_observer;

      QBoxLayout  * m_layout;

      QGroupBox   * m_groupBox;
      QLabel      * m_smoothLabel;
      QSpinBox    * m_smoothValue;
      QLabel      * m_facesLabel;
      QCheckBox   * m_sphereCheckBox;
      QCheckBox   * m_centerCheckBox;
};

#endif // __ELLIPSETOOLWIDGET_H
