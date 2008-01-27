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


#ifndef __BACKGROUNDWIN_H
#define __BACKGROUNDWIN_H

#include "backgroundwin.base.h"

#include <QtGui/QDialog>

class Model;
class BackgroundSelect;

class BackgroundWin : public QDialog, public Ui::BackgroundWinBase
{
   Q_OBJECT

   public:
      BackgroundWin( Model *, QWidget * parent = NULL );
      virtual ~BackgroundWin();

   public slots:
      void selectedPageEvent( const QString & str );
      void accept();
      void reject();

      void helpNowEvent();

   protected:
      Model * m_model;
      BackgroundSelect * m_bgSelect[6];
};

#endif // __BACKGROUNDWIN_H
