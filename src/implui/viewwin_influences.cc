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


#include "viewwin.h"

#include "viewpanel.h"
#include "model.h"
#include "modelstatus.h"
#include "jointwin.h"
#include "autoassignjointwin.h"
#include "log.h"

void ViewWindow::jointWinEvent()
{
   // this calls operationComplete and updates the model itself
   JointWin * win = new JointWin( m_model );  
   win->show();
}

void ViewWindow::jointAssignSelectedToJoint()
{
   std::list<int> j;
   std::list<int>::iterator jit;

   m_model->getSelectedBoneJoints( j );

   if ( !j.empty() )
   {
      list<int>::iterator it;

      list<int> vertList;
      m_model->getSelectedVertices( vertList );
      list<int> pointList;
      m_model->getSelectedPoints( pointList );

      log_debug( "assigning %d vertices and %d points to joints\n", vertList.size(), pointList.size() );
      QString str = tr( "Assigning %1 vertices and %2 points to joints").arg(vertList.size()).arg( pointList.size() );
      model_status( m_model, StatusNormal, STATUSTIME_SHORT, "%s", (const char *) str.toUtf8() );

      for ( it = vertList.begin(); it != vertList.end(); it++ )
      {
         for ( jit = j.begin(); jit != j.end(); jit++ )
         {
            double w = m_model->calculateVertexInfluenceWeight( *it, *jit );
            m_model->addVertexInfluence( *it, *jit, Model::IT_Auto, w );
         }
      }

      for ( it = pointList.begin(); it != pointList.end(); it++ )
      {
         for ( jit = j.begin(); jit != j.end(); jit++ )
         {
            double w = m_model->calculatePointInfluenceWeight( *it, *jit );
            m_model->addPointInfluence( *it, *jit, Model::IT_Auto, w );
         }
      }

      m_model->operationComplete( tr("Assign Selected to Joint").toUtf8() );
      m_viewPanel->modelUpdatedEvent();
   }
   else
   {
      model_status( m_model, StatusError, STATUSTIME_LONG, tr("You must have at least one bone joint selected.").toUtf8() );
   }
}

void ViewWindow::jointAutoAssignSelected()
{
   std::list<Model::Position> p;
   std::list<Model::Position>::iterator pit;

   m_model->getSelectedPositions( p );

   if ( !p.empty() )
   {
      AutoAssignJointWin win( m_model, this );
      if ( win.exec() )
      {
         double sensitivity = ((double) win.getSensitivity() ) / 100.0;
         bool selected = win.getSelected();

         log_debug( "auto-assigning %p vertices and points to joints\n", p.size() );

         for ( pit = p.begin(); pit != p.end(); pit++ )
         {
            m_model->autoSetPositionInfluences( *pit, sensitivity, selected );
         }

         m_model->operationComplete( tr("Auto-Assign Selected to Bone Joints").toUtf8() );
         m_viewPanel->modelUpdatedEvent();
      }
   }
   else
   {
      model_status( m_model, StatusError, STATUSTIME_LONG, tr("You must have at least one vertex or point selected.").toUtf8() );
   }
}

void ViewWindow::jointRemoveInfluencesFromSelected()
{
   std::list< Model::Position > posList;
   std::list< Model::Position >::iterator it;

   m_model->getSelectedPositions( posList );
   for ( it = posList.begin(); it != posList.end(); it++ )
   {
      m_model->removeAllPositionInfluences( *it );
   }

   m_model->operationComplete( tr("Remove All Influences from Selected").toUtf8() );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::jointRemoveInfluenceJoint()
{
   std::list<int> jointList;
   std::list<int>::iterator it;

   unsigned vcount = m_model->getVertexCount();
   unsigned pcount = m_model->getPointCount();

   m_model->getSelectedBoneJoints( jointList );

   for ( it = jointList.begin(); it != jointList.end(); it++ )
   {
      for ( unsigned v = 0; v < vcount; v++ )
      {
         m_model->removeVertexInfluence( v, *it );
      }
      for ( unsigned p = 0; p < pcount; p++ )
      {
         m_model->removePointInfluence( p, *it );
      }
   }

   m_model->operationComplete( tr("Remove Joint from Influencing").toUtf8() );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::jointMakeSingleInfluence()
{
   unsigned vcount = m_model->getVertexCount();
   unsigned pcount = m_model->getPointCount();
   int      bcount = m_model->getBoneJointCount();

   for ( unsigned v = 0; v < vcount; v++ )
   {
      int joint = m_model->getPrimaryVertexInfluence( v );
      
      if ( joint >= 0 )
      {
         for ( int b = 0; b < bcount; b++ )
         {
            if ( b != joint )
            {
               m_model->removeVertexInfluence( v, b );
            }
         }
      }
   }
   for ( unsigned p = 0; p < pcount; p++ )
   {
      int joint = m_model->getPrimaryPointInfluence( p );
      
      if ( joint >= 0 )
      {
         for ( int b = 0; b < bcount; b++ )
         {
            if ( b != joint )
            {
               m_model->removePointInfluence( p, b );
            }
         }
      }
   }

   m_model->operationComplete( tr("Convert To Single Influence").toUtf8() );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::jointSelectUnassignedVertices()
{
   m_model->unselectAllVertices();

   m_model->beginSelectionDifference();

   unsigned vcount = m_model->getVertexCount();
   for ( unsigned v = 0; v < vcount; v++ )
   {
      Model::InfluenceList l;
      m_model->getVertexInfluences( v, l );
      if ( l.empty() )
      {
         m_model->selectVertex( v );
      }
   }

   m_model->endSelectionDifference();

   m_model->operationComplete( tr("Select Unassigned Vertices").toUtf8() );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::jointSelectUnassignedPoints()
{
   m_model->unselectAllVertices();

   m_model->beginSelectionDifference();

   unsigned pcount = m_model->getPointCount();
   for ( unsigned p = 0; p < pcount; p++ )
   {
      Model::InfluenceList l;
      m_model->getPointInfluences( p, l );
      if ( l.empty() )
      {
         m_model->selectPoint( p );
      }
   }

   m_model->endSelectionDifference();

   m_model->operationComplete( tr("Select Unassigned Points").toUtf8() );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::jointSelectInfluenceJoints()
{
   m_model->beginSelectionDifference();

   Model::InfluenceList ilist;
   Model::InfluenceList::iterator iit;

   list< Model::Position > posList;
   list< Model::Position >::iterator it;

   m_model->getSelectedPositions( posList );
   for ( it = posList.begin(); it != posList.end(); it++ )
   {
      m_model->getPositionInfluences( *it, ilist );

      for ( iit = ilist.begin(); iit != ilist.end(); iit++ )
      {
         m_model->selectBoneJoint( (*iit).m_boneId );
      }
   }

   m_model->endSelectionDifference();

   m_model->operationComplete( tr("Select Joint Influences").toUtf8() );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::jointSelectInfluencedVertices()
{
   m_model->beginSelectionDifference();

   Model::InfluenceList ilist;
   Model::InfluenceList::iterator iit;

   list< int > jointList;
   list< int >::iterator it;

   unsigned vcount = m_model->getVertexCount();

   m_model->getSelectedBoneJoints( jointList );
   for ( it = jointList.begin(); it != jointList.end(); it++ )
   {
      for ( unsigned v = 0; v < vcount; v++ )
      {
         m_model->getVertexInfluences( v, ilist );
         for ( iit = ilist.begin(); iit != ilist.end(); iit++ )
         {
            if ( (*iit).m_boneId == *it )
            {
               m_model->selectVertex( v );
            }
         }
      }
   }

   m_model->endSelectionDifference();

   m_model->operationComplete( tr("Select Influences Vertices").toUtf8() );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::jointSelectInfluencedPoints()
{
   m_model->beginSelectionDifference();

   Model::InfluenceList ilist;
   Model::InfluenceList::iterator iit;

   list< int > jointList;
   list< int >::iterator it;

   unsigned pcount = m_model->getBoneJointCount();

   m_model->getSelectedBoneJoints( jointList );
   for ( it = jointList.begin(); it != jointList.end(); it++ )
   {
      for ( unsigned p = 0; p < pcount; p++ )
      {
         m_model->getPointInfluences( p, ilist );
         for ( iit = ilist.begin(); iit != ilist.end(); iit++ )
         {
            if ( (*iit).m_boneId == *it )
            {
               m_model->selectPoint( p );
            }
         }
      }
   }

   m_model->endSelectionDifference();

   m_model->operationComplete( tr("Select Influenced Points").toUtf8() );
   m_viewPanel->modelUpdatedEvent();
}

