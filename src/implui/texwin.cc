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


#include "texwin.h"
#include "textureframe.h"
#include "texwidget.h"
#include "model.h"
#include "texture.h"
#include "texmgr.h"
#include "log.h"
#include "rgbawin.h"
#include "valuewin.h"
#include "decalmgr.h"
#include "msg.h"
#include "3dmprefs.h"
#include "helpwin.h"
#include "errorobj.h"

#include "mq3compat.h"

#include <qcombobox.h>
#include <qpushbutton.h>
#include <list>
#include <string>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qlabel.h>

using std::list;
using std::string;

TextureWindow::TextureWindow( Model * model, QWidget * parent, const char * name )
   : TextureWindowBase( parent, name, true, WDestructiveClose ),
     m_accel( new QAccel(this) ),
     m_model( model ),
     m_editing( false ),
     m_setting( false )
{
   m_textureFrame->setModel( model );

   m_accel->insertItem( Key_F1, 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );

   int count = m_model->getTextureCount();

   for ( int t = 0; t < count; t++ )
   {
      //Texture * texture = m_model->getTextureData( t );
      //if ( texture )
      {
         m_textureComboBox->insertItem( QString::fromUtf8( m_model->getTextureName(t) ), t+1 );
      }
   }

   int textureId = -1;
   list<int> triangles;
   m_model->getSelectedTriangles( triangles );

   list<int>::iterator it;
   for ( it = triangles.begin(); it != triangles.end(); it++ )
   {
      int g = m_model->getTriangleGroup( *it );
      if ( g >= 0 )
      {
         textureId = m_model->getGroupTextureId( g );
         break;
      }
   }

   m_textureComboBox->setCurrentItem( textureId + 1 );
   textureChangedEvent( textureId + 1 );

   int previewIndex = 0;
   if ( g_prefs.exists( "ui_texwin_preview_index" ) )
   {
      int val = g_prefs( "ui_texwin_preview_index" ).intValue();
      if ( val >= 0 && val <= 1 )
      {
         previewIndex = val;
      }
   }

   m_previewType->setCurrentItem( previewIndex );
   previewValueChanged( previewIndex );
}

TextureWindow::~TextureWindow()
{
}

void TextureWindow::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_texturewin.html", true );
   win->show();
}

void TextureWindow::changeTextureFileEvent()
{
   list<string> formats = TextureManager::getInstance()->getAllReadTypes();

   QString formatsStr = tr( "All Supported Formats (", "all texture formats" );

   list<string>::iterator it = formats.begin();
   while(  it != formats.end() )
   {
      formatsStr += QString( (*it).c_str() );

      it++;

      if ( it != formats.end() )
      {
         formatsStr += " ";
      }
   }

   formatsStr += ")";

   QString dir = QString::fromUtf8( g_prefs( "ui_texture_dir" ).stringValue().c_str() );
   if ( dir.isEmpty() )
   {
      dir = ".";
   }

#ifdef HAVE_QT4
   QFileDialog d(NULL, "", dir, formatsStr + QString(";; All Files (*)" ) );

   d.setCaption( tr("Open texture image") );
   d.selectFilter( formatsStr );

   int execval = d.exec();
   std::string file = (const char *) d.selectedFile().utf8();
   QString path = d.directory().absolutePath().utf8();
#else
   QFileDialog d(dir, formatsStr, NULL, "", true );

   d.setCaption( tr("Open texture image") );
   d.addFilter( tr( "All Files (*)" ) );
   d.setSelectedFilter( formatsStr );

   int execval = d.exec();
   std::string file = (const char *) d.selectedFile().utf8();
   QString path = d.dir()->absPath();
#endif

   if ( QDialog::Accepted == execval )
   {
      g_prefs( "ui_texture_dir" ) = (const char *) path.utf8();

      Texture * tex = TextureManager::getInstance()->getTexture( file.c_str() );

      if ( tex )
      {
         int textureId = m_textureComboBox->currentItem() - 1;
         m_model->setMaterialTexture( textureId, tex );
         log_debug( "changed texture %d to %s\n", textureId, file.c_str() );
         textureChangedEvent( textureId + 1 );
      }
      else
      {
         QString err = tr(file.c_str()) + "\n";
         Texture::ErrorE e = TextureManager::getInstance()->getLastError();
         if ( e != Texture::ERROR_NONE )
         {
            err += textureErrStr( e );
         }
         else
         {
            err += tr("Could not open file");
         }
         msg_error( (const char *) err.utf8() );
      }
   }
}

void TextureWindow::noTextureFileEvent()
{
   int textureId = m_textureComboBox->currentItem() - 1;
   m_model->removeMaterialTexture( textureId );
   log_debug( "removed texture from material %d\n", textureId );
   textureChangedEvent( textureId + 1 );
}

void TextureWindow::newMaterialClickedEvent()
{
   bool ok = false;
   QString name = QInputDialog::getText( tr( "Color Material", "window title" ), tr( "Enter new material name:" ), QLineEdit::Normal, QString(""), &ok );

   if ( ok )
   {
      int num = m_model->addColorMaterial( name.utf8() );
      m_textureComboBox->insertItem( QString::fromUtf8( m_model->getTextureName(num)), num+1 );
      m_textureComboBox->setCurrentItem( num+1 );
      textureChangedEvent( num+1 );
      log_debug( "added %s as %d\n", (const char *) name.utf8(), num );
   }
}

void TextureWindow::renameClickedEvent()
{
   int textureId = m_textureComboBox->currentItem() - 1;

   if ( textureId >= 0 )
   {
      bool ok = false;
      QString name = QInputDialog::getText( tr( "Rename texture", "window title" ), tr( "Enter new texture name:" ), QLineEdit::Normal, QString::fromUtf8( m_model->getTextureName( textureId ) ), &ok );

      if ( ok )
      {
         m_model->setTextureName( textureId, name.utf8() );
         m_textureComboBox->changeItem( name, textureId + 1 );
      }
   }
}

void TextureWindow::deleteClickedEvent()
{
   int id = m_textureComboBox->currentItem();
   if ( id > 0 )
   {
      m_model->deleteTexture( id - 1 );
      m_textureComboBox->removeItem( id );
      m_textureComboBox->setCurrentItem( 0 );
      textureChangedEvent( 0 );
   }
}

void TextureWindow::clampSChangedEvent( int index )
{
   int id = m_textureComboBox->currentItem() - 1;

   if ( id >= 0 )
   {
      m_model->setTextureSClamp( id, (index == 1) ? true : false );
      m_textureFrame->getTextureWidget()->setSClamp( (index == 1) ? true : false );
   }
}

void TextureWindow::clampTChangedEvent( int index )
{
   int id = m_textureComboBox->currentItem() - 1;

   if ( id >= 0 )
   {
      m_model->setTextureTClamp( id, (index == 1) ? true : false );
      m_textureFrame->getTextureWidget()->setTClamp( (index == 1) ? true : false );
   }
}

/*
void TextureWindow::shininessClickedEvent()
{
   int id = (unsigned) m_textureComboBox->currentItem() - 1;
   float val;

   if ( m_model->getTextureShininess( id, val) )
   {
      ValueWin value;

      value.setLabel( "Shininess" );

      value.setValue( val );

      if ( value.exec() )
      {
         val = value.getValue();
         m_model->setTextureShininess( id, val );
         DecalManager::getInstance()->modelUpdated( m_model );
      }
   }
   else
   {
      log_error( "could not get shininess values for %d\n", id );
   }
}
*/

void TextureWindow::textureChangedEvent( int id )
{
   m_textureFrame->textureChangedEvent( id );

   id = id - 1;

   if ( id >= 0 )
   {
      m_redSlider->setEnabled( true );
      m_greenSlider->setEnabled( true );
      m_blueSlider->setEnabled( true );
      m_alphaSlider->setEnabled( true );
      m_redEdit->setEnabled( true );
      m_greenEdit->setEnabled( true );
      m_blueEdit->setEnabled( true );
      m_alphaEdit->setEnabled( true );
      m_lightingValue->setEnabled( true );
      m_deleteButton->setEnabled( true );
      m_renameButton->setEnabled( true );
      m_sClamp->setEnabled( true );
      m_tClamp->setEnabled( true );
      m_fileButton->setEnabled( true );

      lightValueChanged( m_lightingValue->currentItem() );
      m_sClamp->setCurrentItem( (m_model->getTextureSClamp( id )) ? 1 : 0 );
      m_tClamp->setCurrentItem( (m_model->getTextureTClamp( id )) ? 1 : 0 );
      m_textureFrame->getTextureWidget()->setSClamp( m_model->getTextureSClamp( id ) );
      m_textureFrame->getTextureWidget()->setTClamp( m_model->getTextureTClamp( id ) );
   }
   else
   {
      m_redSlider->setEnabled( false );
      m_greenSlider->setEnabled( false );
      m_blueSlider->setEnabled( false );
      m_alphaSlider->setEnabled( false );
      m_redEdit->setEnabled( false );
      m_greenEdit->setEnabled( false );
      m_blueEdit->setEnabled( false );
      m_alphaEdit->setEnabled( false );
      m_lightingValue->setEnabled( false );
      m_deleteButton->setEnabled( false );
      m_renameButton->setEnabled( false );
      m_sClamp->setEnabled( false );
      m_tClamp->setEnabled( false );
      m_fileButton->setEnabled( false );
      m_noFileButton->setEnabled( false );
   }

   updateChangeButton();
}

void TextureWindow::updateEvent()
{
   int id = (unsigned) m_textureComboBox->currentItem() - 1;

   float val[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
   val[0] = m_redSlider->value()   / 100.0;
   val[1] = m_greenSlider->value() / 100.0;
   val[2] = m_blueSlider->value()  / 100.0;
   val[3] = m_alphaSlider->value() / 100.0;

   switch ( m_lightingValue->currentItem() )
   {
      case 0:
         m_model->setTextureAmbient( id, val );
         break;
      case 1:
         m_model->setTextureDiffuse( id, val );
         break;
      case 2:
         m_model->setTextureSpecular( id, val );
         break;
      case 3:
         m_model->setTextureEmissive( id, val );
         break;
      case 4:
         m_model->setTextureShininess( id, val[0] * 100.0);
         break;
      default:
         break;
   }
   DecalManager::getInstance()->modelUpdated( m_model );
   m_textureFrame->getTextureWidget()->updateGL();
}

void TextureWindow::accept()
{
   m_model->operationComplete( tr("Texture changes").utf8() );
   TextureWindowBase::accept();
   DecalManager::getInstance()->modelUpdated( m_model );
}

void TextureWindow::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   TextureWindowBase::reject();
}

void TextureWindow::previewValueChanged( int index )
{
   m_textureFrame->set3d( (index == 1) ? true : false );

   g_prefs( "ui_texwin_preview_index" ) = index;
}

void TextureWindow::lightValueChanged( int index )
{
   int id = (unsigned) m_textureComboBox->currentItem() - 1;
   float val[4];

   bool haveLighting = false;

   switch ( m_lightingValue->currentItem() )
   {
      case 0: // Ambient
         haveLighting = m_model->getTextureAmbient( id, val);
         break;
      case 1: // Diffuse
         haveLighting = m_model->getTextureDiffuse( id, val);
         break;
      case 2: // Specular
         haveLighting = m_model->getTextureSpecular( id, val);
         break;
      case 3: // Emissive
         haveLighting = m_model->getTextureEmissive( id, val);
         break;
      case 4: // Shininess
         haveLighting = m_model->getTextureShininess( id, val[0] );
         break;
      default:
         break;
   }

   if ( m_lightingValue->currentItem() == 4 )
   {
      m_redLabel->setText( tr( "Shininess" ) );
      m_redSlider->setMaxValue( 100 );
      m_redSlider->setMinValue( 0 );

      m_greenLabel->hide();
      m_greenSlider->hide();
      m_greenEdit->hide();

      m_blueLabel->hide();
      m_blueSlider->hide();
      m_blueEdit->hide();

      m_alphaLabel->hide();
      m_alphaSlider->hide();
      m_alphaEdit->hide();
   }
   else
   {
      m_redLabel->setText( tr( "Red" ) );
      m_redSlider->setMaxValue( 100 );
      m_redSlider->setMinValue( -100 );

      m_greenLabel->show();
      m_greenSlider->show();
      m_greenEdit->show();

      m_blueLabel->show();
      m_blueSlider->show();
      m_blueEdit->show();

      m_alphaLabel->show();
      m_alphaSlider->show();
      m_alphaEdit->show();
   }

   if ( haveLighting )
   {
      RgbaWin rgba;

      m_setting = true;
      m_redSlider->setValue( (int) (val[0] * 100) );
      m_greenSlider->setValue( (int) (val[1] * 100) );
      m_blueSlider->setValue( (int) (val[2] * 100) );
      m_alphaSlider->setValue( (int) (val[3] * 100) );
      m_setting = false;
   }
   else
   {
      log_error( "could not get lighting values for %d\n", id );
   }
}

//------------------------------------------------------------------
// RGBA Functions
//------------------------------------------------------------------

void TextureWindow::redSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%1.02f", (float) v / 100.0 );
      m_redEdit->setText( str );
   }
   if ( !m_setting )
   {
      updateEvent();
   }
}

void TextureWindow::greenSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%1.02f", (float) v / 100.0 );
      m_greenEdit->setText( str );
   }
   if ( !m_setting )
   {
      updateEvent();
   }
}

void TextureWindow::blueSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%1.02f", (float) v / 100.0 );
      m_blueEdit->setText( str );
   }
   if ( !m_setting )
   {
      updateEvent();
   }
}

void TextureWindow::alphaSliderChanged( int v )
{
   if ( ! m_editing )
   {
      QString str;
      str.sprintf( "%1.02f", (float) v / 100 );
      m_alphaEdit->setText( str );
   }
   if ( !m_setting )
   {
      updateEvent();
   }
}

void TextureWindow::redEditChanged( const QString & str )
{
   m_editing = true;
   float v = atof( str.latin1() );
   m_redSlider->setValue( (int) (v * 100) );
   m_editing = false;
}

void TextureWindow::greenEditChanged( const QString & str )
{
   m_editing = true;
   float v = atof( str.latin1() );
   m_greenSlider->setValue( (int) (v * 100) );
   m_editing = false;
}

void TextureWindow::blueEditChanged( const QString & str )
{
   m_editing = true;
   float v = atof( str.latin1() );
   m_blueSlider->setValue( (int) (v * 100) );
   m_editing = false;
}

void TextureWindow::alphaEditChanged( const QString & str )
{
   m_editing = true;
   float v = atof( str.latin1() );
   m_alphaSlider->setValue( (int) (v * 100) );
   m_editing = false;
}

void TextureWindow::updateChangeButton()
{
   int textureId = m_textureComboBox->currentItem() - 1;

   if ( textureId >= 0 )
   {
      if ( m_model->getMaterialType( textureId ) == Model::Material::MATTYPE_TEXTURE )
      {
         m_fileButton->setText( tr( "Change texture...", "Change material's texture file" ) );
         m_noFileButton->setEnabled( true );
      }
      else
      {
         m_fileButton->setText( tr( "Set texture...", "Add texture file to material" ) );
         m_noFileButton->setEnabled( false );
      }
   }
}

