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


#ifndef __EXTRUDEWIN_H
#define __EXTRUDEWIN_H

#include "extrudewin.base.h"

#include <list>
#include <map>

#include <QDialog>

class Model;

class ExtrudeWin : public QDialog, public Ui::ExtrudeWinBase
{
   Q_OBJECT

   public:
      ExtrudeWin( Model *, QWidget * parent = NULL );
      virtual ~ExtrudeWin();

   public slots:
      void helpNowEvent();

      void absoluteExtrudeEvent();
      void normalExtrudeEvent();
      void backFacesChanged( bool );

   protected:
      struct _Side_t
      {
         unsigned a;
         unsigned b;
         int count;
      };
      typedef struct _Side_t SideT;

      typedef struct
      {
         float val[3];
      } VertexNormal;

      typedef std::list<SideT>    SideList;
      typedef std::map<int, int> ExtrudedVertexMap;
      typedef std::map<int, VertexNormal> VertexNormalMap;

      void makeFaces( unsigned a, unsigned b );

      void addSide( unsigned a, unsigned b );
      bool sideIsEdge( unsigned a, unsigned b );

      Model * m_model;

      SideList          m_sides;
      ExtrudedVertexMap m_evMap;
      VertexNormalMap   m_vnMap;
};

#endif // __EXTRUDEWIN_H
