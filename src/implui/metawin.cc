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


#include "metawin.h"

#include "model.h"
#include "keyvaluewin.h"
#include "decalmgr.h"
#include "helpwin.h"

#include "mq3compat.h"

#include <qpushbutton.h>
#include <stdio.h>
#include <stdlib.h>

MetaWindow::MetaWindow( Model * model, QWidget * parent, const char * name )
   : MetaWindowBase( parent, name, true, WDestructiveClose ),
     m_accel( new QAccel(this) ),
     m_model( model )
{
   unsigned count = m_model->getMetaDataCount();

   for ( unsigned int m = 0; m < count; m++ )
   {
      char key[1024];
      char value[1024];

      m_model->getMetaData( m, key, sizeof(key), value, sizeof(value) );
      QListViewItem * item = new QListViewItem( m_list );
      item->setText( 0, QString::fromUtf8( key ) );
      item->setText( 1, QString::fromUtf8( value ) );
   }

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );
}

MetaWindow::~MetaWindow()
{
}

void MetaWindow::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_metawin.html", true );
   win->show();
}

void MetaWindow::newClicked()
{
   QListViewItem * item = new QListViewItem( m_list, tr("Name", "meta value key name"), tr("Value", "meta value 'value'") );

   m_list->clearSelection();

   QListViewItem * i = m_list->firstChild();
   while ( i )
   {
      i->setSelected( false );
      i = i->nextSibling();
   }

   item->setSelected( true );
   m_list->setCurrentItem( item );
}

void MetaWindow::deleteClicked()
{
   QListViewItem * item = m_list->selectedItem();

   if ( item )
   {
      delete item;
   }
}

void MetaWindow::editItemEvent( QListViewItem * item )
{
   KeyValueWindow w( item );
   w.exec();
}

void MetaWindow::accept()
{
   m_model->clearMetaData();
   QListViewItem * item = m_list->firstChild();

   while ( item )
   {
      m_model->addMetaData( item->text(0).latin1(), item->text(1).utf8() );
      item = item->nextSibling();
   }
   
   m_model->operationComplete( tr( "Change meta data", "operation complete" ).utf8() );
   MetaWindowBase::accept();
}

void MetaWindow::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   MetaWindowBase::reject();
}


