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

#include "contextposition.h"

#include "model.h"

#include <QtWidgets/QLineEdit>

#include <stdlib.h>

ContextPosition::ContextPosition( QWidget * parent )
   : QWidget( parent ),
     m_change( false ),
     m_update( false )
{
   setupUi( this );
}

ContextPosition::~ContextPosition()
{
}

void ContextPosition::setModel( Model * m )
{
   m_model = m;
   modelChanged( ~0 );
}

void ContextPosition::modelChanged( int changeBits )
{
   if ( !m_update )
   {
      m_change = true;

      // Update coordinates in text fields
      bool first = true;
      double cmin[3];
      double cmax[3];
      double coords[3];

      int i;

      for ( i = 0; i < 3; i++ )
      {
         cmin[i] = 0.0;
         cmax[i] = 0.0;
      }

      /*
      unsigned vcount = m_model->getVertexCount();
      for ( unsigned v = 0; v < vcount; v++ )
      {
         if ( m_model->isVertexSelected( v ) )
         {
            m_model->getVertexCoords( v, coords );

            if ( !first )
            {
               for ( i = 0; i < 3; i++ )
               {
                  if ( coords[i] < cmin[i] )
                     cmin[i] = coords[i];
                  if ( coords[i] > cmax[i] )
                     cmax[i] = coords[i];
               }
            }
            else
            {
               first = false;
               for ( i = 0; i < 3; i++ )
               {
                  cmin[i] = coords[i];
                  cmax[i] = coords[i];
               }
            }
         }
      }

      unsigned pcount = m_model->getPointCount();
      */

      std::list<Model::Position> posList;
      m_model->getSelectedPositions( posList );

      std::list<Model::Position>::iterator it;
      for ( it = posList.begin(); it != posList.end(); it++ )
      {
         m_model->getPositionCoords( *it, coords );

         if ( !first )
         {
            for ( i = 0; i < 3; i++ )
            {
               if ( coords[i] < cmin[i] )
                  cmin[i] = coords[i];
               if ( coords[i] > cmax[i] )
                  cmax[i] = coords[i];
            }
         }
         else
         {
            first = false;
            for ( i = 0; i < 3; i++ )
            {
               cmin[i] = coords[i];
               cmax[i] = coords[i];
            }
         }
      }

      for ( i = 0; i < 3; i++ )
      {
         m_coords[i] = (cmin[i] + cmax[i]) / 2.0;
      }

      QString str;

      str.sprintf( "%f", m_coords[0] );
      m_xValue->setText( str );

      str.sprintf( "%f", m_coords[1] );
      m_yValue->setText( str );

      str.sprintf( "%f", m_coords[2] );
      m_zValue->setText( str );

      str.sprintf( "%g, %g, %g", cmax[0]-cmin[0], cmax[1]-cmin[1], cmax[2]-cmin[2] );
      m_dimensionsValue->setText( str );

      m_change = false;
   }
}

void ContextPosition::updatePosition()
{
   if ( !m_change )
   {
      m_update = true;

      // Change model based on text field input
      double coords[3];
      double trans[3];
      coords[0] = m_xValue->text().toDouble();
      coords[1] = m_yValue->text().toDouble();
      coords[2] = m_zValue->text().toDouble();

      trans[0] = coords[0] - m_coords[0];
      trans[1] = coords[1] - m_coords[1];
      trans[2] = coords[2] - m_coords[2];

      m_coords[0] = coords[0];
      m_coords[1] = coords[1];
      m_coords[2] = coords[2];


      Matrix m;
      m.setTranslation( trans[0], trans[1], trans[2] );
      m_model->translateSelected( m );
      m_model->operationComplete( tr( "Set Position", "operation complete" ).toUtf8() );

      emit panelChange();

      m_update = false;
   }
}

