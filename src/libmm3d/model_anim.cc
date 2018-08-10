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


#include "model.h"

#ifdef MM3D_EDIT
#include "modelundo.h"
#endif // MM3D_EDIT

#include "log.h"
#include "glmath.h"
#include <math.h>

#ifdef MM3D_EDIT

int Model::addAnimation( AnimationModeE m, const char * name )
{
   LOG_PROFILE();

   int num = -1;
   if ( name )
   {
      switch ( m )
      {
         case ANIMMODE_SKELETAL:
            {
               num = m_skelAnims.size();

               SkelAnim * anim = SkelAnim::get();
               anim->m_name = name;
               anim->m_fps  = 30.0;
               anim->m_spf  = (1.0 / anim->m_fps);
               anim->m_loop = true;
               anim->m_frameCount = 1;
               anim->m_validNormals = false;

               for ( unsigned j = 0; j < m_joints.size(); j++ )
               {
                  anim->m_jointKeyframes.push_back( KeyframeList() );
               }

               MU_AddAnimation * undo = new MU_AddAnimation();
               undo->addAnimation( num, anim );
               sendUndo( undo );

               insertSkelAnim( num, anim );
            }

            break;
         case ANIMMODE_FRAME:
            {
               num = m_frameAnims.size();

               FrameAnim * anim = FrameAnim::get();
               anim->m_name = name;
               anim->m_fps = 10.0;
               anim->m_loop = true;
               anim->m_validNormals = false;

               MU_AddAnimation * undo = new MU_AddAnimation();
               undo->addAnimation( num, anim );
               sendUndo( undo );

               insertFrameAnim( num, anim );
            }

            break;
         default:
            break;
      }
   }
   return num;
}

void Model::deleteAnimation( AnimationModeE m, unsigned index )
{
   LOG_PROFILE();

   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( index < m_skelAnims.size() )
         {
            MU_DeleteAnimation * undo = new MU_DeleteAnimation;
            undo->deleteAnimation( index, m_skelAnims[index] );
            sendUndo( undo );

            removeSkelAnim( index );
         }
         break;
      case ANIMMODE_FRAME:
         if ( index < m_frameAnims.size() )
         {
            MU_DeleteAnimation * undo = new MU_DeleteAnimation;
            undo->deleteAnimation( index, m_frameAnims[index] );
            sendUndo( undo );

            removeFrameAnim( index );
         }
         break;
      default:
         break;
   }
}

bool Model::setAnimName( AnimationModeE m, unsigned anim, const char * name )
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            MU_SetAnimName * undo = new MU_SetAnimName();
            undo->setName( m, anim, name, m_skelAnims[anim]->m_name.c_str() );
            sendUndo( undo );

            m_skelAnims[ anim ]->m_name = name;
            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            MU_SetAnimName * undo = new MU_SetAnimName();
            undo->setName( m, anim, name, m_frameAnims[anim]->m_name.c_str() );
            sendUndo( undo );

            m_frameAnims[ anim ]->m_name = name;
            return true;
         }
         break;
      default:
         break;
   }
   return false;
}

bool Model::setAnimFrameCount( AnimationModeE m, unsigned anim, unsigned count )
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            SkelAnim * sa = m_skelAnims[anim];

            for ( unsigned j = 0; j < m_joints.size(); j++ )
            {
               KeyframeList & list = sa->m_jointKeyframes[j];
               unsigned k = 0;
               while ( k < list.size() )
               {
                  if ( list[k]->m_frame >= count )
                  {
                     deleteSkelAnimKeyframe( anim, list[k]->m_frame, list[k]->m_jointIndex, list[k]->m_isRotation );
                  }
                  else
                  {
                     k++;
                  }
               }
            }

            MU_SetAnimFrameCount * undo = new MU_SetAnimFrameCount();
            undo->setAnimFrameCount( m, anim, count, m_skelAnims[anim]->m_frameCount );
            sendUndo( undo );

            m_skelAnims[anim]->m_frameCount = count;

            if ( m_currentAnim == anim && m_currentFrame >= count )
            {
               setCurrentAnimationFrame( 0 );
            }
            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            FrameAnim * fa = m_frameAnims[anim];

            if ( count == fa->m_frameData.size() )
            {
               return true;
            }

            if ( count < fa->m_frameData.size() )
            {
               MU_DeleteFrameAnimFrame * undo = new MU_DeleteFrameAnimFrame;
               undo->setAnimationData( anim );

               while ( count < fa->m_frameData.size() )
               {
                  unsigned off = fa->m_frameData.size() - 1;
                  removeFrameAnimFrame( anim, off );
                  undo->deleteFrame( off, fa->m_frameData[off] );
               }

               sendUndo( undo );
            }
            if ( count > fa->m_frameData.size() )
            {
               MU_AddFrameAnimFrame * undo = new MU_AddFrameAnimFrame;
               undo->setAnimationData( anim );

               while ( count > fa->m_frameData.size() )
               {
                  fa->m_validNormals = false;
                  FrameAnimData * d = new FrameAnimData;
                  d->m_frameVertices = new FrameAnimVertexList;
                  d->m_framePoints   = new FrameAnimPointList;

                  undo->addFrame( fa->m_frameData.size(), d );
                  insertFrameAnimFrame( anim, fa->m_frameData.size(), d );

                  unsigned t;
                  for ( t = 0; t < m_vertices.size(); t++ )
                  {
                     FrameAnimVertex * fav = FrameAnimVertex::get();
                     for ( unsigned v = 0; v < 3; v++ )
                     {
                        fav->m_coord[v] = m_vertices[t]->m_coord[v];
                     }
                     d->m_frameVertices->push_back( fav );
                  }
                  for ( t = 0; t < m_points.size(); t++ )
                  {
                     FrameAnimPoint * fap = FrameAnimPoint::get();
                     for ( unsigned v = 0; v < 3; v++ )
                     {
                        fap->m_trans[v] = m_points[t]->m_trans[v];
                        fap->m_rot[v] = m_points[t]->m_rot[v];
                     }
                     d->m_framePoints->push_back( fap );
                  }
               }

               sendUndo( undo );
            }

            return true;
         }
         break;
      default:
         break;
   }

   return false;
}

bool Model::setAnimFPS( AnimationModeE m, unsigned anim, double fps )
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            MU_SetAnimFPS * undo = new MU_SetAnimFPS();
            undo->setFPS( m, anim, fps, m_skelAnims[anim]->m_fps );
            sendUndo( undo );

            m_skelAnims[anim]->m_fps = fps;
            m_skelAnims[anim]->m_spf = (1.0 / fps);

            Keyframe * kf;
            for ( unsigned j = 0; j < m_skelAnims[anim]->m_jointKeyframes.size(); j++ )
            {
               for ( unsigned f = 0; f < m_skelAnims[anim]->m_jointKeyframes[j].size(); f++ )
               {
                  kf = m_skelAnims[anim]->m_jointKeyframes[j][f];
                  kf->m_time = m_skelAnims[anim]->m_spf * kf->m_frame;
               }
            }
            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            MU_SetAnimFPS * undo = new MU_SetAnimFPS();
            undo->setFPS( m, anim, fps, m_frameAnims[anim]->m_fps );
            sendUndo( undo );

            m_frameAnims[anim]->m_fps = fps;
            return true;
         }
         break;
      default:
         break;
   }

   return false;
}

bool Model::setAnimLooping( AnimationModeE m, unsigned anim, bool loop )
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            MU_SetAnimLoop * undo = new MU_SetAnimLoop();
            undo->setAnimLoop( m, anim, loop, m_skelAnims[anim]->m_loop );
            sendUndo( undo );

            m_skelAnims[anim]->m_loop = loop;
            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            MU_SetAnimLoop * undo = new MU_SetAnimLoop();
            undo->setAnimLoop( m, anim, loop, m_frameAnims[anim]->m_loop );
            sendUndo( undo );

            m_frameAnims[anim]->m_loop = loop;
            return true;
         }
         break;
      default:
         break;
   }

   return false;
}

void Model::setFrameAnimPointCount( unsigned pointCount )
{
   unsigned anim = 0;
   unsigned frame = 0;

   unsigned oldCount = 0;

   unsigned acount = m_frameAnims.size();
   for ( anim = 0; anim < acount; anim++ )
   {
      unsigned fcount = m_frameAnims[ anim ]->m_frameData.size();
      oldCount = m_frameAnims[anim]->m_frameData[0]->m_framePoints->size();
      for ( frame = 0; frame < fcount; frame++ )
      {
         FrameAnimPoint * fap = NULL;
         unsigned pcount = m_frameAnims[anim]->m_frameData[frame]->m_framePoints->size();
         while ( pointCount > pcount )
         {
            fap = FrameAnimPoint::get();
            m_frameAnims[anim]->m_frameData[frame]->m_framePoints->push_back( fap );
            pcount++;
         }
         while ( pointCount < pcount )
         {
            m_frameAnims[anim]->m_frameData[frame]->m_framePoints->back()->release();
            m_frameAnims[anim]->m_frameData[frame]->m_framePoints->pop_back();
            pcount--;
         }
      }
   }

   MU_SetFrameAnimPointCount * undo = new MU_SetFrameAnimPointCount();
   undo->setCount( pointCount, oldCount );
   sendUndo( undo );
}

bool Model::setFrameAnimPointCoords( unsigned anim, unsigned frame, unsigned point,
      double x, double y, double z )
{
   if ( anim < m_frameAnims.size() 
         && frame < m_frameAnims[anim]->m_frameData.size()
         && point < m_points.size() )
   {
      FrameAnimPoint * fap = NULL;

      while ( point >= m_frameAnims[anim]->m_frameData[frame]->m_framePoints->size() )
      {
         fap = FrameAnimPoint::get();
         m_frameAnims[anim]->m_frameData[frame]->m_framePoints->push_back( fap );
      }

      fap = (*m_frameAnims[anim]->m_frameData[frame]->m_framePoints)[point];

      MU_MoveFramePoint * undo = new MU_MoveFramePoint;
      undo->setAnimationData( anim, frame );
      undo->addPoint( point, x, y, z, fap->m_trans[0], fap->m_trans[1], fap->m_trans[2] );
      sendUndo( undo );

      fap->m_trans[0] = x;
      fap->m_trans[1] = y;
      fap->m_trans[2] = z;

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setFrameAnimPointRotation( unsigned anim, unsigned frame, unsigned point,
      double x, double y, double z )
{
   if ( anim < m_frameAnims.size() 
         && frame < m_frameAnims[anim]->m_frameData.size()
         && point < m_points.size() )
   {
      FrameAnimPoint * fap = NULL;

      while ( point >= m_frameAnims[anim]->m_frameData[frame]->m_framePoints->size() )
      {
         fap = FrameAnimPoint::get();
         m_frameAnims[anim]->m_frameData[frame]->m_framePoints->push_back( fap );
      }

      fap = (*m_frameAnims[anim]->m_frameData[frame]->m_framePoints)[point];

      MU_RotateFramePoint * undo = new MU_RotateFramePoint;
      undo->setAnimationData( anim, frame );
      undo->addPointRotation( point, x, y, z, fap->m_rot[0], fap->m_rot[1], fap->m_rot[2] );
      sendUndo( undo );

      fap->m_rot[0] = x;
      fap->m_rot[1] = y;
      fap->m_rot[2] = z;

      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getFrameAnimPointCoords( unsigned anim, unsigned frame, unsigned point,
      double & x, double & y, double & z ) const
{
   if ( anim < m_frameAnims.size() 
         && frame < m_frameAnims[anim]->m_frameData.size()
         && point < m_frameAnims[anim]->m_frameData[frame]->m_framePoints->size() )
   {
      FrameAnimPoint * fap = (*m_frameAnims[anim]->m_frameData[frame]->m_framePoints)[point];
      x = fap->m_trans[0];
      y = fap->m_trans[1];
      z = fap->m_trans[2];
      return true;
   }
   else if ( point < m_points.size() )
   {
      x = m_points[point]->m_trans[0];
      y = m_points[point]->m_trans[1];
      z = m_points[point]->m_trans[2];
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getFrameAnimPointRotation( unsigned anim, unsigned frame, unsigned point,
      double & x, double & y, double & z ) const
{
   if ( anim < m_frameAnims.size() 
         && frame < m_frameAnims[anim]->m_frameData.size()
         && point < m_frameAnims[anim]->m_frameData[frame]->m_framePoints->size() )
   {
      FrameAnimPoint * fap = (*m_frameAnims[anim]->m_frameData[frame]->m_framePoints)[point];
      x = fap->m_rot[0];
      y = fap->m_rot[1];
      z = fap->m_rot[2];
      return true;
   }
   else if ( point < m_points.size() )
   {
      x = m_points[point]->m_rot[0];
      y = m_points[point]->m_rot[1];
      z = m_points[point]->m_rot[2];
      return true;
   }
   else
   {
      return false;
   }
}

void Model::setFrameAnimVertexCount( unsigned vertexCount )
{
   unsigned anim = 0;
   unsigned frame = 0;

   unsigned oldCount = 0;

   unsigned acount = m_frameAnims.size();
   for ( anim = 0; anim < acount; anim++ )
   {
      unsigned fcount = m_frameAnims[ anim ]->m_frameData.size();
      oldCount = m_frameAnims[anim]->m_frameData[0]->m_frameVertices->size();
      for ( frame = 0; frame < fcount; frame++ )
      {
         FrameAnimVertex * fav = NULL;
         unsigned vcount = m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->size();
         while ( vertexCount > vcount )
         {
            fav = FrameAnimVertex::get();
            m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->push_back( fav );
            vcount++;
         }
         while ( vertexCount < vcount )
         {
            m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->back()->release();
            m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->pop_back();
            vcount--;
         }
      }
   }

   MU_SetFrameAnimVertexCount * undo = new MU_SetFrameAnimVertexCount();
   undo->setCount( vertexCount, oldCount );
   sendUndo( undo );
}

bool Model::setFrameAnimVertexCoords( unsigned anim, unsigned frame, unsigned vertex,
      double x, double y, double z )
{
   if ( anim < m_frameAnims.size() 
         && frame < m_frameAnims[anim]->m_frameData.size()
         && vertex < m_vertices.size() )
   {
      FrameAnimVertex * fav = NULL;

      while ( vertex >= m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->size() )
      {
         int newVert = m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->size();
         fav = FrameAnimVertex::get();
         fav->m_coord[0] = m_vertices[newVert]->m_coord[0];
         fav->m_coord[1] = m_vertices[newVert]->m_coord[1];
         fav->m_coord[2] = m_vertices[newVert]->m_coord[2];
         m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->push_back( fav );
      }

      fav = (*m_frameAnims[anim]->m_frameData[frame]->m_frameVertices)[vertex];

      MU_MoveFrameVertex * undo = new MU_MoveFrameVertex;
      undo->setAnimationData( anim, frame );
      undo->addVertex( vertex, x, y, z, fav->m_coord[0], fav->m_coord[1], fav->m_coord[2] );
      sendUndo( undo );

      fav->m_coord[0] = x;
      fav->m_coord[1] = y;
      fav->m_coord[2] = z;

      m_frameAnims[anim]->m_validNormals = false;
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setQuickFrameAnimVertexCoords( unsigned anim, unsigned frame, unsigned vertex,
      double x, double y, double z )
{
   if ( anim < m_frameAnims.size() 
         && frame < m_frameAnims[anim]->m_frameData.size()
         && vertex < m_vertices.size() )
   {
      if ( vertex >= m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->size() )
      {
         log_warning( "resize the animation vertex list before calling setQuickFrameAnimVertexCoords\n" );
         setFrameAnimVertexCount( vertex );
      }

      FrameAnimVertex * fav = (*m_frameAnims[anim]->m_frameData[frame]->m_frameVertices)[vertex];

      fav->m_coord[0] = x;
      fav->m_coord[1] = y;
      fav->m_coord[2] = z;

      m_frameAnims[anim]->m_validNormals = false;
      return true;
   }
   else
   {
      return false;
   }
}

int Model::copyAnimation( AnimationModeE mode, unsigned anim, const char * newName )
{
   int num = -1;
   switch ( mode )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            num = addAnimation( mode, newName );
            if ( num >= 0 )
            {
               setAnimFrameCount( mode, num, getAnimFrameCount( mode, anim ) );
               setAnimFPS( mode, num, getAnimFPS( mode, anim ) );
               setAnimLooping( mode, num, getAnimLooping( mode, anim ) );

               SkelAnim * sa = m_skelAnims[anim];

               for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
               {
                  for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
                  {
                     Keyframe * kf = sa->m_jointKeyframes[j][k];

                     setSkelAnimKeyframe( num, kf->m_frame, j, kf->m_isRotation,
                           kf->m_parameter[0], kf->m_parameter[1], kf->m_parameter[2] );
                  }
               }
            }
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            num = addAnimation( mode, newName );
            if ( num >= 0 )
            {
               setAnimFrameCount( mode, num, getAnimFrameCount( mode, anim ) );
               setAnimFPS( mode, num, getAnimFPS( mode, anim ) );
               setAnimLooping( mode, num, getAnimLooping( mode, anim ) );

               FrameAnim * fa = m_frameAnims[anim];

               for ( unsigned f = 0; f < fa->m_frameData.size(); f++ )
               {
                  for ( unsigned v = 0; v < fa->m_frameData[f]->m_frameVertices->size(); v++ )
                  {
                     FrameAnimVertex * fav = (*fa->m_frameData[f]->m_frameVertices)[v];
                     setFrameAnimVertexCoords( num, f, v,
                           fav->m_coord[0], fav->m_coord[1], fav->m_coord[2] );
                  }
                  for ( unsigned p = 0; p < fa->m_frameData[f]->m_framePoints->size(); p++ )
                  {
                     FrameAnimPoint * fap = (*fa->m_frameData[f]->m_framePoints)[p];
                     setFrameAnimPointCoords( num, f, p,
                           fap->m_trans[0], fap->m_trans[1], fap->m_trans[2] );
                  }
               }
            }
         }
         break;
      case ANIMMODE_FRAMERELATIVE:
         break;
      default:
         break;
   }

   return num;
}

int Model::splitAnimation( AnimationModeE mode, unsigned anim, const char * newName, unsigned frame )
{
   int num = -1;
   switch ( mode )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            num = addAnimation( mode, newName );
            if ( num >= 0 )
            {
               setAnimFrameCount( mode, num, getAnimFrameCount( mode, anim ) - frame );
               setAnimFPS( mode, num, getAnimFPS( mode, anim ) );

               SkelAnim * sa = m_skelAnims[anim];

               for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
               {
                  for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
                  {
                     Keyframe * kf = sa->m_jointKeyframes[j][k];

                     if ( kf->m_frame >= frame )
                     {
                        setSkelAnimKeyframe( num, kf->m_frame - frame, j, kf->m_isRotation,
                              kf->m_parameter[0], kf->m_parameter[1], kf->m_parameter[2] );
                     }
                  }
               }

               setAnimFrameCount( mode, anim, frame );
               moveAnimation( mode, num, anim + 1 );
            }
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            num = addAnimation( mode, newName );
            if ( num >= 0 )
            {
               setAnimFrameCount( mode, num, getAnimFrameCount( mode, anim ) - frame );
               setAnimFPS( mode, num, getAnimFPS( mode, anim ) );

               FrameAnim * fa = m_frameAnims[anim];

               for ( unsigned f = frame; f < fa->m_frameData.size(); f++ )
               {
                  for ( unsigned v = 0; v < fa->m_frameData[f]->m_frameVertices->size(); v++ )
                  {
                     FrameAnimVertex * fav = (*fa->m_frameData[f]->m_frameVertices)[v];
                     setFrameAnimVertexCoords( num, f - frame, v,
                           fav->m_coord[0], fav->m_coord[1], fav->m_coord[2] );
                  }
                  for ( unsigned p = 0; p < fa->m_frameData[f]->m_framePoints->size(); p++ )
                  {
                     FrameAnimPoint * fap = (*fa->m_frameData[f]->m_framePoints)[p];
                     setFrameAnimPointCoords( num, f - frame, p,
                           fap->m_trans[0], fap->m_trans[1], fap->m_trans[2] );
                  }
               }

               setAnimFrameCount( mode, anim, frame );
               moveAnimation( mode, num, anim + 1 );
            }
         }
         break;
      case ANIMMODE_FRAMERELATIVE:
         break;
      default:
         break;
   }

   return num;
}

bool Model::joinAnimations( AnimationModeE mode, unsigned anim1, unsigned anim2 )
{
   log_debug( "join %d anim %d + %d\n", mode, anim1, anim2 );
   if ( anim1 == anim2 )
   {
      return true;
   }

   switch ( mode )
   {
      case ANIMMODE_SKELETAL:
         if ( anim1 < m_skelAnims.size() && anim2 < m_skelAnims.size() )
         {
            unsigned fc1 = getAnimFrameCount( mode, anim1 );
            unsigned fc2 = getAnimFrameCount( mode, anim2 );

            setAnimFrameCount( mode, anim1, fc1 + fc2 );

            SkelAnim * sa2 = m_skelAnims[anim2];

            for ( unsigned j = 0; j < sa2->m_jointKeyframes.size(); j++ )
            {
               for ( unsigned k = 0; k < sa2->m_jointKeyframes[j].size(); k++ )
               {
                  Keyframe * kf = sa2->m_jointKeyframes[j][k];

                  setSkelAnimKeyframe( anim1, kf->m_frame + fc1, j, kf->m_isRotation,
                        kf->m_parameter[0], kf->m_parameter[1], kf->m_parameter[2] );
               }
            }

            deleteAnimation( mode, anim2 );
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim1 < m_frameAnims.size() && anim2 < m_frameAnims.size() )
         {
            unsigned fc1 = getAnimFrameCount( mode, anim1 );
            unsigned fc2 = getAnimFrameCount( mode, anim2 );

            setAnimFrameCount( mode, anim1, fc1 + fc2 );

            FrameAnim * fa2 = m_frameAnims[anim2];

            for ( unsigned f = 0; f < fc2; f++ )
            {
               for ( unsigned v = 0; v < fa2->m_frameData[f]->m_frameVertices->size(); v++ )
               {
                  FrameAnimVertex * fav = (*fa2->m_frameData[f]->m_frameVertices)[v];
                  setFrameAnimVertexCoords( anim1, f + fc1, v,
                        fav->m_coord[0], fav->m_coord[1], fav->m_coord[2] );
               }
               for ( unsigned p = 0; p < fa2->m_frameData[f]->m_framePoints->size(); p++ )
               {
                  FrameAnimPoint * fap = (*fa2->m_frameData[f]->m_framePoints)[p];
                  setFrameAnimPointCoords( anim1, f + fc1, p,
                        fap->m_trans[0], fap->m_trans[1], fap->m_trans[2] );
               }
            }

            deleteAnimation( mode, anim2 );

            return true;
         }
         break;
      case ANIMMODE_FRAMERELATIVE:
         break;
      default:
         break;
   }


   return false;
}

bool Model::mergeAnimations( AnimationModeE mode, unsigned anim1, unsigned anim2 )
{
   log_debug( "merge %d anim %d + %d\n", mode, anim1, anim2 );
   if ( anim1 == anim2 )
   {
      return true;
   }

   switch ( mode )
   {
      case ANIMMODE_SKELETAL:
         if ( anim1 < m_skelAnims.size() && anim2 < m_skelAnims.size() )
         {
            unsigned fc1 = getAnimFrameCount( mode, anim1 );
            unsigned fc2 = getAnimFrameCount( mode, anim2 );

            if ( fc1 == fc2 )
            {
               SkelAnim * sa = m_skelAnims[anim2];

               for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
               {
                  for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
                  {
                     Keyframe * kf = sa->m_jointKeyframes[j][k];

                     setSkelAnimKeyframe( anim1, kf->m_frame, j, kf->m_isRotation,
                           kf->m_parameter[0], kf->m_parameter[1], kf->m_parameter[2] );
                  }
               }

               deleteAnimation( mode, anim2 );

               return true;
            }
         }
         break;
      default:
         break;
   }

   return false;
}

bool Model::moveAnimation( AnimationModeE mode, unsigned oldIndex, unsigned newIndex )
{
   if ( oldIndex == newIndex )
   {
      return true;
   }

   switch ( mode )
   {
      case ANIMMODE_SKELETAL:
         if ( oldIndex < m_skelAnims.size() && newIndex < m_skelAnims.size() )
         {
            vector<SkelAnim *>::iterator it = m_skelAnims.begin();
            unsigned t = 0;
            while ( t < oldIndex && it != m_skelAnims.end() )
            {
               t++;
               it++;
            }
            SkelAnim * sa = m_skelAnims[t];
            m_skelAnims.erase( it );
            t = 0;
            it = m_skelAnims.begin();
            while ( t < newIndex && it != m_skelAnims.end() )
            {
               t++;
               it++;
            }
            m_skelAnims.insert( it, sa );

            MU_MoveAnimation * undo = new MU_MoveAnimation();
            undo->moveAnimation( mode, oldIndex, newIndex );
            sendUndo( undo );
            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( oldIndex < m_frameAnims.size() && newIndex < m_frameAnims.size() )
         {
            vector<FrameAnim *>::iterator it = m_frameAnims.begin();
            unsigned t = 0;
            while ( t < oldIndex && it != m_frameAnims.end() )
            {
               t++;
               it++;
            }
            FrameAnim * fa = m_frameAnims[t];
            m_frameAnims.erase( it );
            t = 0;
            it = m_frameAnims.begin();
            while ( t < newIndex && it != m_frameAnims.end() )
            {
               t++;
               it++;
            }
            m_frameAnims.insert( it, fa );

            MU_MoveAnimation * undo = new MU_MoveAnimation();
            undo->moveAnimation( mode, oldIndex, newIndex );
            sendUndo( undo );
            return true;
         }
         break;
      default:
         break;
   }

   return false;
}

int Model::convertAnimToFrame( AnimationModeE mode, unsigned anim, const char * newName, unsigned frameCount )
{
   int num = -1;
   
   switch ( mode )
   {
      case ANIMMODE_SKELETAL:
         if ( newName && anim < m_skelAnims.size() )
         {
            num = addAnimation( ANIMMODE_FRAME, newName );
            if ( num >= 0 )
            {
               setAnimFrameCount( ANIMMODE_FRAME, num, frameCount );
               setAnimLooping( ANIMMODE_FRAME, num, getAnimLooping( ANIMMODE_SKELETAL, anim ) );

               if ( frameCount > 0 )
               {
                  double time = (m_skelAnims[anim]->m_frameCount * (1.0 / m_skelAnims[anim]->m_fps));
                  double fps  = (double) frameCount / time;
                  double spf  = time / (double) frameCount;

                  log_debug( "resampling %d frames at %.2f fps to %d frames at %.2f fps\n",
                        m_skelAnims[anim]->m_frameCount, m_skelAnims[anim]->m_fps, frameCount, fps );

                  setAnimFPS( ANIMMODE_FRAME, num, fps );

                  setCurrentAnimation( ANIMMODE_SKELETAL, anim );

                  for ( unsigned f = 0; f < frameCount; f++ )
                  {
                     double currentTime = spf * (double) f;
                     setCurrentAnimationTime( currentTime );

                     unsigned vcount = m_vertices.size();
                     for ( unsigned v = 0; v < vcount; v++ )
                     {
                        double coord[3];
                        getVertexCoords( v, coord );

                        setFrameAnimVertexCoords( num, f, v,
                              coord[0], coord[1], coord[2] );
                     }

                     unsigned pcount = m_points.size();
                     for ( unsigned p = 0; p < pcount; p++ )
                     {
                        double coord[3];
                        double rot[3];
                        getPointCoords( p, coord );
                        getPointOrientation( p, rot );

                        setFrameAnimPointCoords( num, f, p,
                              coord[0], coord[1], coord[2] );
                        setFrameAnimPointRotation( num, f, p,
                              rot[0], rot[1], rot[2] );
                     }
                  }

                  setNoAnimation();
               }
               else
               {
                  setAnimFPS( ANIMMODE_FRAME, num, 10.0 );
               }
            }
         }
         break;
      default:
         break;
   }

   return num;
}

bool Model::clearAnimFrame( AnimationModeE m, unsigned anim, unsigned frame )
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() && frame < m_skelAnims[anim]->m_frameCount )
         {
            for ( unsigned j = 0; j < m_skelAnims[anim]->m_jointKeyframes.size(); j++ )
            {
               KeyframeList & list = m_skelAnims[anim]->m_jointKeyframes[j];
               KeyframeList::iterator it = list.begin();
               while ( it != list.end() )
               {
                  if ( (*it)->m_frame == frame )
                  {
                     deleteSkelAnimKeyframe( anim, frame, j, (*it)->m_isRotation );
                     it = list.begin();
                  }
                  else
                  {
                     it++;
                  }
               }
            }

            if ( anim == m_currentAnim && frame == m_currentFrame )
            {
               setCurrentAnimationFrame( m_currentFrame );
            }

            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() && frame < m_frameAnims[anim]->m_frameData.size() )
         {
            FrameAnim * fa = m_frameAnims[anim];
            for ( unsigned t = 0; t < m_vertices.size(); t++ )
            {
               setFrameAnimVertexCoords( anim, frame, t, 
                     m_vertices[t]->m_coord[0], m_vertices[t]->m_coord[1], m_vertices[t]->m_coord[2] );
            }
            for ( unsigned p = 0; p < m_points.size(); p++ )
            {
               setFrameAnimPointCoords( anim, frame, p, 
                     m_points[p]->m_trans[0], m_points[p]->m_trans[1], m_points[p]->m_trans[2] );
            }
            fa->m_validNormals = false;
            return true;
         }
         break;
      default:
         break;
   }

   return false;
}

int Model::setSkelAnimKeyframe( unsigned anim, unsigned frame, unsigned joint, bool isRotation,
      double x, double y, double z )
{
   if ( anim < m_skelAnims.size() 
         && frame < m_skelAnims[anim]->m_frameCount
         && joint < m_joints.size() )
   {
      //log_debug( "set %s of %d (%f,%f,%f) at frame %d\n", 
      //      (isRotation ? "rotation" : "translation"), joint, x, y, z, frame );

      while ( joint >= m_skelAnims[anim]->m_jointKeyframes.size() )
      {
         KeyframeList kl;
         m_skelAnims[anim]->m_jointKeyframes.push_back( kl );
      }

      //int num = m_skelAnims[anim]->m_jointKeyframes[joint].size();
      Keyframe * kf = Keyframe::get();

      kf->m_frame        = frame;
      kf->m_isRotation   = isRotation;

      bool   isNew = false;
      double oldx  = 0.0;
      double oldy  = 0.0;
      double oldz  = 0.0;

      unsigned index = 0;
      if ( m_skelAnims[anim]->m_jointKeyframes[joint].find_sorted( kf, index ) )
      {
         isNew = false;

         kf->release();

         kf = m_skelAnims[anim]->m_jointKeyframes[joint][index];

         oldx = kf->m_parameter[0];
         oldy = kf->m_parameter[1];
         oldz = kf->m_parameter[2];

         kf->m_parameter[0] = x;
         kf->m_parameter[1] = y;
         kf->m_parameter[2] = z;
         kf->m_jointIndex   = joint;
         kf->m_time         = m_skelAnims[anim]->m_spf * frame;
      }
      else
      {
         isNew = true;

         kf->m_parameter[0] = x;
         kf->m_parameter[1] = y;
         kf->m_parameter[2] = z;
         kf->m_jointIndex   = joint;
         kf->m_time         = m_skelAnims[anim]->m_spf * frame;

         m_skelAnims[anim]->m_jointKeyframes[joint].insert_sorted( kf );

         // Do lookup to return proper index
         m_skelAnims[anim]->m_jointKeyframes[joint].find_sorted( kf, index );
      }

      MU_SetAnimKeyframe * undo = new MU_SetAnimKeyframe();
      undo->setAnimationData( anim, frame, isRotation );
      undo->addBoneJoint( joint, isNew, x, y, z, oldx, oldy, oldz );
      sendUndo( undo );

      return index;
   }
   else
   {
      log_error( "anim keyframe out of range: anim %d, frame %d, joint %d\n", anim, frame, joint );
      if ( anim < m_skelAnims.size() )
      {
         log_error( "max frame is %d, max joint is %d\n", m_skelAnims[anim]->m_frameCount, m_joints.size() );
      }
      return -1;
   }
}

bool Model::deleteSkelAnimKeyframe( unsigned anim, unsigned frame, unsigned joint, bool isRotation )
{
   if ( anim < m_skelAnims.size() && frame < m_skelAnims[anim]->m_frameCount )
   {
      KeyframeList & list = m_skelAnims[anim]->m_jointKeyframes[joint];
      KeyframeList::iterator it = list.begin();
      while ( it != list.end() )
      {
         if ( (*it)->m_frame == frame && (*it)->m_isRotation == isRotation )
         {
            log_debug( "deleting keyframe for anim %d frame %d joint %d\n", anim, frame, joint, isRotation ? "rotation" : "translation" );
            MU_DeleteKeyframe * undo = new MU_DeleteKeyframe;
            undo->setAnimationData( anim );
            undo->deleteKeyframe( *it );
            sendUndo( undo );
            break;
         }
         it++;
      }
   }

   bool rval = removeSkelAnimKeyframe( anim, frame, joint, isRotation );

   if ( anim == m_currentAnim && frame == m_currentFrame )
   {
      setCurrentAnimationFrame( m_currentFrame );
   }

   return rval;
}

bool Model::insertSkelAnimKeyframe( unsigned anim, Keyframe * keyframe )
{
   if ( anim < m_skelAnims.size() 
         && keyframe->m_frame < m_skelAnims[anim]->m_frameCount 
         && keyframe->m_jointIndex < (signed int) m_joints.size() )
   {
      log_debug( "inserted keyframe for anim %d frame %d joint %d\n", anim, keyframe->m_frame, keyframe->m_jointIndex );
      m_skelAnims[anim]->m_jointKeyframes[keyframe->m_jointIndex].insert_sorted( keyframe );
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::removeSkelAnimKeyframe( unsigned anim, unsigned frame, unsigned joint, bool isRotation, bool release )
{
   if ( anim < m_skelAnims.size() && frame < m_skelAnims[anim]->m_frameCount )
   {
      KeyframeList & list = m_skelAnims[anim]->m_jointKeyframes[joint];
      KeyframeList::iterator it = list.begin();
      while ( it != list.end() )
      {
         if ( (*it)->m_frame == frame && (*it)->m_isRotation == isRotation )
         {
            Keyframe * kf = *it;
            list.erase( it );
            if ( release )
            {
               kf->release();
            }
            break;
         }
         it++;
      }

      return true;
   }
   else
   {
      return false;
   }
}

void Model::insertFrameAnim( unsigned index, FrameAnim * anim )
{
   LOG_PROFILE();

   if ( index == m_frameAnims.size() )
   {
      m_frameAnims.push_back( anim );
   }
   else if ( index < m_frameAnims.size() )
   {
      unsigned count = 0;
      vector<FrameAnim *>::iterator it;
      for ( it = m_frameAnims.begin(); it != m_frameAnims.end(); it++ )
      {
         if ( count == index )
         {
            m_frameAnims.insert( it, anim );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "index %d/%d out of range in insertFrameAnim\n", index, m_frameAnims.size() );
   }
}

void Model::removeFrameAnim( unsigned index )
{
   LOG_PROFILE();

   if ( index < m_frameAnims.size() )
   {
      unsigned num = 0;
      vector<FrameAnim *>::iterator it = m_frameAnims.begin();
      while ( num < index )
      {
         num++;
         it++;
      }

      m_frameAnims.erase( it );

      if ( m_frameAnims.size() > 0 )
      {
         while ( m_animationMode == ANIMMODE_FRAME && m_currentAnim >= m_frameAnims.size() )
         {
            m_currentAnim--;
         }
      }
   }
}

void Model::insertSkelAnim( unsigned index, SkelAnim * anim )
{
   LOG_PROFILE();

   if ( index == m_skelAnims.size() )
   {
      m_skelAnims.push_back( anim );
   }
   else if ( index < m_skelAnims.size() )
   {
      unsigned count = 0;
      vector<SkelAnim *>::iterator it;
      for ( it = m_skelAnims.begin(); it != m_skelAnims.end(); it++ )
      {
         if ( count == index )
         {
            m_skelAnims.insert( it, anim );
            break;
         }
         count++;
      }
   }
   else
   {
      log_error( "index %d/%d out of range in insertSkelAnim\n", index, m_skelAnims.size() );
   }
}

void Model::removeSkelAnim( unsigned index )
{
   LOG_PROFILE();

   if ( index < m_skelAnims.size() )
   {
      unsigned num = 0;
      vector<SkelAnim *>::iterator it = m_skelAnims.begin();
      while ( num < index )
      {
         num++;
         it++;
      }

      m_skelAnims.erase( it );

      if ( m_skelAnims.size() > 0 )
      {
         while ( m_animationMode == ANIMMODE_SKELETAL && m_currentAnim >= m_skelAnims.size() )
         {
            m_currentAnim--;
         }
      }
   }
}

void Model::insertFrameAnimFrame( unsigned anim, unsigned frame, 
      Model::FrameAnimData * data )
{
   if ( anim < m_frameAnims.size() )
   {
      FrameAnim * fa = m_frameAnims[anim];
      if ( frame == fa->m_frameData.size() )
      {
         fa->m_frameData.push_back( data );
      }
      else if ( frame < fa->m_frameData.size() )
      {
         unsigned count = 0;
         vector<FrameAnimData *>::iterator it;
         for ( it = fa->m_frameData.begin(); it != fa->m_frameData.end(); it++ )
         {
            if ( count == frame )
            {
               fa->m_frameData.insert( it, data );
               break;
            }
            count++;
         }
      }
      else
      {
         log_error( "frame %d/%d out of range in insertFrameAnimFrame for frame %d\n", frame, fa->m_frameData.size(), anim );
      }
   }
}

void Model::removeFrameAnimFrame( unsigned anim, unsigned frame )
{
   if ( anim < m_frameAnims.size() && m_frameAnims[anim]->m_frameData.size() )
   {
      FrameAnim * fa = m_frameAnims[anim];
      unsigned count = 0;
      vector<FrameAnimData *>::iterator it;
      for ( it = fa->m_frameData.begin(); it != fa->m_frameData.end(); it++ )
      {
         if ( count == frame )
         {
            fa->m_frameData.erase( it );
            break;
         }
         count++;
      }
   }
}

#endif // MM3D_EDIT

bool Model::setCurrentAnimation( AnimationModeE m, const char * name )
{
#ifdef MM3D_EDIT
   bool needUndo = m_animationMode ? false : true;
#endif // MM3D_EDIT

   AnimationModeE oldMode = m_animationMode;
   m_animationMode = ANIMMODE_NONE;

   log_debug( "Changing animation from %d to %d\n", oldMode, m );

   m_changeBits |= AnimationSet;
   if ( m != oldMode )
   {
      m_changeBits |= AnimationMode;
   }

   unsigned t;
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         for ( t = 0; t < m_skelAnims.size(); t++ )
         {
            if ( m_skelAnims[t]->m_name == name )
            {
               if ( ! m_skelAnims[t]->m_validNormals )
               {
                  calculateSkelNormals();
               }

               m_currentAnim = t;
               m_animationMode = m;
               m_currentFrame = 0;
               m_currentTime = 0;

#ifdef MM3D_EDIT
               if ( needUndo )
               {
                  MU_ChangeAnimState * undo = new MU_ChangeAnimState();
                  undo->setState( m_animationMode, ANIMMODE_NONE, m_currentAnim, m_currentFrame );
                  sendUndo( undo );
               }
               updateObservers();
#endif // MM3D_EDIT

               log_debug( "current animation: skeletal '%s'\n", name );

               unsigned vcount = m_vertices.size();
               for ( unsigned v = 0; v < vcount; v++ )
               {
                  m_vertices[ v ]->m_drawSource = m_vertices[v]->m_kfCoord;
               }

               unsigned pcount = m_points.size();
               for ( unsigned p = 0; p < pcount; p++ )
               {
                  m_points[ p ]->m_drawSource = m_points[p]->m_kfTrans;
                  m_points[ p ]->m_rotSource  = m_points[p]->m_kfRot;
               }

               unsigned tcount = m_triangles.size();
               for ( unsigned t = 0; t < tcount; t++ )
               {
                  m_triangles[t]->m_flatSource = m_triangles[t]->m_kfFlatNormals;
                  m_triangles[t]->m_normalSource[0] = m_triangles[t]->m_kfNormals[0];
                  m_triangles[t]->m_normalSource[1] = m_triangles[t]->m_kfNormals[1];
                  m_triangles[t]->m_normalSource[2] = m_triangles[t]->m_kfNormals[2];
               }

               return true;
            }
         }
         break;
      case ANIMMODE_FRAME:
         for ( t = 0; t < m_frameAnims.size(); t++ )
         {
            if ( m_frameAnims[t]->m_name == name )
            {
               if ( ! m_frameAnims[t]->m_validNormals )
               {
                  calculateFrameNormals( t );
               }

               m_currentAnim = t;
               m_animationMode = m;
               m_currentFrame = 0;
               m_currentTime = 0;

#ifdef MM3D_EDIT
               if ( needUndo )
               {
                  MU_ChangeAnimState * undo = new MU_ChangeAnimState();
                  undo->setState( m_animationMode, ANIMMODE_NONE, m_currentAnim, m_currentFrame );
                  sendUndo( undo );
               }
               updateObservers();
#endif // MM3D_EDIT

               log_debug( "current animation: frame '%s'\n", name );

               return true;
            }
         }
         break;
      default:
         break;
   }

   if ( m != ANIMMODE_SKELETAL )
   {
      unsigned vcount = m_vertices.size();
      for ( unsigned v = 0; v < vcount; v++ )
      {
         m_vertices[ v ]->m_drawSource = m_vertices[v]->m_coord;
      }

      unsigned pcount = m_points.size();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         m_points[ p ]->m_drawSource = m_points[p]->m_trans;
         m_points[ p ]->m_rotSource  = m_points[p]->m_rot;
      }

      unsigned tcount = m_triangles.size();
      for ( unsigned t = 0; t < tcount; t++ )
      {
         m_triangles[t]->m_flatSource = m_triangles[t]->m_flatNormals;
         m_triangles[t]->m_normalSource[0] = m_triangles[t]->m_finalNormals[0];
         m_triangles[t]->m_normalSource[1] = m_triangles[t]->m_finalNormals[1];
         m_triangles[t]->m_normalSource[2] = m_triangles[t]->m_finalNormals[2];
      }
   }

   m_animationMode = m;
   m_currentAnim   = 0;
   m_currentFrame  = 0;
#ifdef MM3D_EDIT
   if ( needUndo )
   {
      log_debug( "sending anim state undo\n" );
      MU_ChangeAnimState * undo = new MU_ChangeAnimState();
      undo->setState( m_animationMode, ANIMMODE_NONE, m_currentAnim, m_currentFrame );
      sendUndo( undo );
   }
   updateObservers();
#endif // MM3D_EDIT

   return false;
}

bool Model::setCurrentAnimation( AnimationModeE m, unsigned index )
{
#ifdef MM3D_EDIT
   bool needUndo = (m_animationMode) ? false : true;
#endif // MM3D_EDIT

   AnimationModeE oldMode = m_animationMode;
   m_animationMode = ANIMMODE_NONE;

   m_changeBits |= AnimationSet;
   if ( m != oldMode )
   {
      m_changeBits |= AnimationMode;
   }

   log_debug( "Changing animation from %d to %d\n", oldMode, m );

   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( index < m_skelAnims.size() )
         {
            if ( ! m_skelAnims[index]->m_validNormals )
            {
               calculateSkelNormals();
            }

            m_currentAnim = index;
            m_animationMode = m;
            m_currentFrame = 0;
            m_currentTime = 0;

#ifdef MM3D_EDIT
            if ( needUndo )
            {
               MU_ChangeAnimState * undo = new MU_ChangeAnimState();
               undo->setState( m_animationMode, ANIMMODE_NONE, m_currentAnim, m_currentFrame );
               sendUndo( undo );
            }
            updateObservers();
#endif // MM3D_EDIT

            log_debug( "current animation: skeletal '%s'\n", m_skelAnims[index]->m_name.c_str() );

            unsigned vcount = m_vertices.size();
            for ( unsigned v = 0; v < vcount; v++ )
            {
               m_vertices[ v ]->m_drawSource = m_vertices[v]->m_kfCoord;
            }

            unsigned pcount = m_points.size();
            for ( unsigned p = 0; p < pcount; p++ )
            {
               m_points[ p ]->m_drawSource = m_points[p]->m_kfTrans;
               m_points[ p ]->m_rotSource  = m_points[p]->m_kfRot;
            }

            unsigned tcount = m_triangles.size();
            for ( unsigned t = 0; t < tcount; t++ )
            {
               m_triangles[t]->m_flatSource = m_triangles[t]->m_kfFlatNormals;
               m_triangles[t]->m_normalSource[0] = m_triangles[t]->m_kfNormals[0];
               m_triangles[t]->m_normalSource[1] = m_triangles[t]->m_kfNormals[1];
               m_triangles[t]->m_normalSource[2] = m_triangles[t]->m_kfNormals[2];
            }

            return true;
         }
         break;
      case ANIMMODE_FRAME:
         if ( index < m_frameAnims.size() )
         {
            if ( ! m_frameAnims[index]->m_validNormals )
            {
               calculateFrameNormals( index );
            }

            m_currentAnim = index;
            m_animationMode = m;
            m_currentFrame = 0;
            m_currentTime = 0;

#ifdef MM3D_EDIT
            if ( needUndo )
            {
               MU_ChangeAnimState * undo = new MU_ChangeAnimState();
               undo->setState( m_animationMode, ANIMMODE_NONE, m_currentAnim, m_currentFrame );
               sendUndo( undo );
            }
            updateObservers();
#endif // MM3D_EDIT

            log_debug( "current animation: frame '%s'\n", m_frameAnims[index]->m_name.c_str() );

            return true;
         }
         break;
      default:
         break;
   }

   if ( m != ANIMMODE_SKELETAL )
   {
      unsigned vcount = m_vertices.size();
      for ( unsigned v = 0; v < vcount; v++ )
      {
         m_vertices[ v ]->m_drawSource = m_vertices[v]->m_coord;
      }

      unsigned pcount = m_points.size();
      for ( unsigned p = 0; p < pcount; p++ )
      {
         m_points[ p ]->m_drawSource = m_points[p]->m_trans;
         m_points[ p ]->m_rotSource  = m_points[p]->m_rot;
      }

      unsigned tcount = m_triangles.size();
      for ( unsigned t = 0; t < tcount; t++ )
      {
         m_triangles[t]->m_flatSource = m_triangles[t]->m_flatNormals;
         m_triangles[t]->m_normalSource[0] = m_triangles[t]->m_finalNormals[0];
         m_triangles[t]->m_normalSource[1] = m_triangles[t]->m_finalNormals[1];
         m_triangles[t]->m_normalSource[2] = m_triangles[t]->m_finalNormals[2];
      }
   }

   m_animationMode = m;
   m_currentAnim   = 0;
   m_currentFrame  = 0;
#ifdef MM3D_EDIT
   if ( needUndo )
   {
      log_debug( "sending anim state undo\n" );
      MU_ChangeAnimState * undo = new MU_ChangeAnimState();
      undo->setState( m_animationMode, ANIMMODE_NONE, m_currentAnim, m_currentFrame );
      sendUndo( undo );
   }
   updateObservers();
#endif // MM3D_EDIT

   return false;
}

bool Model::setCurrentAnimationFrame( unsigned frame )
{
   m_changeBits |= AnimationFrame;

   if ( m_animationMode == ANIMMODE_FRAME 
         && m_currentAnim < m_frameAnims.size() 
         && frame < m_frameAnims[m_currentAnim]->m_frameData.size() )
   {
      if ( !m_frameAnims[m_currentAnim]->m_validNormals )
      {
         calculateFrameNormals( m_currentAnim );
      }

      m_currentFrame = frame;

      updateObservers();
      return true;
   }
   else if ( m_animationMode == ANIMMODE_SKELETAL
         && m_currentAnim < m_skelAnims.size()
         && frame < m_skelAnims[m_currentAnim]->m_frameCount )
   {
      m_currentFrame = frame;
      setCurrentAnimationTime( frame * m_skelAnims[m_currentAnim]->m_spf );
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::setCurrentAnimationTime( double frameTime )
{
   m_currentTime = 0;

   m_changeBits |= AnimationFrame;

   if ( m_animationMode == ANIMMODE_FRAME && m_currentAnim < m_frameAnims.size() )
   {
      if ( !m_frameAnims[m_currentAnim]->m_validNormals )
      {
         calculateFrameNormals( m_currentAnim );
      }

      size_t totalFrames = m_frameAnims[m_currentAnim]->m_frameData.size();

      if ( totalFrames > 0 )
      {
         double spf = 1.0 / m_frameAnims[m_currentAnim]->m_fps;
         double totalTime = spf * m_frameAnims[m_currentAnim]->m_frameData.size();
         while ( frameTime >= totalTime )
         {
            if ( !m_frameAnims[m_currentAnim]->m_loop )
            {
               return false;
            }
            frameTime -= totalTime;
         }
         m_currentFrame = (unsigned) (frameTime / spf);
      }
      else
      {
         frameTime = 0.0;
         m_currentFrame = 0;
      }

      m_currentTime = frameTime;

      updateObservers();
      return true;
   }
   else if ( m_animationMode == ANIMMODE_SKELETAL && m_currentAnim < m_skelAnims.size() && m_skelAnims[m_currentAnim]->m_frameCount > 0 )
   {
      LOG_PROFILE();

      SkelAnim * sa = m_skelAnims[m_currentAnim];
      if ( sa->m_frameCount > 0 )
      {
         double totalTime = sa->m_spf * sa->m_frameCount;
         while ( frameTime > totalTime )
         {
            if ( !sa->m_loop )
            {
               return false;
            }
            frameTime -= totalTime;
         }
      }
      else
      {
         m_currentTime = 0.0;
      }

      m_currentTime = frameTime;

      for ( unsigned j = 0; j < sa->m_jointKeyframes.size(); j++ )
      {
         Matrix transform;
         interpSkelAnimKeyframeTime( m_currentAnim, frameTime,
               sa->m_loop, j, transform );

         Matrix relativeFinal( m_joints[j]->m_relative );
         relativeFinal = transform * relativeFinal;

         if ( m_joints[j]->m_parent == -1 )
         {
            relativeFinal = relativeFinal * m_localMatrix;
            m_joints[j]->m_final = relativeFinal;
         }
         else
         {
            m_joints[j]->m_final = m_joints[ m_joints[j]->m_parent ]->m_final;
            m_joints[j]->m_final = relativeFinal * m_joints[j]->m_final;
         }
      }

      for ( unsigned v = 0; v < m_vertices.size(); v++ )
      {
         Vertex * vptr = m_vertices[v];
         if ( !vptr->m_influences.empty() )
         {
            vptr->m_kfCoord[0] = 0.0;
            vptr->m_kfCoord[1] = 0.0;
            vptr->m_kfCoord[2] = 0.0;

            double total = 0.0;

            Vector vert;

            InfluenceList::iterator it;
            for ( it = vptr->m_influences.begin(); it != vptr->m_influences.end(); it++ )
            {
               if ( it->m_weight > 0.00001 )
               {
                  const Matrix & final = m_joints[ (*it).m_boneId ]->m_final;
                  const Matrix & abs = m_joints[ (*it).m_boneId ]->m_absolute;

                  vert.setAll( vptr->m_coord );
                  vert[3] = 1.0;

                  abs.inverseTranslateVector( (double *) vert.getVector() );
                  abs.inverseRotateVector( (double *) vert.getVector() );

                  vert.transform( final );

                  vert.scale3( (*it).m_weight );
                  vptr->m_kfCoord[0] += vert[0];
                  vptr->m_kfCoord[1] += vert[1];
                  vptr->m_kfCoord[2] += vert[2];

                  total += (*it).m_weight;
               }
            }

            if ( total > 0.00001 )
            {
               vptr->m_kfCoord[0] /= total;
               vptr->m_kfCoord[1] /= total;
               vptr->m_kfCoord[2] /= total;
            }
            else
            {
               vptr->m_kfCoord[0] = vptr->m_coord[0];
               vptr->m_kfCoord[1] = vptr->m_coord[1];
               vptr->m_kfCoord[2] = vptr->m_coord[2];
            }
         }
         else
         {
            vptr->m_kfCoord[0] = vptr->m_coord[0];
            vptr->m_kfCoord[1] = vptr->m_coord[1];
            vptr->m_kfCoord[2] = vptr->m_coord[2];
         }
      }

      for ( unsigned p = 0; p < m_points.size(); p++ )
      {
         Point * pptr = m_points[p];

         if ( !pptr->m_influences.empty() )
         {
            pptr->m_kfTrans[0] = 0;
            pptr->m_kfTrans[1] = 0;
            pptr->m_kfTrans[2] = 0;

            double axisX[3] = { 0, 0, 0 };
            double axisY[3] = { 0, 0, 0 };
            double axisZ[3] = { 0, 0, 0 };

            Vector ax;
            Vector ay;
            Vector az;

            double total = 0.0;

            Vector vert;
            Vector rot;

            InfluenceList::iterator it;
            for ( it = pptr->m_influences.begin(); it != pptr->m_influences.end(); it++ )
            {
               const Matrix & final = m_joints[ (*it).m_boneId ]->m_final;
               const Matrix & inv = m_joints[ (*it).m_boneId ]->m_absolute.getInverse();

               Matrix mat;

               mat.setTranslation( pptr->m_trans );
               mat.setRotation( pptr->m_rot );

               mat = mat * inv;

               mat = mat * final;

               vert[ 0 ] = mat.get( 3, 0 );
               vert[ 1 ] = mat.get( 3, 1 );
               vert[ 2 ] = mat.get( 3, 2 );

               for ( int i = 0; i < 3; i++ )
               {
                  ax[i] = mat.get( 0, i );
                  ay[i] = mat.get( 1, i );
                  az[i] = mat.get( 2, i );
               }

               vert.scale3( (*it).m_weight );

               ax.scale3( (*it).m_weight );
               ay.scale3( (*it).m_weight );
               az.scale3( (*it).m_weight );

               pptr->m_kfTrans[0] += vert[0];
               pptr->m_kfTrans[1] += vert[1];
               pptr->m_kfTrans[2] += vert[2];

               for ( int j = 0; j < 3; j++ )
               {
                  axisX[j] += ax[j];
                  axisY[j] += ay[j];
                  axisZ[j] += az[j];
               }

               total += (*it).m_weight;
            }

            if ( total > 0.0 )
            {
               pptr->m_kfTrans[0] /= total;
               pptr->m_kfTrans[1] /= total;
               pptr->m_kfTrans[2] /= total;
            }
            else
            {
               pptr->m_kfTrans[0] = 0.0;
               pptr->m_kfTrans[1] = 0.0;
               pptr->m_kfTrans[2] = 0.0;
               pptr->m_kfRot[0] = 0.0;
               pptr->m_kfRot[1] = 0.0;
               pptr->m_kfRot[2] = 0.0;
            }

            normalize3( axisX );
            normalize3( axisY );
            normalize3( axisZ );
            Matrix m;
            for ( int i = 0; i < 3; i++ )
            {
               m.set( 0, i, axisX[i] );
               m.set( 1, i, axisY[i] );
               m.set( 2, i, axisZ[i] );
            }
            m.getRotation( pptr->m_kfRot );
         }
         else
         {
            pptr->m_kfTrans[0] = pptr->m_trans[0];
            pptr->m_kfTrans[1] = pptr->m_trans[1];
            pptr->m_kfTrans[2] = pptr->m_trans[2];
            pptr->m_kfRot[0] = pptr->m_rot[0];
            pptr->m_kfRot[1] = pptr->m_rot[1];
            pptr->m_kfRot[2] = pptr->m_rot[2];
         }
      }

      unsigned tcount = m_triangles.size();
      for ( unsigned t = 0; t < tcount; t++ )
      {
         Triangle * tri = m_triangles[t];
         for ( int v = 0; v < 3; v++ )
         {
            int vertexIndex = tri->m_vertexIndices[v];
            InfluenceList * ilist = &m_vertices[ vertexIndex ]->m_influences;
            if ( ilist->empty() )
            {
               tri->m_kfNormals[v][0] = tri->m_finalNormals[v][0];
               tri->m_kfNormals[v][1] = tri->m_finalNormals[v][1];
               tri->m_kfNormals[v][2] = tri->m_finalNormals[v][2];
            }
            else
            {
               double fnorm[3] = { 0, 0, 0 };
               double total = 0.0;

               Vector norm;

               InfluenceList::iterator it;
               for ( it = ilist->begin(); it != ilist->end(); it++ )
               {
                  const Matrix & final = m_joints[ (*it).m_boneId ]->m_final;
                  const Matrix & abs = m_joints[ (*it).m_boneId ]->m_absolute;

                  norm.setAll( tri->m_finalNormals[v] );
                  abs.inverseRotateVector( (double *) norm.getVector() );
                  norm.transform3( final );

                  norm.scale3( (*it).m_weight );
                  fnorm[0] += norm[0];
                  fnorm[1] += norm[1];
                  fnorm[2] += norm[2];

                  total += (*it).m_weight;
               }

               if ( total > 0.0 )
               {
                  fnorm[0] /= total;
                  fnorm[1] /= total;
                  fnorm[2] /= total;
               }

               normalize3( fnorm );
               tri->m_kfNormals[v][0] = fnorm[0];
               tri->m_kfNormals[v][1] = fnorm[1];
               tri->m_kfNormals[v][2] = fnorm[2];
            }
         }

         InfluenceList * ilist = &m_vertices[ tri->m_vertexIndices[0] ]->m_influences;
         double fnorm[3] = { 0, 0, 0 };
         double total = 0.0;

         Vector norm;

         InfluenceList::iterator it;
         for ( it = ilist->begin(); it != ilist->end(); it++ )
         {
            const Matrix & final = m_joints[ (*it).m_boneId ]->m_final;
            const Matrix & abs = m_joints[ (*it).m_boneId ]->m_absolute;

            norm.setAll( tri->m_flatNormals );
            abs.inverseRotateVector( (double *) norm.getVector() );
            norm.transform3( final );

            norm.scale3( (*it).m_weight );
            fnorm[0] += norm[0];
            fnorm[1] += norm[1];
            fnorm[2] += norm[2];

            total += (*it).m_weight;
         }

         if ( total > 0.0 )
         {
            fnorm[0] /= total;
            fnorm[1] /= total;
            fnorm[2] /= total;
         }

         normalize3( fnorm );
         tri->m_kfFlatNormals[0] = fnorm[0];
         tri->m_kfFlatNormals[1] = fnorm[1];
         tri->m_kfFlatNormals[2] = fnorm[2];
      }

      updateObservers();
      return true;
   }
   else
   {
      return false;
   }
}

unsigned Model::getCurrentAnimation() const
{
   return m_currentAnim;
}

unsigned Model::getCurrentAnimationFrame() const
{
   return m_currentFrame;
}

double Model::getCurrentAnimationTime() const
{
   return m_currentTime;
}

void Model::setNoAnimation()
{
#ifdef MM3D_EDIT
   if ( m_animationMode )
   {
      MU_ChangeAnimState * undo = new MU_ChangeAnimState();
      undo->setState( ANIMMODE_NONE, m_animationMode, m_currentAnim, m_currentFrame );
      sendUndo( undo );
   }
#endif // MM3D_EDIT

   m_changeBits |= AnimationSet;

   m_changeBits |= AnimationSet;
   if ( m_animationMode != ANIMMODE_NONE )
   {
      m_changeBits |= AnimationMode;
   }

   unsigned vcount = m_vertices.size();
   for ( unsigned v = 0; v < vcount; v++ )
   {
      m_vertices[ v ]->m_drawSource = m_vertices[v]->m_coord;
   }

   unsigned pcount = m_points.size();
   for ( unsigned p = 0; p < pcount; p++ )
   {
      m_points[ p ]->m_drawSource = m_points[p]->m_trans;
      m_points[ p ]->m_rotSource  = m_points[p]->m_rot;
   }

   unsigned tcount = m_triangles.size();
   for ( unsigned t = 0; t < tcount; t++ )
   {
      m_triangles[t]->m_flatSource = m_triangles[t]->m_flatNormals;
      m_triangles[t]->m_normalSource[0] = m_triangles[t]->m_finalNormals[0];
      m_triangles[t]->m_normalSource[1] = m_triangles[t]->m_finalNormals[1];
      m_triangles[t]->m_normalSource[2] = m_triangles[t]->m_finalNormals[2];
   }

   m_animationMode = ANIMMODE_NONE;
   setupJoints();
}

void Model::setupJoints()
{
   LOG_PROFILE();
   
   m_validJoints = true;

   if ( m_animationMode )
   {
      return;
   }

   log_debug( "setupJoints()\n" );

   for ( unsigned j = 0; j < m_joints.size(); j++ )
   {
      Joint * joint = m_joints[ j ];
      joint->m_relative.loadIdentity();
      joint->m_relative.setRotation( joint->m_localRotation );
      joint->m_relative.setTranslation( joint->m_localTranslation );

      if ( joint->m_parent == -1 ) // parent
      {
         joint->m_absolute = joint->m_relative;
      }
      else
      {
         joint->m_absolute = m_joints[ joint->m_parent ]->m_absolute;
         joint->m_absolute = joint->m_relative * joint->m_absolute;
      }

      joint->m_final = joint->m_absolute;
//      log_debug( "\n" );
//      log_debug( "Joint %d:\n", j );
//      joint->m_final.show();
//      log_debug( "local rotation: %.2f %.2f %.2f\n", 
//            joint->m_localRotation[0], joint->m_localRotation[1], joint->m_localRotation[2] );
//      log_debug( "\n" );
   }
   
   for ( unsigned anim = 0; anim < m_skelAnims.size(); ++anim )
   {
      while ( m_joints.size() > m_skelAnims[anim]->m_jointKeyframes.size() )
      {
         KeyframeList kl;
         m_skelAnims[anim]->m_jointKeyframes.push_back( kl );
      }
   }
}

unsigned Model::getAnimCount( AnimationModeE m ) const
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         return m_skelAnims.size();
         break;
      case ANIMMODE_FRAME:
         return m_frameAnims.size();
         break;
      default:
         break;
   }

   return 0;
}
const char * Model::getAnimName( AnimationModeE m, unsigned anim ) const
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            return m_skelAnims[anim]->m_name.c_str();
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            return m_frameAnims[anim]->m_name.c_str();
         }
         break;
      default:
         break;
   }

   return NULL;
}

unsigned Model::getAnimFrameCount( AnimationModeE m, unsigned anim ) const
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            return m_skelAnims[anim]->m_frameCount;
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            return m_frameAnims[anim]->m_frameData.size();
         }
         break;
      default:
         break;
   }

   return 0;
}

double Model::getAnimFPS( AnimationModeE m, unsigned anim ) const
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            return m_skelAnims[anim]->m_fps;
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            return m_frameAnims[anim]->m_fps;
         }
         break;
      default:
         break;
   }

   return 0;
}

bool Model::getAnimLooping( AnimationModeE m, unsigned anim ) const
{
   switch ( m )
   {
      case ANIMMODE_SKELETAL:
         if ( anim < m_skelAnims.size() )
         {
            return m_skelAnims[anim]->m_loop;
         }
         break;
      case ANIMMODE_FRAME:
         if ( anim < m_frameAnims.size() )
         {
            return m_frameAnims[anim]->m_loop;
         }
         break;
      default:
         break;
   }

   return true;
}

bool Model::getFrameAnimVertexCoords( unsigned anim, unsigned frame, unsigned vertex,
      double & x, double & y, double & z ) const
{
   if ( anim < m_frameAnims.size() 
         && frame < m_frameAnims[anim]->m_frameData.size()
         && vertex < m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->size() )
   {
      FrameAnimVertex * fav = (*m_frameAnims[anim]->m_frameData[frame]->m_frameVertices)[vertex];
      x = fav->m_coord[0];
      y = fav->m_coord[1];
      z = fav->m_coord[2];
      return true;
   }
   else if ( vertex < m_vertices.size() )
   {
      x = m_vertices[vertex]->m_coord[0];
      y = m_vertices[vertex]->m_coord[1];
      z = m_vertices[vertex]->m_coord[2];
      return true;
   }
   else
   {
      return false;
   }
}

bool Model::getFrameAnimVertexNormal( unsigned anim, unsigned frame, unsigned vertex,
      double & x, double & y, double & z ) const
{
   if ( anim < m_frameAnims.size() 
         && frame < m_frameAnims[anim]->m_frameData.size()
         && vertex < m_frameAnims[anim]->m_frameData[frame]->m_frameVertices->size() )
   {
      FrameAnimVertex * fav = (*m_frameAnims[anim]->m_frameData[frame]->m_frameVertices)[vertex];
      x = fav->m_normal[0];
      y = fav->m_normal[1];
      z = fav->m_normal[2];
      return true;
   }
   else
   {
      size_t tcount = m_triangles.size();
      for ( size_t tri = 0; tri < tcount; tri++ )
      {
         for ( int i = 0; i < 3; i++ )
         {
            if ( m_triangles[tri]->m_vertexIndices[i] == vertex )
            {
               x = m_triangles[tri]->m_vertexNormals[i][0];
               y = m_triangles[tri]->m_vertexNormals[i][1];
               z = m_triangles[tri]->m_vertexNormals[i][2];
            }
         }
      }
      return false;
   }
}

bool Model::hasSkelAnimKeyframe( unsigned anim, unsigned frame,
      unsigned joint, bool isRotation ) const
{
   if ( anim < m_skelAnims.size() 
         && frame < m_skelAnims[anim]->m_frameCount
         && joint < m_joints.size() )
   {
      while ( joint >= m_skelAnims[anim]->m_jointKeyframes.size() )
      {
         KeyframeList kl;
         m_skelAnims[anim]->m_jointKeyframes.push_back( kl );
      }

      Keyframe * kf = Keyframe::get();

      kf->m_frame        = frame;
      kf->m_isRotation   = isRotation;

      unsigned index;
      bool found = false;
      if ( m_skelAnims[anim]->m_jointKeyframes[joint].find_sorted( kf, index ) )
      {
         found = true;
      }
      kf->release();

      return found;
   }
   else
   {
      log_error( "anim keyframe out of range: anim %d, frame %d, joint %d\n", anim, frame, joint );
      return false;
   }
}

bool Model::getSkelAnimKeyframe( unsigned anim, unsigned frame,
      unsigned joint, bool isRotation,
      double & x, double & y, double & z ) const
{
   if ( anim < m_skelAnims.size() 
         && frame < m_skelAnims[anim]->m_frameCount
         && joint < m_joints.size() )
   {
      while ( joint >= m_skelAnims[anim]->m_jointKeyframes.size() )
      {
         KeyframeList kl;
         m_skelAnims[anim]->m_jointKeyframes.push_back( kl );
      }

      Keyframe * kf = Keyframe::get();

      kf->m_frame        = frame;
      kf->m_isRotation   = isRotation;

      unsigned index;
      bool found = false;
      if ( m_skelAnims[anim]->m_jointKeyframes[joint].find_sorted( kf, index ) )
      {
//         log_debug( "found keyframe anim %d, frame %d, joint %d, %s\n",
//               anim, frame, joint, isRotation ? "rotation" : "translation" );
         x = m_skelAnims[anim]->m_jointKeyframes[joint][index]->m_parameter[0];
         y = m_skelAnims[anim]->m_jointKeyframes[joint][index]->m_parameter[1];
         z = m_skelAnims[anim]->m_jointKeyframes[joint][index]->m_parameter[2];
         found = true;
      }
      else
      {
         //log_debug( "could not find keyframe anim %d, frame %d, joint %d, %s\n",
         //      anim, frame, joint, isRotation ? "rotation" : "translation" );
      }
      kf->release();

      return found;
   }
   else
   {
      log_error( "anim keyframe out of range: anim %d, frame %d, joint %d\n", anim, frame, joint );
      return false;
   }
}

bool Model::interpSkelAnimKeyframe( unsigned anim, unsigned frame,
      bool loop, unsigned j, bool isRotation,
      double & x, double & y, double & z ) const
{
   if ( anim < m_skelAnims.size() 
         && frame < m_skelAnims[anim]->m_frameCount
         && j < m_joints.size() )
   {
      SkelAnim * sa = m_skelAnims[anim];
      double totalTime = sa->m_spf * sa->m_frameCount;
      double frameTime = (double) frame / (double) sa->m_frameCount
                         * totalTime;

      Matrix relativeFinal;
      bool rval = interpSkelAnimKeyframeTime(anim, frameTime, loop, j,
            relativeFinal );

      if ( isRotation )
         relativeFinal.getRotation( x, y, z );
      else
         relativeFinal.getTranslation( x, y, z );

      return rval;
   }
   else
   {
      log_error( "anim keyframe out of range: anim %d, frame %d, joint %d\n", anim, frame, j );
      return false;
   }
}

bool Model::interpSkelAnimKeyframeTime( unsigned anim, double frameTime,
      bool loop, unsigned j, Matrix & transform ) const
{
   if ( anim < m_skelAnims.size() && j < m_joints.size() )
   {
      SkelAnim * sa = m_skelAnims[anim];
      double totalTime = sa->m_spf * sa->m_frameCount;
      while ( frameTime > totalTime )
      {
         if ( !loop )
         {
            return false;
         }
         frameTime -= totalTime;
      }

      transform.loadIdentity();

      if ( !sa->m_jointKeyframes[j].empty() )
      {
         int firstRot  = -1;
         int firstTran = -1;
         int lastRot  = -1;
         int lastTran = -1;
         int stopRot  = -1;
         int stopTran = -1;
         int rot  = -1;
         int tran = -1;
         for ( unsigned k = 0; k < sa->m_jointKeyframes[j].size(); k++ )
         {
            if ( sa->m_jointKeyframes[j][k]->m_isRotation )
            {
               if ( firstRot == -1 )
                  firstRot = k;
               lastRot = k;
            }
            else
            {
               if ( firstTran == -1 )
                  firstTran = k;
               lastTran = k;
            }

            if ( sa->m_jointKeyframes[j][k]->m_time <= frameTime )
            {
               // Less than current time
               // get latest keyframe for rotation and translation
               if ( sa->m_jointKeyframes[j][k]->m_isRotation )
                  rot = k;
               else
                  tran = k;
            }
            else
            {
               // Greater than current time
               // get earliest keyframe for rotation and translation
               if ( sa->m_jointKeyframes[j][k]->m_isRotation )
               {
                  if ( stopRot == -1 )
                     stopRot = k;
               }
               else
               {
                  if ( stopTran == -1 )
                     stopTran = k;
               }
            }
         }

         if ( loop )
         {
            if ( rot == -1 )
               rot = lastRot;
            if ( tran == -1 )
               tran = lastTran;

            if ( stopRot == -1 )
               stopRot = firstRot;
            if ( stopTran == -1 )
               stopTran = firstTran;
         }

         stopRot  = (stopRot  == -1) ? rot  : stopRot;
         stopTran = (stopTran == -1) ? tran : stopTran;

         if ( rot >= 0 )
         {
            double temp[3];
            double diff = sa->m_jointKeyframes[j][stopRot]->m_time - sa->m_jointKeyframes[j][rot]->m_time;

            double tempTime = frameTime;

            if ( tempTime < sa->m_jointKeyframes[j][rot]->m_time )
            {
               tempTime += (sa->m_spf * sa->m_frameCount);
            }

            if ( diff < 0.0 )
            {
               diff += (sa->m_spf * sa->m_frameCount);
            }

            Quaternion va;
            va.setEulerAngles( sa->m_jointKeyframes[j][rot]->m_parameter );

            if ( diff > 0 )
            {
               Quaternion vb;
               vb.setEulerAngles( sa->m_jointKeyframes[j][stopRot]->m_parameter );

               double tm = (tempTime - sa->m_jointKeyframes[j][rot]->m_time) / diff;

               // Negate if necessary to get shortest rotation path for
               // interpolation
               if ( va.dot4(vb) < -0.00001 )
               {
                  vb[0] = -vb[0]; vb[1] = -vb[1]; vb[2] = -vb[2]; vb[3] = -vb[3];
               }

               va = va * (1.0 - tm) + (vb * tm);
               va = va * (1.0 / va.mag());
            }
            else
            {
               temp[0] = sa->m_jointKeyframes[j][rot]->m_parameter[0];
               temp[1] = sa->m_jointKeyframes[j][rot]->m_parameter[1];
               temp[2] = sa->m_jointKeyframes[j][rot]->m_parameter[2];
            }

            transform.setRotationQuaternion( va );
         }

         if ( tran >= 0 )
         {
            double temp[3];
            double diff = sa->m_jointKeyframes[j][stopTran]->m_time - sa->m_jointKeyframes[j][tran]->m_time;

            double tempTime = frameTime;

            if ( tempTime < sa->m_jointKeyframes[j][tran]->m_time )
            {
               tempTime += (sa->m_spf * sa->m_frameCount);
            }

            if ( diff < 0.0 )
            {
               diff += (sa->m_spf * sa->m_frameCount);
            }

            Vector va( sa->m_jointKeyframes[j][tran]->m_parameter );
            if ( diff > 0 )
            {
               Vector vb( sa->m_jointKeyframes[j][stopTran]->m_parameter );
               double tm = (tempTime - sa->m_jointKeyframes[j][tran]->m_time) / diff;
               va = va + (vb - va) * tm;
            }
            else
            {
               temp[0] = sa->m_jointKeyframes[j][tran]->m_parameter[0];
               temp[1] = sa->m_jointKeyframes[j][tran]->m_parameter[1];
               temp[2] = sa->m_jointKeyframes[j][tran]->m_parameter[2];
            }

            transform.setTranslation( va.getVector() );
         }
      }

      return true;
   }
   else
   {
      log_error( "anim keyframe out of range: anim %d, joint %d\n", anim, j );
      return false;
   }
}

