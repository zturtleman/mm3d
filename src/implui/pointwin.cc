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


#include "pointwin.h"
#include "decalmgr.h"
#include "helpwin.h"

#include "mq3compat.h"

#include <qinputdialog.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <list>

using std::list;

#include "model.h"
#include "decalmgr.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"


PointWin::PointWin( Model * model, QWidget * parent, const char * name )
   : PointWinBase( parent, name, true, Qt::WDestructiveClose ),
     m_accel( new QAccel(this) ),
     m_model( model )
{
   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );

   int t;
   for ( t = 0; t < m_model->getPointCount(); t++ )
   {
      m_pointName->insertItem( QString::fromUtf8( m_model->getPointName(t) ), t );
   }

   for ( t = 0; t < m_model->getBoneJointCount(); t++ )
   {
      m_pointJoint->insertItem( QString::fromUtf8( m_model->getBoneJointName(t) ), t + 1);
   }

   list<int> points;
   m_model->getSelectedPoints( points );
   if ( ! points.empty() )
   {
      m_pointName->setCurrentItem( points.front() );
      pointNameSelected( points.front() );
   }
   else
   {
      m_pointName->setCurrentItem( 0 );
      pointNameSelected( 0 );
   }
}

PointWin::~PointWin()
{
}

void PointWin::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_pointwin.html", true );
   win->show();
}

void PointWin::pointNameSelected( int index )
{
   if ( index < m_pointName->count() )
   {
      m_model->unselectAllPoints();
      m_model->selectPoint( index );

      m_deleteButton->setEnabled( true );
      m_renameButton->setEnabled( true );
      m_pointJoint->setEnabled( true );
      m_pointJoint->setCurrentItem( m_model->getPointBoneJoint( index ) + 1 );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
   else
   {
      m_deleteButton->setEnabled( false );
      m_renameButton->setEnabled( false );
      m_pointJoint->setEnabled( false );
      m_pointJoint->setCurrentItem( 0 );
   }
}

void PointWin::pointJointSelected( int index )
{
   if ( m_pointName->count() > 0 )
   {
      if ( index >= 0 && index < m_pointJoint->count() )
      {
         m_model->setPointBoneJoint( m_pointName->currentItem(), index - 1 );
      }
   }
}

void PointWin::deleteClicked()
{
   if ( m_pointName->count() )
   {
      m_model->deletePoint( m_pointName->currentItem() );
   }
}

void PointWin::renameClicked()
{
   if ( m_pointName->count() )
   {
      bool ok = false;
      int pointNum = m_pointName->currentItem();
      QString pointName = QInputDialog::getText( tr("Rename point", "window title"), tr("Enter new point name:"), QLineEdit::Normal, QString::fromUtf8( m_model->getPointName( pointNum )), &ok );
      if ( ok )
      {
         m_model->setPointName( pointNum, pointName.utf8() );
         m_pointName->changeItem( pointName, pointNum );
      }
   }
}

void PointWin::accept()
{
   log_debug( "Point changes complete\n" );
   m_model->operationComplete( tr( "Point changes", "operation complete" ).utf8() );
   PointWinBase::accept();
}

void PointWin::reject()
{
   log_debug( "Point changes canceled\n" );
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   PointWinBase::reject();
}

