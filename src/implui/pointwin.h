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


#ifndef __POINTWIN_H
#define __POINTWIN_H

#include "pointwin.base.h"

#include <QtGui/QDialog>

class Model;
class Q3Accel;

class PointWin : public QDialog, public Ui::PointWinBase
{
   Q_OBJECT

   public:
      PointWin( Model * model, QWidget * parent = NULL );
      virtual ~PointWin();

   public slots:
      void helpNowEvent();

      void deleteClicked();
      void renameClicked();
      void pointNameSelected( int index );
      void pointJointSelected( int index );

   protected slots:
      void accept();
      void reject();

   protected:
      Q3Accel * m_accel;
      Model  * m_model;
};

#endif // __POINTWIN_H
