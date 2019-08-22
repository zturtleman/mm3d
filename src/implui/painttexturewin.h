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


#ifndef __PAINTTEXTUREWIN_H
#define __PAINTTEXTUREWIN_H

#include <QtGui/QPixmap>
#include "painttexturewin.base.h"

#include <list>
#include <map>

using std::list;
using std::map;

#include <QtWidgets/QDialog>

class Model;
class TextureWidget;

class PaintTextureWin : public QDialog, public Ui::PaintTextureWinBase
{
   Q_OBJECT
   public:
      PaintTextureWin( Model * model, QWidget * parent = NULL );
      ~PaintTextureWin();

   public slots:
      void helpNowEvent();
      void textureSizeChangeEvent();
      void displayChangedEvent();
      void clearEvent();
      void saveEvent();

      void accept();

   protected:
      void updateDisplay();
      void addTriangles( const list<int> & triangles );

      TextureWidget * m_textureWidget;
      Model * m_model;
      bool    m_saved;
};

#endif //  __PAINTTEXTUREWIN_H
