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


#ifndef __CONTEXTPOSITION_H
#define __CONTEXTPOSITION_H

#include "contextposition.base.h"

#include <QtWidgets/QWidget>

#include "contextwidget.h"

class Model;

class ContextPosition : public QWidget, public Ui::ContextPositionBase, public ContextWidget
{
   Q_OBJECT
   public:
      ContextPosition( QWidget * parent );
      virtual ~ContextPosition();

      // ContextWidget methods
      void setModel( Model * );
      void modelChanged( int changeBits );
      bool isUpdating() { return m_update; };

   signals:
      void panelChange();

   public slots:
      // Position slots
      void updatePosition();

   protected:
      Model * m_model;
      double  m_coords[3];
      bool    m_change;
      bool    m_update;
};

#endif // __CONTEXTPOSITION_H
