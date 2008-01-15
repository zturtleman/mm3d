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


#ifndef __VALUEWIN_H
#define __VALUEWIN_H

#include "valuewin.base.h"

#include <QDialog>

class Q3Accel;

class ValueWin : public QDialog, public Ui::ValueWinBase
{
   Q_OBJECT

   public:

      ValueWin( QWidget * parent = NULL, bool modal = true,
            Qt::WFlags flags = 0 );
      virtual ~ValueWin();

      void setLabel( const char * newLabel );

      float getValue();
      void setValue( const float & v );

   public slots:
      void helpNowEvent( int );

      void valueEditChanged( const QString & );
      void valueSliderChanged( int );

   protected:

      virtual void showHelp();

      Q3Accel * m_accel;

      // We don't want to update the edit box if the user is typing in it
      // So we set this to true when editing.  When we update the slider,
      // our slot will check this value.  If false, it will update the edit box
      bool m_editing;
};

#endif // __VALUEWIN_H
