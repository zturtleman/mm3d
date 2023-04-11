/*  Maverick Model 3D
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

#ifndef __OFFSETWIN_H
#define __OFFSETWIN_H

#include "offsetwin.base.h"
#include "model.h"

#include <list>

class Model;

class OffsetWin : public QDialog, public Ui::OffsetWinBase
{
   Q_OBJECT

   public:
      OffsetWin( Model * model, QWidget * parent = NULL );
      virtual ~OffsetWin();

   public slots:
      void rangeEditChanged( const QString & );
      void valueEditChanged( const QString & );
      void valueSliderChanged( int );

      void accept();
      void reject();

   protected:
      virtual void showHelp();

      typedef struct _OffsetPosition_t
      {
         int vert;
         double coords[3];
         float normal[3];
      } OffsetPosition;
      typedef std::list<OffsetPosition> OffsetPositionList;

      Model * m_model;
      double  m_range;
      OffsetPositionList m_positions;

      // We don't want to update the edit box if the user is typing in it
      // So we set this to true when editing.  When we update the slider,
      // our slot will check this value.  If false, it will update the edit box
      bool m_editing;
};

#endif // __SPHERIFYWIN_H
