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


#include "backgroundwin.h"
#include "backgroundselect.h"

#include "model.h"
#include "log.h"
#include "decalmgr.h"
#include "textureframe.h"
#include "helpwin.h"

#include <QPushButton>
#include <QLayout>
#include <QTabWidget>
#include <QHBoxLayout>
#include <q3accel.h>

#include <stdlib.h>

using std::list;
using std::map;

BackgroundWin::BackgroundWin( Model * model, QWidget * parent )
   : QDialog( parent, Qt::WDestructiveClose ),
     m_accel( new Q3Accel(this) ),
     m_model( model )
{
   setupUi( this );
   setModal( true );

   for ( unsigned t = 0; t < 6; t++ )
   {
      QWidget * p = m_tabs->page( t );
      QHBoxLayout * l = new QHBoxLayout( p );
      m_bgSelect[t] = new BackgroundSelect( m_model, t, p );
      l->addWidget( m_bgSelect[t] );
   }

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );
}

BackgroundWin::~BackgroundWin()
{
}

void BackgroundWin::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_backgroundwin.html", true );
   win->show();
}

void BackgroundWin::selectedPageEvent( const QString & str )
{
   QWidget * widget = m_tabs->currentPage();
   widget->repaint();
}

void BackgroundWin::accept()
{
   m_model->operationComplete( tr( "Background Image", "operation complete" ).utf8() );
   QDialog::accept();
}

void BackgroundWin::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}

