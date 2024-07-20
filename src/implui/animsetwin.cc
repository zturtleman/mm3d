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


#include "animsetwin.h"

#include "animconvertwin.h"
#include "decalmgr.h"
#include "log.h"
#include "msg.h"
#include "3dmprefs.h"
#include "helpwin.h"
#include "newanim.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QShortcut>

#include <list>

using std::list;

AnimSetWindow::AnimSetWindow( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_model( model )
{
   setupUi( this );
   setModal( true );

   m_animType->insertItem( 0, tr( "Skeletal Animation" ) );
   m_animType->insertItem( 1, tr( "Frame Animation" ) );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   unsigned skelCount     = m_model->getAnimCount( Model::ANIMMODE_SKELETAL );
   unsigned frameCount    = m_model->getAnimCount( Model::ANIMMODE_FRAME );
   unsigned relativeCount = m_model->getAnimCount( Model::ANIMMODE_FRAMERELATIVE );

   if ( skelCount > 0 )
   {
      m_animType->setCurrentIndex(0);
   }
   else if ( frameCount > 0 )
   {
      m_animType->setCurrentIndex(1);
   }
   else if ( relativeCount > 0 )
   {
      m_animType->setCurrentIndex(2);
   }
   else
   {
      m_animType->setCurrentIndex(0);
   }

   fillAnimationList();
   animModeSelected( m_animType->currentIndex() );

   //Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );

}

AnimSetWindow::~AnimSetWindow()
{
}

void AnimSetWindow::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_animsetwin.html", true );
   win->show();
}

void AnimSetWindow::animModeSelected( int index )
{
   switch( index )
   {
      case 0:
         m_convertButton->setEnabled( true );
         m_mergeButton->setEnabled( true );
         break;
      case 1:
         m_convertButton->setEnabled( false );
         m_mergeButton->setEnabled( false );
         break;
      case 2:
         m_convertButton->setEnabled( true );
         m_mergeButton->setEnabled( false );
         break;
      default:
         break;
   }

   fillAnimationList();
}

void AnimSetWindow::upClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );
   unsigned count = m_animList->count();

   unsigned t = 0;

   for ( t = 0; t < count && m_animList->item(t)->isSelected(); t++ )
   {
      // do nothing (loop finds first unselected item)
   }

   unsigned lastUnselected = t;
   bool moving = false;

   // t is initialized above
   for ( ; t < count; t++ )
   {
      if ( moving )
      {
         if ( ! m_animList->item(t)->isSelected() )
         {
            unsigned oldIndex = lastUnselected;
            unsigned newIndex = t - 1; 

            m_model->moveAnimation( mode, oldIndex, newIndex );
            moving = false;

            QListWidgetItem * item = m_animList->takeItem( oldIndex );
            m_animList->insertItem( newIndex, item );

            log_debug( "moved item from %d to %d\n", oldIndex, newIndex );

            lastUnselected = newIndex;
            t              = newIndex;
         }
      }
      else
      {
         if ( m_animList->item(t)->isSelected() )
         {
            moving = true;
         }
         else
         {
            lastUnselected = t;
         }
      }
   }

   if ( moving )
   {
      unsigned newIndex = count - 1;

      m_model->moveAnimation( mode, lastUnselected, newIndex );

      QListWidgetItem * item = m_animList->takeItem( lastUnselected );
      m_animList->insertItem( newIndex, item );

      log_debug( "moved item from %d to %d\n", lastUnselected, newIndex );
   }
}

void AnimSetWindow::downClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );
   unsigned count = m_animList->count();

   int t = 0;

   for ( t = count - 1; t >= 0 && m_animList->item(t)->isSelected(); t-- )
   {
      // do nothing (loop finds first unselected item)
   }

   unsigned lastUnselected = t;
   bool moving = false;

   // t is initialized above
   for ( ; t >= 0; t-- )
   {
      if ( moving )
      {
         if ( ! m_animList->item(t)->isSelected() )
         {
            unsigned oldIndex = lastUnselected;
            unsigned newIndex = t + 1; 

            m_model->moveAnimation( mode, oldIndex, newIndex );
            moving = false;

            QListWidgetItem * item = m_animList->takeItem( oldIndex );
            m_animList->insertItem( newIndex, item );

            log_debug( "moved item from %d to %d\n", oldIndex, newIndex );

            lastUnselected = newIndex;
            t              = newIndex;
         }
      }
      else
      {
         if ( m_animList->item(t)->isSelected() )
         {
            moving = true;
         }
         else
         {
            lastUnselected = t;
         }
      }
   }

   if ( moving )
   {
      unsigned newIndex = 0;

      m_model->moveAnimation( mode, lastUnselected, newIndex );

      QListWidgetItem * item = m_animList->takeItem( lastUnselected );
      m_animList->insertItem( newIndex, item );

      log_debug( "moved item from %d to %d\n", lastUnselected, newIndex );
   }
}

void AnimSetWindow::newClicked()
{
   NewAnim win( this );
   g_prefs.setDefault( "ui_new_anim_type", 0 );
   win.setSkeletal( g_prefs( "ui_new_anim_type" ).intValue() == 0 );
   if ( win.exec() )
   {
      g_prefs( "ui_new_anim_type" ) = win.isSkeletal() ? 0 : 1;
      QString name = win.getAnimName();
      Model::AnimationModeE mode = win.isSkeletal()
         ? Model::ANIMMODE_SKELETAL
         : Model::ANIMMODE_FRAME;
      unsigned frameCount = win.getAnimFrameCount();
      double fps = win.getAnimFPS();
      bool loop = win.getAnimLooping();

      int anim = m_model->addAnimation( mode, name.toUtf8() );
      m_model->setAnimFrameCount( mode, anim, frameCount );
      m_model->setAnimFPS( mode, anim, fps );
      m_model->setAnimLooping( mode, anim, loop );
      m_model->setCurrentAnimation( mode, anim );

      if ( mode == Model::ANIMMODE_SKELETAL )
      {
         m_animType->setCurrentIndex(0);
      }
      else if ( mode == Model::ANIMMODE_FRAME )
      {
         m_animType->setCurrentIndex(1);
      }
      else if ( mode == Model::ANIMMODE_FRAMERELATIVE )
      {
         m_animType->setCurrentIndex(2);
      }
      else
      {
         m_animType->setCurrentIndex(0);
      }

      animModeSelected( m_animType->currentIndex() );

      int num = m_animList->count() - 1;

      if ( num >= 0 )
      {
         m_animList->setCurrentItem(m_animList->item(num));

         m_animList->clearSelection();
         m_animList->item(num)->setSelected( true );
      }
   }
}

void AnimSetWindow::renameClicked()
{
   bool isSelection = false;
   unsigned firstSelection = 0;

   unsigned count = m_animList->count();
   for ( firstSelection = 0; firstSelection < count; firstSelection++ )
   {
      if ( m_animList->item(firstSelection)->isSelected() )
      {
         isSelection = true;
         break;
      }
   }

   if ( isSelection )
   {
      bool ok = false;

      QString name = QInputDialog::getText( this,
            tr( "Rename Animation" ),
            tr( "New name:" ),
            QLineEdit::Normal, m_animList->item( firstSelection )->text(), &ok );

      if ( ok && !name.isEmpty() )
      {
         Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );

         for ( unsigned t = 0; t < count; t++ )
         {
            if ( m_animList->item(t)->isSelected() )
            {
               m_model->setAnimName( mode, t, name.toUtf8() );
               m_animList->item(t)->setText( name );
            }
         }
      }
   }
}

void AnimSetWindow::deleteClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );
   unsigned count = m_animList->count();
   unsigned lastDeleted = 0;

   bool refillList = false;

   for ( int t = count - 1; t >= 0; t-- )
   {
      if ( m_animList->item(t)->isSelected() )
      {
         m_model->deleteAnimation( mode, t );
         refillList = true;
         lastDeleted = t;
      }
   }

   if ( refillList )
   {
      fillAnimationList();

      count = m_animList->count();
      for ( unsigned int t = 0; t < count; ++t )
      {
         m_animList->item(t)->setSelected( false );
      }
      if ( count > 0 )
      {
         if ( lastDeleted < count )
         {
            m_animList->setCurrentItem(m_animList->item(lastDeleted));
         }
         else
         {
            m_animList->setCurrentItem(m_animList->item(count - 1));
         }
      }
   }
}

void AnimSetWindow::copyClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );
   list<int> newAnims;

   unsigned count = m_animList->count();
   for ( unsigned t = 0; t < count; t++ )
   {
      if ( m_animList->item(t)->isSelected() )
      {
         QString name = m_animList->item( t )->text();
         name += QString( " " ) + tr( "copy" );

         int num = m_model->copyAnimation( mode, t, name.toUtf8() );

         if ( num >= 0 )
         {
            newAnims.push_back( num );
         }
      }
   }

   if ( !newAnims.empty() )
   {
      fillAnimationList();

      list<int>::iterator it;
      
      m_animList->setCurrentItem(m_animList->item(newAnims.back()));

      count = m_animList->count();
      for ( unsigned int t = 0; t < count; ++t )
      {
         m_animList->item(t)->setSelected( false );
      }
      for ( it = newAnims.begin(); it != newAnims.end(); it++ )
      {
         m_animList->item( *it )->setSelected( true );
      }
   }
}

void AnimSetWindow::splitClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );
   unsigned count = m_animList->count();

   bool refillList = false;

   int splitNum = 0;

   for ( int t = count - 1; t >= 0; t-- )
   {
      if ( m_animList->item(t)->isSelected() )
      {
         splitNum = t;
         if ( m_model->getAnimFrameCount( mode, t ) >= 2 )
         {
            bool ok = false;
            QString name = QString::fromUtf8( m_model->getAnimName( mode, t ) );
            unsigned frame = QInputDialog::getInt( this,
                  tr("Split at frame", "Split animation frame window title" ), tr("Split", "'Split' refers to splitting an animation into two separate animations" ) 
                  + QString(" ") + name + QString(" ") + tr("at frame number", "the frame number where the second (split) animation begins" ),
                  2, 2, m_model->getAnimFrameCount( mode, t ), 1, &ok );

            if ( ok )
            {
               name = name + " " + tr("split");
               if ( m_model->splitAnimation( mode, t, name.toUtf8(), frame - 1 ) )
               {
                  refillList = true;
               }
            }
         }
         else
         {
            QMessageBox::information( this, tr("Cannot Split", "Cannot split animation window title"), tr("Must have at least 2 frames to split", "split animation" ), QMessageBox::Ok );
         }
      }
   }

   if ( refillList )
   {
      fillAnimationList();
      m_animList->setCurrentItem(m_animList->item(splitNum));
      count = m_animList->count();
      for ( unsigned int t = 0; t < count; ++t )
      {
         m_animList->item(t)->setSelected( false );
      }
      m_animList->item( splitNum )->setSelected( true );
   }
}

void AnimSetWindow::joinClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );

   unsigned count = m_animList->count();
   int joinNum = -1;
   bool joined = false;
   unsigned currentAnim = 0; // indices change, need this to keep track of real index

   for ( unsigned t = 0; t < count; t++ )
   {
      if ( m_animList->item(t)->isSelected() )
      {
         if ( joinNum >= 0 )
         {
            log_debug( "joining %d to %d\n", t, joinNum );
            m_model->joinAnimations( mode, joinNum, currentAnim );
            joined = true;
         }
         else
         {
            log_debug( "animation to join to is %d\n", t );
            joinNum = t;

            currentAnim++;
         }
      }
      else
      {
         currentAnim++;
      }
   }

   if ( joined )
   {
      fillAnimationList();

      m_animList->setCurrentItem(m_animList->item(joinNum));
      count = m_animList->count();
      for ( unsigned int t = 0; t < count; ++t )
      {
         m_animList->item(t)->setSelected( false );
      }
      m_animList->item( joinNum )->setSelected( true );
   }
}

void AnimSetWindow::mergeClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );

   if ( mode == Model::ANIMMODE_SKELETAL )
   {
      unsigned count = m_animList->count();
      int mergeNum = -1;
      bool merged = false;
      unsigned currentAnim = 0; // indices change, need this to keep track of real index

      for ( unsigned t = 0; t < count; t++ )
      {
         if ( m_animList->item(t)->isSelected() )
         {
            if ( mergeNum >= 0 )
            {
               unsigned fc1 = m_model->getAnimFrameCount( mode, mergeNum );
               unsigned fc2 = m_model->getAnimFrameCount( mode, currentAnim );

               if ( fc1 == fc2 )
               {
                  log_debug( "merging %d to %d\n", t, mergeNum );
                  m_model->mergeAnimations( mode, mergeNum, currentAnim );
                  merged = true;
               }
               else
               {
                  QString str;
                  str = tr("Cannot merge animation %1 and %2,\n frame counts differ.")
                     .arg(m_model->getAnimName( mode, mergeNum ))
                     .arg(m_model->getAnimName( mode, currentAnim ));
                  msg_error( (const char *) str.toUtf8() );
               }
            }
            else
            {
               log_debug( "animation to merge to is %d\n", t );
               mergeNum = t;

               currentAnim++;
            }
         }
         else
         {
            currentAnim++;
         }
      }

      if ( merged )
      {
         fillAnimationList();

         m_animList->setCurrentItem(m_animList->item(mergeNum));
         count = m_animList->count();
         for ( unsigned int t = 0; t < count; ++t )
         {
            m_animList->item(t)->setSelected( false );
         }
         m_animList->item( mergeNum )->setSelected( true );
      }
   }
   else
   {
      msg_error( (const char *) tr("Can only merge skeletal animations.").toUtf8() );
   }
}

void AnimSetWindow::convertClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );
   unsigned count = m_animList->count();
   std::list<unsigned> animIndicies;

   for ( unsigned t = 0; t < count; t++ )
   {
      if ( m_animList->item(t)->isSelected() )
      {
         animIndicies.push_back( t );
      }
   }

   AnimConvertWindow acw( this );

   acw.setAnimationData( m_model, mode, animIndicies );
   acw.exec();
}

void AnimSetWindow::accept()
{
   m_model->operationComplete( tr( "Animation changes", "operation complete" ).toUtf8() );
   QDialog::accept();
}

void AnimSetWindow::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}

void AnimSetWindow::fillAnimationList()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentIndex() );

   m_animList->clear();

   unsigned count = m_model->getAnimCount( mode );
   for ( unsigned t = 0; t < count; t++ )
   {
      m_animList->insertItem( t, QString::fromUtf8( m_model->getAnimName( mode, t ) ) );
   }

   if ( count > 0 )
   {
      m_animList->setCurrentItem(m_animList->item(0));
   }
}

Model::AnimationModeE AnimSetWindow::indexToMode( int index )
{
   Model::AnimationModeE mode = Model::ANIMMODE_SKELETAL;

   switch( index )
   {
      case 0:
         mode = Model::ANIMMODE_SKELETAL;
         break;
      case 1:
         mode = Model::ANIMMODE_FRAME;
         break;
      case 2:
         mode = Model::ANIMMODE_FRAMERELATIVE;
         break;
      default:
         break;
   }

   return mode;
}
