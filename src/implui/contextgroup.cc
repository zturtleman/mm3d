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

#include "contextgroup.h"

#include "model.h"
#include "contextpanelobserver.h"
#include "groupwin.h"
#include "texwin.h"
#include "projectionwin.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#include <stdlib.h>

ContextGroup::ContextGroup( QWidget * parent, ContextPanelObserver * ob )
   : ContextGroupBase( parent ),
     m_model( NULL ),
     m_observer( ob ),
     m_change( false ),
     m_update( false )
{
}

ContextGroup::~ContextGroup()
{
}

void ContextGroup::setModel( Model * m )
{
   m_model = m;
   modelChanged( ~0 );
}

void ContextGroup::modelChanged( int changeBits )
{
   // Only change if it's a group change or a selection change
   if ( (changeBits & Model::AddOther) || (changeBits & Model::SelectionChange ) )
   {
      if ( !m_update )
      {
         m_change = true;

         // Update group fields
         m_groupValue->clear();
         m_groupValue->insertItem( tr( "<None>" ), 0 );
         m_materialValue->clear();
         m_materialValue->insertItem( tr( "<None>" ), 0 );
         m_projectionValue->clear();
         m_projectionValue->insertItem( tr( "<None>" ), 0 );

         unsigned int gcount = m_model->getGroupCount();
         for ( unsigned int g = 0; g < gcount; g++ )
         {
            m_groupValue->insertItem( QString::fromUtf8( m_model->getGroupName( g ) ), g + 1 );
         }
         m_groupValue->insertItem( tr( "<New>" ), m_groupValue->count() );

         unsigned int mcount = m_model->getTextureCount();
         for ( unsigned int m = 0; m < mcount; m++ )
         {
            m_materialValue->insertItem( QString::fromUtf8( m_model->getTextureName( m ) ), m + 1 );
         }

         unsigned int pcount = m_model->getProjectionCount();
         for ( unsigned int p = 0; p < pcount; p++ )
         {
            m_projectionValue->insertItem( QString::fromUtf8( m_model->getProjectionName( p ) ), p + 1 );
         }

         int group = -1;
         int proj  = -1;
         unsigned tcount = m_model->getTriangleCount();
         for ( unsigned t = 0; (group < 0 || proj < 0) && t < tcount; t++ )
         {
            if ( m_model->isTriangleSelected( t ) )
            {
               if ( group < 0 )
               {
                  group = m_model->getTriangleGroup( t );
               }
               if ( proj < 0 )
               {
                  proj = m_model->getTriangleProjection( t );
               }
            }
         }

         if ( group >= 0 )
         {
            m_materialValue->setEnabled( true );
            m_materialProperties->setEnabled( true );
            m_groupValue->setCurrentItem( group + 1 );
            int texId = m_model->getGroupTextureId( group );
            if ( texId >= 0 )
            {
               m_materialValue->setCurrentItem( texId + 1 );
            }
         }
         else
         {
            m_materialValue->setEnabled( false );
            m_materialProperties->setEnabled( false );
         }

         if ( proj >= 0 )
         {
            m_projectionProperties->setEnabled( true );
            m_projectionValue->setCurrentItem( proj + 1 );
         }
         else
         {
            m_projectionProperties->setEnabled( false );
         }

         m_change = false;
      }
   }
   m_lastGroup = m_groupValue->currentItem();
}

void ContextGroup::groupChanged()
{
   if ( !m_change )
   {
      m_update = true;

      int g = m_groupValue->currentItem() - 1;

      if ( g >= 0 )
      {
         m_materialValue->setEnabled( true );
         m_materialProperties->setEnabled( true );

         bool addSelected = true;
         if ( g >= m_model->getGroupCount() )
         {
            // FIXME pick unique name?
            QString groupName = QInputDialog::getText( tr("New Group", "Name of new group, window title" ), tr("Enter new group name:"), QLineEdit::Normal, QString::null, &addSelected );
            if ( groupName.length() == 0 )
            {
               addSelected = false;
            }

            if ( addSelected )
            {
               m_model->addGroup( groupName.utf8() );
               m_groupValue->changeItem( groupName, g + 1 );
               m_groupValue->insertItem( tr( "<New>" ), m_groupValue->count() );
            }
            else
            {
               m_groupValue->setCurrentItem( m_lastGroup );
            }
         }

         if ( addSelected )
         {
            m_model->addSelectedToGroup( g );
            m_model->operationComplete( tr( "Set Group", "operation complete" ).utf8() );
         }

         int texId = m_model->getGroupTextureId( g );
         if ( texId >= 0 )
         {
            m_materialValue->setCurrentItem( texId + 1 );
         }
         else
         {
            m_materialValue->setCurrentItem( 0 );
         }

      }
      else
      {
         m_materialValue->setEnabled( false );
         m_materialProperties->setEnabled( false );

         unsigned tcount = m_model->getTriangleCount();
         for ( unsigned t = 0; t < tcount; t++ )
         {
            if ( m_model->isTriangleSelected( t ) )
            {
               g = m_model->getTriangleGroup( t );
               if ( g >= 0 )
               {
                  m_model->removeTriangleFromGroup( g, t );
               }
            }
         }
         m_model->operationComplete( tr( "Unset Group", "operation complete" ).utf8() );
      }

      emit panelChange();

      m_update = false;
   }
}

void ContextGroup::materialChanged()
{
   if ( !m_change )
   {
      m_update = true;

      int g = m_groupValue->currentItem() - 1;
      int m = m_materialValue->currentItem() - 1;

      if ( g >= 0 )
      {
         m_model->setGroupTextureId( g, m );
         m_model->operationComplete( tr( "Set Material", "operation complete" ).utf8() );
      }

      emit panelChange();

      m_update = false;
   }
}

void ContextGroup::projectionChanged()
{
   if ( !m_change )
   {
      m_update = true;

      int proj = m_projectionValue->currentItem() - 1;

      unsigned tcount = m_model->getTriangleCount();
      for ( unsigned t = 0; t < tcount; t++ )
      {
         if ( m_model->isTriangleSelected( t ) )
         {
            m_model->setTriangleProjection( t, proj );
         }
      }

      if ( proj >= 0 )
      {
         m_model->applyProjection( proj );
         m_projectionProperties->setEnabled( true );
      }
      else
      {
         m_projectionProperties->setEnabled( false );
      }

      m_model->operationComplete( tr( "Set Projection", "operation complete" ).utf8() );

      emit panelChange();

      m_update = false;
   }
}

void ContextGroup::groupPropertiesClicked()
{
   GroupWindow * win = new GroupWindow( m_model );
   win->show();
}

void ContextGroup::materialPropertiesClicked()
{
   TextureWindow * win = new TextureWindow( m_model );
   win->show();
}

void ContextGroup::projectionPropertiesClicked()
{
   m_observer->showProjectionEvent();
}

