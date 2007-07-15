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


#include "keyvaluewin.h"

#include "model.h"
#include "glmath.h"
#include "helpwin.h"

#include "mq3compat.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <stdio.h>
#include <stdlib.h>

KeyValueWindow::KeyValueWindow( QListViewItem * item, QWidget * parent, const char * name )
   : KeyValueWindowBase( parent, name, true ),
     m_item( item )
{
   m_nameEdit->setText( item->text(0) );
   m_valueEdit->setText( item->text(1) );
}

KeyValueWindow::~KeyValueWindow()
{
}

void KeyValueWindow::accept()
{
   m_item->setText( 0, m_nameEdit->text() );
   m_item->setText( 1, m_valueEdit->text() );
   KeyValueWindowBase::accept();
}

void KeyValueWindow::reject()
{
   KeyValueWindowBase::reject();
}


