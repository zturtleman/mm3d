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


#include "rgbawin.h"
#include "helpwin.h"
#include "log.h"

#include "mq3compat.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qlabel.h>
#include <stdlib.h>

RgbaWin::RgbaWin( QWidget * parent, const char * name )
   : RgbaWinBase( parent, name, true ),
     m_accel( new QAccel(this) ),
     m_editing( false )
{
   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );
}

RgbaWin::~RgbaWin()
{
}

void RgbaWin::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_rgbawin.html", true );
   win->show();
}

void RgbaWin::redSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%1.02f", (float) v / 100.0 );
      m_redEdit->setText( str );
   }
   emit valuesChanged();
}

void RgbaWin::greenSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%1.02f", (float) v / 100.0 );
      m_greenEdit->setText( str );
   }
   emit valuesChanged();
}

void RgbaWin::blueSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%1.02f", (float) v / 100.0 );
      m_blueEdit->setText( str );
   }
   emit valuesChanged();
}

void RgbaWin::alphaSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%1.02f", (float) v / 100 );
      m_alphaEdit->setText( str );
   }
   emit valuesChanged();
}

void RgbaWin::redEditChanged( const QString & str )
{
   m_editing = true;
   float v = atof( str.latin1() );
   m_redSlider->setValue( (int) (v * 100) );
   m_editing = false;
}

void RgbaWin::greenEditChanged( const QString & str )
{
   m_editing = true;
   float v = atof( str.latin1() );
   m_greenSlider->setValue( (int) (v * 100) );
   m_editing = false;
}

void RgbaWin::blueEditChanged( const QString & str )
{
   m_editing = true;
   float v = atof( str.latin1() );
   m_blueSlider->setValue( (int) (v * 100) );
   m_editing = false;
}

void RgbaWin::alphaEditChanged( const QString & str )
{
   m_editing = true;
   float v = atof( str.latin1() );
   m_alphaSlider->setValue( (int) (v * 100) );
   m_editing = false;
}

float RgbaWin::getRed()
{
   return (float) m_redSlider->value() / 100.0;
}

float RgbaWin::getGreen()
{
   return (float) m_greenSlider->value() / 100.0;
}

float RgbaWin::getBlue()
{
   return (float) m_blueSlider->value() / 100.0;
}

float RgbaWin::getAlpha()
{
   return (float) m_alphaSlider->value() / 100.0;
}

void RgbaWin::setLabel( const char * newLabel )
{
   if ( newLabel )
   {
      setCaption( QString( newLabel ) );
      QString str;
      str.sprintf( "<b>%s<b>", newLabel );
      m_propertyLabel->setText( str );
   }
}

void RgbaWin::setRed( const float & v )
{
   m_redSlider->setValue( (int) (v * 100) );
}

void RgbaWin::setGreen( const float & v )
{
   m_greenSlider->setValue( (int) (v * 100) );
}

void RgbaWin::setBlue( const float & v )
{
   m_blueSlider->setValue( (int) (v * 100) );
}

void RgbaWin::setAlpha( const float & v )
{
   m_alphaSlider->setValue( (int) (v * 100) );
}
