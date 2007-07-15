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


#ifndef __ANIMWIDGET_H
#define __ANIMWIDGET_H

#include "animwidget.base.h"

#include "mq3macro.h"

#include "mm3dport.h"

#include "model.h"

class QTimer;
class QAccel;

class AnimWidget : public AnimWidgetBase
{
   Q_OBJECT

   public:
      AnimWidget( Model * model, bool isUndo, QWidget * parent = NULL, const char * name = "" );
      virtual ~AnimWidget();

      void initialize( Model * model, bool isUndo );
      void stopAnimationMode();

      void doPlay();
      void doPause();

   signals:
      void animWindowClosed();

   public slots:
      void helpNowEvent( int );
      void close();

      void nameSelected(int);
      void setCurrentFrame(int);
      void changeFPS();
      void changeFrameCount();

      void timeElapsed();

      void playClicked();
      void stopClicked();
      void loopToggled(bool);

      bool copyFrame( bool selected );
      void pasteFrame();
      void clearFrame();

      void undoRequest();
      void redoRequest();
      void accelActivated( int id );

      void refreshPage();

   protected:
      bool indexIsSkel( int index );
      Model::AnimationModeE indexToMode( int index );
      int indexToAnim( int index );
      int animToIndex( Model::AnimationModeE mode, int anim );
      void undoGuts();

      Model  * m_model;
      bool     m_doLoop;
      bool     m_playing;
      double   m_timeInterval;
      double   m_currentTime;
      unsigned m_skelAnimCount;
      unsigned m_frameAnimCount;
      unsigned m_animCount;
      unsigned m_currentAnim;
      unsigned m_currentFrame;
      Model::AnimationModeE m_mode;
      bool     m_undoing;
      bool     m_ignoreChange;
      bool     m_needShutdown;

      PORT_timeval m_startTime;

      typedef struct _KeyframeCopy_t
      {
         unsigned joint;
         double x;
         double y;
         double z;
         bool   isRotation;
      } KeyframeCopy;

      typedef struct _FrameCopy_t
      {
         unsigned vertex;
         double x;
         double y;
         double z;
      } FrameCopy;

      typedef struct _FramePointCopy_t
      {
         unsigned point;
         double x;
         double y;
         double z;
         double rx;
         double ry;
         double rz;
      } FramePointCopy;

      typedef std::list<KeyframeCopy> KeyframeCopyList;
      typedef std::list<FrameCopy>    FrameCopyList;
      typedef std::list<FramePointCopy>    FramePointCopyList;

      KeyframeCopyList m_keyframeCopyList;
      FrameCopyList    m_frameCopyList;
      FramePointCopyList    m_framePointCopyList;

      QTimer * m_animTimer;
      QAccel * m_accel;
};

#endif // __ANIMWIDGET_H

