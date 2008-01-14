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

class QMainWindow;

class QVBoxLayout;
class QHBoxLayout;

#ifdef HAVE_QT4
#define QGroupBox Q3GroupBox
#endif
class QGroupBox;
class QLineEdit;
class QLabel;

class RotateToolWidget : public QDockWindow
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

      QBoxLayout * m_layout;

      QGroupBox   * m_groupBox;
      QLabel      * m_xLabel;
      QLineEdit   * m_xValue;
      QLabel      * m_yLabel;
      QLineEdit   * m_yValue;
      QLabel      * m_zLabel;
      QLineEdit   * m_zValue;
};

#endif // ROTATETOOLWIDGET_H_INC__
