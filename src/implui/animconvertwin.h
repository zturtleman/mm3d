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

#ifndef __ANIMCONVERTWIN_H
#define __ANIMCONVERTWIN_H

#include "animconvertwin.base.h"

#include "model.h"

#include <QtGui/QDialog>

class Q3Accel;

class AnimConvertWindow : public QDialog, public Ui::AnimConvertWinBase
{
   Q_OBJECT

   public:
      enum _Operation_e
      {
         OP_CONTINUE,
         OP_CANCEL,
         OP_CANCEL_ALL
      };
      typedef enum _Operation_e OperationE;

      AnimConvertWindow( QWidget * parent = NULL );
      virtual ~AnimConvertWindow();

      void setAnimationData( Model * model, Model::AnimationModeE mode, unsigned animIndex );

      QString getNewName();
      unsigned getNewFrameCount();

      OperationE requestedOperation();

   public slots:
      void helpNowEvent();

      void continueClicked();
      void cancelClicked();
      void cancelAllClicked();

   protected:
      Q3Accel * m_accel;

      OperationE m_operation;
};

#endif // __ANIMCONVERTWIN_H
