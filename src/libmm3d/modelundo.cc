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


#include "modelundo.h"
#include "model.h"

#include "log.h"

int ModelUndo::s_allocated = 0;

// Well, I suppose you can call it an implementation...
bool MU_NoOp::combine( Undo * u )
{
   if ( dynamic_cast<MU_NoOp *>( u ) )
   {
      return true;
   }
   else
   {
      log_debug( "couldn't combine with NoOp\n" );
      return false;
   }
}

MU_TranslateSelected::MU_TranslateSelected()
{
}

MU_TranslateSelected::~MU_TranslateSelected()
{
}

bool MU_TranslateSelected::combine( Undo * u )
{
   MU_TranslateSelected * undo = dynamic_cast<MU_TranslateSelected *>( u );

   if ( undo )
   {
      m_matrix = m_matrix * undo->m_matrix;
      return true;
   }
   else
   {
      return false;
   }
}

void MU_TranslateSelected::setMatrix( const Matrix & rhs )
{
   m_matrix = rhs;
}

void MU_TranslateSelected::undo( Model * model )
{
   log_debug( "undo translate selected\n" );

   Matrix m = m_matrix.getInverse();

   /*
   // Invert our translation matrix
   for ( int c = 0; c < 3; c++ )
   {
      m.set( 3, c, -m_matrix.get( 3, c ) );
   }
   */

   model->translateSelected( m );
}

void MU_TranslateSelected::redo( Model * model )
{
   model->translateSelected( m_matrix );
}

MU_RotateSelected::MU_RotateSelected()
{
}

MU_RotateSelected::~MU_RotateSelected()
{
}

bool MU_RotateSelected::combine( Undo * u )
{
   MU_RotateSelected * undo = dynamic_cast<MU_RotateSelected *>( u );

   if ( undo )
   {
      for ( int t = 0; t < 3; t++ )
      {
         if ( m_point[t] != undo->m_point[t] )
         {
            return false;
         }
      }

      m_matrix = m_matrix * undo->m_matrix;
      return true;
   }
   else
   {
      return false;
   }
}

void MU_RotateSelected::setMatrixPoint( const Matrix & rhs, double * point )
{
   m_matrix = rhs;
   for ( int t = 0; t < 3; t++ )
   {
      m_point[t] = point[t];
   }
}

void MU_RotateSelected::undo( Model * model )
{
   log_debug( "undo rotate selected\n" );
   Matrix inv = m_matrix.getInverse();
   model->rotateSelected( inv, m_point );
}

void MU_RotateSelected::redo( Model * model )
{
   log_debug( "undo rotate selected\n" );
   model->rotateSelected( m_matrix, m_point );
}

MU_ApplyMatrix::MU_ApplyMatrix()
{
}

MU_ApplyMatrix::~MU_ApplyMatrix()
{
}

bool MU_ApplyMatrix::combine( Undo * u )
{
   MU_ApplyMatrix * undo = dynamic_cast<MU_ApplyMatrix *>( u );

   if ( undo )
   {
      m_matrix = m_matrix * undo->m_matrix;
      return true;
   }
   else
   {
      return false;
   }
}

void MU_ApplyMatrix::setMatrix( const Matrix & m, Model::OperationScopeE scope, bool animations )
{
   m_matrix = m;
   m_scope = scope;
   m_animations = animations;
}

void MU_ApplyMatrix::undo( Model * model )
{
   log_debug( "undo apply matrix\n" );
   Matrix m;

   m = m_matrix.getInverse();

   model->applyMatrix( m, m_scope, m_animations, true );
}

void MU_ApplyMatrix::redo( Model * model )
{
   model->applyMatrix( m_matrix, m_scope, m_animations, true );
}

MU_SelectionMode::MU_SelectionMode()
{
}

MU_SelectionMode::~MU_SelectionMode()
{
}

void MU_SelectionMode::undo( Model * model )
{
   log_debug( "undo selection mode %d\n", m_oldMode );

   model->setSelectionMode( m_oldMode );
}

void MU_SelectionMode::redo( Model * model )
{
   log_debug( "redo selection mode %d\n", m_mode );

   model->setSelectionMode( m_mode );
}

bool MU_SelectionMode::combine( Undo * u )
{
   MU_SelectionMode * undo = dynamic_cast< MU_SelectionMode * >( u );
   if ( undo )
   {
      m_mode = undo->m_mode;
      return true;
   }
   else
   {
      return false;
   }
}

unsigned MU_SelectionMode::size()
{
   return sizeof(MU_SelectionMode);
}

MU_Select::MU_Select( Model::SelectionModeE mode )
   : m_mode( mode )
{
}

MU_Select::~MU_Select()
{
}

void MU_Select::undo( Model * model )
{
   SelectionDifferenceList::iterator it;

   // Invert selection from our list
   for ( it = m_diff.begin(); it != m_diff.end(); it++ )
   {
      if ( !(*it).oldSelected )
      {
         switch ( m_mode )
         {
            case Model::SelectVertices:
               model->unselectVertex( (*it).number );
               break;
            case Model::SelectTriangles:
               model->unselectTriangle( (*it).number );
               break;
            case Model::SelectGroups:
               model->unselectGroup( (*it).number );
               break;
            case Model::SelectJoints:
               model->unselectBoneJoint( (*it).number );
               break;
            case Model::SelectPoints:
               model->unselectPoint( (*it).number );
               break;
            case Model::SelectNone:
            default:
               break;
         }
      }
      else
      {
         switch ( m_mode )
         {
            case Model::SelectVertices:
               model->selectVertex( (*it).number );
               break;
            case Model::SelectTriangles:
               model->selectTriangle( (*it).number );
               break;
            case Model::SelectGroups:
               model->selectGroup( (*it).number );
               break;
            case Model::SelectJoints:
               model->selectBoneJoint( (*it).number );
               break;
            case Model::SelectPoints:
               model->selectPoint( (*it).number );
               break;
            case Model::SelectNone:
            default:
               break;
         }
      }
   }
}

void MU_Select::redo( Model * model )
{
   SelectionDifferenceList::iterator it;

   // Set selection from our list
   if ( m_diff.size() )
   {
      for ( it = m_diff.begin(); it != m_diff.end(); it++ )
      {
         if ( (*it).selected )
         {
            switch ( m_mode )
            {
               case Model::SelectVertices:
                  model->selectVertex( (*it).number );
                  break;
               case Model::SelectTriangles:
                  model->selectTriangle( (*it).number );
                  break;
               case Model::SelectGroups:
                  model->selectGroup( (*it).number );
                  break;
               case Model::SelectJoints:
                  model->selectBoneJoint( (*it).number );
                  break;
               case Model::SelectPoints:
                  model->selectPoint( (*it).number );
                  break;
               case Model::SelectNone:
               default:
                  break;
            }
         }
         else
         {
            switch ( m_mode )
            {
               case Model::SelectVertices:
                  model->unselectVertex( (*it).number );
                  break;
               case Model::SelectTriangles:
                  model->unselectTriangle( (*it).number );
                  break;
               case Model::SelectGroups:
                  model->unselectGroup( (*it).number );
                  break;
               case Model::SelectJoints:
                  model->unselectBoneJoint( (*it).number );
                  break;
               case Model::SelectPoints:
                  model->unselectPoint( (*it).number );
                  break;
               case Model::SelectNone:
               default:
                  break;
            }
         }
      }
   }
}

bool MU_Select::combine( Undo * u )
{
   /*
   MU_Select * undo = dynamic_cast< MU_Select * >( u );
   if ( undo && getSelectionMode() == undo->getSelectionMode() )
   {
      SelectionDifferenceList::iterator it;
      for ( it = undo->m_diff.begin(); it != undo->m_diff.end(); it++ )
      {
         setSelectionDifference( (*it).number, (*it).selected, (*it).oldSelected );
      }
      return true;
   }
   else
   */
   {
      log_debug( "couldn't combine with select\n" );
      return false;
   }
}

unsigned MU_Select::size()
{
   return sizeof(MU_Select) + m_diff.size() * sizeof(SelectionDifferenceT);
}

void MU_Select::setSelectionDifference( int number, bool selected, bool oldSelected )
{
   unsigned index;
   SelectionDifferenceT diff;
   diff.number   = number;
   // Change selection state if it exists in our list
   if ( m_diff.find_sorted( diff, index ) )
   {
      m_diff[index].selected = selected;
   }

   // add selection state to our list
   diff.selected = selected;
   diff.oldSelected = oldSelected;

   m_diff.insert_sorted( diff );
}

MU_Hide::MU_Hide( Model::SelectionModeE mode )
   : m_mode( mode )
{
}

MU_Hide::~MU_Hide()
{
}

void MU_Hide::undo( Model * model )
{
   log_debug( "undo hide\n" );

   HideDifferenceList::iterator it;

   // Invert visible from our list
   for ( it = m_diff.begin(); it != m_diff.end(); it++ )
   {
      if ( (*it).visible )
      {
         switch ( m_mode )
         {
            case Model::SelectVertices:
               model->hideVertex( (*it).number );
               break;
            case Model::SelectTriangles:
               model->hideTriangle( (*it).number );
               break;
            case Model::SelectJoints:
               model->hideJoint( (*it).number );
               break;
            case Model::SelectPoints:
               model->hidePoint( (*it).number );
               break;
            case Model::SelectNone:
            default:
               break;
         }
      }
      else
      {
         switch ( m_mode )
         {
            case Model::SelectVertices:
               model->unhideVertex( (*it).number );
               break;
            case Model::SelectTriangles:
               model->unhideTriangle( (*it).number );
               break;
            case Model::SelectJoints:
               model->unhideJoint( (*it).number );
               break;
            case Model::SelectPoints:
               model->unhidePoint( (*it).number );
               break;
            case Model::SelectNone:
            default:
               break;
         }
      }
   }
}

void MU_Hide::redo( Model * model )
{
   HideDifferenceList::iterator it;

   // Set visible from our list
   for ( it = m_diff.begin(); it != m_diff.end(); it++ )
   {
      if ( (*it).visible )
      {
         switch ( m_mode )
         {
            case Model::SelectVertices:
               model->unhideVertex( (*it).number );
               break;
            case Model::SelectTriangles:
               model->unhideTriangle( (*it).number );
               break;
            case Model::SelectJoints:
               model->unhideJoint( (*it).number );
               break;
            case Model::SelectPoints:
               model->unhidePoint( (*it).number );
               break;
            case Model::SelectNone:
            default:
               break;
         }
      }
      else
      {
         switch ( m_mode )
         {
            case Model::SelectVertices:
               model->hideVertex( (*it).number );
               break;
            case Model::SelectTriangles:
               model->hideTriangle( (*it).number );
               break;
            case Model::SelectJoints:
               model->hideJoint( (*it).number );
               break;
            case Model::SelectPoints:
               model->hidePoint( (*it).number );
               break;
            case Model::SelectNone:
            default:
               break;
         }
      }
   }
}

bool MU_Hide::combine( Undo * u )
{
   MU_Hide * undo = dynamic_cast< MU_Hide * >( u );

   if ( undo && getSelectionMode() == undo->getSelectionMode() )
   {
      HideDifferenceList::iterator it;
      for ( it = undo->m_diff.begin(); it != undo->m_diff.end(); it++ )
      {
         setHideDifference( (*it).number, (*it).visible );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with hide\n" );
      return false;
   }
}

unsigned MU_Hide::size()
{
   return sizeof(MU_Hide) + m_diff.size() * sizeof(HideDifferenceT);
}

void MU_Hide::setHideDifference( int number, bool visible )
{
   HideDifferenceList::iterator it;

   // Change selection state if it exists in our list
   for ( it = m_diff.begin(); it != m_diff.end(); it++ )
   {
      if ( (*it).number == number )
      {
         (*it).visible = visible;
         return;
      }
   }

   // add selection state to our list
   HideDifferenceT diff;
   diff.number  = number;
   diff.visible = visible;

   m_diff.push_back( diff );
}

MU_InvertNormal::MU_InvertNormal()
{
}

MU_InvertNormal::~MU_InvertNormal()
{
}

void MU_InvertNormal::undo( Model * model )
{
   log_debug( "undo invert normal\n" );
   list<int>::iterator it;
   for ( it = m_triangles.begin(); it != m_triangles.end(); it++ )
   {
      model->invertNormals( *it );
   }
}

void MU_InvertNormal::redo( Model * model )
{
   list<int>::reverse_iterator it;
   for ( it = m_triangles.rbegin(); it != m_triangles.rend(); it++ )
   {
      model->invertNormals( *it );
   }
}

bool MU_InvertNormal::combine( Undo * u )
{
   MU_InvertNormal * undo = dynamic_cast< MU_InvertNormal * >( u );

   if ( undo )
   {
      list<int>::iterator it;
      for ( it = undo->m_triangles.begin(); it != undo->m_triangles.end(); it++ )
      {
         addTriangle( *it );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with invert normal\n" );
      return false;
   }
}

unsigned MU_InvertNormal::size()
{
   return sizeof(MU_InvertNormal) + m_triangles.size() * sizeof(int);
}

void MU_InvertNormal::addTriangle( int triangle )
{
   list<int>::iterator it;

   for ( it = m_triangles.begin(); it != m_triangles.end(); it++ )
   {
      if ( triangle == *it )
      {
         m_triangles.remove( triangle );
         return;
      }
   }

   m_triangles.push_back( triangle );
}

MU_MovePrimitive::MU_MovePrimitive()
{
}

MU_MovePrimitive::~MU_MovePrimitive()
{
}

void MU_MovePrimitive::undo( Model * model )
{
   log_debug( "undo move vertex\n" );

   MovePrimitiveList::iterator it;

   // Modify a vertex we already have
   for ( it = m_objects.begin(); it != m_objects.end(); it++ )
   {
      switch( (*it).type )
      {
         case MT_Vertex:
            model->moveVertex( (*it).number, (*it).oldx, (*it).oldy, (*it).oldz );
            break;
         case MT_Joint:
            model->moveBoneJoint( (*it).number, (*it).oldx, (*it).oldy, (*it).oldz );
            break;
         case MT_Point:
            model->movePoint( (*it).number, (*it).oldx, (*it).oldy, (*it).oldz );
            break;
         case MT_Projection:
            model->moveProjection( (*it).number, (*it).oldx, (*it).oldy, (*it).oldz );
            break;
         default:
            log_error( "Unknown type in move object undo\n" );
            break;
      }
   }
}

void MU_MovePrimitive::redo( Model * model )
{
   MovePrimitiveList::iterator it;

   // Modify a vertex we already have
   for ( it = m_objects.begin(); it != m_objects.end(); it++ )
   {
      switch( (*it).type )
      {
         case MT_Vertex:
            model->moveVertex( (*it).number, (*it).x, (*it).y, (*it).z );
            break;
         case MT_Joint:
            model->moveBoneJoint( (*it).number, (*it).x, (*it).y, (*it).z );
            break;
         case MT_Point:
            model->movePoint( (*it).number, (*it).x, (*it).y, (*it).z );
            break;
         case MT_Projection:
            model->moveProjection( (*it).number, (*it).x, (*it).y, (*it).z );
            break;
         default:
            log_error( "Unknown type in move object redo\n" );
            break;
      }
   }
}

bool MU_MovePrimitive::combine( Undo * u )
{
   MU_MovePrimitive * undo = dynamic_cast< MU_MovePrimitive * >( u );

   if ( undo )
   {
      MovePrimitiveList::iterator it;

      for ( it = undo->m_objects.begin(); it != undo->m_objects.end(); it++ )
      {
         addMovePrimitive( (*it).type, (*it).number, (*it).x, (*it).y, (*it).z,
               (*it).oldx, (*it).oldy, (*it).oldz );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with move vertex\n" );
      return false;
   }
}

unsigned MU_MovePrimitive::size()
{
   return sizeof(MU_MovePrimitive) + m_objects.size() * sizeof(MovePrimitiveT);
}

void MU_MovePrimitive::addMovePrimitive( MU_MovePrimitive::MoveTypeE type, int i, double x, double y, double z,
      double oldx, double oldy, double oldz )
{
   unsigned index;
   MovePrimitiveT mv;
   mv.number = i;
   mv.type = type;

   // Modify an object we already have
   if ( m_objects.find_sorted( mv, index ) )
   {
      m_objects[index].x = x;
      m_objects[index].y = y;
      m_objects[index].z = z;
      return;
   }

   // Not found, add object information
   mv.x       = x;
   mv.y       = y;
   mv.z       = z;
   mv.oldx    = oldx;
   mv.oldy    = oldy;
   mv.oldz    = oldz;
   m_objects.insert_sorted( mv );
}

MU_SetPointRotation::MU_SetPointRotation()
{
}

MU_SetPointRotation::~MU_SetPointRotation()
{
}

void MU_SetPointRotation::undo( Model * model )
{
   log_debug( "undo point rotation\n" );

   log_debug( "point %d rotation %f,%f,%f\n",
         number, oldx, oldy, oldz );

   double rot[3];
   rot[0] = oldx;
   rot[1] = oldy;
   rot[2] = oldz;
   model->setPointRotation( number, rot );
}

void MU_SetPointRotation::redo( Model * model )
{
   log_debug( "redo point rotation\n" );

   log_debug( "point %d rotation %f,%f,%f\n",
         number, x, y, z );

   double rot[3];
   rot[0] = x;
   rot[1] = y;
   rot[2] = z;
   model->setPointRotation( number, rot );
}

bool MU_SetPointRotation::combine( Undo * u )
{
   MU_SetPointRotation * undo = dynamic_cast< MU_SetPointRotation * >( u );

   if ( undo && undo->number == this->number )
   {
      x = undo->x;
      y = undo->y;
      z = undo->z;

      return true;
   }
   else
   {
      log_debug( "couldn't combine with point rotation\n" );
      return false;
   }
}

unsigned MU_SetPointRotation::size()
{
   return sizeof(MU_SetPointRotation);
}

void MU_SetPointRotation::setPointRotation( int p, double xval, double yval, double zval,
      double oldxval, double oldyval, double oldzval )
{
   log_debug( "point %d rotation changed from %f,%f,%f to %f,%f,%f\n",
         p, oldxval, oldyval, oldzval, xval, yval, zval );
   number  = p;
   x       = xval;
   y       = yval;
   z       = zval;
   oldx    = oldxval;
   oldy    = oldyval;
   oldz    = oldzval;
}

MU_SetPointTranslation::MU_SetPointTranslation()
{
}

MU_SetPointTranslation::~MU_SetPointTranslation()
{
}

void MU_SetPointTranslation::undo( Model * model )
{
   log_debug( "undo point translation\n" );

   double rot[3];
   rot[0] = oldx;
   rot[1] = oldy;
   rot[2] = oldz;
   model->setPointTranslation( number, rot );
}

void MU_SetPointTranslation::redo( Model * model )
{
   log_debug( "redo point translation\n" );

   double rot[3];
   rot[0] = x;
   rot[1] = y;
   rot[2] = z;
   model->setPointTranslation( number, rot );
}

bool MU_SetPointTranslation::combine( Undo * u )
{
   MU_SetPointTranslation * undo = dynamic_cast< MU_SetPointTranslation * >( u );

   if ( undo && undo->number == this->number )
   {
      x = undo->x;
      y = undo->y;
      z = undo->z;

      return true;
   }
   else
   {
      log_debug( "couldn't point translation\n" );
      return false;
   }
}

unsigned MU_SetPointTranslation::size()
{
   return sizeof(MU_SetPointTranslation);
}

void MU_SetPointTranslation::setPointTranslation( int p, double xval, double yval, double zval,
      double oldxval, double oldyval, double oldzval )
{
   number  = p;
   x       = xval;
   y       = yval;
   z       = zval;
   oldx    = oldxval;
   oldy    = oldyval;
   oldz    = oldzval;
}

MU_SetTexture::MU_SetTexture()
{
}

MU_SetTexture::~MU_SetTexture()
{
}

void MU_SetTexture::undo( Model * model )
{
   log_debug( "undo set texture\n" );
   SetTextureList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setGroupTextureId( (*it).groupNumber, (*it).oldTexture );
   }
}

void MU_SetTexture::redo( Model * model )
{
   SetTextureList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setGroupTextureId( (*it).groupNumber, (*it).newTexture );
   }
}

bool MU_SetTexture::combine( Undo * u )
{
   MU_SetTexture * undo = dynamic_cast< MU_SetTexture * >( u );

   if ( undo )
   {
      SetTextureList::iterator it;

      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         setTexture( (*it).groupNumber, (*it).newTexture, (*it).oldTexture );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set texture\n" );
      return false;
   }
}

unsigned MU_SetTexture::size()
{
   return sizeof(MU_SetTexture) + m_list.size() * sizeof(SetTextureT);
}

void MU_SetTexture::setTexture( unsigned groupNumber, int newTexture, int oldTexture )
{
   SetTextureList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).groupNumber == groupNumber )
      {
         (*it).newTexture = newTexture;
         return;
      }
   }

   SetTextureT st;
   st.groupNumber = groupNumber;
   st.newTexture  = newTexture;
   st.oldTexture  = oldTexture;
   m_list.push_back( st );
}

MU_AddVertex::MU_AddVertex()
{
}

MU_AddVertex::~MU_AddVertex()
{
}

void MU_AddVertex::undo( Model * model )
{
   log_debug( "undo add vertex\n" );

   AddVertexList::reverse_iterator it;
   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->removeVertex( (*it).index );
   }
}

void MU_AddVertex::redo( Model * model )
{
   AddVertexList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->insertVertex( (*it).index, (*it).vertex );
   }
}

bool MU_AddVertex::combine( Undo * u )
{
   MU_AddVertex * undo = dynamic_cast<MU_AddVertex *>( u );

   if ( undo )
   {
      AddVertexList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         addVertex( (*it).index, (*it).vertex );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with add vertex\n" );
      return false;
   }
}

void MU_AddVertex::redoRelease()
{
   AddVertexList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it).vertex->release();
   }
}

unsigned MU_AddVertex::size()
{
   return sizeof(MU_AddVertex) + m_list.size() * (sizeof(AddVertexT) + sizeof(Model::Vertex));
}

void MU_AddVertex::addVertex( unsigned index, Model::Vertex * vertex )
{
   if ( vertex )
   {
      AddVertexT v;
      v.index  = index;
      v.vertex = vertex;
      m_list.push_back( v );
   }
}

MU_AddTriangle::MU_AddTriangle()
{
}

MU_AddTriangle::~MU_AddTriangle()
{
}

void MU_AddTriangle::undo( Model * model )
{
   log_debug( "undo add triangle\n" );

   AddTriangleList::reverse_iterator it;
   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->removeTriangle( (*it).index );
   }
}

void MU_AddTriangle::redo( Model * model )
{
   AddTriangleList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->insertTriangle( (*it).index, (*it).triangle );
   }
}

bool MU_AddTriangle::combine( Undo * u )
{
   MU_AddTriangle * undo = dynamic_cast<MU_AddTriangle *>( u );

   if ( undo )
   {
      AddTriangleList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         addTriangle( (*it).index, (*it).triangle );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with add triangle\n" );
      return false;
   }
}

void MU_AddTriangle::redoRelease()
{
   AddTriangleList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it).triangle->release();
   }
}

unsigned MU_AddTriangle::size()
{
   return sizeof(MU_AddTriangle) + m_list.size() * (sizeof(AddTriangleT) + sizeof(Model::Triangle));
}

void MU_AddTriangle::addTriangle( unsigned index, Model::Triangle * triangle )
{
   if ( triangle )
   {
      AddTriangleT v;
      v.index  = index;
      v.triangle = triangle;
      m_list.push_back( v );
   }
}

MU_AddGroup::MU_AddGroup()
{
}

MU_AddGroup::~MU_AddGroup()
{
}

void MU_AddGroup::undo( Model * model )
{
   log_debug( "undo add group\n" );
   AddGroupList::reverse_iterator it;
   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->removeGroup( (*it).index );
   }
}

void MU_AddGroup::redo( Model * model )
{
   AddGroupList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->insertGroup( (*it).index, (*it).group );
   }
}

bool MU_AddGroup::combine( Undo * u )
{
   MU_AddGroup * undo = dynamic_cast<MU_AddGroup *>( u );

   if ( undo )
   {
      AddGroupList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         addGroup( (*it).index, (*it).group );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with add group\n" );
      return false;
   }
}

void MU_AddGroup::redoRelease()
{
   AddGroupList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it).group->release();
   }
}

unsigned MU_AddGroup::size()
{
   return sizeof(MU_AddGroup) + m_list.size() * (sizeof(AddGroupT) + sizeof(Model::Group));
}

void MU_AddGroup::addGroup( unsigned index, Model::Group * group )
{
   if ( group )
   {
      AddGroupT v;
      v.index  = index;
      v.group  = group;
      m_list.push_back( v );
   }
}

MU_AddTexture::MU_AddTexture()
{
}

MU_AddTexture::~MU_AddTexture()
{
}

void MU_AddTexture::undo( Model * model )
{
   log_debug( "undo add texture\n" );
   AddTextureList::reverse_iterator it;
   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->removeTexture( (*it).index );
   }
}

void MU_AddTexture::redo( Model * model )
{
   AddTextureList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->insertTexture( (*it).index, (*it).texture );
   }
}

bool MU_AddTexture::combine( Undo * u )
{
   MU_AddTexture * undo = dynamic_cast<MU_AddTexture *>( u );

   if ( undo )
   {
      AddTextureList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         addTexture( (*it).index, (*it).texture );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with add texture\n" );
      return false;
   }
}

void MU_AddTexture::redoRelease()
{
   AddTextureList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it).texture->release();
   }
}

unsigned MU_AddTexture::size()
{
   return sizeof(MU_AddTexture) + m_list.size() * (sizeof(AddTextureT) + sizeof(Model::Material));
}

void MU_AddTexture::addTexture( unsigned index, Model::Material * texture )
{
   if ( texture )
   {
      AddTextureT v;
      v.index  = index;
      v.texture  = texture;
      m_list.push_back( v );
   }
}

MU_SetTextureCoords::MU_SetTextureCoords()
{
   m_list.clear();
}

MU_SetTextureCoords::~MU_SetTextureCoords()
{
}

void MU_SetTextureCoords::undo( Model * model )
{
   STCList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setTextureCoords( (*it).triangle, (*it).vertexIndex, (*it).oldS, (*it).oldT );
   }
}

void MU_SetTextureCoords::redo( Model * model )
{
   STCList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setTextureCoords( (*it).triangle, (*it).vertexIndex, (*it).s, (*it).t );
   }
}

bool MU_SetTextureCoords::combine( Undo * u )
{
   MU_SetTextureCoords * undo = dynamic_cast<MU_SetTextureCoords *>( u );

   if ( undo )
   {
      STCList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         addTextureCoords( (*it).triangle, (*it).vertexIndex, 
               (*it).s, (*it).t, (*it).oldS, (*it).oldT );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with set texture coords\n" );
      return false;
   }
}

unsigned MU_SetTextureCoords::size()
{
   return sizeof(MU_SetTextureCoords) + m_list.size() * sizeof(SetTextureCoordsT);
}

void MU_SetTextureCoords::addTextureCoords( unsigned triangle, unsigned vertexIndex, float s, float t, float oldS, float oldT )
{
   unsigned index;
   SetTextureCoordsT stc;
   stc.triangle    = triangle;
   stc.vertexIndex = vertexIndex;

   if ( m_list.find_sorted( stc, index ) )
   {
      m_list[index].s = s;
      m_list[index].t = t;
      return;
   }

   stc.s           = s;
   stc.t           = t;
   stc.oldS        = oldS;
   stc.oldT        = oldT;

   m_list.insert_sorted( stc );
}

MU_AddToGroup::MU_AddToGroup()
{
}

MU_AddToGroup::~MU_AddToGroup()
{
}

void MU_AddToGroup::undo( Model * model )
{
   log_debug( "undo add to group\n" );
   AddToGroupList::reverse_iterator it;
   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->removeTriangleFromGroup( (*it).groupNum, (*it).triangleNum );
   }
}

void MU_AddToGroup::redo( Model * model )
{
   AddToGroupList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->addTriangleToGroup( (*it).groupNum, (*it).triangleNum );
   }
}

bool MU_AddToGroup::combine( Undo * u )
{
   MU_AddToGroup * undo = dynamic_cast<MU_AddToGroup *>( u );

   if ( undo )
   {
      AddToGroupList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         addToGroup( (*it).groupNum, (*it).triangleNum );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with add to group\n" );
      return false;
   }
}

unsigned MU_AddToGroup::size()
{
   return sizeof(MU_AddToGroup) + m_list.size() * sizeof(AddToGroupT);
}

void MU_AddToGroup::addToGroup( unsigned groupNum, unsigned triangleNum )
{
   AddToGroupList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).triangleNum == triangleNum )
      {
         (*it).groupNum = groupNum;
         return;
      }
   }

   AddToGroupT add;
   add.groupNum    = groupNum;
   add.triangleNum = triangleNum;
   m_list.push_back( add );
}

MU_RemoveFromGroup::MU_RemoveFromGroup()
{
}

MU_RemoveFromGroup::~MU_RemoveFromGroup()
{
}

void MU_RemoveFromGroup::undo( Model * model )
{
   log_debug( "undo remove from group\n" );
   RemoveFromGroupList::reverse_iterator it;
   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->addTriangleToGroup( (*it).groupNum, (*it).triangleNum );
   }
}

void MU_RemoveFromGroup::redo( Model * model )
{
   RemoveFromGroupList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->removeTriangleFromGroup( (*it).groupNum, (*it).triangleNum );
   }
}

bool MU_RemoveFromGroup::combine( Undo * u )
{
   MU_RemoveFromGroup * undo = dynamic_cast<MU_RemoveFromGroup *>( u );

   if ( undo )
   {
      RemoveFromGroupList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         removeFromGroup( (*it).groupNum, (*it).triangleNum );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with remove from group\n" );
      return false;
   }
}

unsigned MU_RemoveFromGroup::size()
{
   return sizeof(MU_RemoveFromGroup) + m_list.size() * sizeof(RemoveFromGroupT);
}

void MU_RemoveFromGroup::removeFromGroup( unsigned groupNum, unsigned triangleNum )
{
   /*
   RemoveFromGroupList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).triangleNum == triangleNum )
      {
         (*it).groupNum = groupNum;
         return;
      }
   }
   */

   RemoveFromGroupT add;
   add.groupNum    = groupNum;
   add.triangleNum = triangleNum;
   m_list.push_back( add );
}

MU_DeleteTriangle::MU_DeleteTriangle()
{
}

MU_DeleteTriangle::~MU_DeleteTriangle()
{
}

void MU_DeleteTriangle::undo( Model * model )
{
   log_debug( "undo delete triangle\n" );
   DeleteTriangleList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->insertTriangle( (*it).triangleNum, (*it).triangle );
   }
}

void MU_DeleteTriangle::redo( Model * model )
{
   DeleteTriangleList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->removeTriangle( (*it).triangleNum );
   }
}

bool MU_DeleteTriangle::combine( Undo * u )
{
   MU_DeleteTriangle * undo = dynamic_cast<MU_DeleteTriangle *>( u );

   if ( undo )
   {
      DeleteTriangleList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         deleteTriangle( (*it).triangleNum, (*it).triangle );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with delete triangle\n" );
      return false;
   }
}

void MU_DeleteTriangle::undoRelease()
{
   DeleteTriangleList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it).triangle->release();
   }
}

unsigned MU_DeleteTriangle::size()
{
   return sizeof(MU_DeleteTriangle) + m_list.size() * (sizeof(DeleteTriangleT) + sizeof(Model::Triangle));
}

void MU_DeleteTriangle::deleteTriangle( unsigned triangleNum, Model::Triangle * triangle )
{
   DeleteTriangleT del;

   del.triangleNum = triangleNum;
   del.triangle    = triangle;

   m_list.push_back( del );
}

MU_DeleteVertex::MU_DeleteVertex()
{
}

MU_DeleteVertex::~MU_DeleteVertex()
{
}

void MU_DeleteVertex::undo( Model * model )
{
   log_debug( "undo delete vertex\n" );
   DeleteVertexList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->insertVertex( (*it).vertexNum, (*it).vertex );
   }
}

void MU_DeleteVertex::redo( Model * model )
{
   DeleteVertexList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->removeVertex( (*it).vertexNum );
   }
}

bool MU_DeleteVertex::combine( Undo * u )
{
   MU_DeleteVertex * undo = dynamic_cast<MU_DeleteVertex *>( u );

   if ( undo )
   {
      DeleteVertexList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         deleteVertex( (*it).vertexNum, (*it).vertex );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with delete vertex\n" );
      return false;
   }
}

void MU_DeleteVertex::undoRelease()
{
   DeleteVertexList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it).vertex->release();
   }
}

unsigned MU_DeleteVertex::size()
{
   return sizeof(MU_DeleteVertex) + m_list.size() * (sizeof(DeleteVertexT) + sizeof(Model::Vertex));
}

void MU_DeleteVertex::deleteVertex( unsigned vertexNum, Model::Vertex * vertex )
{
   DeleteVertexT del;

   del.vertexNum = vertexNum;
   del.vertex    = vertex;

   m_list.push_back( del );
}

MU_DeleteGroup::MU_DeleteGroup()
{
}

MU_DeleteGroup::~MU_DeleteGroup()
{
}

void MU_DeleteGroup::undo( Model * model )
{
   log_debug( "undo delete group\n" );
   DeleteGroupList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->insertGroup( (*it).groupNum, (*it).group );
   }
}

void MU_DeleteGroup::redo( Model * model )
{
   DeleteGroupList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->removeGroup( (*it).groupNum );
   }
}

bool MU_DeleteGroup::combine( Undo * u )
{
   MU_DeleteGroup * undo = dynamic_cast<MU_DeleteGroup *>( u );

   if ( undo )
   {
      DeleteGroupList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         deleteGroup( (*it).groupNum, (*it).group );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with delete group\n" );
      return false;
   }
}

void MU_DeleteGroup::undoRelease()
{
   DeleteGroupList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it).group->release();
   }
}

unsigned MU_DeleteGroup::size()
{
   return sizeof(MU_DeleteGroup) + m_list.size() * (sizeof(DeleteGroupT) + sizeof(Model::Group));
}

void MU_DeleteGroup::deleteGroup( unsigned groupNum, Model::Group * group )
{
   DeleteGroupT del;

   del.groupNum = groupNum;
   del.group    = group;

   m_list.push_back( del );
}

MU_DeleteTexture::MU_DeleteTexture()
{
}

MU_DeleteTexture::~MU_DeleteTexture()
{
}

void MU_DeleteTexture::undo( Model * model )
{
   log_debug( "undo delete texture\n" );
   DeleteTextureList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->insertTexture( (*it).textureNum, (*it).texture );
   }
}

void MU_DeleteTexture::redo( Model * model )
{
   DeleteTextureList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->removeTexture( (*it).textureNum );
   }
}

bool MU_DeleteTexture::combine( Undo * u )
{
   MU_DeleteTexture * undo = dynamic_cast<MU_DeleteTexture *>( u );

   if ( undo )
   {
      DeleteTextureList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         deleteTexture( (*it).textureNum, (*it).texture );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with delete texture\n" );
      return false;
   }
}

void MU_DeleteTexture::undoRelease()
{
   DeleteTextureList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it).texture->release();
   }
}

unsigned MU_DeleteTexture::size()
{
   return sizeof(MU_DeleteTexture) + m_list.size() * (sizeof(DeleteTextureT) + sizeof(Model::Material));
}

void MU_DeleteTexture::deleteTexture( unsigned textureNum, Model::Material * texture )
{
   DeleteTextureT del;

   del.textureNum = textureNum;
   del.texture    = texture;

   m_list.push_back( del );
}

MU_SetLightProperties::MU_SetLightProperties()
{
}

MU_SetLightProperties::~MU_SetLightProperties()
{
}

void MU_SetLightProperties::undo( Model * model )
{
   log_debug( "undo set light properties\n" );
   LightPropertiesList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      if ( (*it).isSet[0] )
      {
         model->setTextureAmbient( (*it).textureNum, (*it).oldLight[0] );
      }
      if ( (*it).isSet[1] )
      {
         model->setTextureDiffuse( (*it).textureNum, (*it).oldLight[1] );
      }
      if ( (*it).isSet[2] )
      {
         model->setTextureSpecular( (*it).textureNum, (*it).oldLight[2] );
      }
      if ( (*it).isSet[3] )
      {
         model->setTextureEmissive( (*it).textureNum, (*it).oldLight[3] );
      }
   }
}

void MU_SetLightProperties::redo( Model * model )
{
   LightPropertiesList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).isSet[0] )
      {
         model->setTextureAmbient( (*it).textureNum, (*it).newLight[0] );
      }
      if ( (*it).isSet[1] )
      {
         model->setTextureDiffuse( (*it).textureNum, (*it).newLight[1] );
      }
      if ( (*it).isSet[2] )
      {
         model->setTextureSpecular( (*it).textureNum, (*it).newLight[2] );
      }
      if ( (*it).isSet[3] )
      {
         model->setTextureEmissive( (*it).textureNum, (*it).newLight[3] );
      }
   }
}

bool MU_SetLightProperties::combine( Undo * u )
{
   MU_SetLightProperties * undo = dynamic_cast<MU_SetLightProperties *>( u );

   if ( undo )
   {
      LightPropertiesList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         for ( int t = 0; t < LightTypeMax; t++ )
         {
            if ( (*it).isSet[t] )
            {
               setLightProperties( (*it).textureNum, (LightTypeE) t, (*it).newLight[t], (*it).oldLight[t] );
            }
         }
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set light properties\n" );
      return false;
   }
}

unsigned MU_SetLightProperties::size()
{
   return sizeof(MU_SetLightProperties) + m_list.size() * sizeof(LightPropertiesT);
}

void MU_SetLightProperties::setLightProperties( unsigned textureNum, LightTypeE type, const float * newLight, const float * oldLight )
{
   LightPropertiesList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).textureNum == textureNum )
      {
         // Set old light if this is the first time we set this type
         if ( ! (*it).isSet[type] )
         {
            for ( int n = 0; n < 4; n++ )
            {
               (*it).oldLight[type][n] = oldLight[n];
            }
         }

         // Set new light for this type
         (*it).isSet[ type ] = true;
         for ( int n = 0; n < 4; n++ )
         {
            (*it).newLight[type][n] = newLight[n];
         }
         return;
      }
   }

   // Add new LightPropertiesT to list
   LightPropertiesT prop;

   for ( int t = 0; t < LightTypeMax; t++ )
   {
      prop.isSet[t] = false;
   }

   prop.textureNum = textureNum;
   prop.isSet[ type ] = true;
   for ( int n = 0; n < 4; n++ )
   {
      prop.oldLight[type][n] = oldLight[n];
      prop.newLight[type][n] = newLight[n];
   }

   m_list.push_back( prop );
}

MU_SetShininess::MU_SetShininess()
{
}

MU_SetShininess::~MU_SetShininess()
{
}

void MU_SetShininess::undo( Model * model )
{
   log_debug( "undo set shininess\n" );
   ShininessList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->setTextureShininess( (*it).textureNum, (*it).oldValue );
   }
}

void MU_SetShininess::redo( Model * model )
{
   ShininessList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setTextureShininess( (*it).textureNum, (*it).newValue );
   }
}

bool MU_SetShininess::combine( Undo * u )
{
   MU_SetShininess * undo = dynamic_cast<MU_SetShininess *>( u );

   if ( undo )
   {
      ShininessList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         setShininess( (*it).textureNum, (*it).newValue, (*it).oldValue );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set shininess\n" );
      return false;
   }
}

unsigned MU_SetShininess::size()
{
   return sizeof(MU_SetShininess) + m_list.size() * sizeof(ShininessT);
}

void MU_SetShininess::setShininess( unsigned textureNum, const float & newValue, const float & oldValue )
{
   ShininessList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).textureNum == textureNum )
      {
         (*it).newValue = newValue;
         return;
      }
   }

   // Add new ShininessT to list
   ShininessT shine;

   shine.textureNum = textureNum;
   shine.oldValue = oldValue;
   shine.newValue = newValue;

   m_list.push_back( shine );
}

MU_SetTextureName::MU_SetTextureName()
{
}

MU_SetTextureName::~MU_SetTextureName()
{
}

void MU_SetTextureName::undo( Model * model )
{
   log_debug( "undo set texture name\n" );
   model->setTextureName( m_textureNum, m_oldName.c_str() );
}

void MU_SetTextureName::redo( Model * model )
{
   model->setTextureName( m_textureNum, m_newName.c_str() );
}

bool MU_SetTextureName::combine( Undo * u )
{
   MU_SetTextureName * undo = dynamic_cast< MU_SetTextureName * >( u );

   if ( undo && undo->m_textureNum == m_textureNum )
   {
      m_newName = undo->m_newName;
      return true;
   }
   else
   {
      return false;
   }
}

unsigned MU_SetTextureName::size()
{
   return sizeof(MU_SetTextureName);
}

void MU_SetTextureName::setTextureName( unsigned textureNum, const char * newName, const char * oldName )
{
   m_textureNum = textureNum;
   m_newName    = newName;
   m_oldName    = oldName;
}

MU_SetTriangleVertices::MU_SetTriangleVertices()
{
}

MU_SetTriangleVertices::~MU_SetTriangleVertices()
{
}

void MU_SetTriangleVertices::undo( Model * model )
{
   log_debug( "undo set triangle vertices\n" );
   TriangleVerticesList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->setTriangleVertices( (*it).triangleNum, 
            (*it).oldVertices[0],
            (*it).oldVertices[1],
            (*it).oldVertices[2] );
   }
}

void MU_SetTriangleVertices::redo( Model * model )
{
   TriangleVerticesList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setTriangleVertices( (*it).triangleNum, 
            (*it).newVertices[0],
            (*it).newVertices[1],
            (*it).newVertices[2] );
   }
}

bool MU_SetTriangleVertices::combine( Undo * u )
{
   MU_SetTriangleVertices * undo = dynamic_cast<MU_SetTriangleVertices *>( u );

   if ( undo )
   {
      TriangleVerticesList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         setTriangleVertices( (*it).triangleNum, 
               (*it).newVertices[0], 
               (*it).newVertices[1], 
               (*it).newVertices[2], 
               (*it).oldVertices[0], 
               (*it).oldVertices[1], 
               (*it).oldVertices[2] );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set triangle vertices\n" );
      return false;
   }
}

unsigned MU_SetTriangleVertices::size()
{
   return sizeof(MU_SetTriangleVertices) + m_list.size() * sizeof(TriangleVerticesT);
}

void MU_SetTriangleVertices::setTriangleVertices( unsigned triangleNum, 
      unsigned newV1, unsigned newV2, unsigned newV3, 
      unsigned oldV1, unsigned oldV2, unsigned oldV3 )
{
   TriangleVerticesList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).triangleNum == triangleNum )
      {
         (*it).newVertices[0] = newV1;
         (*it).newVertices[1] = newV2;
         (*it).newVertices[2] = newV3;
         return;
      }
   }

   // Add new ShininessT to list
   TriangleVerticesT tv;

   tv.triangleNum = triangleNum;
   tv.oldVertices[0] = oldV1;
   tv.oldVertices[1] = oldV2;
   tv.oldVertices[2] = oldV3;
   tv.newVertices[0] = newV1;
   tv.newVertices[1] = newV2;
   tv.newVertices[2] = newV3;

   m_list.push_back( tv );
}

MU_SubdivideSelected::MU_SubdivideSelected()
{
}

MU_SubdivideSelected::~MU_SubdivideSelected()
{
}

void MU_SubdivideSelected::undo( Model * model )
{
   log_debug( "undo subdivide selected\n" );
}

void MU_SubdivideSelected::redo( Model * model )
{
   model->subdivideSelectedTriangles();
}

bool MU_SubdivideSelected::combine( Undo * u )
{
   log_debug( "couldn't combine with subdivide selected\n" );
   return false;
}

MU_SubdivideTriangle::MU_SubdivideTriangle()
{
}

MU_SubdivideTriangle::~MU_SubdivideTriangle()
{
}

void MU_SubdivideTriangle::undo( Model * model )
{
   log_debug( "undo subdivide\n" );

   SubdivideTriangleList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->unsubdivideTriangles( (*it).a, (*it).b, (*it).c, (*it).d );
   }

   list<unsigned>::reverse_iterator iit;

   for ( iit = m_vlist.rbegin(); iit != m_vlist.rend(); iit++ )
   {
      model->deleteVertex( *iit );
   }
}

void MU_SubdivideTriangle::redo( Model * model )
{
}

bool MU_SubdivideTriangle::combine( Undo * u )
{
   MU_SubdivideTriangle * undo = dynamic_cast<MU_SubdivideTriangle *>( u );
   if ( undo )
   {
      SubdivideTriangleT st;

      while ( ! undo->m_list.empty() )
      {
         st = undo->m_list.front();
         subdivide( st.a, st.b, st.c, st.d );
         undo->m_list.pop_front();
      }

      while ( ! undo->m_vlist.empty() )
      {
         addVertex( undo->m_vlist.front() );
         undo->m_vlist.pop_front();
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with subdivide triangles\n" );
      return false;
   }
}

unsigned MU_SubdivideTriangle::size()
{
   return sizeof( MU_SubdivideTriangle ) 
      + m_list.size()  * sizeof( SubdivideTriangleT )
      + m_vlist.size() * sizeof(int);
}

void MU_SubdivideTriangle::subdivide( unsigned a, unsigned b, unsigned c, unsigned d )
{
   SubdivideTriangleT st;

   st.a = a;
   st.b = b;
   st.c = c;
   st.d = d;

   m_list.push_back( st );
}

void MU_SubdivideTriangle::addVertex( unsigned v )
{
   m_vlist.push_back( v );
}

MU_ChangeAnimState::MU_ChangeAnimState()
{
}

MU_ChangeAnimState::~MU_ChangeAnimState()
{
}

void MU_ChangeAnimState::undo( Model * model )
{
   log_debug( "undo change anim state: old %d\n", m_oldMode );
   if ( m_oldMode )
   {
      model->setCurrentAnimation( m_oldMode, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
   else
   {
      model->setNoAnimation();
   }
}

void MU_ChangeAnimState::redo( Model * model )
{
   log_debug( "redo change anim state: new %d\n", m_newMode );
   if ( m_newMode )
   {
      model->setCurrentAnimation( m_newMode, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
   else
   {
      model->setNoAnimation();
   }
}

bool MU_ChangeAnimState::combine( Undo * u )
{
   return false;
}

unsigned MU_ChangeAnimState::size()
{
   return sizeof( MU_ChangeAnimState );
}

void MU_ChangeAnimState::setState( Model::AnimationModeE newMode, Model::AnimationModeE oldMode, unsigned anim, unsigned frame )
{
   log_debug( "ChangeAnimState undo info: old = %d, new = %d\n", oldMode, newMode );
   m_newMode = newMode;
   m_oldMode = oldMode;
   m_anim    = anim;
   m_frame   = frame;
}

MU_SetAnimName::MU_SetAnimName()
{
}

MU_SetAnimName::~MU_SetAnimName()
{
}

void MU_SetAnimName::undo( Model * model )
{
   model->setAnimName( m_mode, m_animNum, m_oldName.c_str() );
   if ( model->getAnimationMode() != Model::ANIMMODE_NONE && model->getAnimationMode() != m_mode || model->getCurrentAnimation() != m_animNum )
   {
      model->setCurrentAnimation( m_mode, m_oldName.c_str() );
   }
}

void MU_SetAnimName::redo( Model * model )
{
   model->setAnimName( m_mode, m_animNum, m_newName.c_str() );
   if ( model->getAnimationMode() != Model::ANIMMODE_NONE && model->getAnimationMode() != m_mode || model->getCurrentAnimation() != m_animNum )
   {
      model->setCurrentAnimation( m_mode, m_newName.c_str() );
   }
}

bool MU_SetAnimName::combine( Undo * u )
{
   return false;
}

unsigned MU_SetAnimName::size()
{
   return sizeof(MU_SetAnimName);
}

void MU_SetAnimName::setName( Model::AnimationModeE mode, unsigned animNum, const char * newName, const char * oldName )
{
   m_mode       = mode;
   m_animNum    = animNum;
   m_newName    = newName;
   m_oldName    = oldName;
}

MU_SetAnimFrameCount::MU_SetAnimFrameCount()
{
}

MU_SetAnimFrameCount::~MU_SetAnimFrameCount()
{
}

void MU_SetAnimFrameCount::undo( Model * model )
{
   model->setAnimFrameCount( m_mode, m_animNum, m_oldCount );
}

void MU_SetAnimFrameCount::redo( Model * model )
{
   model->setAnimFrameCount( m_mode, m_animNum, m_newCount );
}

bool MU_SetAnimFrameCount::combine( Undo * u )
{
   return false;
}

unsigned MU_SetAnimFrameCount::size()
{
   return sizeof(MU_SetAnimFrameCount);
}

void MU_SetAnimFrameCount::setAnimFrameCount( Model::AnimationModeE mode, unsigned animNum, unsigned newCount, unsigned oldCount )
{
   m_mode       = mode;
   m_animNum    = animNum;
   m_newCount   = newCount;
   m_oldCount   = oldCount;
}

MU_SetAnimFPS::MU_SetAnimFPS()
{
}

MU_SetAnimFPS::~MU_SetAnimFPS()
{
}

void MU_SetAnimFPS::undo( Model * model )
{
   model->setAnimFPS( m_mode, m_animNum, m_oldFPS );
   if ( model->getAnimationMode() != Model::ANIMMODE_NONE && (model->getAnimationMode() != m_mode || model->getCurrentAnimation() != m_animNum) ) 
   {
      model->setCurrentAnimation( m_mode, m_animNum );
   }
}

void MU_SetAnimFPS::redo( Model * model )
{
   model->setAnimFPS( m_mode, m_animNum, m_newFPS );
   if ( model->getAnimationMode() != Model::ANIMMODE_NONE && (model->getAnimationMode() != m_mode || model->getCurrentAnimation() != m_animNum) ) 
   {
      model->setCurrentAnimation( m_mode, m_animNum );
   }
}

bool MU_SetAnimFPS::combine( Undo * u )
{
   MU_SetAnimFPS * undo = dynamic_cast< MU_SetAnimFPS * >( u );

   if ( undo && undo->m_mode == m_mode && undo->m_animNum == m_animNum )
   {
      m_newFPS = undo->m_newFPS;
      return true;
   }
   else
   {
      log_debug( "couldn't combine with set anim fps\n" );
      return false;
   }
}

unsigned MU_SetAnimFPS::size()
{
   return sizeof(MU_SetAnimFPS);
}

void MU_SetAnimFPS::setFPS( Model::AnimationModeE mode, unsigned animNum, double newFps, double oldFps )
{
   m_mode       = mode;
   m_animNum    = animNum;
   m_newFPS     = newFps;
   m_oldFPS     = oldFps;
}

MU_SetAnimKeyframe::MU_SetAnimKeyframe()
{
}

MU_SetAnimKeyframe::~MU_SetAnimKeyframe()
{
}

void MU_SetAnimKeyframe::undo( Model * model )
{
   SetKeyframeList::iterator it;

   for ( it = m_keyframes.begin(); it != m_keyframes.end(); it++ )
   {
      if ( (*it).isNew )
      {
         log_debug( "undoing new keyframe\n" );
         model->removeSkelAnimKeyframe( m_anim, m_frame, (*it).number, m_isRotation, true );
      }
      else
      {
         log_debug( "undoing existing keyframe\n" );
         model->setSkelAnimKeyframe( m_anim, m_frame, (*it).number, m_isRotation,
               (*it).oldx, (*it).oldy, (*it).oldz );
      }
   }
   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_SKELETAL, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
}

void MU_SetAnimKeyframe::redo( Model * model )
{
   SetKeyframeList::iterator it;

   for ( it = m_keyframes.begin(); it != m_keyframes.end(); it++ )
   {
      model->setSkelAnimKeyframe( m_anim, m_frame, (*it).number, m_isRotation,
            (*it).x, (*it).y, (*it).z );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_SKELETAL, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
}

bool MU_SetAnimKeyframe::combine( Undo * u )
{
   MU_SetAnimKeyframe * undo = dynamic_cast< MU_SetAnimKeyframe * >( u );

   if ( undo && undo->m_anim == m_anim && undo->m_frame == m_frame && m_isRotation == undo->m_isRotation )
   {
      SetKeyframeList::iterator it;

      for ( it = undo->m_keyframes.begin(); it != undo->m_keyframes.end(); it++ )
      {
         addBoneJoint( (*it).number, (*it).isNew, (*it).x, (*it).y, (*it).z,
               (*it).oldx, (*it).oldy, (*it).oldz );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set anim keyframe\n" );
      return false;
   }
}

unsigned MU_SetAnimKeyframe::size()
{
   return sizeof(MU_SetAnimKeyframe) + m_keyframes.size() * sizeof(SetKeyFrameT);
}

void MU_SetAnimKeyframe::setAnimationData( const unsigned & anim, const unsigned & frame, const bool & isRotation )
{
   m_anim       = anim;
   m_frame      = frame;
   m_isRotation = isRotation;
}

void MU_SetAnimKeyframe::addBoneJoint( int j, bool isNew, double x, double y, double z,
      double oldx, double oldy, double oldz )
{
   unsigned index = 0;
   SetKeyFrameT mv;
   mv.number = j;

   // Modify a joint we already have
   if ( m_keyframes.find_sorted( mv, index ) )
   {
      m_keyframes[index].x = x;
      m_keyframes[index].y = y;
      m_keyframes[index].z = z;
      return;
   }

   // Not found, add new joint information
   mv.isNew   = isNew;
   mv.x       = x;
   mv.y       = y;
   mv.z       = z;
   mv.oldx    = oldx;
   mv.oldy    = oldy;
   mv.oldz    = oldz;
   m_keyframes.insert_sorted( mv );
}

MU_DeleteKeyframe::MU_DeleteKeyframe()
{
}

MU_DeleteKeyframe::~MU_DeleteKeyframe()
{
}

void MU_DeleteKeyframe::undo( Model * model )
{
   log_debug( "undo delete keyframe\n" );
   DeleteKeyframeList::reverse_iterator it;

   unsigned frame = 0;
   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->insertSkelAnimKeyframe( m_anim, *it );
      frame = (*it)->m_frame;
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE )
   {
      if ( m_anim != model->getCurrentAnimation() )
      {
         model->setCurrentAnimation( Model::ANIMMODE_SKELETAL, m_anim );
      }
      model->setCurrentAnimationFrame( frame );
   }
}

void MU_DeleteKeyframe::redo( Model * model )
{
   DeleteKeyframeList::iterator it;

   unsigned frame = 0;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->removeSkelAnimKeyframe( m_anim, (*it)->m_frame, (*it)->m_jointIndex, (*it)->m_isRotation );
      frame = (*it)->m_frame;
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE )
   {
      if ( m_anim != model->getCurrentAnimation() )
      {
         model->setCurrentAnimation( Model::ANIMMODE_SKELETAL, m_anim );
      }
      model->setCurrentAnimationFrame( frame );
   }
}

bool MU_DeleteKeyframe::combine( Undo * u )
{
   MU_DeleteKeyframe * undo = dynamic_cast<MU_DeleteKeyframe *>( u );

   if ( undo && m_anim == undo->m_anim )
   {
      DeleteKeyframeList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         deleteKeyframe( *it );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with delete keyframe\n" );
      return false;
   }
}

void MU_DeleteKeyframe::undoRelease()
{
   DeleteKeyframeList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      (*it)->release();
   }
}

unsigned MU_DeleteKeyframe::size()
{
   return sizeof(MU_DeleteKeyframe) + m_list.size() * sizeof(Model::Keyframe);
}

void MU_DeleteKeyframe::setAnimationData( const unsigned & anim )
{
   m_anim = anim;
}

void MU_DeleteKeyframe::deleteKeyframe( Model::Keyframe * keyframe )
{
   m_list.push_back( keyframe );
}

MU_SetJointName::MU_SetJointName()
{
}

MU_SetJointName::~MU_SetJointName()
{
}

void MU_SetJointName::undo( Model * model )
{
   model->setBoneJointName( m_joint, m_oldName.c_str() );
}

void MU_SetJointName::redo( Model * model )
{
   model->setBoneJointName( m_joint, m_newName.c_str() );
}

bool MU_SetJointName::combine( Undo * u )
{
   return false;
}

unsigned MU_SetJointName::size()
{
   return sizeof(MU_SetJointName);
}

void MU_SetJointName::setName( unsigned joint, const char * newName, const char * oldName )
{
   m_joint   = joint;
   m_newName = newName;
   m_oldName = oldName;
}

MU_SetPointName::MU_SetPointName()
{
}

MU_SetPointName::~MU_SetPointName()
{
}

void MU_SetPointName::undo( Model * model )
{
   model->setPointName( m_point, m_oldName.c_str() );
}

void MU_SetPointName::redo( Model * model )
{
   model->setPointName( m_point, m_newName.c_str() );
}

bool MU_SetPointName::combine( Undo * u )
{
   return false;
}

unsigned MU_SetPointName::size()
{
   return sizeof(MU_SetPointName);
}

void MU_SetPointName::setName( unsigned point, const char * newName, const char * oldName )
{
   m_point   = point;
   m_newName = newName;
   m_oldName = oldName;
}

MU_SetProjectionName::MU_SetProjectionName()
{
}

MU_SetProjectionName::~MU_SetProjectionName()
{
}

void MU_SetProjectionName::undo( Model * model )
{
   model->setProjectionName( m_projection, m_oldName.c_str() );
}

void MU_SetProjectionName::redo( Model * model )
{
   model->setProjectionName( m_projection, m_newName.c_str() );
}

bool MU_SetProjectionName::combine( Undo * u )
{
   return false;
}

unsigned MU_SetProjectionName::size()
{
   return sizeof(MU_SetProjectionName);
}

void MU_SetProjectionName::setName( unsigned projection, const char * newName, const char * oldName )
{
   m_projection = projection;
   m_newName    = newName;
   m_oldName    = oldName;
}

MU_SetProjectionType::MU_SetProjectionType()
{
}

MU_SetProjectionType::~MU_SetProjectionType()
{
}

void MU_SetProjectionType::undo( Model * model )
{
   model->setProjectionType( m_projection, m_oldType );
}

void MU_SetProjectionType::redo( Model * model )
{
   model->setProjectionType( m_projection, m_newType );
}

bool MU_SetProjectionType::combine( Undo * u )
{
   return false;
}

unsigned MU_SetProjectionType::size()
{
   return sizeof(MU_SetProjectionType);
}

void MU_SetProjectionType::setType( unsigned projection, int newType, int oldType )
{
   m_projection = projection;
   m_newType    = newType;
   m_oldType    = oldType;
}

MU_MoveFrameVertex::MU_MoveFrameVertex()
{
}

MU_MoveFrameVertex::~MU_MoveFrameVertex()
{
}

void MU_MoveFrameVertex::undo( Model * model )
{
   MoveFrameVertexList::iterator it;

   // Modify a vertex we already have
   for ( it = m_vertices.begin(); it != m_vertices.end(); it++ )
   {
      model->setFrameAnimVertexCoords( m_anim, m_frame, (*it).number, (*it).oldx, (*it).oldy, (*it).oldz );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
}

void MU_MoveFrameVertex::redo( Model * model )
{
   MoveFrameVertexList::iterator it;

   // Modify a vertex we already have
   for ( it = m_vertices.begin(); it != m_vertices.end(); it++ )
   {
      model->setFrameAnimVertexCoords( m_anim, m_frame, (*it).number, (*it).x, (*it).y, (*it).z );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
}

bool MU_MoveFrameVertex::combine( Undo * u )
{
   MU_MoveFrameVertex * undo = dynamic_cast< MU_MoveFrameVertex * >( u );

   if ( undo )
   {
      MoveFrameVertexList::iterator it;

      for ( it = undo->m_vertices.begin(); it != undo->m_vertices.end(); it++ )
      {
         addVertex( (*it).number, (*it).x, (*it).y, (*it).z,
               (*it).oldx, (*it).oldy, (*it).oldz );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with move frame vertex\n" );
      return false;
   }
}

unsigned MU_MoveFrameVertex::size()
{
   return sizeof(MU_MoveFrameVertex) + m_vertices.size() * sizeof(MoveFrameVertexT);
}

void MU_MoveFrameVertex::setAnimationData( const unsigned & anim, const unsigned & frame )
{
   m_anim = anim;
   m_frame = frame;
}

void MU_MoveFrameVertex::addVertex( int v, double x, double y, double z,
      double oldx, double oldy, double oldz )
{
   unsigned index;
   MoveFrameVertexT mv;
   mv.number = v;

   // Modify a vertex we already have
   if ( m_vertices.find_sorted( mv, index ) )
   {
      m_vertices[index].x = x;
      m_vertices[index].y = y;
      m_vertices[index].z = z;
      return;
   }

   // Not found, add new vertex information
   mv.x       = x;
   mv.y       = y;
   mv.z       = z;
   mv.oldx    = oldx;
   mv.oldy    = oldy;
   mv.oldz    = oldz;
   m_vertices.insert_sorted( mv );
}

MU_MoveFramePoint::MU_MoveFramePoint()
{
}

MU_MoveFramePoint::~MU_MoveFramePoint()
{
}

void MU_MoveFramePoint::undo( Model * model )
{
   MoveFramePointList::iterator it;

   for ( it = m_points.begin(); it != m_points.end(); it++ )
   {
      model->setFrameAnimPointCoords( m_anim, m_frame, (*it).number, (*it).oldx, (*it).oldy, (*it).oldz );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
}

void MU_MoveFramePoint::redo( Model * model )
{
   MoveFramePointList::iterator it;

   for ( it = m_points.begin(); it != m_points.end(); it++ )
   {
      model->setFrameAnimPointCoords( m_anim, m_frame, (*it).number, (*it).x, (*it).y, (*it).z );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
}

bool MU_MoveFramePoint::combine( Undo * u )
{
   MU_MoveFramePoint * undo = dynamic_cast< MU_MoveFramePoint * >( u );

   if ( undo )
   {
      MoveFramePointList::iterator it;

      for ( it = undo->m_points.begin(); it != undo->m_points.end(); it++ )
      {
         addPoint( (*it).number, (*it).x, (*it).y, (*it).z,
               (*it).oldx, (*it).oldy, (*it).oldz );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with move frame point\n" );
      return false;
   }
}

unsigned MU_MoveFramePoint::size()
{
   return sizeof(MU_MoveFramePoint) + m_points.size() * sizeof(MoveFramePointT);
}

void MU_MoveFramePoint::setAnimationData( const unsigned & anim, const unsigned & frame )
{
   m_anim = anim;
   m_frame = frame;
}

void MU_MoveFramePoint::addPoint( int p, double x, double y, double z,
      double oldx, double oldy, double oldz )
{
   unsigned index;
   MoveFramePointT mv;
   mv.number = p;

   // Modify a point we already have
   if ( m_points.find_sorted( mv, index ) )
   {
      m_points[index].x = x;
      m_points[index].y = y;
      m_points[index].z = z;
      return;
   }

   // Not found, add new point information
   mv.x       = x;
   mv.y       = y;
   mv.z       = z;
   mv.oldx    = oldx;
   mv.oldy    = oldy;
   mv.oldz    = oldz;
   m_points.insert_sorted( mv );
}

MU_RotateFramePoint::MU_RotateFramePoint()
{
}

MU_RotateFramePoint::~MU_RotateFramePoint()
{
}

void MU_RotateFramePoint::undo( Model * model )
{
   RotateFramePointList::iterator it;

   for ( it = m_points.begin(); it != m_points.end(); it++ )
   {
      model->setFrameAnimPointRotation( m_anim, m_frame, (*it).number, (*it).oldx, (*it).oldy, (*it).oldz );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
}

void MU_RotateFramePoint::redo( Model * model )
{
   RotateFramePointList::iterator it;

   for ( it = m_points.begin(); it != m_points.end(); it++ )
   {
      model->setFrameAnimPointRotation( m_anim, m_frame, (*it).number, (*it).x, (*it).y, (*it).z );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( m_frame );
   }
}

bool MU_RotateFramePoint::combine( Undo * u )
{
   MU_RotateFramePoint * undo = dynamic_cast< MU_RotateFramePoint * >( u );

   if ( undo )
   {
      RotateFramePointList::iterator it;

      for ( it = undo->m_points.begin(); it != undo->m_points.end(); it++ )
      {
         addPointRotation( (*it).number, (*it).x, (*it).y, (*it).z,
               (*it).oldx, (*it).oldy, (*it).oldz );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with rotate frame point\n" );
      return false;
   }
}

unsigned MU_RotateFramePoint::size()
{
   return sizeof(MU_RotateFramePoint) + m_points.size() * sizeof(RotateFramePointT);
}

void MU_RotateFramePoint::setAnimationData( const unsigned & anim, const unsigned & frame )
{
   m_anim = anim;
   m_frame = frame;
}

void MU_RotateFramePoint::addPointRotation( int p, double x, double y, double z,
      double oldx, double oldy, double oldz )
{
   unsigned index;
   RotateFramePointT mv;
   mv.number = p;

   // Modify a point we already have
   if ( m_points.find_sorted( mv, index ) )
   {
      m_points[index].x = x;
      m_points[index].y = y;
      m_points[index].z = z;
      return;
   }

   // Not found, add new point information
   mv.x       = x;
   mv.y       = y;
   mv.z       = z;
   mv.oldx    = oldx;
   mv.oldy    = oldy;
   mv.oldz    = oldz;
   m_points.insert_sorted( mv );
}

MU_AddFrameAnimFrame::MU_AddFrameAnimFrame()
{
}

MU_AddFrameAnimFrame::~MU_AddFrameAnimFrame()
{
}

void MU_AddFrameAnimFrame::undo( Model * model )
{
   log_debug( "undo add frame\n" );

   FrameDataList::reverse_iterator it;
   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->removeFrameAnimFrame( m_anim, (*it).frame );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( 0 );
   }
}

void MU_AddFrameAnimFrame::redo( Model * model )
{
   FrameDataList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->insertFrameAnimFrame( m_anim, (*it).frame, (*it).data );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( 0 );
   }
}

bool MU_AddFrameAnimFrame::combine( Undo * u )
{
   MU_AddFrameAnimFrame * undo = dynamic_cast<MU_AddFrameAnimFrame *>( u );

   if ( undo && m_anim == undo->m_anim )
   {
      FrameDataList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         addFrame( (*it).frame, (*it).data );
      }
      return true;
   }
   else
   {
      log_debug( "couldn't combine with add frame anim frame\n" );
      return false;
   }
}

void MU_AddFrameAnimFrame::redoRelease()
{
   FrameDataList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      unsigned t;
      for ( t = 0; t < (*it).data->m_frameVertices->size(); t++ )
      {
         (*(*it).data->m_frameVertices)[t]->release();
      }
      (*it).data->m_frameVertices->clear();
      for ( t = 0; t < (*it).data->m_framePoints->size(); t++ )
      {
         (*(*it).data->m_framePoints)[t]->release();
      }
      (*it).data->m_framePoints->clear();
   }
}

unsigned MU_AddFrameAnimFrame::size()
{
   unsigned listSize = 0;
   FrameDataList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      listSize += sizeof( Model::FrameAnimDataList ) + (*it).data->m_frameVertices->size() * sizeof( Model::FrameAnimVertex ) + (*it).data->m_framePoints->size() * sizeof( Model::FrameAnimPoint );
   }
   return sizeof(MU_AddFrameAnimFrame) + m_list.size() * sizeof(AddFrameT) + listSize;
}

void MU_AddFrameAnimFrame::setAnimationData( const unsigned & anim )
{
   m_anim = anim;
}

void MU_AddFrameAnimFrame::addFrame( const unsigned & frame, Model::FrameAnimData * data )
{
   if ( data )
   {
      AddFrameT l;
      l.frame  = frame;
      l.data   = data;
      m_list.push_back( l );
   }
}

MU_DeleteFrameAnimFrame::MU_DeleteFrameAnimFrame()
{
}

MU_DeleteFrameAnimFrame::~MU_DeleteFrameAnimFrame()
{
}

void MU_DeleteFrameAnimFrame::undo( Model * model )
{
   log_debug( "undo delete frame anim frame\n" );
   FrameDataList::reverse_iterator it;

   for ( it = m_list.rbegin(); it != m_list.rend(); it++ )
   {
      model->insertFrameAnimFrame( m_anim, (*it).frame, (*it).data );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( 0 );
   }
}

void MU_DeleteFrameAnimFrame::redo( Model * model )
{
   FrameDataList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->removeFrameAnimFrame( m_anim, (*it).frame );
   }

   if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
   {
      model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      model->setCurrentAnimationFrame( 0 );
   }
}

bool MU_DeleteFrameAnimFrame::combine( Undo * u )
{
   MU_DeleteFrameAnimFrame * undo = dynamic_cast<MU_DeleteFrameAnimFrame *>( u );

   if ( undo && m_anim == undo->m_anim )
   {
      FrameDataList::iterator it;
      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         deleteFrame( (*it).frame, (*it).data );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with delete frame\n" );
      return false;
   }
}

void MU_DeleteFrameAnimFrame::undoRelease()
{
   FrameDataList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      unsigned t;
      for ( t = 0; t < (*it).data->m_frameVertices->size(); t++ )
      {
         (*(*it).data->m_frameVertices)[t]->release();
      }
      for ( t = 0; t < (*it).data->m_framePoints->size(); t++ )
      {
         (*(*it).data->m_framePoints)[t]->release();
      }
      (*it).data->m_frameVertices->clear();
      (*it).data->m_framePoints->clear();
   }
}

unsigned MU_DeleteFrameAnimFrame::size()
{
   unsigned listSize = 0;
   FrameDataList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      listSize += sizeof( Model::FrameAnimVertexList ) + (*it).data->m_frameVertices->size() * sizeof( Model::FrameAnimVertex ) + (*it).data->m_framePoints->size() * sizeof( Model::FrameAnimPoint );
   }
   return sizeof(MU_DeleteFrameAnimFrame) + m_list.size() * sizeof(DeleteFrameT) + listSize;
}

void MU_DeleteFrameAnimFrame::setAnimationData( const unsigned & anim )
{
   m_anim = anim;
}

void MU_DeleteFrameAnimFrame::deleteFrame( const unsigned & frame, Model::FrameAnimData * data )
{
   DeleteFrameT del;

   del.frame = frame;
   del.data  = data;

   m_list.push_back( del );
}

MU_SetPositionInfluence::MU_SetPositionInfluence()
{
}

MU_SetPositionInfluence::~MU_SetPositionInfluence()
{
}

void MU_SetPositionInfluence::undo( Model * model )
{
   if ( m_isAdd )
   {
      model->removeInfluence( m_pos, m_index );
   }
   else
   {
      model->insertInfluence( m_pos, m_index, m_influence );
   }
}

void MU_SetPositionInfluence::redo( Model * model )
{
   if ( m_isAdd )
   {
      model->insertInfluence( m_pos, m_index, m_influence );
   }
   else
   {
      model->removeInfluence( m_pos, m_index );
   }
}

bool MU_SetPositionInfluence::combine( Undo * u )
{
   return false;
}

unsigned MU_SetPositionInfluence::size()
{
   return sizeof(MU_SetPositionInfluence);
}

void MU_SetPositionInfluence::setPositionInfluence( bool isAdd,
      const Model::Position & pos, 
      unsigned index, const Model::InfluenceT & influence )
{
   m_isAdd = isAdd;
   m_index = index;
   m_pos = pos;
   m_influence = influence;
}

MU_UpdatePositionInfluence::MU_UpdatePositionInfluence()
{
}

MU_UpdatePositionInfluence::~MU_UpdatePositionInfluence()
{
}

void MU_UpdatePositionInfluence::undo( Model * model )
{
   model->setPositionInfluenceType( 
         m_pos, m_oldInf.m_boneId, m_oldInf.m_type );
   model->setPositionInfluenceWeight( 
         m_pos, m_oldInf.m_boneId, m_oldInf.m_weight );
}

void MU_UpdatePositionInfluence::redo( Model * model )
{
   model->setPositionInfluenceType( 
         m_pos, m_newInf.m_boneId, m_newInf.m_type );
   model->setPositionInfluenceWeight( 
         m_pos, m_newInf.m_boneId, m_newInf.m_weight );
}

bool MU_UpdatePositionInfluence::combine( Undo * u )
{
   return false;
}

unsigned MU_UpdatePositionInfluence::size()
{
   return sizeof(MU_UpdatePositionInfluence);
}

void MU_UpdatePositionInfluence::updatePositionInfluence( const Model::Position & pos, 
      const Model::InfluenceT & newInf,
      const Model::InfluenceT & oldInf )
{
   m_pos = pos;
   m_newInf = newInf;
   m_oldInf = oldInf;
}

MU_SetVertexBoneJoint::MU_SetVertexBoneJoint()
{
}

MU_SetVertexBoneJoint::~MU_SetVertexBoneJoint()
{
}

void MU_SetVertexBoneJoint::undo( Model * model )
{
   log_debug( "undo set vertex bone joint\n" );
   SetJointList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setVertexBoneJoint( (*it).vertex, (*it).oldBone );
   }
}

void MU_SetVertexBoneJoint::redo( Model * model )
{
   SetJointList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setVertexBoneJoint( (*it).vertex, (*it).newBone );
   }
}

bool MU_SetVertexBoneJoint::combine( Undo * u )
{
   MU_SetVertexBoneJoint * undo = dynamic_cast< MU_SetVertexBoneJoint * >( u );

   if ( undo )
   {
      SetJointList::iterator it;

      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         setVertexBoneJoint( (*it).vertex, (*it).newBone, (*it).oldBone );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set vertex bone joint\n" );
      return false;
   }
}

unsigned MU_SetVertexBoneJoint::size()
{
   return sizeof(MU_SetVertexBoneJoint) + m_list.size() * sizeof(SetJointT);
}

void MU_SetVertexBoneJoint::setVertexBoneJoint( const unsigned & vertex, 
      const int & newBone, const int & oldBone )
{
   SetJointList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).vertex == vertex )
      {
         (*it).newBone = newBone;
         return;
      }
   }

   SetJointT sj;
   sj.vertex   = vertex;
   sj.newBone  = newBone;
   sj.oldBone  = oldBone;
   m_list.push_back( sj );
}

MU_SetPointBoneJoint::MU_SetPointBoneJoint()
{
}

MU_SetPointBoneJoint::~MU_SetPointBoneJoint()
{
}

void MU_SetPointBoneJoint::undo( Model * model )
{
   log_debug( "undo set point bone joint\n" );
   SetJointList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setPointBoneJoint( (*it).point, (*it).oldBone );
   }
}

void MU_SetPointBoneJoint::redo( Model * model )
{
   SetJointList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setPointBoneJoint( (*it).point, (*it).newBone );
   }
}

bool MU_SetPointBoneJoint::combine( Undo * u )
{
   MU_SetPointBoneJoint * undo = dynamic_cast< MU_SetPointBoneJoint * >( u );

   if ( undo )
   {
      SetJointList::iterator it;

      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         setPointBoneJoint( (*it).point, (*it).newBone, (*it).oldBone );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set point bone joint\n" );
      return false;
   }
}

unsigned MU_SetPointBoneJoint::size()
{
   return sizeof(MU_SetPointBoneJoint) + m_list.size() * sizeof(SetJointT);
}

void MU_SetPointBoneJoint::setPointBoneJoint( const unsigned & point, 
      const int & newBone, const int & oldBone )
{
   SetJointList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).point == point )
      {
         (*it).newBone = newBone;
         return;
      }
   }

   SetJointT sj;
   sj.point    = point;
   sj.newBone  = newBone;
   sj.oldBone  = oldBone;
   m_list.push_back( sj );
}

MU_SetTriangleProjection::MU_SetTriangleProjection()
{
}

MU_SetTriangleProjection::~MU_SetTriangleProjection()
{
}

void MU_SetTriangleProjection::undo( Model * model )
{
   log_debug( "undo set triangle projection\n" );
   SetProjectionList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setTriangleProjection( (*it).triangle, (*it).oldProj );
   }
}

void MU_SetTriangleProjection::redo( Model * model )
{
   SetProjectionList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setTriangleProjection( (*it).triangle, (*it).newProj );
   }
}

bool MU_SetTriangleProjection::combine( Undo * u )
{
   MU_SetTriangleProjection * undo = dynamic_cast< MU_SetTriangleProjection * >( u );

   if ( undo )
   {
      SetProjectionList::iterator it;

      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         setTriangleProjection( (*it).triangle, (*it).newProj, (*it).oldProj );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set triangle projection\n" );
      return false;
   }
}

unsigned MU_SetTriangleProjection::size()
{
   return sizeof(MU_SetTriangleProjection) + m_list.size() * sizeof(SetProjectionT);
}

void MU_SetTriangleProjection::setTriangleProjection( const unsigned & triangle, 
      const int & newProj, const int & oldProj )
{
   SetProjectionList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).triangle == triangle )
      {
         (*it).newProj = newProj;
         return;
      }
   }

   SetProjectionT sj;
   sj.triangle = triangle;
   sj.newProj  = newProj;
   sj.oldProj  = oldProj;
   m_list.push_back( sj );
}

MU_AddAnimation::MU_AddAnimation()
   : m_skelAnim( NULL ),
     m_frameAnim( NULL )
{
}

MU_AddAnimation::~MU_AddAnimation()
{
}

void MU_AddAnimation::undo( Model * model )
{
   if ( m_skelAnim )
   {
      model->removeSkelAnim( m_anim );
      unsigned num = ( m_anim < model->getAnimCount( Model::ANIMMODE_SKELETAL ) ) ? m_anim : m_anim - 1;

      if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
      {
         model->setCurrentAnimation( Model::ANIMMODE_SKELETAL, num );
      }
   }
   if ( m_frameAnim )
   {
      model->removeFrameAnim( m_anim );
      unsigned num = ( m_anim < model->getAnimCount( Model::ANIMMODE_FRAME ) ) ? m_anim : m_anim - 1;

      if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
      {
         model->setCurrentAnimation( Model::ANIMMODE_FRAME, num );
      }
   }
}

void MU_AddAnimation::redo( Model * model )
{
   if ( m_skelAnim )
   {
      model->insertSkelAnim( m_anim, m_skelAnim );

      if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
      {
         model->setCurrentAnimation( Model::ANIMMODE_SKELETAL, m_anim );
      }
   }
   if ( m_frameAnim )
   {
      model->insertFrameAnim( m_anim, m_frameAnim );

      if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
      {
         model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      }
   }
}

bool MU_AddAnimation::combine( Undo * u )
{
   log_debug( "couldn't combine with add animation\n" );
   return false;
}

void MU_AddAnimation::redoRelease()
{
   log_debug("releasing animation in redo\n" );
   if ( m_skelAnim )
   {
      m_skelAnim->release();
   }
   if ( m_frameAnim )
   {
      m_frameAnim->release();
   }
}

unsigned MU_AddAnimation::size()
{
   unsigned frameAnimSize = (m_frameAnim) ? sizeof( m_frameAnim ) : 0;
   unsigned frame = 0;

   if ( m_frameAnim )
   {
      unsigned count = m_frameAnim->m_frameData.size();
      for ( frame = 0; frame < count; frame++ )
      {
         Model::FrameAnimVertexList * vertexList = m_frameAnim->m_frameData[ frame ]->m_frameVertices;
         frameAnimSize += sizeof( Model::FrameAnimVertexList ) + vertexList->size() * sizeof( Model::FrameAnimVertex );
         Model::FrameAnimPointList * pointList = m_frameAnim->m_frameData[ frame ]->m_framePoints;
         frameAnimSize += sizeof( Model::FrameAnimPointList ) + pointList->size() * sizeof( Model::FrameAnimPoint );
      }
   }

   unsigned skelAnimSize = (m_skelAnim) ? sizeof( m_skelAnim ) : 0;

   if ( m_skelAnim )
   {
      unsigned jointCount = m_skelAnim->m_jointKeyframes.size();
      for ( unsigned j = 0; j < jointCount; j++ )
      {
         Model::KeyframeList & list = m_skelAnim->m_jointKeyframes[j];
         skelAnimSize += sizeof( Model::KeyframeList ) + list.size() * sizeof( Model::Keyframe );
      }
      skelAnimSize += jointCount * sizeof( Model::JointKeyframeList );
   }

   return sizeof(MU_AddAnimation) + frameAnimSize + skelAnimSize;
}

void MU_AddAnimation::addAnimation( const unsigned & anim, Model::SkelAnim * skelanim )
{
   m_anim      = anim;
   m_skelAnim  = skelanim;
   m_frameAnim = NULL;
}

void MU_AddAnimation::addAnimation( const unsigned & anim, Model::FrameAnim * frameanim )
{
   m_anim      = anim;
   m_skelAnim  = NULL;
   m_frameAnim = frameanim;
}

MU_DeleteAnimation::MU_DeleteAnimation()
   : m_skelAnim( NULL ),
     m_frameAnim( NULL )
{
}

MU_DeleteAnimation::~MU_DeleteAnimation()
{
}

void MU_DeleteAnimation::undo( Model * model )
{
   if ( m_skelAnim )
   {
      model->insertSkelAnim( m_anim, m_skelAnim );
      if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
      {
         model->setCurrentAnimation( Model::ANIMMODE_SKELETAL, m_anim );
      }
   }
   if ( m_frameAnim )
   {
      model->insertFrameAnim( m_anim, m_frameAnim );
      if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
      {
         model->setCurrentAnimation( Model::ANIMMODE_FRAME, m_anim );
      }
   }
}

void MU_DeleteAnimation::redo( Model * model )
{
   if ( m_skelAnim )
   {
      model->removeSkelAnim( m_anim );
      unsigned num = ( m_anim < model->getAnimCount( Model::ANIMMODE_SKELETAL ) ) ? m_anim : m_anim - 1;
      if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
      {
         model->setCurrentAnimation( Model::ANIMMODE_SKELETAL, num );
      }
   }
   if ( m_frameAnim )
   {
      model->removeFrameAnim( m_anim );
      unsigned num = ( m_anim < model->getAnimCount( Model::ANIMMODE_FRAME ) ) ? m_anim : m_anim - 1;
      if ( model->getAnimationMode() != Model::ANIMMODE_NONE ) 
      {
         model->setCurrentAnimation( Model::ANIMMODE_FRAME, num );
      }
   }
}

bool MU_DeleteAnimation::combine( Undo * u )
{
   log_debug( "couldn't combine with delete animation\n" );
   return false;
}

void MU_DeleteAnimation::undoRelease()
{
   log_debug("releasing animation in undo\n" );
   if ( m_skelAnim )
   {
      m_skelAnim->release();
   }
   if ( m_frameAnim )
   {
      m_frameAnim->release();
   }
}

unsigned MU_DeleteAnimation::size()
{
   unsigned frameAnimSize = (m_frameAnim) ? sizeof( m_frameAnim ) : 0;
   unsigned frame = 0;

   if ( m_frameAnim )
   {
      unsigned count = m_frameAnim->m_frameData.size();
      for ( frame = 0; frame < count; frame++ )
      {
         Model::FrameAnimVertexList * vertexList = m_frameAnim->m_frameData[ frame ]->m_frameVertices;
         frameAnimSize += sizeof( Model::FrameAnimVertexList ) + vertexList->size() * sizeof( Model::FrameAnimVertex );
         Model::FrameAnimPointList * pointList = m_frameAnim->m_frameData[ frame ]->m_framePoints;
         frameAnimSize += sizeof( Model::FrameAnimPointList ) + pointList->size() * sizeof( Model::FrameAnimPoint );
      }
   }

   unsigned skelAnimSize = (m_skelAnim) ? sizeof( m_skelAnim ) : 0;

   if ( m_skelAnim )
   {
      unsigned jointCount = m_skelAnim->m_jointKeyframes.size();
      for ( unsigned j = 0; j < jointCount; j++ )
      {
         Model::KeyframeList & list = m_skelAnim->m_jointKeyframes[j];
         skelAnimSize += sizeof( Model::KeyframeList ) + list.size() * sizeof( Model::Keyframe );
      }
      skelAnimSize += jointCount * sizeof( Model::JointKeyframeList );
   }

   return sizeof(MU_DeleteAnimation) + frameAnimSize + skelAnimSize;
}

void MU_DeleteAnimation::deleteAnimation( const unsigned & anim, Model::SkelAnim * skelanim )
{
   m_anim      = anim;
   m_skelAnim  = skelanim;
   m_frameAnim = NULL;
}

void MU_DeleteAnimation::deleteAnimation( const unsigned & anim, Model::FrameAnim * frameanim )
{
   m_anim      = anim;
   m_skelAnim  = NULL;
   m_frameAnim = frameanim;
}

MU_SetJointParent::MU_SetJointParent()
{
}

MU_SetJointParent::~MU_SetJointParent()
{
}

void MU_SetJointParent::undo( Model * model )
{
   model->setBoneJointParent( m_joint, m_oldParent );
}

void MU_SetJointParent::redo( Model * model )
{
   model->setBoneJointParent( m_joint, m_newParent );
}

void MU_SetJointParent::setJointParent( unsigned joint, int newParent, int oldParent )
{
   m_joint = joint;
   m_newParent = newParent;
   m_oldParent = oldParent;
}

MU_SetJointRotation::MU_SetJointRotation()
{
}

MU_SetJointRotation::~MU_SetJointRotation()
{
}

void MU_SetJointRotation::undo( Model * model )
{
   model->setBoneJointRotation( m_joint, m_oldRotation );
}

void MU_SetJointRotation::redo( Model * model )
{
   model->setBoneJointRotation( m_joint, m_newRotation );
}

void MU_SetJointRotation::setJointRotation( const unsigned & joint, 
      const double *  newRotation, const double * oldRotation )
{
   m_joint = joint;
   if ( newRotation )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         m_newRotation[i] = newRotation[i];
      }
   }
   if ( oldRotation )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         m_oldRotation[i] = oldRotation[i];
      }
   }
}

MU_SetJointTranslation::MU_SetJointTranslation()
{
}

MU_SetJointTranslation::~MU_SetJointTranslation()
{
}

void MU_SetJointTranslation::undo( Model * model )
{
   model->setBoneJointTranslation( m_joint, m_oldTranslation );
}

void MU_SetJointTranslation::redo( Model * model )
{
   model->setBoneJointTranslation( m_joint, m_newTranslation );
}

void MU_SetJointTranslation::setJointTranslation( const unsigned & joint, 
      const double *  newTranslation, const double * oldTranslation )
{
   m_joint = joint;
   if ( newTranslation )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         m_newTranslation[i] = newTranslation[i];
      }
   }
   if ( oldTranslation )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         m_oldTranslation[i] = oldTranslation[i];
      }
   }
}

MU_SetProjectionUp::MU_SetProjectionUp()
{
}

MU_SetProjectionUp::~MU_SetProjectionUp()
{
}

void MU_SetProjectionUp::undo( Model * model )
{
   model->setProjectionUp( m_proj, m_oldUp );
}

void MU_SetProjectionUp::redo( Model * model )
{
   model->setProjectionUp( m_proj, m_newUp );
}

void MU_SetProjectionUp::setProjectionUp( const unsigned & proj, 
      const double *  newUp, const double * oldUp )
{
   m_proj = proj;
   if ( newUp )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         m_newUp[i] = newUp[i];
      }
   }
   if ( oldUp )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         m_oldUp[i] = oldUp[i];
      }
   }
}

MU_SetProjectionSeam::MU_SetProjectionSeam()
{
}

MU_SetProjectionSeam::~MU_SetProjectionSeam()
{
}

void MU_SetProjectionSeam::undo( Model * model )
{
   model->setProjectionSeam( m_proj, m_oldSeam );
}

void MU_SetProjectionSeam::redo( Model * model )
{
   model->setProjectionSeam( m_proj, m_newSeam );
}

void MU_SetProjectionSeam::setProjectionSeam( const unsigned & proj, 
      const double *  newSeam, const double * oldSeam )
{
   m_proj = proj;
   if ( newSeam )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         m_newSeam[i] = newSeam[i];
      }
   }
   if ( oldSeam )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         m_oldSeam[i] = oldSeam[i];
      }
   }
}

MU_SetProjectionRange::MU_SetProjectionRange()
{
}

MU_SetProjectionRange::~MU_SetProjectionRange()
{
}

void MU_SetProjectionRange::undo( Model * model )
{
   model->setProjectionRange( m_proj, 
         m_oldRange[0], m_oldRange[1], m_oldRange[2], m_oldRange[3]);
}

void MU_SetProjectionRange::redo( Model * model )
{
   model->setProjectionRange( m_proj, 
         m_newRange[0], m_newRange[1], m_newRange[2], m_newRange[3]);
}

void MU_SetProjectionRange::setProjectionRange( const unsigned & proj, 
      double newXMin, double newYMin, double newXMax, double newYMax,
      double oldXMin, double oldYMin, double oldXMax, double oldYMax )
{
   m_proj = proj;

   m_newRange[0] = newXMin;
   m_newRange[1] = newYMin;
   m_newRange[2] = newXMax;
   m_newRange[3] = newYMax;

   m_oldRange[0] = oldXMin;
   m_oldRange[1] = oldYMin;
   m_oldRange[2] = oldXMax;
   m_oldRange[3] = oldYMax;
}

MU_AddBoneJoint::MU_AddBoneJoint()
{
}

MU_AddBoneJoint::~MU_AddBoneJoint()
{
}

void MU_AddBoneJoint::undo( Model * model )
{
   model->removeBoneJoint( m_jointNum );
}

void MU_AddBoneJoint::redo( Model * model )
{
   model->insertBoneJoint( m_jointNum, m_joint );
}

bool MU_AddBoneJoint::combine( Undo * u )
{
   log_debug( "couldn't combine with add bone joint\n" );
   return false;
}

void MU_AddBoneJoint::redoRelease()
{
   m_joint->release();
}

unsigned MU_AddBoneJoint::size()
{
   return sizeof(MU_AddBoneJoint) + sizeof(Model::Joint);
}

void MU_AddBoneJoint::addBoneJoint( unsigned jointNum, Model::Joint * joint )
{
   m_jointNum = jointNum;
   m_joint    = joint;
}

MU_AddPoint::MU_AddPoint()
{
}

MU_AddPoint::~MU_AddPoint()
{
}

void MU_AddPoint::undo( Model * model )
{
   model->removePoint( m_pointNum );
}

void MU_AddPoint::redo( Model * model )
{
   model->insertPoint( m_pointNum, m_point );
}

bool MU_AddPoint::combine( Undo * u )
{
   log_debug( "couldn't combine with add point\n" );
   return false;
}

void MU_AddPoint::redoRelease()
{
   m_point->release();
}

unsigned MU_AddPoint::size()
{
   return sizeof(MU_AddPoint) + sizeof(Model::Point);
}

void MU_AddPoint::addPoint( unsigned pointNum, Model::Point * point )
{
   m_pointNum = pointNum;
   m_point    = point;
}

MU_AddProjection::MU_AddProjection()
{
}

MU_AddProjection::~MU_AddProjection()
{
}

void MU_AddProjection::undo( Model * model )
{
   model->removeProjection( m_projNum );
}

void MU_AddProjection::redo( Model * model )
{
   model->insertProjection( m_projNum, m_proj );
}

bool MU_AddProjection::combine( Undo * u )
{
   log_debug( "couldn't combine with add projection\n" );
   return false;
}

void MU_AddProjection::redoRelease()
{
   m_proj->release();
}

unsigned MU_AddProjection::size()
{
   return sizeof(MU_AddProjection) + sizeof(Model::TextureProjection);
}

void MU_AddProjection::addProjection( unsigned projNum, Model::TextureProjection * proj )
{
   m_projNum = projNum;
   m_proj    = proj;
}

MU_DeleteBoneJoint::MU_DeleteBoneJoint()
{
}

MU_DeleteBoneJoint::~MU_DeleteBoneJoint()
{
}

void MU_DeleteBoneJoint::undo( Model * model )
{
   log_debug( "undo delete joint\n" );
   model->insertBoneJoint( m_jointNum, m_joint );
}

void MU_DeleteBoneJoint::redo( Model * model )
{
   log_debug( "redo delete joint\n" );
   model->removeBoneJoint( m_jointNum );
}

bool MU_DeleteBoneJoint::combine( Undo * u )
{
   log_debug( "couldn't combine with delete bone joint\n" );
   return false;
}

void MU_DeleteBoneJoint::undoRelease()
{
   m_joint->release();
}

unsigned MU_DeleteBoneJoint::size()
{
   return sizeof(MU_DeleteBoneJoint) + sizeof(Model::Joint);
}

void MU_DeleteBoneJoint::deleteBoneJoint( unsigned jointNum, Model::Joint * joint )
{
   m_jointNum = jointNum;
   m_joint    = joint;
}

MU_DeletePoint::MU_DeletePoint()
{
}

MU_DeletePoint::~MU_DeletePoint()
{
}

void MU_DeletePoint::undo( Model * model )
{
   log_debug( "undo delete point\n" );
   model->insertPoint( m_pointNum, m_point );
}

void MU_DeletePoint::redo( Model * model )
{
   log_debug( "redo delete point\n" );
   model->removePoint( m_pointNum );
}

bool MU_DeletePoint::combine( Undo * u )
{
   log_debug( "couldn't combine with delete point\n" );
   return false;
}

void MU_DeletePoint::undoRelease()
{
   m_point->release();
}

unsigned MU_DeletePoint::size()
{
   return sizeof(MU_DeletePoint) + sizeof(Model::Point);
}

void MU_DeletePoint::deletePoint( unsigned pointNum, Model::Point * point )
{
   m_pointNum = pointNum;
   m_point    = point;
}

MU_DeleteProjection::MU_DeleteProjection()
{
}

MU_DeleteProjection::~MU_DeleteProjection()
{
}

void MU_DeleteProjection::undo( Model * model )
{
   log_debug( "undo delete proj\n" );
   model->insertProjection( m_projNum, m_proj );
}

void MU_DeleteProjection::redo( Model * model )
{
   log_debug( "redo delete proj\n" );
   model->removeProjection( m_projNum );
}

bool MU_DeleteProjection::combine( Undo * u )
{
   log_debug( "couldn't combine with delete projection\n" );
   return false;
}

void MU_DeleteProjection::undoRelease()
{
   m_proj->release();
}

unsigned MU_DeleteProjection::size()
{
   return sizeof(MU_DeleteProjection) + sizeof(Model::TextureProjection);
}

void MU_DeleteProjection::deleteProjection( unsigned projNum, Model::TextureProjection * proj )
{
   m_projNum = projNum;
   m_proj    = proj;
}

MU_SetGroupSmooth::MU_SetGroupSmooth()
{
}

MU_SetGroupSmooth::~MU_SetGroupSmooth()
{
}

void MU_SetGroupSmooth::undo( Model * model )
{
   SetSmoothList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setGroupSmooth( (*it).group, (*it).oldSmooth );
   }
}

void MU_SetGroupSmooth::redo( Model * model )
{
   SetSmoothList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setGroupSmooth( (*it).group, (*it).newSmooth );
   }
}

bool MU_SetGroupSmooth::combine( Undo * u )
{
   MU_SetGroupSmooth * undo = dynamic_cast< MU_SetGroupSmooth * >( u );

   if ( undo )
   {
      SetSmoothList::iterator it;

      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         setGroupSmooth( (*it).group, (*it).newSmooth, (*it).oldSmooth );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set smooth\n" );
      return false;
   }
}

unsigned MU_SetGroupSmooth::size()
{
   return sizeof(MU_SetGroupSmooth) + m_list.size() * sizeof(SetSmoothT);
}

void MU_SetGroupSmooth::setGroupSmooth( const unsigned & group, 
      const uint8_t & newSmooth, const uint8_t & oldSmooth )
{
   SetSmoothList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).group == group )
      {
         (*it).newSmooth = newSmooth;
         return;
      }
   }

   SetSmoothT ss;
   ss.group      = group;
   ss.newSmooth  = newSmooth;
   ss.oldSmooth  = oldSmooth;
   m_list.push_back( ss );
}

MU_SetGroupAngle::MU_SetGroupAngle()
{
}

MU_SetGroupAngle::~MU_SetGroupAngle()
{
}

void MU_SetGroupAngle::undo( Model * model )
{
   SetAngleList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setGroupAngle( (*it).group, (*it).oldAngle );
   }
}

void MU_SetGroupAngle::redo( Model * model )
{
   SetAngleList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->setGroupAngle( (*it).group, (*it).newAngle );
   }
}

bool MU_SetGroupAngle::combine( Undo * u )
{
   MU_SetGroupAngle * undo = dynamic_cast< MU_SetGroupAngle * >( u );

   if ( undo )
   {
      SetAngleList::iterator it;

      for ( it = undo->m_list.begin(); it != undo->m_list.end(); it++ )
      {
         setGroupAngle( (*it).group, (*it).newAngle, (*it).oldAngle );
      }

      return true;
   }
   else
   {
      log_debug( "couldn't combine with set smooth\n" );
      return false;
   }
}

unsigned MU_SetGroupAngle::size()
{
   return sizeof(MU_SetGroupAngle) + m_list.size() * sizeof(SetAngleT);
}

void MU_SetGroupAngle::setGroupAngle( const unsigned & group, 
      const uint8_t & newAngle, const uint8_t & oldAngle )
{
   SetAngleList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).group == group )
      {
         (*it).newAngle = newAngle;
         return;
      }
   }

   SetAngleT ss;
   ss.group     = group;
   ss.newAngle  = newAngle;
   ss.oldAngle  = oldAngle;
   m_list.push_back( ss );
}

MU_SetGroupName::MU_SetGroupName()
{
}

MU_SetGroupName::~MU_SetGroupName()
{
}

void MU_SetGroupName::undo( Model * model )
{
   model->setGroupName( m_groupNum, m_oldName.c_str() );
}

void MU_SetGroupName::redo( Model * model )
{
   model->setGroupName( m_groupNum, m_newName.c_str() );
}

bool MU_SetGroupName::combine( Undo * u )
{
   MU_SetGroupName * undo = dynamic_cast< MU_SetGroupName * >( u );

   if ( undo && undo->m_groupNum == m_groupNum )
   {
      m_newName = undo->m_newName;
      return true;
   }
   else
   {
      return false;
   }
}

unsigned MU_SetGroupName::size()
{
   return sizeof(MU_SetGroupName);
}

void MU_SetGroupName::setGroupName( unsigned groupNum, const char * newName, const char * oldName )
{
   m_groupNum   = groupNum;
   m_newName    = newName;
   m_oldName    = oldName;
}

MU_MoveAnimation::MU_MoveAnimation()
{
}

MU_MoveAnimation::~MU_MoveAnimation()
{
}

void MU_MoveAnimation::undo( Model * model )
{
   model->moveAnimation( m_mode, m_newIndex, m_oldIndex );
}

void MU_MoveAnimation::redo( Model * model )
{
   model->moveAnimation( m_mode, m_oldIndex, m_newIndex );
}

bool MU_MoveAnimation::combine( Undo * u )
{
   return false;
}

unsigned MU_MoveAnimation::size()
{
   return sizeof(MU_MoveAnimation);
}

void MU_MoveAnimation::moveAnimation( const Model::AnimationModeE & mode, 
      const unsigned & oldIndex, const unsigned & newIndex )
{
   m_mode     = mode;
   m_oldIndex = oldIndex;
   m_newIndex = newIndex;
}

MU_SetFrameAnimVertexCount::MU_SetFrameAnimVertexCount()
{
}

MU_SetFrameAnimVertexCount::~MU_SetFrameAnimVertexCount()
{
}

void MU_SetFrameAnimVertexCount::undo( Model * model )
{
   model->setFrameAnimVertexCount( m_oldCount );
}

void MU_SetFrameAnimVertexCount::redo( Model * model )
{
   model->setFrameAnimVertexCount( m_newCount );
}

unsigned MU_SetFrameAnimVertexCount::size()
{
   return sizeof(MU_SetFrameAnimVertexCount);
}

void MU_SetFrameAnimVertexCount::setCount( const unsigned & newCount, const unsigned & oldCount )
{
   m_newCount = newCount;
   m_oldCount = oldCount;
}

MU_SetFrameAnimPointCount::MU_SetFrameAnimPointCount()
{
}

MU_SetFrameAnimPointCount::~MU_SetFrameAnimPointCount()
{
}

void MU_SetFrameAnimPointCount::undo( Model * model )
{
   model->setFrameAnimPointCount( m_oldCount );
}

void MU_SetFrameAnimPointCount::redo( Model * model )
{
   model->setFrameAnimPointCount( m_newCount );
}

unsigned MU_SetFrameAnimPointCount::size()
{
   return sizeof(MU_SetFrameAnimPointCount);
}

void MU_SetFrameAnimPointCount::setCount( const unsigned & newCount, const unsigned & oldCount )
{
   m_newCount = newCount;
   m_oldCount = oldCount;
}

MU_SetMaterialClamp::MU_SetMaterialClamp()
{
}

MU_SetMaterialClamp::~MU_SetMaterialClamp()
{
}

void MU_SetMaterialClamp::undo( Model * model )
{
   if ( m_isS )
   {
      model->setTextureSClamp( m_material, m_oldClamp );
   }
   else
   {
      model->setTextureTClamp( m_material, m_oldClamp );
   }
}

void MU_SetMaterialClamp::redo( Model * model )
{
   if ( m_isS )
   {
      model->setTextureSClamp( m_material, m_newClamp );
   }
   else
   {
      model->setTextureTClamp( m_material, m_newClamp );
   }
}

bool MU_SetMaterialClamp::combine( Undo * u )
{
   log_debug( "couldn't combine with set material clamp\n" );
   return false;
}

unsigned MU_SetMaterialClamp::size()
{
   return sizeof(MU_SetMaterialClamp);
}

void MU_SetMaterialClamp::setMaterialClamp( const unsigned & material, const bool & isS,
      const bool & newClamp, const bool & oldClamp )
{
   m_material = material;
   m_isS = isS;
   m_newClamp = newClamp;
   m_oldClamp = oldClamp;
}

MU_SetMaterialTexture::MU_SetMaterialTexture()
{
}

MU_SetMaterialTexture::~MU_SetMaterialTexture()
{
}

void MU_SetMaterialTexture::undo( Model * model )
{
   if ( m_oldTexture )
   {
      model->setMaterialTexture( m_material, m_oldTexture );
   }
   else
   {
      model->removeMaterialTexture( m_material );
   }
}

void MU_SetMaterialTexture::redo( Model * model )
{
   if ( m_newTexture )
   {
      model->setMaterialTexture( m_material, m_newTexture );
   }
   else
   {
      model->removeMaterialTexture( m_material );
   }
}

bool MU_SetMaterialTexture::combine( Undo * u )
{
   log_debug( "couldn't combine with set material texture\n" );
   return false;
}

unsigned MU_SetMaterialTexture::size()
{
   return sizeof(MU_SetMaterialTexture);
}

void MU_SetMaterialTexture::setMaterialTexture( const unsigned & material,
      Texture * newTexture, Texture * oldTexture )
{
   m_material = material;
   m_newTexture = newTexture;
   m_oldTexture = oldTexture;
}

MU_SetBackgroundImage::MU_SetBackgroundImage()
{
}

MU_SetBackgroundImage::~MU_SetBackgroundImage()
{
}

void MU_SetBackgroundImage::undo( Model * model )
{
   model->setBackgroundImage( m_index, m_oldFilename.c_str() );
}

void MU_SetBackgroundImage::redo( Model * model )
{
   model->setBackgroundImage( m_index, m_newFilename.c_str() );
}

bool MU_SetBackgroundImage::combine( Undo * u )
{
   MU_SetBackgroundImage * undo = dynamic_cast< MU_SetBackgroundImage * >( u );

   if ( undo && undo->m_index == m_index )
   {
      m_newFilename = undo->m_newFilename;
      return true;
   }
   else
   {
      return false;
   }
}

unsigned MU_SetBackgroundImage::size()
{
   return sizeof(MU_SetBackgroundImage);
}

void MU_SetBackgroundImage::setBackgroundImage( const unsigned & index, 
      const char * newFilename, const char * oldFilename )
{
   m_index = index;
   m_newFilename    = newFilename;
   m_oldFilename    = oldFilename;
}

MU_SetBackgroundScale::MU_SetBackgroundScale()
{
}

MU_SetBackgroundScale::~MU_SetBackgroundScale()
{
}

void MU_SetBackgroundScale::undo( Model * model )
{
   model->setBackgroundScale( m_index, m_oldScale );
}

void MU_SetBackgroundScale::redo( Model * model )
{
   model->setBackgroundScale( m_index, m_newScale );
}

bool MU_SetBackgroundScale::combine( Undo * u )
{
   MU_SetBackgroundScale * undo = dynamic_cast< MU_SetBackgroundScale * >( u );

   if ( undo && undo->m_index == m_index )
   {
      m_newScale = undo->m_newScale;
      return true;
   }
   else
   {
      return false;
   }
}

unsigned MU_SetBackgroundScale::size()
{
   return sizeof(MU_SetBackgroundScale);
}

void MU_SetBackgroundScale::setBackgroundScale( const unsigned & index, 
      float newScale, float oldScale )
{
   m_index = index;
   m_newScale    = newScale;
   m_oldScale    = oldScale;
}

MU_SetBackgroundCenter::MU_SetBackgroundCenter()
{
}

MU_SetBackgroundCenter::~MU_SetBackgroundCenter()
{
}

void MU_SetBackgroundCenter::undo( Model * model )
{
   model->setBackgroundCenter( m_index, m_old[0], m_old[1], m_old[2] );
}

void MU_SetBackgroundCenter::redo( Model * model )
{
   model->setBackgroundCenter( m_index, m_new[0], m_new[1], m_new[2] );
}

bool MU_SetBackgroundCenter::combine( Undo * u )
{
   MU_SetBackgroundCenter * undo = dynamic_cast< MU_SetBackgroundCenter * >( u );

   if ( undo && undo->m_index == m_index )
   {
      m_new[0] = undo->m_new[0];
      m_new[1] = undo->m_new[1];
      m_new[2] = undo->m_new[2];
      return true;
   }
   else
   {
      return false;
   }
}

unsigned MU_SetBackgroundCenter::size()
{
   return sizeof(MU_SetBackgroundCenter);
}

void MU_SetBackgroundCenter::setBackgroundCenter( const unsigned & index, 
      float newX, float newY, float newZ,
      float oldX, float oldY, float oldZ )
{
   m_index = index;
   m_new[0] = newX;
   m_new[1] = newY;
   m_new[2] = newZ;
   m_old[0] = oldX;
   m_old[1] = oldY;
   m_old[2] = oldZ;
}

MU_AddMetaData::MU_AddMetaData()
{
}

MU_AddMetaData::~MU_AddMetaData()
{
}

void MU_AddMetaData::undo( Model * model )
{
   model->removeLastMetaData();
}

void MU_AddMetaData::redo( Model * model )
{
   model->addMetaData( m_key.c_str(), m_value.c_str() );
}

bool MU_AddMetaData::combine( Undo * u )
{
   return false;
}

unsigned MU_AddMetaData::size()
{
   return sizeof(MU_AddMetaData) + m_key.size() + m_value.size();
}

void MU_AddMetaData::addMetaData( const std::string & key,
      const std::string & value )
{
   m_key   = key;
   m_value = value;
}

MU_UpdateMetaData::MU_UpdateMetaData()
{
}

MU_UpdateMetaData::~MU_UpdateMetaData()
{
}

void MU_UpdateMetaData::undo( Model * model )
{
   model->updateMetaData( m_key.c_str(), m_oldValue.c_str() );
}

void MU_UpdateMetaData::redo( Model * model )
{
   model->updateMetaData( m_key.c_str(), m_newValue.c_str() );
}

bool MU_UpdateMetaData::combine( Undo * u )
{
   return false;
}

unsigned MU_UpdateMetaData::size()
{
   return sizeof(MU_UpdateMetaData) + m_key.size() + m_newValue.size() + m_oldValue.size();
}

void MU_UpdateMetaData::updateMetaData( const std::string & key,
      const std::string & newValue, const std::string & oldValue )
{
   m_key   = key;
   m_newValue = newValue;
   m_oldValue = oldValue;
}

MU_ClearMetaData::MU_ClearMetaData()
{
}

MU_ClearMetaData::~MU_ClearMetaData()
{
}

void MU_ClearMetaData::undo( Model * model )
{
   Model::MetaDataList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      model->addMetaData( (*it).key.c_str(), (*it).value.c_str() );
   }
}

void MU_ClearMetaData::redo( Model * model )
{
   model->clearMetaData();
}

bool MU_ClearMetaData::combine( Undo * u )
{
   return false;
}

unsigned MU_ClearMetaData::size()
{
   unsigned int s = sizeof( MU_ClearMetaData );
   s += m_list.size() * sizeof( Model::MetaData );
   Model::MetaDataList::iterator it;
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      s += (*it).key.size();
      s += (*it).value.size();
   }
   return s;
}

void MU_ClearMetaData::clearMetaData( const Model::MetaDataList & list )
{
   m_list = list;
}

