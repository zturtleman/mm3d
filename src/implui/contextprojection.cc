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

#include "contextprojection.h"

#include "model.h"
#include "contextpanelobserver.h"
#include "groupwin.h"
#include "texwin.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>

#include <stdlib.h>

ContextProjection::ContextProjection( QWidget * parent, ContextPanelObserver * ob )
   : QWidget( parent ),
     m_model( NULL ),
     m_observer( ob ),
     m_change( false ),
     m_update( false )
{
   setupUi( this );
}

ContextProjection::~ContextProjection()
{
}

void ContextProjection::setModel( Model * m )
{
   m_model = m;
   modelChanged( ~0 );
}

void ContextProjection::modelChanged( int changeBits )
{
   // Only change if it's a group change or a selection change
   if ( (changeBits & Model::AddOther) || (changeBits & Model::SelectionChange ) )
   {
      if ( !m_update )
      {
         m_change = true;

         // Update projection fields

         unsigned int pcount = m_model->getProjectionCount();
         for ( unsigned int p = 0; p < pcount; p++ )
         {
            if ( m_model->isProjectionSelected( p ) )
            {
               m_typeValue->setCurrentIndex( m_model->getProjectionType( p ) );
               break;
            }
         }

         m_change = false;
      }
   }
}

void ContextProjection::typeChanged()
{
   if ( !m_change )
   {
      m_update = true;

      int type = m_typeValue->currentIndex();

      unsigned pcount = m_model->getProjectionCount();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         if ( m_model->isProjectionSelected( p ) )
         {
            m_model->setProjectionType( p, type );
         }
      }
      m_model->operationComplete( tr( "Set Projection Type", "operation complete" ).toUtf8() );

      emit panelChange();

      m_update = false;
   }
}

void ContextProjection::projectionPropertiesClicked()
{
   m_observer->showProjectionEvent();
}
