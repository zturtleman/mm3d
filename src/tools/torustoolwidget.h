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


#ifndef __TORUSTOOLWIDGET_H
#define __TORUSTOOLWIDGET_H

class QMainWindow;

class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;

class QGroupBox;
class QSpinBox;
class QCheckBox;
class QSlider;
class QLabel;

#include "toolwidget.h"

class TorusToolWidget : public ToolWidget
{
   Q_OBJECT

   public:
      class Observer
      {
         public:
            virtual ~Observer() {};

            virtual void setSegmentsValue( int newValue ) = 0;
            virtual void setSidesValue( int newValue )    = 0;
            virtual void setWidthValue( int newValue )    = 0;
            virtual void setCircleValue( bool newValue )  = 0;
            virtual void setCenterValue( bool newValue )  = 0;
      };

      TorusToolWidget( Observer * observer, QMainWindow * parent );
      virtual ~TorusToolWidget();

   public slots:
      void segmentsValueChanged( int newValue );
      void sidesValueChanged( int newValue );
      void widthValueChanged( int newValue );
      void circleValueChanged( bool newValue );
      void centerValueChanged( bool newValue );

   protected:
      Observer    * m_observer;

      QBoxLayout  * m_layout;

      QGroupBox   * m_groupBox;
      QLabel      * m_segmentsLabel;
      QSpinBox    * m_segmentsValue;
      QLabel      * m_sidesLabel;
      QSpinBox    * m_sidesValue;
      QLabel      * m_widthLabel;
      QSpinBox    * m_widthValue;
      QCheckBox   * m_circleValue;
      QCheckBox   * m_centerValue;
};

#endif // __TORUSTOOLWIDGET_H
