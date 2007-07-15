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

#include "contextname.h"

#include "model.h"

#include <qlineedit.h>
#include <stdlib.h>

ContextName::ContextName( QWidget * parent )
   : ContextNameBase( parent ),
     m_change( false ),
     m_update( false )
{
}

ContextName::~ContextName()
{
}

void ContextName::setModel( Model * m )
{
   m_model = m;
   modelChanged( ~0 );
}

void ContextName::modelChanged( int changeBits )
{
   if ( !m_update )
   {
      m_change = true;

      unsigned bcount = m_model->getBoneJointCount();
      for ( unsigned b = 0; b < bcount; b++ )
      {
         if ( m_model->isBoneJointSelected( b ) )
         {
            m_name->setText( QString::fromUtf8( m_model->getBoneJointName( b ) ) );
            break;
         }
      }

      unsigned pcount = m_model->getPointCount();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         if ( m_model->isPointSelected( p ) )
         {
            m_name->setText( QString::fromUtf8( m_model->getPointName( p ) ) );
            break;
         }
      }

      m_change = false;
   }
}

void ContextName::textChangedEvent( const QString & nameStr )
{
   if ( !m_change )
   {
      m_update = true;

      // Change model based on text field input
      unsigned bcount = m_model->getBoneJointCount();
      for ( unsigned b = 0; b < bcount; b++ )
      {
         if ( m_model->isBoneJointSelected( b ) )
         {
            m_model->setBoneJointName( b, (const char*) nameStr.utf8() );
            break;
         }
      }

      unsigned pcount = m_model->getPointCount();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         if ( m_model->isPointSelected( p ) )
         {
            m_model->setPointName( p, (const char*) nameStr.utf8() );
            break;
         }
      }

      m_model->operationComplete( tr( "Rename", "operation complete" ).utf8() );

      emit panelChange();

      m_update = false;
   }
}

