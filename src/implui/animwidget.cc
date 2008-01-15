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


// FIXME: Prevent model edit for invalid animation (modifying model)

#include "animwidget.h"

#include "decalmgr.h"
#include "log.h"
#include "msg.h"
#include "newanim.h"
#include "3dmprefs.h"

#include "helpwin.h"

#include <q3accel.h>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QInputDialog>

enum
{
   ANIMWIN_HELP_ID,
   ANIMWIN_UNDO_ID,
   ANIMWIN_REDO_ID
};

static int getSliderTickInterval( int val )
{
   if ( val > 25000 )
   {
      return 5000;
   }
   if ( val > 10000 )
   {
      return 1000;
   }
   if ( val > 2500 )
   {
      return 500;
   }
   if ( val > 1000 )
   {
      return 100;
   }
   if ( val > 250 )
   {
      return 50;
   }
   if ( val > 100 )
   {
      return 10;
   }
   if ( val > 25 )
   {
      return 5;
   }
   return 1;
}

AnimWidget::AnimWidget( Model * model, bool isUndo, QWidget * parent )
   : QWidget( parent ),
     m_model( model ),
     m_doLoop( true ),
     m_playing( false ),
     m_undoing( isUndo ),
     m_ignoreChange( false )
{
   setupUi( this );

   log_debug( "AnimWidget constructor\n" );

   m_animTimer = new QTimer( this );
   m_accel     = new Q3Accel( this );

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), ANIMWIN_HELP_ID );

   connect( m_animTimer, SIGNAL(timeout()),      this, SLOT(timeElapsed()) );
   connect( m_accel,     SIGNAL(activated(int)), this, SLOT(accelActivated(int)) );

   //setCloseMode( QDockWindow::Always );

   initialize( model, isUndo );
}

AnimWidget::~AnimWidget()
{
}

void AnimWidget::initialize( Model * model, bool isUndo )
{
   m_needShutdown = true;
   m_model = model;
   m_animName->clear();

   //m_skelNew->setDefault( false );

   m_countSlider->setTickmarks( QSlider::Below );

   m_doLoop = m_model->isAnimationLooping();
   m_loop->setChecked( m_doLoop );

   m_skelAnimCount = m_model->getAnimCount( Model::ANIMMODE_SKELETAL );
   m_frameAnimCount = m_model->getAnimCount( Model::ANIMMODE_FRAME );
   m_animCount = m_skelAnimCount + m_frameAnimCount;

   insertAnimationNames();

   if ( isUndo )
   {
      Model::AnimationModeE mode = m_model->getAnimationMode();

      if ( !mode )
      {
         if ( m_skelAnimCount > 0 )
         {
            mode = Model::ANIMMODE_SKELETAL;
            m_model->setCurrentAnimation( mode, (unsigned int) 0 );
         }
         else if ( m_frameAnimCount > 0 )
         {
            mode = Model::ANIMMODE_FRAME;
            m_model->setCurrentAnimation( mode, (unsigned int) 0 );
         }
      }

      unsigned anim     = m_model->getCurrentAnimation();
      unsigned frame    = m_model->getCurrentAnimationFrame();

      m_model->setCurrentAnimation( mode, anim );
      m_model->setCurrentAnimationFrame( frame );

      m_animName->setCurrentItem( animToIndex( mode, anim ) );

      m_undoing = false;

      m_currentAnim  = anim;
      m_currentFrame = frame;
      m_mode         = mode;

      refreshPage();
   }
   else
   {
      m_mode = Model::ANIMMODE_SKELETAL;
      if ( m_skelAnimCount > 0 )
      {
         m_model->setCurrentAnimation( m_mode, indexToAnim( m_animName->currentItem() ) );
      }
      else if ( m_frameAnimCount > 0 )
      {
         m_mode = Model::ANIMMODE_FRAME;
         m_model->setCurrentAnimation( m_mode, indexToAnim( m_animName->currentItem() ) );
      }

      refreshPage();

      m_model->operationComplete( tr( "Start animation mode", "operation complete" ).utf8() );
   }

   m_stop->setEnabled( false );
}

void AnimWidget::helpNowEvent( int )
{
   HelpWin * win = new HelpWin( "olh_animwin.html", true );
   win->show();
}

void AnimWidget::nameSelected( int index )
{
   log_debug( "anim name selected: %d\n", index );

   if ( !m_ignoreChange && index >= (int) m_animCount )
   {
      NewAnim win( this );
      g_prefs.setDefault( "ui_new_anim_type", 0 );
      win.setSkeletal( g_prefs( "ui_new_anim_type" ).intValue() == 0 );
      if ( win.exec() )
      {
         stopClicked();

         g_prefs( "ui_new_anim_type" ) = win.isSkeletal() ? 0 : 1;
         QString name = win.getAnimName();
         Model::AnimationModeE mode = win.isSkeletal() 
            ? Model::ANIMMODE_SKELETAL
            : Model::ANIMMODE_FRAME;

         int anim = m_model->addAnimation( mode, name.utf8() );
         m_model->setCurrentAnimation( mode, anim );
         m_model->operationComplete( tr( "New Animation", "operation complete" ).utf8() );
         initialize( m_model, true );
         return;
      }
      else
      {
         // FIXME get previously selected index
         if ( m_animCount > 0 )
         {
            index = animToIndex( m_mode, m_currentAnim );
            m_animName->setCurrentItem( index );
         }
         else
         {
            return;
         }
      }
   }

   m_currentTime = 0;

   m_mode = indexToMode( index );
   index  = indexToAnim( index );

   m_currentAnim  = index;
   m_currentFrame = 0;

   if( index < (int) m_animCount )
   {
      m_model->setCurrentAnimation( m_mode, m_currentAnim );
   }
   else
   {
      m_model->setCurrentAnimation( m_mode, (unsigned) 0 );
   }

   refreshPage();
}

void AnimWidget::setCurrentFrame( int frame )
{
   if ( m_model->setCurrentAnimationFrame( frame - 1 ) )
   {
      m_currentFrame = frame - 1;
      QString str = tr("Frame: ");
      QString numStr;
      numStr.sprintf( "%03d", frame );
      m_countLabel->setText( str + numStr );
   }
   else
   {
      m_countLabel->setText( tr( "Frame: n/a" ) );
   }
   DecalManager::getInstance()->modelUpdated( m_model );
}

void AnimWidget::deleteClicked()
{
   int index = m_animName->currentItem();
   if ( (unsigned int) index < m_animCount )
   {
      if ( QMessageBox::Ok == QMessageBox::warning( this, tr("Delete Animation?", "window title"), 
            tr("Are you sure you want to delete this animation?"),
            QMessageBox::Ok, QMessageBox::Cancel ) )
      {
         m_ignoreChange = true;
         Model::AnimationModeE mode = indexToMode( index );
         int anim = indexToAnim( index );
         m_model->deleteAnimation( mode, anim );

         refreshPage();
         m_ignoreChange = true;
         insertAnimationNames();
         index--;

         if ( index < 0 )
         {
            index = 0;
         }
         m_animName->setCurrentItem( index );
         nameSelected( index );
         m_ignoreChange = false;
         m_model->operationComplete( tr( "Delete Animation", "Delete animation, operation complete" ).utf8() );
      }
   }
}

void AnimWidget::changeFPS()
{
   if ( !m_ignoreChange && m_animCount > 0 )
   {
      log_debug( "changing FPS\n" );
      m_model->setAnimFPS( m_mode, indexToAnim( m_animName->currentItem() ), atof(m_fps->text().utf8()) );
      m_model->operationComplete( tr( "Set FPS", "Frames per second, operation complete" ).utf8() );
   }
}

void AnimWidget::changeFrameCount()
{
   if ( !m_ignoreChange )
   {
      if ( m_animCount > 0 )
      {
         m_model->setAnimFrameCount( m_mode, indexToAnim( m_animName->currentItem() ), m_frameCount->value() );
         m_model->operationComplete( tr( "Change Frame Count", "operation complete" ).utf8() );
         m_countSlider->setMinValue( 1 );
         m_countSlider->setValue( m_model->getCurrentAnimationFrame() + 1 );
         setCurrentFrame( m_model->getCurrentAnimationFrame() + 1 );

         m_countSlider->setMaxValue( m_frameCount->value() );
         m_countSlider->update();
         DecalManager::getInstance()->modelAnimate( m_model );
      }
   }
}

void AnimWidget::clearFrame()
{
   Model::AnimationModeE mode = m_model->getAnimationMode();

   if ( mode == Model::ANIMMODE_FRAME
         || mode == Model::ANIMMODE_SKELETAL )
   {
      int anim  = m_model->getCurrentAnimation();
      int frame = m_model->getCurrentAnimationFrame();
      m_model->clearAnimFrame( mode, anim, frame );
      m_model->operationComplete( tr( "Clear frame", "Remove animation data from frame, operation complete" ).utf8() );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}

bool AnimWidget::copyFrame( bool selected )
{
   Model::AnimationModeE mode = m_model->getAnimationMode();

   m_frameCopyList.clear();
   m_framePointCopyList.clear();
   m_keyframeCopyList.clear();

   if ( mode == Model::ANIMMODE_FRAME )
   {
      unsigned numVertices = m_model->getVertexCount();
      unsigned numPoints = m_model->getPointCount();
      unsigned v = 0;

      FrameCopy copy;
      copy.vertex = 0;
      copy.x = 0;
      copy.y = 0;
      copy.z = 0;

      FramePointCopy copyPoint;
      copyPoint.point = 0;
      copyPoint.x = 0;
      copyPoint.y = 0;
      copyPoint.z = 0;
      copyPoint.rx = 0;
      copyPoint.ry = 0;
      copyPoint.rz = 0;

      for ( v = 0; v < numVertices; v++ )
      {
         if ( !selected || m_model->isVertexSelected(v) )
         {
            if ( m_model->getFrameAnimVertexCoords( m_currentAnim, m_currentFrame, v, copy.x, copy.y, copy.z ) )
            {
               copy.vertex = v;
               m_frameCopyList.push_back( copy );
            }
         }
      }

      for ( v = 0; v < numPoints; v++ )
      {
         if ( !selected || m_model->isPointSelected(v) )
         {
            if ( m_model->getFrameAnimPointCoords( m_currentAnim, m_currentFrame, v, copyPoint.x, copyPoint.y, copyPoint.z ) && m_model->getFrameAnimPointRotation( m_currentAnim, m_currentFrame, v, copyPoint.rx, copyPoint.ry, copyPoint.rz ) )
            {
               copyPoint.point = v;
               m_framePointCopyList.push_back( copyPoint );
            }
         }
      }

      return ( m_frameCopyList.size() > 0 ? true : false );
   }
   else if ( mode == Model::ANIMMODE_SKELETAL )
   {
      unsigned numJoints = m_model->getBoneJointCount();
      unsigned j = 0;

      KeyframeCopy copy;
      copy.joint = 0;
      copy.x = 0;
      copy.y = 0;
      copy.z = 0;
      copy.isRotation = false;

      for ( j = 0; j < numJoints; j++ )
      {
         if ( !selected || m_model->isBoneJointSelected(j) )
         {
            if ( m_model->getSkelAnimKeyframe( m_currentAnim, m_currentFrame, j, false, copy.x, copy.y, copy.z ) )
            {
               copy.joint = j;
               copy.isRotation = false;

               m_keyframeCopyList.push_back( copy );
            }
         }
      }

      for ( j = 0; j < numJoints; j++ )
      {
         if ( !selected || m_model->isBoneJointSelected(j) )
         {
            if ( m_model->getSkelAnimKeyframe( m_currentAnim, m_currentFrame, j, true, copy.x, copy.y, copy.z ) )
            {
               copy.joint = j;
               copy.isRotation = true;

               m_keyframeCopyList.push_back( copy );
            }
         }
      }

      return ( m_keyframeCopyList.size() > 0 ? true : false );
   }
   return false;
}

void AnimWidget::pasteFrame()
{
   Model::AnimationModeE mode = m_model->getAnimationMode();

   if ( mode == Model::ANIMMODE_FRAME )
   {
      if ( m_frameCopyList.empty() && m_framePointCopyList.empty() )
      {
         msg_error( tr("No frame animation data to paste").utf8() );
         return;
      }

      FrameCopyList::iterator it;
      FramePointCopyList::iterator pit;

      for ( it = m_frameCopyList.begin(); it != m_frameCopyList.end(); it++ )
      {
         m_model->setFrameAnimVertexCoords( m_currentAnim, m_currentFrame, (*it).vertex,
               (*it).x, (*it).y, (*it).z );
      }

      for ( pit = m_framePointCopyList.begin(); pit != m_framePointCopyList.end(); pit++ )
      {
         m_model->setFrameAnimPointCoords( m_currentAnim, m_currentFrame, (*pit).point,
               (*pit).x, (*pit).y, (*pit).z );
         m_model->setFrameAnimPointRotation( m_currentAnim, m_currentFrame, (*pit).point,
               (*pit).rx, (*pit).ry, (*pit).rz );
      }

      m_model->operationComplete( tr( "Paste frame", "paste frame animation position, operation complete" ).utf8() );

      m_model->setCurrentAnimationFrame( m_currentFrame );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
   else if ( mode == Model::ANIMMODE_SKELETAL )
   {
      KeyframeCopyList::iterator it;
      if ( m_keyframeCopyList.empty() )
      {
         msg_error( tr("No skeletal animation data to paste").utf8() );
         return;
      }

      for ( it = m_keyframeCopyList.begin(); it != m_keyframeCopyList.end(); it++ )
      {
         m_model->setSkelAnimKeyframe( m_currentAnim, m_currentFrame, (*it).joint, (*it).isRotation,
               (*it).x, (*it).y, (*it).z );
      }
      m_model->operationComplete( tr( "Paste keyframe", "Paste keyframe animation data complete" ).utf8() );

      m_model->setCurrentAnimationFrame( m_currentFrame );  // Force refresh of joints
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}

void AnimWidget::playClicked()
{
   if ( m_playing )
   {
      doPause();
   }
   else
   {
      doPlay();
   }
   m_stop->setEnabled( true );
}

void AnimWidget::stopClicked()
{
   m_currentTime = 0;
   m_stop->setEnabled( false );
   m_play->setEnabled(true);
   m_playing = false;
   m_animTimer->stop();
   m_model->setCurrentAnimationFrame( m_countSlider->value() - 1 );
   DecalManager::getInstance()->modelUpdated( m_model );
}

void AnimWidget::loopToggled( bool o )
{
   m_doLoop = o;
   m_model->setAnimationLooping( o );
   m_model->setCurrentAnimationFrame( m_countSlider->value() - 1 );
   DecalManager::getInstance()->modelUpdated( m_model );
}

void AnimWidget::doPlay()
{
   m_playing = true;
   if ( m_animCount == 0 )
   {
      return;
   }
   m_play->setEnabled(false);
   m_timeInterval = double (1.0 / m_model->getAnimFPS( m_mode, indexToAnim( m_animName->currentItem() ) ));

   PORT_gettimeofday( &m_startTime );

   m_animTimer->start( (int) (m_timeInterval * 1000), false );
   log_debug( "starting %s animation, update every %.03f seconds\n", (m_mode == Model::ANIMMODE_SKELETAL ? "skeletal" : "frame" ), m_timeInterval );
}

void AnimWidget::doPause()
{
   m_playing = false;
   m_play->setEnabled(true);
   m_animTimer->stop();
}

/*
void AnimWidget::closeEvent( QCloseEvent * e )
{
   AnimWidgetBase::closeEvent( e );
   //emit animWindowClosed();
   stopAnimationMode();
}
*/

void AnimWidget::close()
{
   log_debug( "hiding window on close\n" );
   emit animWindowClosed();
   stopAnimationMode();
   hide();
}

void AnimWidget::timeElapsed()
{
   PORT_timeval tv;
   PORT_gettimeofday( &tv );
   unsigned t = (tv.tv_sec - m_startTime.tv_sec) * 1000 + (tv.tv_msec - m_startTime.tv_msec);

   m_currentTime = ((double) t) / 1000.0;
   if ( m_model->setCurrentAnimationTime( m_currentTime ) )
   {
      //log_debug( "animation time: %f (%d)\n", m_currentTime, t );
   }
   else
   {
      stopClicked();
      //log_debug( "animation time: %f (complete)\n", m_currentTime );
   }
   DecalManager::getInstance()->modelAnimate( m_model );
}

void AnimWidget::undoRequest()
{
   log_debug( "anim undo request\n" );

   m_model->undo();

   undoGuts();
}

void AnimWidget::redoRequest()
{
   log_debug( "anim redo request\n" );

   m_model->redo();

   undoGuts();
}

void AnimWidget::undoGuts()
{
   if ( !m_model->getAnimationMode() )
   {
      stopAnimationMode();
      return;
   }

   m_undoing = true;

   Model::AnimationModeE mode = m_model->inSkeletalMode() ? Model::ANIMMODE_SKELETAL : Model::ANIMMODE_FRAME;
   unsigned anim   = m_model->getCurrentAnimation();
   unsigned frame  = m_model->getCurrentAnimationFrame();

   insertAnimationNames();

   m_animName->setCurrentItem( animToIndex( mode, anim ) );
   m_model->setCurrentAnimation( mode, anim );
   m_model->setCurrentAnimationFrame( frame );

   m_currentAnim  = anim;
   m_currentFrame = frame;
   m_mode         = mode;

   m_undoing = false;

   refreshPage();
   DecalManager::getInstance()->modelUpdated( m_model );
}

void AnimWidget::insertAnimationNames()
{
   m_animName->clear();

   unsigned int t;

   m_skelAnimCount = m_model->getAnimCount( Model::ANIMMODE_SKELETAL );
   for ( t = 0; t < m_skelAnimCount; t++ )
   {
      m_animName->insertItem( QString::fromUtf8( m_model->getAnimName( Model::ANIMMODE_SKELETAL, t )), t );
   }

   m_frameAnimCount = m_model->getAnimCount( Model::ANIMMODE_FRAME );
   for ( t = 0; t < m_frameAnimCount; t++ )
   {
      m_animName->insertItem( QString( m_model->getAnimName( Model::ANIMMODE_FRAME, t )), t + m_skelAnimCount );
   }

   m_animCount = m_skelAnimCount + m_frameAnimCount;

   m_animName->insertItem( tr( "<New Animation>" ), m_animCount );
}

void AnimWidget::accelActivated( int id )
{
   switch ( id )
   {
      case ANIMWIN_HELP_ID:
         helpNowEvent( id );
         break;
      case ANIMWIN_UNDO_ID:
         undoRequest();
         break;
      case ANIMWIN_REDO_ID:
         redoRequest();
         break;
      default:
         log_error( "Unknown animwindow accel id: %d", id );
         break;
   }
}

void AnimWidget::refreshPage()
{
   log_debug( "refresh anim window page\n" );

   if ( !m_undoing )
   {
      if( m_animCount > 0 )
      {
         int index = m_animName->currentItem();

         Model::AnimationModeE mode = indexToMode( index );
         index = indexToAnim( index );

         m_deleteButton->setEnabled( true );
         m_frameCount->setEnabled( true );
         m_fps->setEnabled( true );
         m_animName->setEnabled( true );
         m_countSlider->setEnabled( true );
         m_play->setEnabled( true );
         m_loop->setEnabled( true );

         unsigned count = m_model->getAnimFrameCount( mode, index );

         m_ignoreChange = true;  // Qt alerts us even if we're responsible
         m_frameCount->setValue( count );
         m_fps->setText( QString::number(m_model->getAnimFPS( mode, index ) ) );
         m_ignoreChange = false;

         m_countSlider->setMinValue( 1 );
         m_countSlider->setMaxValue( count );
         m_countSlider->setValue( m_currentFrame + 1 );
         m_countSlider->setTickInterval( getSliderTickInterval( count ) );
         m_countSlider->update();

         setCurrentFrame( m_currentFrame + 1 );
      }
      else
      {
         m_deleteButton->setEnabled( false );
         m_frameCount->setEnabled( false );
         m_fps->setEnabled( false );
         m_countSlider->setEnabled( false );
         m_play->setEnabled( false );
         m_stop->setEnabled( false );
         m_loop->setEnabled( false );

         m_fps->setText( QString("0") );
         m_ignoreChange = true;  // Qt alerts us even if we're responsible
         m_frameCount->setValue( 0 );
         m_ignoreChange = false;
         m_countSlider->setMinValue( 0 );
         m_countSlider->setMaxValue( 0 );
         m_countSlider->setValue( 0 );
         setCurrentFrame( 0 );
      }
   }
   else
   {
      if ( m_animCount > 0)
      {
         m_animName->setEnabled( true );
         m_frameCount->setEnabled( true );
         m_fps->setEnabled( true );
         m_countSlider->setEnabled( true );
         m_play->setEnabled( true );
         m_stop->setEnabled( true );
         m_loop->setEnabled( true );
      }
      else
      {
         //m_animName->setEnabled( false );
         m_frameCount->setEnabled( false );
         m_fps->setEnabled( false );
         m_countSlider->setEnabled( false );
         m_play->setEnabled( false );
         m_stop->setEnabled( false );
         m_loop->setEnabled( false );

         m_fps->setText( QString("0") );
         m_ignoreChange = true;  // Qt alerts us even if we're responsible
         m_frameCount->setValue( 0 );
         m_ignoreChange = false;
         m_countSlider->setMinValue( 0 );
         m_countSlider->setMaxValue( 0 );
         m_countSlider->setValue( 0 );
      }
   }
}

void AnimWidget::stopAnimationMode()
{
   if ( m_needShutdown )
   {
      if ( m_model->getAnimationMode() ) 
      {
         m_model->setNoAnimation();
         m_model->operationComplete( tr( "End animation mode", "operation complete" ).utf8() );
         DecalManager::getInstance()->modelUpdated( m_model );
      }
      emit animWindowClosed();
   }
   m_needShutdown = false;
}

bool AnimWidget::indexIsSkel( int index )
{
   if ( index >= (int) m_skelAnimCount )
   {
      return false;
   }
   else
   {
      return true;
   }
}

Model::AnimationModeE AnimWidget::indexToMode( int index )
{
   if ( index >= (int) m_skelAnimCount )
   {
      return Model::ANIMMODE_FRAME;
   }
   else
   {
      return Model::ANIMMODE_SKELETAL;
   }
}

int AnimWidget::indexToAnim( int index )
{
   if ( index >= (int) m_skelAnimCount )
   {
      return index -= m_skelAnimCount;
   }
   else
   {
      return index;
   }
}

int AnimWidget::animToIndex( Model::AnimationModeE mode, int anim )
{
   if ( mode == Model::ANIMMODE_FRAME )
   {
      return anim + m_skelAnimCount;
   }
   else
   {
      return anim;
   }
}

