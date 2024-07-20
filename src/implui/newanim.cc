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

#include "newanim.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>

NewAnim::NewAnim( QWidget * parent )
   : QDialog( parent )
{
   setModal( true );
   setupUi( this );

   m_name->setText( QString("") );
   m_okButton->setEnabled( false );
}

NewAnim::~NewAnim()
{
}

void NewAnim::editMode()
{
   setWindowTitle( tr( "Edit Animation" ) );

   m_skeletal->setEnabled( false );
   m_frame->setEnabled( false );
}

QString NewAnim::getAnimName()
{
   return m_name->text();
}

void NewAnim::setAnimName( const QString &name )
{
   m_name->setText( name );
}

bool NewAnim::isSkeletal()
{
   return m_skeletal->isChecked();
}

void NewAnim::setSkeletal( bool o )
{
   if ( o )
      m_skeletal->setChecked( true );
   else
      m_frame->setChecked( true );
}

void NewAnim::nameChangedEvent()
{
   if ( m_name->text().length() == 0 )
   {
      m_okButton->setEnabled( false );
   }
   else
   {
      m_okButton->setEnabled( true );
   }
}

unsigned NewAnim::getAnimFrameCount()
{
	return m_frames->value();
}

void NewAnim::setAnimFrameCount( unsigned value )
{
   m_frames->setValue( value );
}

double NewAnim::getAnimFPS()
{
   return m_fps->text().toDouble();
}

void NewAnim::setAnimFPS( double fps )
{
    m_fps->setText( QString::number( fps ) );
}

bool NewAnim::getAnimLooping()
{
   return m_loop->isChecked();
}

void NewAnim::setAnimLooping( bool loop )
{
   m_loop->setChecked( loop );
}
