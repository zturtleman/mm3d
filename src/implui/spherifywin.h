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

#ifndef __SPHERIFYWIN_H
#define __SPHERIFYWIN_H

#include "valuewin.h"
#include "model.h"

#include <list>

class Model;

class SpherifyWin : public ValueWin
{
   Q_OBJECT

   public:
      SpherifyWin( Model * model, QWidget * parent = NULL, const char * name = "" );
      virtual ~SpherifyWin();

   public slots:
      void valueEditChanged( const QString & );
      void valueSliderChanged( int );

      void accept();
      void reject();

   protected:
      virtual void showHelp();

      typedef struct _SpherifyPosition_t
      {
         Model::Position pos;
         double coords[3];
      } SpherifyPosition;
      typedef std::list<SpherifyPosition> SpherifyPositionList;

      Model * m_model;
      double  m_center[3];
      double  m_radius;
      SpherifyPositionList m_positions;
};

#endif // __SPHERIFYWIN_H
