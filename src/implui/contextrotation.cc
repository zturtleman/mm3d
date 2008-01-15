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

#include "contextrotation.h"

#include "model.h"

#include <QLineEdit>

#include <stdlib.h>

ContextRotation::ContextRotation( QWidget * parent )
   : QWidget( parent ),
     m_change( false ),
     m_update( false )
{
   setupUi( this );
}

ContextRotation::~ContextRotation()
{
}

void ContextRotation::setModel( Model * m )
{
   m_model = m;
   modelChanged( ~0 );
}

void ContextRotation::modelChanged( int changeBits )
{
   // FIXME deal with animation
   // FIXME only allow points in None and Frame
   // FIXME only allow joints (for keyframe) in Skel
   if ( m_model->getAnimationMode() == Model::ANIMMODE_SKELETAL )
   {
      //return;
   }

   if ( !m_update )
   {
      m_change = true;

      bool searching = true;

      // Update coordinates in text fields
      double rad[3];

      memset( rad, 0, sizeof(rad) );

      unsigned int pcount = m_model->getPointCount();
      for ( unsigned int p = 0; searching && p < pcount; p++ )
      {
         if ( m_model->isPointSelected( p ) )
         {
            searching = false;
            m_model->getPointRotation( p, rad );
         }
      }

      if ( m_model->getAnimationMode() == Model::ANIMMODE_SKELETAL )
      {
         unsigned int bcount = m_model->getBoneJointCount();
         for ( unsigned int b = 0; searching && b < bcount; b++ )
         {
            if ( m_model->isBoneJointSelected(b) )
            {
               int anim = m_model->getCurrentAnimation();
               int frame = m_model->getCurrentAnimationFrame();
               if ( m_model->getSkelAnimKeyframe( anim, frame, b, true,
                        rad[0], rad[1], rad[2] ) )
               {
                  searching = false;
               }
            }
         }
      }

      for ( int i = 0; i < 3; i++ )
      {
         rad[i] = rad[i] / PIOVER180;
      }

      QString str;

      str.sprintf( "%f", rad[0] );
      m_xValue->setText( str );

      str.sprintf( "%f", rad[1] );
      m_yValue->setText( str );

      str.sprintf( "%f", rad[2] );
      m_zValue->setText( str );

      this->setEnabled( searching ? false : true );

      m_change = false;
   }
}

void ContextRotation::updateRotation()
{
   // FIXME deal with animation
   // FIXME only allow points in None and Frame
   // FIXME only allow joints (for keyframe) in Skel
   if ( m_model->getAnimationMode() == Model::ANIMMODE_SKELETAL )
   {
      //return;
   }

   if ( !m_change )
   {
      m_update = true;

      // Change model based on text field input
      double rad[3];
      rad[0] = atof( m_xValue->text().latin1() );
      rad[1] = atof( m_yValue->text().latin1() );
      rad[2] = atof( m_zValue->text().latin1() );

      rad[0] = rad[0] * PIOVER180;
      rad[1] = rad[1] * PIOVER180;
      rad[2] = rad[2] * PIOVER180;

      bool searching = true;

      unsigned int pcount = m_model->getPointCount();
      for ( unsigned int p = 0; searching && p < pcount; p++ )
      {
         if ( m_model->isPointSelected( p ) )
         {
            searching = false;
            m_model->setPointRotation( p, rad );
         }
      }

      if ( m_model->getAnimationMode() == Model::ANIMMODE_SKELETAL )
      {
         unsigned int bcount = m_model->getBoneJointCount();
         for ( unsigned int b = 0; searching && b < bcount; b++ )
         {
            if ( m_model->isBoneJointSelected(b) )
            {
               int anim = m_model->getCurrentAnimation();
               int frame = m_model->getCurrentAnimationFrame();
               if ( m_model->setSkelAnimKeyframe( anim, frame, b, true,
                        rad[0], rad[1], rad[2] ) )
               {
                  searching = false;
                  m_model->setCurrentAnimationFrame( frame ); // Force re-animate
               }
            }
         }
      }

      if ( !searching )
      {
         m_model->operationComplete( tr( "Set Rotation", "operation complete" ).utf8() );

         emit panelChange();
      }

      m_update = false;
   }
}

