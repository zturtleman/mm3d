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


#ifndef ROTATETOOLWIDGET_H_INC__
#define ROTATETOOLWIDGET_H_INC__

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
class QLineEdit;
class QLabel;

class RotateToolWidget : public Q3DockWindow
{
   Q_OBJECT

   public:
      class Observer
      {
         public:
            virtual ~Observer() {};

            virtual void setXValue( double newValue )    = 0;
            virtual void setYValue( double newValue )    = 0;
            virtual void setZValue( double newValue )    = 0;
      };

      RotateToolWidget( Observer * observer, QWidget * parent,
            double x, double y, double z );
      virtual ~RotateToolWidget();

      void setCoords( double x, double y, double z );

   public slots:
      void xValueChanged( const QString & newValue );
      void yValueChanged( const QString & newValue );
      void zValueChanged( const QString & newValue );

   protected:
      Observer    * m_observer;
      bool m_ignore;

      Q3BoxLayout * m_layout;

      Q3GroupBox   * m_groupBox;
      QLabel      * m_xLabel;
      QLineEdit   * m_xValue;
      QLabel      * m_yLabel;
      QLineEdit   * m_yValue;
      QLabel      * m_zLabel;
      QLineEdit   * m_zValue;
};

#endif // ROTATETOOLWIDGET_H_INC__
