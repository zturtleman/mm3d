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


#ifndef __BOOLPANEL_H
#define __BOOLPANEL_H

#include "mq3macro.h"
#include "mq3compat.h"

#include "contextwidget.h"
#include "model.h"

class ViewPanel;
class BoolWin;

class BoolPanel : public QDockWindow, public Model::Observer
{
   Q_OBJECT

   public:
      BoolPanel( Model * model, QWidget * parent, ViewPanel * panel );
      virtual ~BoolPanel();

      // QDockWindow methods
   public slots:
      void show();
      void close();
      void hide();

      void setModel( Model * m );

      // Model::Observer methods
      void modelChanged( int changeBits );

   protected:
      void contextMenuEvent( QContextMenuEvent * e );
      Model      * m_model;
      ViewPanel  * m_panel;
      BoolWin    * m_boolWidget;
};

#endif // __BOOLPANEL_H
