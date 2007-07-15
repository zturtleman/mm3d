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


#include "animsetwin.h"

#include "animconvertwin.h"
#include "decalmgr.h"
#include "log.h"
#include "msg.h"
#include "helpwin.h"

#include "mq3compat.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

#include <list>

using std::list;

AnimSetWindow::AnimSetWindow( Model * model, QWidget * parent, const char * name )
   : AnimSetWinBase( parent, name, true ),
     m_accel( new QAccel(this) ),
     m_model( model )
{
   m_animType->insertItem( tr( "Skeletal Animation" ) );
   m_animType->insertItem( tr( "Frame Animation" ) );

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );

   unsigned skelCount     = m_model->getAnimCount( Model::ANIMMODE_SKELETAL );
   unsigned frameCount    = m_model->getAnimCount( Model::ANIMMODE_FRAME );
   unsigned relativeCount = m_model->getAnimCount( Model::ANIMMODE_FRAMERELATIVE );

   if ( skelCount > 0 )
   {
      m_animType->setCurrentItem( 0 );
   }
   else if ( frameCount > 0 )
   {
      m_animType->setCurrentItem( 1 );
   }
   else if ( relativeCount > 0 )
   {
      m_animType->setCurrentItem( 2 );
   }
   else
   {
      m_animType->setCurrentItem( 0 );
   }

   fillAnimationList();
   animModeSelected( m_animType->currentItem() );

   //Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );

}

AnimSetWindow::~AnimSetWindow()
{
}

void AnimSetWindow::helpNowEvent( int id )
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
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );
   unsigned count = m_animList->count();

   unsigned t = 0;

   for ( t = 0; t < count && m_animList->isSelected( t ); t++ )
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
         if ( ! m_animList->isSelected( t ) )
         {
            unsigned oldIndex = lastUnselected;
            unsigned newIndex = t - 1; 

            m_model->moveAnimation( mode, oldIndex, newIndex );
            moving = false;

            QString text = m_animList->text( oldIndex );
            m_animList->removeItem( oldIndex );
            m_animList->insertItem( text, newIndex );

            log_debug( "moved item from %d to %d\n", oldIndex, newIndex );

            lastUnselected = newIndex;
            t              = newIndex;
         }
      }
      else
      {
         if ( m_animList->isSelected( t ) )
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

      QString text = m_animList->text( lastUnselected );
      m_animList->removeItem( lastUnselected );
      m_animList->insertItem( text, newIndex );

      log_debug( "moved item from %d to %d\n", lastUnselected, newIndex );
   }
}

void AnimSetWindow::downClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );
   unsigned count = m_animList->count();

   int t = 0;

   for ( t = count - 1; t >= 0 && m_animList->isSelected( t ); t-- )
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
         if ( ! m_animList->isSelected( t ) )
         {
            unsigned oldIndex = lastUnselected;
            unsigned newIndex = t + 1; 

            m_model->moveAnimation( mode, oldIndex, newIndex );
            moving = false;

            QString text = m_animList->text( oldIndex );
            m_animList->removeItem( oldIndex );
            m_animList->insertItem( text, newIndex );

            log_debug( "moved item from %d to %d\n", oldIndex, newIndex );

            lastUnselected = newIndex;
            t              = newIndex;
         }
      }
      else
      {
         if ( m_animList->isSelected( t ) )
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

      QString text = m_animList->text( lastUnselected );
      m_animList->removeItem( lastUnselected );
      m_animList->insertItem( text, newIndex );

      log_debug( "moved item from %d to %d\n", lastUnselected, newIndex );
   }
}

void AnimSetWindow::newClicked()
{
   bool ok = false;

   QString name = QInputDialog::getText(
         tr( "Misfit 3D" ),
         tr( "New name:" ),
         QLineEdit::Normal, QString(""), &ok, this );

   if ( ok && !name.isEmpty() )
   {
      int num = m_model->addAnimation( indexToMode( m_animType->currentItem() ), name.utf8() );
      if ( num >= 0 )
      {
         m_animList->insertItem( name, num );
         m_animList->setCurrentItem( num );

         m_animList->clearSelection();
         m_animList->setSelected( num, true );
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
      if ( m_animList->isSelected( firstSelection ) )
      {
         isSelection = true;
         break;
      }
   }

   if ( isSelection )
   {
      bool ok = false;

      QString name = QInputDialog::getText(
            tr( "Misfit 3D" ),
            tr( "New name:" ),
            QLineEdit::Normal, m_animList->text( firstSelection ), &ok, this );

      if ( ok && !name.isEmpty() )
      {
         Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );

         for ( unsigned t = 0; t < count; t++ )
         {
            if ( m_animList->isSelected( t ) )
            {
               m_model->setAnimName( mode, t, name.utf8() );
               m_animList->changeItem( name, t );
            }
         }
      }
   }
}

void AnimSetWindow::deleteClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );
   unsigned count = m_animList->count();
   unsigned lastDeleted = 0;

   bool refillList = false;

   for ( int t = count - 1; t >= 0; t-- )
   {
      if ( m_animList->isSelected( t ) )
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
      if ( count > 0 )
      {
         if ( lastDeleted < count )
         {
            m_animList->setSelected( lastDeleted, true );
            m_animList->setCurrentItem( lastDeleted );
         }
         else
         {
            m_animList->setSelected( count - 1, true );
            m_animList->setCurrentItem( count - 1 );
         }
      }
   }
}

void AnimSetWindow::copyClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );
   list<int> newAnims;

   unsigned count = m_animList->count();
   for ( unsigned t = 0; t < count; t++ )
   {
      if ( m_animList->isSelected( t ) )
      {
         QString name = m_animList->text( t );
         name += tr( " copy" );

         int num = m_model->copyAnimation( mode, t, name.latin1() );

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
      
      for ( it = newAnims.begin(); it != newAnims.end(); it++ )
      {
         m_animList->setSelected( (*it), true );
      }

      m_animList->setCurrentItem( newAnims.back() );
   }
}

void AnimSetWindow::splitClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );
   unsigned count = m_animList->count();

   bool refillList = false;

   for ( int t = count - 1; t >= 0; t-- )
   {
      if ( m_animList->isSelected( t ) )
      {
         if ( m_model->getAnimFrameCount( mode, t ) >= 2 )
         {
            bool ok = false;
            QString name = QString::fromUtf8( m_model->getAnimName( mode, t ) );
            unsigned frame = QInputDialog::getInteger( tr("Split at frame", "Split animation frame window title" ), tr("Split ", "'Split' refers to splitting an animation into two separate animations" ) 
                  + name + tr(" at frame number", "the frame number where the second (split) animation begins" ),
                  2, 2, m_model->getAnimFrameCount( mode, t ), 1, &ok );

            if ( ok )
            {
               name = name + tr(" split");
               if ( m_model->splitAnimation( mode, t, name.utf8(), frame ) )
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
   }
}

void AnimSetWindow::joinClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );

   unsigned count = m_animList->count();
   int joinNum = -1;
   bool joined = false;
   unsigned currentAnim = 0; // indices change, need this to keep track of real index

   for ( unsigned t = 0; t < count; t++ )
   {
      if ( m_animList->isSelected( t ) )
      {
         if ( joinNum >= 0 )
         {
            log_debug( "joining %d to %d\n", t, joinNum );
            m_model->joinAnimations( mode, joinNum, currentAnim );
            joined = true;
         }
         else
         {
            log_debug( "animation to join to is %d\n" );
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

      m_animList->setSelected( joinNum, true );
      m_animList->setCurrentItem( joinNum );
   }
}

void AnimSetWindow::mergeClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );

   if ( mode == Model::ANIMMODE_SKELETAL )
   {
      unsigned count = m_animList->count();
      int mergeNum = -1;
      bool merged = false;
      unsigned currentAnim = 0; // indices change, need this to keep track of real index

      for ( unsigned t = 0; t < count; t++ )
      {
         if ( m_animList->isSelected( t ) )
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
                  msg_error( (const char *) str.utf8() );
               }
            }
            else
            {
               log_debug( "animation to merge to is %d\n" );
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

         m_animList->setSelected( mergeNum, true );
         m_animList->setCurrentItem( mergeNum );
      }
   }
   else
   {
      msg_error( (const char *) tr("Can only merge skeletal animations.").utf8() );
   }
}

void AnimSetWindow::convertClicked()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );
   bool cancelled = false;

   unsigned count = m_animList->count();

   for ( unsigned t = 0; !cancelled && t < count; t++ )
   {
      if ( m_animList->isSelected( t ) )
      {
         AnimConvertWindow acw( this );

         acw.setAnimationData( m_model, mode, t );
         acw.exec();

         switch ( acw.requestedOperation() )
         {
            case AnimConvertWindow::OP_CONTINUE:
               {
                  QString name = acw.getNewName();
                  unsigned frameCount = acw.getNewFrameCount();

                  m_model->convertAnimToFrame( mode, t, name.latin1(), frameCount );
               }
               break;
            case AnimConvertWindow::OP_CANCEL:
               break;
            case AnimConvertWindow::OP_CANCEL_ALL:
               cancelled = true;
               break;
         }
      }
   }
}

void AnimSetWindow::accept()
{
   m_model->operationComplete( tr( "Animation changes", "operation complete" ).utf8() );
   AnimSetWinBase::accept();
}

void AnimSetWindow::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   AnimSetWinBase::reject();
}

void AnimSetWindow::fillAnimationList()
{
   Model::AnimationModeE mode = indexToMode( m_animType->currentItem() );

   m_animList->clear();

   unsigned count = m_model->getAnimCount( mode );
   for ( unsigned t = 0; t < count; t++ )
   {
      m_animList->insertItem( QString::fromUtf8( m_model->getAnimName( mode, t ) ), t );
   }

   if ( count > 0 )
   {
      m_animList->setCurrentItem( 0 );
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
