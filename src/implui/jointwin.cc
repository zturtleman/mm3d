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


#include "jointwin.h"
#include "decalmgr.h"
#include "helpwin.h"

#include <QtWidgets/QInputDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QShortcut>

#include <list>

using std::list;

#include "model.h"
#include "decalmgr.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"


JointWin::JointWin( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_model( model )
{
   setAttribute( Qt::WA_DeleteOnClose );
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   for ( int t = 0; t < m_model->getBoneJointCount(); t++ )
   {
      m_jointName->insertItem( 1 + t, QString::fromUtf8( m_model->getBoneJointName(t) ) );
   }

   list<int> joints;
   m_model->getSelectedBoneJoints( joints );
   if ( ! joints.empty() )
   {
      m_jointName->setCurrentIndex( joints.front() );
      jointNameSelected( joints.front() );
   }
   else
   {
      m_jointName->setCurrentIndex( 0 );
      jointNameSelected( 0 );
   }
}

JointWin::~JointWin()
{
}

void JointWin::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_jointwin.html", true );
   win->show();
}

void JointWin::jointNameSelected( int index )
{
   int jointNum = index - 1;

   m_model->unselectAllBoneJoints();

   if ( jointNum >= 0 )
   {
      m_model->selectBoneJoint( jointNum );

      m_deleteButton->setEnabled( true );
      m_renameButton->setEnabled( true );
      m_selectVerticesButton->setEnabled( true );
      m_assignVerticesButton->setEnabled( true );
      m_addVerticesButton->setEnabled( true );
   }
   else
   {
      m_deleteButton->setEnabled( false );
      m_renameButton->setEnabled( false );
      m_selectVerticesButton->setEnabled( false );
      m_assignVerticesButton->setEnabled( false );
      m_addVerticesButton->setEnabled( false );
   }

   DecalManager::getInstance()->modelUpdated( m_model );
}

void JointWin::deleteClicked()
{
   int jointNum = m_jointName->currentIndex() - 1;
   if ( jointNum >= 0 && m_model->deleteBoneJoint( jointNum ) )
   {
      m_jointName->removeItem( 1 + jointNum );
      m_jointName->setCurrentIndex( 0 );
      jointNameSelected( 0 );
   }
}

void JointWin::renameClicked()
{
   int jointNum = m_jointName->currentIndex() - 1;
   if ( jointNum >= 0 )
   {
      bool ok = false;
      QString jointName = QInputDialog::getText( this, tr("Rename joint", "window title"), tr("Enter new joint name:"), QLineEdit::Normal, QString::fromUtf8( m_model->getBoneJointName( jointNum ) ), &ok );
      if ( ok )
      {
         m_model->setBoneJointName( jointNum, jointName.toUtf8() );
         m_jointName->setItemText( 1 + jointNum, jointName );
      }
   }
}

void JointWin::selectVerticesClicked()
{
   int jointNum = m_jointName->currentIndex() - 1;
   if ( jointNum >= 0 )
   {
      m_model->unselectAllVertices();

      unsigned vcount = m_model->getVertexCount();
      for ( unsigned v = 0; v < vcount; v++ )
      {
         Model::InfluenceList l;
         m_model->getVertexInfluences( v, l );

         Model::InfluenceList::iterator it;
         for ( it = l.begin(); it != l.end(); it++ )
         {
            if ( (*it).m_boneId == jointNum )
            {
               m_model->selectVertex( v );
               break;
            }
         }
      }

      unsigned pcount = m_model->getPointCount();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         Model::InfluenceList l;
         m_model->getPointInfluences( p, l );

         Model::InfluenceList::iterator it;
         for ( it = l.begin(); it != l.end(); it++ )
         {
            if ( (*it).m_boneId == jointNum )
            {
               m_model->selectPoint( p );
               break;
            }
         }
      }
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}

void JointWin::selectUnassignedClicked()
{
   m_model->unselectAllVertices();

   unsigned vcount = m_model->getVertexCount();
   for ( unsigned v = 0; v < vcount; v++ )
   {
      Model::InfluenceList l;
      m_model->getVertexInfluences( v, l );
      if ( l.empty() )
      {
         m_model->selectVertex( v );
      }
   }

   unsigned pcount = m_model->getPointCount();
   for ( unsigned p = 0; p < pcount; p++ )
   {
      Model::InfluenceList l;
      m_model->getPointInfluences( p, l );
      if ( l.empty() )
      {
         m_model->selectPoint( p );
      }
   }
   DecalManager::getInstance()->modelUpdated( m_model );
}

void JointWin::assignVerticesClicked()
{
   int jointNum = m_jointName->currentIndex() - 1;
   log_debug( "assignVerticesClicked()\n" );
   if ( jointNum >= 0 )
   {
      list<Model::Position> posList;
      m_model->getSelectedPositions( posList );
      list<Model::Position>::iterator it;
      log_debug( "assigning %" PORTuSIZE " objects to joint %d\n", posList.size(), jointNum );
      for ( it = posList.begin(); it != posList.end(); it++ )
      {
         m_model->setPositionBoneJoint( *it, jointNum );
      }
   }
}

void JointWin::addVerticesClicked()
{
   int jointNum = m_jointName->currentIndex() - 1;
   log_debug( "addVerticesClicked()\n" );
   if ( jointNum >= 0 )
   {
      list<Model::Position> posList;
      m_model->getSelectedPositions( posList );
      list<Model::Position>::iterator it;
      log_debug( "adding %" PORTuSIZE " objects to joint %d\n", posList.size(), jointNum );
      for ( it = posList.begin(); it != posList.end(); it++ )
      {
         m_model->addPositionInfluence( *it, jointNum, Model::IT_Custom, 1.0 );
      }
   }
}

void JointWin::accept()
{
   log_debug( "Joint changes complete\n" );
   m_model->operationComplete( tr( "Joint changes", "operation complete" ).toUtf8() );
   QDialog::accept();
}

void JointWin::reject()
{
   log_debug( "Joint changes canceled\n" );
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}

