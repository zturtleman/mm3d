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


#include "textureframe.h"

#include "texwidget.h"
#include "texture.h"
#include "model.h"
#include "log.h"

#include <QLayout>
#include <QResizeEvent>
#include <QFrame>

TextureFrame::TextureFrame( QWidget * parent )
   : QFrame( parent ),
     m_materialId( -1 ),
     m_texture( NULL ),
     m_3d( false ),
     m_overrideSize( false ),
     m_overWidth( 1 ),
     m_overHeight( 1 )
{
   setFrameStyle( StyledPanel );
   setFrameShadow( Sunken );

   m_textureWidget= new TextureWidget( this );
}

TextureFrame::~TextureFrame()
{
}

void TextureFrame::setModel( Model * model )
{
   m_model = model;
   if ( m_textureWidget )
   {
      m_textureWidget->setModel( model );
   }
}

void TextureFrame::set3d( bool o )
{
   m_3d = o;
   if ( m_textureWidget )
   {
      m_textureWidget->set3d( o );
      updateSize();
   }
}

void TextureFrame::resizeEvent( QResizeEvent * e )
{
   updateSize();
}

void TextureFrame::textureChangedEvent( int materialId )
{
   m_materialId = materialId - 1;
   m_texture = m_model->getTextureData( m_materialId );

   // Okay to pass null here
   m_textureWidget->setTexture( m_materialId, m_texture );
   updateSize();
}

void TextureFrame::resizeTexture( int width, int height )
{
   if ( m_texture || m_overrideSize )
   {
      if ( m_3d )
      {
         m_textureWidget->move( PAD_SIZE, PAD_SIZE );
         m_textureWidget->resize( width, height );
      }
      else
      {
         float x = 0.0;
         float y = 0.0;

         float scaleX = 1.0;
         float scaleY = 1.0;

         if ( m_overrideSize )
         {
            x = m_overWidth;
            y = m_overHeight;
         }
         else
         {
            if ( m_texture )
            {
               x = m_texture->m_origWidth;
               y = m_texture->m_origHeight;
            }
            else
            {
               x = 256.0;
               y = 256.0;
            }
         }

         scaleX = width / x;
         scaleY = height / y;

         if ( scaleX < scaleY )
         {
            m_textureWidget->move( PAD_SIZE, (int) ((this->height() - (y * scaleX)) / 2) + PAD_SIZE );
            m_textureWidget->resize( (int) (x * scaleX), (int) (y * scaleX) );
         }
         else
         {
            m_textureWidget->move( (int) ((this->width() - (x * scaleY)) / 2) + PAD_SIZE, PAD_SIZE );
            m_textureWidget->resize( (int) (x * scaleY), (int) (y * scaleY) );
         }
      }
   }
   else
   {
      m_textureWidget->move( PAD_SIZE, PAD_SIZE );
      m_textureWidget->resize( this->width() - PAD_SIZE * 2, this->height() - PAD_SIZE * 2);
   }
}

void TextureFrame::setTexture( int materialId, Texture * tex )
{
   m_materialId = materialId;
   m_texture = tex;
   m_textureWidget->setTexture( m_materialId, tex );
   updateSize();
}

void TextureFrame::updateSize()
{
   resizeTexture( width() - PAD_SIZE*2, height() - PAD_SIZE*2 );
}

void TextureFrame::sizeOverride( int width, int height )
{
   m_textureWidget->resize( 0, 0 );
   m_overrideSize = true;
   m_overWidth  = width;
   m_overHeight = height;
   updateSize();
}

