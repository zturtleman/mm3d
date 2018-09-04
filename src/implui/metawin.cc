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


#include "metawin.h"

#include "model.h"
#include "keyvaluewin.h"
#include "decalmgr.h"
#include "helpwin.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QShortcut>

#include <stdio.h>
#include <stdlib.h>

MetaWindow::MetaWindow( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_model( model )
{
   setAttribute( Qt::WA_DeleteOnClose );
   setupUi( this );
   setModal( true );

   m_list->header()->setSectionsClickable( false );
   m_list->header()->setSectionsMovable( false );

   unsigned count = m_model->getMetaDataCount();

   for ( unsigned int m = 0; m < count; m++ )
   {
      char key[1024];
      char value[1024];

      m_model->getMetaData( m, key, sizeof(key), value, sizeof(value) );
      QTreeWidgetItem * item = new QTreeWidgetItem( m_list );
      item->setText( 0, QString::fromUtf8( key ) );
      item->setText( 1, QString::fromUtf8( value ) );
   }

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

MetaWindow::~MetaWindow()
{
}

void MetaWindow::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_metawin.html", true );
   win->show();
}

void MetaWindow::newClicked()
{
   QTreeWidgetItem * item = new QTreeWidgetItem( m_list );
   item->setText( 0, tr("Name", "meta value key name") );
   item->setText( 1, tr("Value", "meta value 'value'") );

   int count = m_list->topLevelItemCount();
   for ( int i = 0; i < count; ++i )
   {
      m_list->topLevelItem( i )->setSelected( false );
   }

   item->setSelected( true );
   m_list->setCurrentItem( item );
}

void MetaWindow::deleteClicked()
{
   delete m_list->currentItem();
}

void MetaWindow::editItemEvent( QTreeWidgetItem * item, int col )
{
   KeyValueWindow w( item );
   w.exec();
}

void MetaWindow::accept()
{
   m_model->clearMetaData();

   int count = m_list->topLevelItemCount();
   for ( int i = 0; i < count; ++i )
   {
      QTreeWidgetItem * item = m_list->topLevelItem(i);
      m_model->addMetaData( (const char *) item->text(0).toUtf8(),
            (const char *) item->text(1).toUtf8() );
   }
   
   m_model->operationComplete( tr( "Change meta data", "operation complete" ).toUtf8() );
   QDialog::accept();
}

void MetaWindow::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}


