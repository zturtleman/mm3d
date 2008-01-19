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

#include <QInputDialog>
#include <QPushButton>
#include <QComboBox>
#include <QShortcut>

#include <list>

using std::list;

#include "model.h"
#include "decalmgr.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"


PointWin::PointWin( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_model( model )
{
   setAttribute( Qt::WA_DeleteOnClose );
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   int t;
   for ( t = 0; t < m_model->getPointCount(); t++ )
   {
      m_pointName->insertItem( t,  QString::fromUtf8( m_model->getPointName(t) ) );
   }

   for ( t = 0; t < m_model->getBoneJointCount(); t++ )
   {
      m_pointJoint->insertItem( t + 1, QString::fromUtf8( m_model->getBoneJointName(t) ) );
   }

   list<int> points;
   m_model->getSelectedPoints( points );
   if ( ! points.empty() )
   {
      m_pointName->setCurrentIndex( points.front() );
      pointNameSelected( points.front() );
   }
   else
   {
      m_pointName->setCurrentIndex( 0 );
      pointNameSelected( 0 );
   }
}

PointWin::~PointWin()
{
}

void PointWin::helpNowEvent()
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
      m_pointJoint->setCurrentIndex( m_model->getPointBoneJoint( index ) + 1 );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
   else
   {
      m_deleteButton->setEnabled( false );
      m_renameButton->setEnabled( false );
      m_pointJoint->setEnabled( false );
      m_pointJoint->setCurrentIndex( 0 );
   }
}

void PointWin::pointJointSelected( int index )
{
   if ( m_pointName->count() > 0 )
   {
      if ( index >= 0 && index < m_pointJoint->count() )
      {
         m_model->setPointBoneJoint( m_pointName->currentIndex(), index - 1 );
      }
   }
}

void PointWin::deleteClicked()
{
   if ( m_pointName->count() )
   {
      m_model->deletePoint( m_pointName->currentIndex() );
   }
}

void PointWin::renameClicked()
{
   if ( m_pointName->count() )
   {
      bool ok = false;
      int pointNum = m_pointName->currentIndex();
      QString pointName = QInputDialog::getText( this, tr("Rename point", "window title"), tr("Enter new point name:"), QLineEdit::Normal, QString::fromUtf8( m_model->getPointName( pointNum )), &ok );
      if ( ok )
      {
         m_model->setPointName( pointNum, pointName.toUtf8() );
         m_pointName->setItemText( pointNum, pointName );
      }
   }
}

void PointWin::accept()
{
   log_debug( "Point changes complete\n" );
   m_model->operationComplete( tr( "Point changes", "operation complete" ).toUtf8() );
   QDialog::accept();
}

void PointWin::reject()
{
   log_debug( "Point changes canceled\n" );
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}

