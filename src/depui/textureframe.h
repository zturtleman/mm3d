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


#ifndef __TEXFRAME_H
#define __TEXFRAME_H

#include <QtWidgets/QFrame>
#include <QtGui/QResizeEvent>

class Model;
class TextureWidget;
class Texture;

class TextureFrame : public QFrame
{
   Q_OBJECT

   public:
      TextureFrame( QWidget * parent = NULL );
      virtual ~TextureFrame();

      void setModel( Model * model );
      void set3d( bool o );
      void resizeTexture( int width, int height );
      void updateSize();

      void sizeOverride( int width, int height );
      TextureWidget * getTextureWidget() { return m_textureWidget; };
      void setTexture( int materialId, Texture * tex );

      enum {
         PAD_SIZE = 6
      };

   public slots:

      void resizeEvent( QResizeEvent * e );
      void textureChangedEvent( int textureId );

   protected:

      TextureWidget * m_textureWidget;
      int             m_materialId;
      Texture       * m_texture;
      bool            m_3d;
      bool            m_overrideSize;
      int             m_overWidth;
      int             m_overHeight;
      Model * m_model;
};

#endif // __TEXFRAME_H
