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


#ifndef __TEXWIN_H
#define __TEXWIN_H

#include "texwin.base.h"

#include <QtWidgets/QDialog>

class Model;

class TextureWindow : public QDialog, public Ui::TextureWindowBase
{
   Q_OBJECT

   public:
      TextureWindow( Model * model, QWidget * parent = NULL );
      virtual ~TextureWindow();

   public slots:
      void helpNowEvent();

      void updateEvent();

      void accept();
      void reject();

      void textureChangedEvent(int id );

      void changeTextureFileEvent();
      void noTextureFileEvent();

      void newMaterialClickedEvent();
      void renameClickedEvent();
      void deleteClickedEvent();

      void clampSChangedEvent(int index );
      void clampTChangedEvent(int index );

      void lightValueChanged(   int );
      void previewValueChanged(   int );

      void redSliderChanged(   int );
      void greenSliderChanged( int );
      void blueSliderChanged(  int );
      void alphaSliderChanged( int );

      void redEditChanged(   const QString & );
      void greenEditChanged( const QString & );
      void blueEditChanged(  const QString & );
      void alphaEditChanged( const QString & );

   protected:
      void updateChangeButton();

      Model  * m_model;
      bool      m_editing;
      bool      m_setting;
};

#endif // __TEXWIN_H
