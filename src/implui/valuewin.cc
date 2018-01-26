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


#include "valuewin.h"
#include "helpwin.h"
#include "log.h"

#include <QtWidgets/QSlider>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QShortcut>

#include <math.h>
#include <stdlib.h>

ValueWin::ValueWin( QWidget * parent, bool modal, Qt::WindowFlags flags )
   : QDialog( parent, flags ),
     m_editing( false )
{
   setupUi( this );
   setModal( modal );

   m_valueEdit->setText( QString( "0" ) );
   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

ValueWin::~ValueWin()
{
}

void ValueWin::helpNowEvent()
{
   showHelp();
}

void ValueWin::showHelp()
{
   HelpWin * win = new HelpWin( "olh_valuewin.html", true );
   win->show();
}

void ValueWin::setLabel( const char * newLabel )
{
   if ( newLabel )
   {
      setWindowTitle( QString( newLabel ) );
      QString str;
      str.sprintf( "<b>%s<b>", newLabel );
      m_propertyLabel->setText( str );
   }
}

float ValueWin::getValue()
{
   return (float) m_valueSlider->value();
}

void ValueWin::setValue( const float & v )
{
   m_valueSlider->setValue( (int) v );
}

void ValueWin::valueSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%d", v );
      m_valueEdit->setText( str );
   }
}

void ValueWin::valueEditChanged( const QString & str )
{
   m_editing = true;
   float v = str.toDouble();
   m_valueSlider->setValue( (int) v );
   m_editing = false;
}

