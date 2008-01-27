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

#ifndef __KEYVALUEWIN_H
#define __KEYVALUEWIN_H

#include "keyvaluewin.base.h"

#include <QtGui/QDialog>

class QTreeWidgetItem;

class KeyValueWindow : public QDialog, public Ui::KeyValueWindowBase
{
      Q_OBJECT

   public:
      KeyValueWindow( QTreeWidgetItem * item, QWidget * parent = NULL );
      virtual ~KeyValueWindow();

   public slots:
      void accept();
      void reject();

   protected:
      QTreeWidgetItem * m_item;
};

#endif // __KEYVALUEWIN_H

