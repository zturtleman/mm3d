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


#ifndef __CYLINDERTOOLWIDGET_H
#define __CYLINDERTOOLWIDGET_H

#include "mq3macro.h"
#include "mq3compat.h"
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3MainWindow;

class Q3VBoxLayout;
class Q3HBoxLayout;

#ifdef HAVE_QT4
#define Q3GroupBox Q3GroupBox
#endif
class Q3GroupBox;
class QSpinBox;
class QSlider;
class QLabel;

class CylinderToolWidget : public Q3DockWindow
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
            virtual void setScaleValue( int newValue )    = 0;
            virtual void setShapeValue( int newValue )    = 0;
      };

      CylinderToolWidget( Observer * observer, QWidget * parent );
      virtual ~CylinderToolWidget();

   public slots:
      void segmentsValueChanged( int newValue );
      void sidesValueChanged( int newValue );
      void widthValueChanged( int newValue );
      void scaleValueChanged( int newValue );
      void shapeValueChanged( int newValue );

   protected:
      Observer    * m_observer;

      Q3BoxLayout * m_layout;

      Q3GroupBox   * m_groupBox;
      QLabel      * m_segmentsLabel;
      QSpinBox    * m_segmentsValue;
      QLabel      * m_sidesLabel;
      QSpinBox    * m_sidesValue;
      QLabel      * m_widthLabel;
      QSpinBox    * m_widthValue;
      QLabel      * m_scaleLabel;
      QSpinBox    * m_scaleValue;
      //QLabel      * m_shapeLabel;
      //QSlider     * m_shapeValue;
};

#endif // __CYLINDERTOOLWIDGET_H
