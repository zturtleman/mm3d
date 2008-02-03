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

#include "contextinfluences.h"

#include "groupwin.h"
#include "texwin.h"
#include "config.h"

#include "log.h"

#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QInputDialog>

#include <stdlib.h>

ContextInfluences::JointCount::JointCount()
   : inList( false ),
     count( 0 ),
     typeIndex( Model::IT_Auto + 1 ),
     weight( 0 )
{
   for ( int t = 0; t < Model::IT_MAX; t++ )
   {
      typeCount[t] = 0;
   }
}

ContextInfluences::ContextInfluences( QWidget * parent )
   : QWidget( parent ),
     m_change( false ),
     m_update( false )
{
   setupUi( this );
}

ContextInfluences::~ContextInfluences()
{
}

void ContextInfluences::setModel( Model * m )
{
   m_model = m;
   modelChanged( ~0 );
}

void ContextInfluences::modelChanged( int changeBits )
{
   // Only change if it's a group change or a selection change
   if ( (changeBits & Model::AddOther) || (changeBits & Model::SelectionChange ) )
   {
      if ( !m_update )
      {
         m_change = true;

         m_joint1->clear();
         m_joint2->clear();
         m_joint3->clear();
         m_joint4->clear();

         m_joint1->insertItem( 0, tr("<None>") );
         m_joint2->insertItem( 0, tr("<None>") );
         m_joint3->insertItem( 0, tr("<None>") );
         m_joint4->insertItem( 0, tr("<None>") );

         int bcount = m_model->getBoneJointCount();

         for ( int b = 0; b < bcount; b++ )
         {
            QString name = QString::fromUtf8( m_model->getBoneJointName( b ) );
            m_joint1->insertItem( b+1, name );
            m_joint2->insertItem( b+1, name );
            m_joint3->insertItem( b+1, name );
            m_joint4->insertItem( b+1, name );
         }

         m_joint1->setCurrentIndex( 0 );
         m_joint2->setCurrentIndex( 0 );
         m_joint3->setCurrentIndex( 0 );
         m_joint4->setCurrentIndex( 0 );

         m_joints[0] = -1;
         m_joints[1] = -1;
         m_joints[2] = -1;
         m_joints[3] = -1;

         m_weight1->setEnabled( false );
         m_weight2->setEnabled( false );
         m_weight3->setEnabled( false );
         m_weight4->setEnabled( false );

         if ( m_model->getAnimationMode() != Model::ANIMMODE_NONE )
         {
            m_joint1->setEnabled( false );
            m_joint2->setEnabled( false );
            m_joint3->setEnabled( false );
            m_joint4->setEnabled( false );
         }

         // Update influence fields
         m_jclist.clear();
         m_jclist.resize( bcount );

         list< Model::Position > plist;
         list< Model::Position >::iterator pit;

         Model::InfluenceList ilist;
         Model::InfluenceList::iterator iit;

         m_model->getSelectedPositions( plist );

         // for now just do a sum on which bone joints are used the most
         // TODO: may want to weight by influence later
         for ( pit = plist.begin(); pit != plist.end(); pit++ )
         {
            ilist.clear();
            m_model->getPositionInfluences( (*pit), ilist );
            for ( iit = ilist.begin(); iit != ilist.end(); iit++ )
            {
               int bone = (*iit).m_boneId;
               m_jclist[ bone ].count  += 1;
               m_jclist[ bone ].weight += (int) ((*iit).m_weight * 100.0);
               m_jclist[ bone ].typeCount[ (*iit).m_type ] += 1;
            }
         }

         for ( int joint = 0; joint < bcount; joint++ )
         {
            int typeIndex = -1;
            for ( int t = 0; typeIndex != 0 && t < Model::IT_MAX; t++ )
            {
               if ( m_jclist[ joint ].typeCount[ t ] > 0 )
               {
                  // If type index is unset, set it to the combo box
                  // index for our type (off by one). If type index
                  // is already set, it's mixed (index 0)
                  typeIndex = ( typeIndex < 0 ) ? t + 1 : 0;
               }
            }

            if ( typeIndex >= 0 )
            {
               m_jclist[ joint ].typeIndex = typeIndex;
               m_jclist[ joint ].weight /= m_jclist[ joint ].count;
            }
            else
            {
               m_jclist[ joint ].weight = 100;
            }
         }

         for ( int index = 0; index < Model::MAX_INFLUENCES; index++ )
         {
            int maxVal = 0;
            int joint  = -1;

            for ( int b = 0; b < bcount; b++ )
            {
               if ( !m_jclist[ b ].inList && m_jclist[ b ].count > maxVal )
               {
                  maxVal = m_jclist[b].count;
                  joint = b;
               }
            }

            if ( maxVal > 0 )
            {
               QComboBox * jointBox  = m_joint1;

               switch ( index )
               {
                  case 0:
                     jointBox = m_joint1;
                     break;

                  case 1:
                     jointBox = m_joint2;
                     break;

                  case 2:
                     jointBox = m_joint3;
                     break;

                  case 3:
                     jointBox = m_joint4;
                     break;

                  default:
                     break;
               }

               jointBox->setCurrentIndex( joint + 1 );

               updateWeightField( index, true, 
                     m_jclist[ joint ].typeIndex, m_jclist[ joint ].weight );
               //updateRemainders();

               m_joints[index] = joint;
               log_debug( "joint index %d is joint ID %d\n", index, joint );

               m_jclist[ joint ].inList = true;
            }
            else
            {
               // No more influenes, we're done
               index = Model::MAX_INFLUENCES;
            }
         }

         m_change = false;
      }
   }
}

void ContextInfluences::jointChanged( int index, int oldJoint, int newJoint )
{
   if ( !m_change )
   {
      m_update = true;

      if ( newJoint >= 0 )
      {
         for ( int i = 0; i < Model::MAX_INFLUENCES; i++ )
         {
            if ( m_joints[i] == newJoint )
            {
               // trying to assign new joint when we already have that joint
               // in one of the edit boxes. Change the selection back. This
               // will cause some nasty bugs and probably confuse the user.
               // Change the selection back.
               m_change = true;

               switch ( index )
               {
                  case 0:
                     m_joint1->setCurrentIndex( oldJoint + 1 );
                     break;
                  case 1:
                     m_joint2->setCurrentIndex( oldJoint + 1 );
                     break;
                  case 2:
                     m_joint3->setCurrentIndex( oldJoint + 1 );
                     break;
                  case 3:
                     m_joint4->setCurrentIndex( oldJoint + 1 );
                     break;
                  default:
                     break;
               }

               m_change = false;
               m_update = false;
               return;
            }
         }
      }

      std::list<Model::Position> l;
      m_model->getSelectedPositions( l );

      std::list<Model::Position>::iterator it;
      if ( oldJoint >= 0 )
      {
         m_jclist[ oldJoint ].count = 0;
         for ( it = l.begin(); it != l.end(); it++ )
         {
            m_model->removePositionInfluence( *it, oldJoint );
         }
      }

      int weight = 0;
      if ( newJoint >= 0 )
      {
         m_jclist[ newJoint ].count = l.size();
         for ( it = l.begin(); it != l.end(); it++ )
         {
            double w = m_model->calculatePositionInfluenceWeight( *it, newJoint );

            log_debug( "influence = %f\n", w );

            m_model->addPositionInfluence( *it, newJoint, Model::IT_Auto, w );

            weight += (int) (w * 100.0);
         }

         if ( !l.empty() )
         {
            m_jclist[ newJoint ].weight = weight / l.size();
         }
         m_jclist[ newJoint ].typeIndex = Model::IT_Auto + 1;
      }

      m_joints[ index ] = newJoint;

      bool enabled = false;
      if ( newJoint >= 0 )
      {
         enabled = true;
         m_change = true;

         updateWeightField( index, true,
               m_jclist[ newJoint ].typeIndex, m_jclist[ newJoint ].weight );
         updateRemainders();

         m_change = false;
      }
      else
      {
         updateWeightField( index, false,
               0, 0 );
         updateRemainders();
      }

      m_model->operationComplete( tr( "Change Joint Assignment", "operation complete" ).toUtf8() );

      emit panelChange();

      m_update = false;
   }
}

void ContextInfluences::weightChanged( int index, double weight )
{
   if ( !m_change )
   {
      m_update = true;

      if ( weight < 0.0 )
      {
         weight = 0.0;
      }
      if ( weight > 1.0 )
      {
         weight = 1.0;
      }

      int joint = m_joints[ index ];
      log_debug( "setting joint %d weight to %f\n", joint, weight );

      std::list<Model::Position> l;
      m_model->getSelectedPositions( l );
      
      std::list<Model::Position>::iterator it;
      for ( it = l.begin(); it != l.end(); it++ )
      {
         m_model->setPositionInfluenceType( *it, joint, Model::IT_Custom );
         m_model->setPositionInfluenceWeight( *it, joint, weight );
      }

      m_model->operationComplete( tr( "Change Influence Weight", "operation complete" ).toUtf8() );
      updateRemainders();

      emit panelChange();

      m_update = false;
   }
}

void ContextInfluences::typeChanged( int index )
{
   if ( !m_change )
   {
      m_update = true;

      int joint = m_joints[ index ];

      QComboBox * typeValue = m_weight1;
      switch ( index )
      {
         case 1:
            typeValue = m_weight2;
            break;
         case 2:
            typeValue = m_weight3;
            break;
         case 3:
            typeValue = m_weight4;
            break;
         default:
            break;
      }

      Model::InfluenceTypeE type = Model::IT_Auto;

      switch ( typeValue->currentIndex() )
      {
         case 0:
            // Not really a valid selection, return
            m_update = false;
            return;

         case 1:
            type = Model::IT_Custom;
            break;
         case 2:
            type = Model::IT_Auto;
            break;
         case 3:
            type = Model::IT_Remainder;
            break;
         default:
            break;
      }
      
      log_debug( "setting joint %d type to %d\n", joint, (int) type );


      std::list<Model::Position> l;
      m_model->getSelectedPositions( l );
      
      int weight = 0;
      std::list<Model::Position>::iterator it;
      for ( it = l.begin(); it != l.end(); it++ )
      {
         m_model->setPositionInfluenceType( *it, joint, type );
         if ( type == Model::IT_Auto )
         {
            double w = m_model->calculatePositionInfluenceWeight( *it, joint );
            m_model->setPositionInfluenceWeight( *it, joint, 
                  w );
            weight += (int) (w * 100.0);
         }
         else
         {
            m_model->setPositionInfluenceWeight( *it, joint, 
                  (double) m_jclist[ joint ].weight / 100.0 );
         }
      }

      if ( type == Model::IT_Auto )
      {
         m_jclist[ joint ].weight = weight / m_jclist[ joint ].count;
      }

      m_jclist[ joint ].typeIndex = type + 1;

      m_change = true;
      updateWeightField( index, true,
            m_jclist[ joint ].typeIndex, m_jclist[ joint ].weight );
      updateRemainders();
      m_change = false;

      m_model->operationComplete( tr( "Change Influence Type", "operation complete" ).toUtf8() );

      emit panelChange();

      m_update = false;
   }
}

void ContextInfluences::joint1Changed()
{
   int index = 0;
   jointChanged( index, m_joints[ index ], m_joint1->currentIndex() - 1);
}

void ContextInfluences::joint2Changed()
{
   int index = 1;
   jointChanged( index, m_joints[ index ], m_joint2->currentIndex() - 1);
}

void ContextInfluences::joint3Changed()
{
   int index = 2;
   jointChanged( index, m_joints[ index ], m_joint3->currentIndex() - 1);
}

void ContextInfluences::joint4Changed()
{
   int index = 3;
   jointChanged( index, m_joints[ index ], m_joint4->currentIndex() - 1);
}

void ContextInfluences::type1Changed()
{
   int index = 0;
   typeChanged( index );
}

void ContextInfluences::type2Changed()
{
   int index = 1;
   typeChanged( index );
}

void ContextInfluences::type3Changed()
{
   int index = 2;
   typeChanged( index );
}

void ContextInfluences::type4Changed()
{
   int index = 3;
   typeChanged( index );
}

void ContextInfluences::weight1Changed( const QString & weight )
{
   int index = 0;
   if ( weight.size() != 0 && weight[0].isDigit() )
   {
      weightChanged( index, atof( weight.toUtf8()) / 100.0 );
   }
}

void ContextInfluences::weight2Changed( const QString & weight )
{
   int index = 1;
   if ( weight.size() != 0 && weight[0].isDigit() )
   {
      weightChanged( index, atof( weight.toUtf8() ) / 100.0 );
   }
}

void ContextInfluences::weight3Changed( const QString & weight )
{
   int index = 2;
   if ( weight.size() != 0 && weight[0].isDigit() )
   {
      weightChanged( index, atof( weight.toUtf8()) / 100.0 );
   }
}

void ContextInfluences::weight4Changed( const QString & weight )
{
   int index = 3;
   if ( weight.size() != 0 && weight[0].isDigit() )
   {
      weightChanged( index, atof( weight.toUtf8()) / 100.0 );
   }
}

void ContextInfluences::updateRemainders()
{
   for ( int index = 0; index < Model::MAX_INFLUENCES; index++ )
   {
      QComboBox * weightBox = m_weight1;
      switch ( index )
      {
         case 0:
            weightBox = m_weight1;
            break;
         case 1:
            weightBox = m_weight2;
            break;
         case 2:
            weightBox = m_weight3;
            break;
         case 3:
            weightBox = m_weight4;
            break;
         default:
            break;
      }

      if ( weightBox->currentIndex() - 1 == Model::IT_Remainder )
      {
         int w = getRemainderWeight( m_joints[ index ] );
         log_debug( "updating box %d with remaining weight %d\n", index, w );
         updateWeightField( index, weightBox->isEnabled(), weightBox->currentIndex(), w );
      }
   }
}

void ContextInfluences::updateWeightField( int index, bool enabled, int type, int weight )
{
   QComboBox * weightBox = m_weight1;
   switch ( index )
   {
      case 0:
         weightBox = m_weight1;
         break;
      case 1:
         weightBox = m_weight2;
         break;
      case 2:
         weightBox = m_weight3;
         break;
      case 3:
         weightBox = m_weight4;
         break;
      default:
         break;
   }

   weightBox->setItemText( 0, tr("<Mixed>", "multiple types of bone joint influence") );
   weightBox->setItemText( 1, tr("Custom", "bone joint influence") );
   weightBox->setItemText( 2, tr("Auto", "bone joint influence") );
   weightBox->setItemText( 3, tr("Remainder", "bone joint influence") );

   if ( enabled )
   {
      QString typeStr;

      switch ( type )
      {
         case 0:
            typeStr = tr("<Mixed>", "multiple types of bone joint influence");
            break;
         case 1:
            typeStr.sprintf( "%d", weight );
            break;
         case 2:
            typeStr = tr("Auto: %1").arg( weight);
            break;
         case 3:
            typeStr = tr("Rem: %1").arg(weight);
            break;
         default:
            break;
      }

      weightBox->setCurrentIndex( type );
      weightBox->setItemText( type, typeStr );
      weightBox->setEnabled( true );
   }
   else
   {
      weightBox->setCurrentIndex( 0 );
      weightBox->setEnabled( false );
   }

   // Update remainder fields
   switch ( index )
   {
      case 0:
         weightBox = m_weight1;
         break;
      case 1:
         weightBox = m_weight2;
         break;
      case 2:
         weightBox = m_weight3;
         break;
      case 3:
         weightBox = m_weight4;
         break;
      default:
         break;
   }

}

int ContextInfluences::getRemainderWeight( int joint )
{
   log_debug( "getting remainder weight for joint %d\n" );
   if ( joint >= 0 && joint < (int) m_model->getBoneJointCount() )
   {
      list< Model::Position > plist;
      list< Model::Position >::iterator pit;

      Model::InfluenceList ilist;
      Model::InfluenceList::iterator iit;

      m_model->getSelectedPositions( plist );

      double weight = 0.0;
      int    count  = 0;
      for ( pit = plist.begin(); pit != plist.end(); pit++ )
      {
         m_model->getPositionInfluences( (*pit), ilist );

         for ( iit = ilist.begin(); iit != ilist.end(); iit++ )
         {
            if ( (*iit).m_type == Model::IT_Remainder && (*iit).m_boneId == joint )
            {
               weight += (*iit).m_weight;
               count++;
            }
         }
      }

      if ( count > 0 )
      {
         return (int) ((weight * 100.0) / (double) count);
      }
   }
   return 0;
}

