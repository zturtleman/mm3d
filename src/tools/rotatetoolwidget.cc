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


#include "rotatetoolwidget.h"

#include "3dmprefs.h"

#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>

RotateToolWidget::RotateToolWidget( Observer * observer, QMainWindow * parent,
      double x, double y, double z )
   : QDockWidget ( NULL, Qt::WDestructiveClose ),
     m_observer( observer ),
     m_ignore( false )
{
   m_layout = new QBoxLayout( QBoxLayout::LeftToRight, this );

   m_xLabel = new QLabel( tr("X"), this, "" );
   m_layout->addWidget( m_xLabel );

   m_xValue = new QLineEdit( this, "" );
   m_xValue->setMinimumWidth( 100 );
   m_layout->addWidget( m_xValue );

   m_xValue->setText( QString::number( x, 'f' ) );

   m_yLabel = new QLabel( tr("Y"), this, "" );
   m_layout->addWidget( m_yLabel );

   m_yValue = new QLineEdit( this, "" );
   m_yValue->setMinimumWidth( 100 );
   m_layout->addWidget( m_yValue );

   m_yValue->setText( QString::number( y, 'f' ) );

   m_zLabel = new QLabel( tr("Z"), this, "" );
   m_layout->addWidget( m_zLabel );

   m_zValue = new QLineEdit( this, "" );
   m_zValue->setMinimumWidth( 100 );
   m_layout->addWidget( m_zValue );

   m_zValue->setText( QString::number( z, 'f' ) );

   connect( m_xValue, SIGNAL(textChanged(const QString &)), this, SLOT(xValueChanged(const QString &)) );
   connect( m_yValue, SIGNAL(textChanged(const QString &)), this, SLOT(yValueChanged(const QString &)) );
   connect( m_zValue, SIGNAL(textChanged(const QString &)), this, SLOT(zValueChanged(const QString &)) );

   m_xLabel->show();
   m_xValue->show();
   m_yLabel->show();
   m_yValue->show();
   m_zLabel->show();
   m_zValue->show();

   // Unlike other tool widgets, our initial settings came from the tool,
   // so we don't call the slots directly to update the tool's settings.
}

RotateToolWidget::~RotateToolWidget()
{
}

void RotateToolWidget::xValueChanged( const QString & newValue )
{
   if ( m_ignore )
      return;

   m_observer->setXValue( newValue.toDouble() );
}

void RotateToolWidget::yValueChanged( const QString & newValue )
{
   if ( m_ignore )
      return;

   m_observer->setYValue( newValue.toDouble() );
}

void RotateToolWidget::zValueChanged( const QString & newValue )
{
   if ( m_ignore )
      return;

   m_observer->setZValue( newValue.toDouble() );
}

void RotateToolWidget::setCoords( double x, double y, double z )
{
   m_ignore = true;
   m_xValue->setText( QString::number( x, 'f' ) );
   m_yValue->setText( QString::number( y, 'f' ) );
   m_zValue->setText( QString::number( z, 'f' ) );
   m_ignore = false;
}

