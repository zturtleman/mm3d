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


#include "groupwin.h"

#include "model.h"
#include "textureframe.h"
#include "decalmgr.h"
#include "helpwin.h"

#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QString>

#include <q3accel.h>

#include <list>

GroupWindow::GroupWindow( Model * model, QWidget * parent )
   : QDialog( parent, Qt::WDestructiveClose ),
     m_accel( new Q3Accel(this) ),
     m_model( model )
{
   setupUi( this );
   setModal( true );

   m_textureFrame->setModel( model );

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );

   for ( int t = 0; t < m_model->getTextureCount(); t++ )
   {
      m_textureComboBox->insertItem( QString::fromUtf8( m_model->getTextureName( t ) ), t+1 );
   }

   for ( int t = 0; t < m_model->getGroupCount(); t++ )
   {
      m_groupComboBox->insertItem( QString::fromUtf8( m_model->getGroupName( t ) ), t+1 );
   }

   list<int> triangles;
   m_model->getSelectedTriangles( triangles );

   list<int>::iterator it;
   for ( it = triangles.begin(); it != triangles.end(); it++ )
   {
      int g = m_model->getTriangleGroup( *it );
      if ( g >= 0 )
      {
         m_groupComboBox->setCurrentItem( g + 1 );
         m_textureComboBox->setCurrentItem( m_model->getGroupTextureId( g ) + 1 );
         break;
      }
   }

   groupSelectedEvent( m_groupComboBox->currentItem() );

   updateTexture();
}

GroupWindow::~GroupWindow()
{
}

void GroupWindow::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_groupwin.html", true );
   win->show();
}

void GroupWindow::newClickedEvent()
{
   bool ok = true;
   bool valid = false;

   while ( !valid )
   {
      QString groupName = QInputDialog::getText( tr("New group", "window title"), tr("Enter new group name:"), QLineEdit::Normal, QString::null, &ok );

      if ( ok == true )
      {
         if ( groupName.length() > 0 && groupName.length() < Model::MAX_GROUP_NAME_LEN )
         {
            int groupNum = m_model->addGroup( groupName.utf8() );
            m_groupComboBox->insertItem( groupName, groupNum + 1 );
            m_groupComboBox->setCurrentItem( groupNum + 1 );
            groupSelectedEvent( groupNum + 1 );
            valid = true;
         }
         else
         {
            QString msg = tr( "Group name must be between 1 and %1 characters" ).arg( Model::MAX_GROUP_NAME_LEN - 1 );
            QMessageBox::warning( this, tr("Bad group name", "window title"), msg, QMessageBox::Ok | QMessageBox::Default, 0, 0 );
         }
      }
      else
      {
         valid = true;
      }
   }
}

void GroupWindow::renameClickedEvent()
{
   bool ok = true;
   bool valid = false;

   int groupNum = m_groupComboBox->currentItem();

   if ( groupNum == 0 )
   {
      QMessageBox::information( this, tr("Cannot change", "cannot change group name, window title"), tr("You cannot change the default group name"), QMessageBox::Ok | QMessageBox::Default, 0, 0 );
      return;
   }

   groupNum--;

   while ( !valid )
   {
      QString groupName = QInputDialog::getText(tr("New group", "window title"), tr("Enter new group name:"), QLineEdit::Normal, QString::fromUtf8( m_model->getGroupName( groupNum ) ), &ok );

      if ( ok == true )
      {
         if ( groupName.length() > 0 && groupName.length() < Model::MAX_GROUP_NAME_LEN )
         {
            m_model->setGroupName( groupNum, groupName.utf8() );
            m_groupComboBox->changeItem( groupName, groupNum + 1 );
            valid = true;
         }
         else
         {
            QString msg;
            msg.sprintf( "Group name must be between 1 and %d characters", Model::MAX_GROUP_NAME_LEN - 1 );
            QMessageBox::warning( this, tr("Bad group name", "window title"), msg, QMessageBox::Ok | QMessageBox::Default, 0, 0 );
         }
      }
      else
      {
         valid = true;
      }
   }
}

void GroupWindow::deleteClickedEvent()
{
   int groupNum = m_groupComboBox->currentItem();
   m_groupComboBox->removeItem( groupNum );
   m_model->deleteGroup( groupNum - 1 );

   m_groupComboBox->setCurrentItem( 0 );
   groupSelectedEvent( 0 );
}

void GroupWindow::selectFacesClickedEvent()
{
   m_model->unselectAll();
   m_model->selectGroup( m_groupComboBox->currentItem() - 1 );
   DecalManager::getInstance()->modelUpdated( m_model );
}

void GroupWindow::unselectFacesClickedEvent()
{
   m_model->unselectGroup( m_groupComboBox->currentItem() - 1 );
   DecalManager::getInstance()->modelUpdated( m_model );
}

void GroupWindow::assignAsGroupClickedEvent()
{
   m_model->setSelectedAsGroup( m_groupComboBox->currentItem() - 1 );
   DecalManager::getInstance()->modelAnimate( m_model );
}

void GroupWindow::addToGroupClickedEvent()
{
   m_model->addSelectedToGroup( m_groupComboBox->currentItem() - 1 );
   DecalManager::getInstance()->modelAnimate( m_model );
}

void GroupWindow::smoothChangedEvent( int val )
{
   m_model->setGroupSmooth( m_groupComboBox->currentItem() - 1, val );
   QString text = tr( "Smoothness: " );
   QString valStr;
   valStr.sprintf( "%03d", (int) ((val / 255.0) * 100.0 ) );
   m_smoothLabel->setText( text + valStr );
   m_model->calculateNormals();
   DecalManager::getInstance()->modelUpdated( m_model );
}

void GroupWindow::angleChangedEvent( int val )
{
   m_model->setGroupAngle( m_groupComboBox->currentItem() - 1, val );
   QString text = tr( "Max Angle: " );
   QString valStr;
   valStr.sprintf( "%03d", val );

   m_angleLabel->setText( text + valStr );
   m_model->calculateNormals();
   DecalManager::getInstance()->modelUpdated( m_model );
}

void GroupWindow::groupSelectedEvent( int id )
{
   if ( id > 0 )
   {
      m_smoothSlider->setEnabled( true );
      m_smoothSlider->setValue( m_model->getGroupSmooth( id - 1 ) );

      m_angleSlider->setEnabled( true );
      m_angleSlider->setValue( m_model->getGroupAngle( id - 1 ) );

      m_renameButton->setEnabled( true );
      m_deleteButton->setEnabled( true );
      m_assignAsGroupButton->setEnabled( true );
      m_addToGroupButton->setEnabled( true );
      m_textureComboBox->setEnabled( true );

      int texId = m_model->getGroupTextureId( id - 1 );
      m_textureComboBox->setCurrentItem( texId + 1 );
      updateTexture();
   }
   else
   {
      m_smoothSlider->setEnabled( false );
      m_angleSlider->setEnabled( false );
      m_renameButton->setEnabled( false );
      m_deleteButton->setEnabled( false );
      m_assignAsGroupButton->setEnabled( false );
      m_addToGroupButton->setEnabled( false );
      m_textureComboBox->setEnabled( false );
   }
   DecalManager::getInstance()->modelAnimate( m_model );
}

void GroupWindow::textureSelectedEvent( int id )
{
   int groupId = m_groupComboBox->currentItem() - 1;
   if ( groupId >= 0 )
   {
      m_model->setGroupTextureId( groupId, id - 1 );
   }
   updateTexture();
   DecalManager::getInstance()->modelAnimate( m_model );
}

void GroupWindow::updateTexture()
{
   m_textureFrame->textureChangedEvent( m_textureComboBox->currentItem() );
}

void GroupWindow::accept()
{
   m_model->operationComplete( tr( "Group changes", "operation complete" ).utf8() );
   QDialog::accept();
   DecalManager::getInstance()->modelUpdated( m_model );
}

void GroupWindow::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}
