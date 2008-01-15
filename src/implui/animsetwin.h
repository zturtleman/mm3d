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


#ifndef __ANIMSETWIN_H
#define __ANIMSETWIN_H

#include "animsetwin.base.h"

#include "model.h"

#include <QDialog>

class Q3Accel;

class AnimSetWindow : public QDialog, public Ui::AnimSetWinBase
{
      Q_OBJECT
   public:
      AnimSetWindow( Model * model, QWidget * parent = NULL );
      virtual ~AnimSetWindow();

   public slots:
      void helpNowEvent( int );
      void animModeSelected( int index );

      void upClicked();
      void downClicked();

      void newClicked();
      void renameClicked();
      void deleteClicked();

      void copyClicked();
      void splitClicked();
      void joinClicked();
      void mergeClicked();

      void convertClicked();

      void accept();
      void reject();

   protected:
      void fillAnimationList();
      Model::AnimationModeE indexToMode( int index );

      Q3Accel * m_accel;
      Model  * m_model;
};

#endif // __ANIMSETWIN_H
