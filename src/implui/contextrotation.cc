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

#include "contextrotation.h"

#include "model.h"

#include <QtWidgets/QLineEdit>

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

      Model::AnimationModeE animationMode = m_model->getAnimationMode();
      if ( animationMode == Model::ANIMMODE_NONE
            || animationMode == Model::ANIMMODE_SKELETAL )
      {
         unsigned int bcount = m_model->getBoneJointCount();
         for ( unsigned int b = 0; searching && b < bcount; b++ )
         {
            if ( m_model->isBoneJointSelected(b) )
            {
               if ( animationMode == Model::ANIMMODE_SKELETAL )
               {
                  int anim = m_model->getCurrentAnimation();
                  int frame = m_model->getCurrentAnimationFrame();
                  if ( m_model->getSkelAnimKeyframe( anim, frame, b, true,
                           rad[0], rad[1], rad[2] ) )
                  {
                     searching = false;
                  }
               }
               else
               {
                  searching = false;
                  Matrix rm;
                  m_model->getBoneJointRelativeMatrix( b, rm );
                  rm.getRotation( rad[0], rad[1], rad[2] );
               }
            }
         }
      }

      for ( int i = 0; i < 3; i++ )
      {
         rad[i] = rad[i] / PIOVER180;
      }

      QString str;

      str = QString::asprintf( "%f", (float)rad[0] );
      m_xValue->setText( str );

      str = QString::asprintf( "%f", (float)rad[1] );
      m_yValue->setText( str );

      str = QString::asprintf( "%f", (float)rad[2] );
      m_zValue->setText( str );

      this->setEnabled( searching ? false : true );

      m_change = false;
   }
}

void ContextRotation::updateRotation()
{
   if ( !m_change )
   {
      m_update = true;

      // Change model based on text field input
      double rad[3];
      rad[0] = m_xValue->text().toDouble();
      rad[1] = m_yValue->text().toDouble();
      rad[2] = m_zValue->text().toDouble();

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

      Model::AnimationModeE animationMode = m_model->getAnimationMode();
      if ( animationMode == Model::ANIMMODE_NONE
            || animationMode == Model::ANIMMODE_SKELETAL )
      {
         unsigned int bcount = m_model->getBoneJointCount();
         for ( unsigned int b = 0; searching && b < bcount; b++ )
         {
            if ( m_model->isBoneJointSelected(b) )
            {
               if ( animationMode == Model::ANIMMODE_SKELETAL )
               {
                  int anim = m_model->getCurrentAnimation();
                  int frame = m_model->getCurrentAnimationFrame();
                  searching = false;
                  m_model->setSkelAnimKeyframe( anim, frame, b, true, rad[0], rad[1], rad[2] );
                  m_model->setCurrentAnimationFrame( frame ); // Force re-animate
               }
               else
               {
                  searching = false;
                  m_model->setBoneJointRotation( b, rad );
                  m_model->setupJoints();
               }
            }
         }
      }

      if ( !searching )
      {
         m_model->operationComplete( tr( "Set Rotation", "operation complete" ).toUtf8() );

         emit panelChange();
      }

      m_update = false;
   }
}

