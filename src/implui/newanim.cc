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

#include "newanim.h"

#include <qlineedit.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

NewAnim::NewAnim( QWidget * parent )
   : NewAnimBase( parent, "", true )
{
   m_name->setText( QString("") );
   m_okButton->setEnabled( false );
}

NewAnim::~NewAnim()
{
}

QString NewAnim::getAnimName()
{
   return m_name->text();
}

bool NewAnim::isSkeletal()
{
   return m_skeletal->isChecked();
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
