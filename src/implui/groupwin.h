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


#ifndef __GROUPWIN_H
#define __GROUPWIN_H

#include "groupwin.base.h"

#include <QtWidgets/QDialog>

class Model;

class GroupWindow : public QDialog, public Ui::GroupWinBase
{
   Q_OBJECT

   public:
      GroupWindow( Model * model, QWidget * parent = NULL );
      virtual ~GroupWindow();

   public slots:
      void helpNowEvent();

      void newClickedEvent();
      void renameClickedEvent();
      void deleteClickedEvent();
      void selectFacesClickedEvent();
      void unselectFacesClickedEvent();
      void assignAsGroupClickedEvent();
      void addToGroupClickedEvent();

      void smoothChangedEvent(int);
      void angleChangedEvent(int);
      void groupSelectedEvent(int);
      void textureSelectedEvent(int);

      void accept();
      void reject();

   protected:
      void updateTexture();

      Model  * m_model;
};

#endif // __GROUPWIN_H
