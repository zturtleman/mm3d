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


#ifndef __ALIGNWIN_H
#define __ALIGNWIN_H

#include "alignwin.base.h"

#include "align.h"

#include <QDialog>

class Model;

class AlignWin : public QDialog, public Ui::AlignWinBase
{
   Q_OBJECT

   public:
      AlignWin( Model *, QWidget * parent = NULL );
      virtual ~AlignWin();

      void alignX();
      void alignY();
      void alignZ();

      void selectedXCenter();
      void selectedXMin();
      void selectedXMax();

      void selectedYCenter();
      void selectedYMin();
      void selectedYMax();

      void selectedZCenter();
      void selectedZMin();
      void selectedZMax();

      void accept();
      void reject();

   public slots:
      void helpNowEvent();

   protected:
      Model * m_model;

      double m_xMin;
      double m_xMax;
      double m_yMin;
      double m_yMax;
      double m_zMin;
      double m_zMax;

      AlignTypeE m_atX;
      AlignTypeE m_atY;
      AlignTypeE m_atZ;
};

#endif // __ALIGNWIN_H
