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
#include "log.h"

static bool _model_recycle = true;

int Model::Vertex::s_allocated = 0;
int Model::Triangle::s_allocated = 0;
int Model::Group::s_allocated = 0;
int Model::Material::s_allocated = 0;
int Model::Keyframe::s_allocated = 0;
int Model::Joint::s_allocated = 0;
int Model::Point::s_allocated = 0;
int Model::TextureProjection::s_allocated = 0;
int Model::SkelAnim::s_allocated = 0;
int Model::FrameAnim::s_allocated = 0;
int Model::FrameAnimVertex::s_allocated = 0;
int Model::FrameAnimPoint::s_allocated = 0;

list<Model::Vertex *> Model::Vertex::s_recycle;
list<Model::Triangle *> Model::Triangle::s_recycle;
list<Model::Group *> Model::Group::s_recycle;
list<Model::Material *> Model::Material::s_recycle;
list<Model::Keyframe *> Model::Keyframe::s_recycle;
list<Model::Joint *> Model::Joint::s_recycle;
list<Model::Point *> Model::Point::s_recycle;
list<Model::SkelAnim *> Model::SkelAnim::s_recycle;
list<Model::FrameAnim *> Model::FrameAnim::s_recycle;
list<Model::FrameAnimVertex *> Model::FrameAnimVertex::s_recycle;
list<Model::FrameAnimPoint *> Model::FrameAnimPoint::s_recycle;

Model::Vertex::Vertex()
   : m_selected( false ),
     m_visible( true ),
     m_free( false )
{
   m_coord[0] = 0.0;
   m_coord[1] = 0.0;
   m_coord[2] = 0.0;

   m_kfCoord[0] = 0.0;
   m_kfCoord[1] = 0.0;
   m_kfCoord[2] = 0.0;

   m_drawSource = m_coord;

   s_allocated++;
}

Model::Vertex::~Vertex()
{
   s_allocated--;
}

void Model::Vertex::init()
{
   m_selected = false;
   m_visible  = true;
   m_free     = false;

   m_drawSource = m_coord;

   m_influences.clear();
}

int Model::Vertex::flush()
{
   int c = 0;
   list<Vertex *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::Vertex::stats()
{
   log_debug( "Vertex: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::Vertex * Model::Vertex::get()
{
   if ( ! s_recycle.empty() )
   {
      Vertex * v = s_recycle.front();
      s_recycle.pop_front();
      v->init();
      return v;
   }
   else
   {
      return new Vertex();
   }
}

void Model::Vertex::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

Model::Triangle::Triangle()
   : m_selected( false ),
     m_visible( true ),
     m_marked( false ),
     m_projection( -1 )
{
   m_s[0] = 0.0;
   m_t[0] = 1.0;
   m_s[1] = 0.0;
   m_t[1] = 0.0;
   m_s[2] = 1.0;
   m_t[2] = 0.0;

   for ( int t = 0; t < 3; t++ )
   {
      m_vertexIndices[t] = 0;
      for ( int n = 0; n < 3; n++ )
      {
         m_vertexNormals[t][n] = 0;
      }
      m_normalSource[t] = m_finalNormals[t];
   }

   s_allocated++;
}

Model::Triangle::~Triangle()
{
   s_allocated--;
}

void Model::Triangle::init()
{
   m_s[0] = 0.0;
   m_t[0] = 1.0;
   m_s[1] = 0.0;
   m_t[1] = 0.0;
   m_s[2] = 1.0;
   m_t[2] = 0.0;

   m_selected = false;
   m_marked   = false;
   m_visible  = true;
   m_projection = -1;

   m_normalSource[0] = m_finalNormals[0];
   m_normalSource[1] = m_finalNormals[1];
   m_normalSource[2] = m_finalNormals[2];
}

int Model::Triangle::flush()
{
   int c = 0;
   list<Triangle *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::Triangle::stats()
{
   log_debug( "Triangle: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::Triangle * Model::Triangle::get()
{
   if ( ! s_recycle.empty() )
   {
      Triangle * v = s_recycle.front();
      s_recycle.pop_front();
      v->init();
      return v;
   }
   else
   {
      return new Triangle();
   }
}

void Model::Triangle::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

Model::Group::Group()
   : m_materialIndex( -1 ),
     m_smooth( 255 ),
     m_angle( 180 ),
     m_selected( false ),
     m_visible( true )
{
   m_name = "No Name";
   s_allocated++;
}

Model::Group::~Group()
{
   s_allocated--;
}

void Model::Group::init()
{
   m_materialIndex = -1;
   m_smooth = 255;
   m_angle = 180;
   m_selected = false;
   m_visible = true;
   m_name = "No Name";
   m_triangleIndices.clear();
}

int Model::Group::flush()
{
   int c = 0;
   list<Group *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::Group::stats()
{
   log_debug( "Group: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::Group * Model::Group::get()
{
   if ( ! s_recycle.empty() )
   {
      Group * v = s_recycle.front();
      s_recycle.pop_front();
      v->init();
      return v;
   }
   else
   {
      return new Group();
   }
}

void Model::Group::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

Model::Material::Material()
   : m_type( MATTYPE_TEXTURE ),
     m_sClamp( false ),
     m_tClamp( false ),
     m_filename( "" ),
     m_alphaFilename( "" ),
     m_textureData( NULL )
{
   s_allocated++;
}

Model::Material::~Material()
{
   s_allocated--;
   // Do NOT free m_textureData.  TextureManager does that
}

void Model::Material::init()
{
   m_filename      = "";
   m_alphaFilename = "";
   m_textureData   = NULL;
   m_type          = MATTYPE_TEXTURE;
}

int Model::Material::flush()
{
   int c = 0;
   list<Material *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::Material::stats()
{
   log_debug( "Material: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::Material * Model::Material::get()
{
   if ( ! s_recycle.empty() )
   {
      Material * v = s_recycle.front();
      s_recycle.pop_front();
      v->init();
      return v;
   }
   else
   {
      return new Material();
   }
}

void Model::Material::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

Model::Keyframe::Keyframe()
{
   s_allocated++;
}

Model::Keyframe::~Keyframe()
{
   s_allocated--;
}

void Model::Keyframe::init()
{
}

int Model::Keyframe::flush()
{
   int c = 0;
   list<Keyframe *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::Keyframe::stats()
{
   log_debug( "Keyframe: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::Keyframe * Model::Keyframe::get()
{
   if ( ! s_recycle.empty() )
   {
      Keyframe * v = s_recycle.front();
      s_recycle.pop_front();
      v->init();
      return v;
   }
   else
   {
      return new Keyframe();
   }
}

void Model::Keyframe::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

Model::Joint::Joint()
{
   s_allocated++;
   init();
}

Model::Joint::~Joint()
{
   init();
   s_allocated--;
}

void Model::Joint::init()
{
   m_selected = false;
   m_visible  = true;
}

Model::Joint * Model::Joint::get()
{
   if ( ! s_recycle.empty() )
   {
      Joint * v = s_recycle.front();
      s_recycle.pop_front();
      v->init();
      return v;
   }
   else
   {
      return new Joint();
   }
}

void Model::Joint::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

int Model::Joint::flush()
{
   int c = 0;
   list<Joint *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::Joint::stats()
{
   log_debug( "Joint: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::Point::Point()
{
   s_allocated++;
   init();
}

Model::Point::~Point()
{
   init();
   s_allocated--;
}

void Model::Point::init()
{
   m_selected = false;
   m_visible  = true;
   m_type     = 0;

   m_kfTrans[0] = 0.0;
   m_kfTrans[1] = 0.0;
   m_kfTrans[2] = 0.0;

   m_kfRot[0] = 0.0;
   m_kfRot[1] = 0.0;
   m_kfRot[2] = 0.0;

   m_drawSource = m_trans;
   m_rotSource  = m_rot;

   m_influences.clear();
}

Model::Point * Model::Point::get()
{
   if ( ! s_recycle.empty() )
   {
      Point * v = s_recycle.front();
      s_recycle.pop_front();
      v->init();
      return v;
   }
   else
   {
      return new Point();
   }
}

void Model::Point::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

int Model::Point::flush()
{
   int c = 0;
   list<Point *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::Point::stats()
{
   log_debug( "Point: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::TextureProjection::TextureProjection()
{
   s_allocated++;
   init();
}

Model::TextureProjection::~TextureProjection()
{
   init();
   s_allocated--;
}

void Model::TextureProjection::init()
{
   m_selected = false;
   m_type     = 0;

   m_range[0][0] = 0.0;
   m_range[0][1] = 0.0;
   m_range[1][0] = 1.0;
   m_range[1][1] = 1.0;
}

Model::TextureProjection * Model::TextureProjection::get()
{
   /*
   if ( ! s_recycle.empty() )
   {
      TextureProjection * v = s_recycle.front();
      s_recycle.pop_front();
      v->init();
      return v;
   }
   else
   */
   {
      return new TextureProjection();
   }
}

void Model::TextureProjection::release()
{
   /*
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   */
   {
      delete this;
   }
}

int Model::TextureProjection::flush()
{
   int c = 0;
   /*
   list<TextureProjection *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   */
   return c;
}

void Model::TextureProjection::stats()
{
   log_debug( "TextureProjection: %d/%d\n", 0, s_allocated );
}

Model::SkelAnim::SkelAnim()
   : m_fps( 10.0 )
{
   s_allocated++;
   init();
}

Model::SkelAnim::~SkelAnim()
{
   s_allocated--;
   init();
}

void Model::SkelAnim::init()
{
   m_name = "Skel";
   m_validNormals = false;
   unsigned count = 0;
   for ( unsigned j = 0; j < m_jointKeyframes.size(); j++ )
   {
      for ( unsigned k = 0; k < m_jointKeyframes[j].size(); k++ )
      {
         m_jointKeyframes[j][k]->release();
         count++;
      }
      m_jointKeyframes[j].clear();
   }
   m_jointKeyframes.clear();
}

Model::SkelAnim * Model::SkelAnim::get()
{
   if ( s_recycle.empty() )
   {
      return new SkelAnim;
   }
   else
   {
      SkelAnim * val = s_recycle.front();
      s_recycle.pop_front();
      return val;
   }
}

void Model::SkelAnim::release()
{
   if ( _model_recycle )
   {
      init();
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

int Model::SkelAnim::flush()
{
   int c = 0;
   list<SkelAnim *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::SkelAnim::stats()
{
   log_debug( "SkelAnim: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::FrameAnim::FrameAnim()
   : m_fps( 10.0 )
{
   s_allocated++;
   init();
}

Model::FrameAnim::~FrameAnim()
{
   s_allocated--;
   init();
}

void Model::FrameAnim::init()
{
   m_name = "Frame";
   m_validNormals = false;
   while ( ! m_frameData.empty() )
   {
      for ( unsigned v = 0; v < m_frameData.back()->m_frameVertices->size(); v++ )
      {
         (*m_frameData.back()->m_frameVertices)[v]->release();
      }
      for ( unsigned p = 0; p < m_frameData.back()->m_framePoints->size(); p++ )
      {
         (*m_frameData.back()->m_framePoints)[p]->release();
      }
      m_frameData.back()->m_frameVertices->clear();
      m_frameData.back()->m_framePoints->clear();
      delete m_frameData.back()->m_frameVertices;
      delete m_frameData.back()->m_framePoints;
      m_frameData.pop_back();
   }
   m_frameData.clear();
}

Model::FrameAnim * Model::FrameAnim::get()
{
   if ( s_recycle.empty() )
   {
      return new FrameAnim;
   }
   else
   {
      FrameAnim * val = s_recycle.front();
      s_recycle.pop_front();
      return val;
   }
}

void Model::FrameAnim::release()
{
   if ( _model_recycle )
   {
      init();
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

int Model::FrameAnim::flush()
{
   int c = 0;
   list<FrameAnim *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::FrameAnim::stats()
{
   log_debug( "FrameAnim: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::FrameAnimVertex::FrameAnimVertex()
{
   s_allocated++;
   init();
}

Model::FrameAnimVertex::~FrameAnimVertex()
{
   s_allocated--;
}

void Model::FrameAnimVertex::init()
{
   for ( unsigned t = 0; t < 3; t++ )
   {
      m_coord[t]  = 0;
      m_normal[t] = 0;
   }
}

Model::FrameAnimVertex * Model::FrameAnimVertex::get()
{
   if ( s_recycle.empty() )
   {
      return new FrameAnimVertex;
   }
   else
   {
      FrameAnimVertex * val = s_recycle.front();
      val->init();
      s_recycle.pop_front();
      return val;
   }
}

void Model::FrameAnimVertex::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

int Model::FrameAnimVertex::flush()
{
   int c = 0;
   list<FrameAnimVertex *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::FrameAnimVertex::stats()
{
   log_debug( "FrameAnimVertex: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::FrameAnimPoint::FrameAnimPoint()
{
   s_allocated++;
   init();
}

Model::FrameAnimPoint::~FrameAnimPoint()
{
   s_allocated--;
}

void Model::FrameAnimPoint::init()
{
   for ( unsigned t = 0; t < 3; t++ )
   {
      m_trans[t] = 0;
      m_rot[t]   = 0;
   }
}

Model::FrameAnimPoint * Model::FrameAnimPoint::get()
{
   if ( s_recycle.empty() )
   {
      return new FrameAnimPoint;
   }
   else
   {
      FrameAnimPoint * val = s_recycle.front();
      val->init();
      s_recycle.pop_front();
      return val;
   }
}

void Model::FrameAnimPoint::release()
{
   if ( _model_recycle )
   {
      s_recycle.push_front( this );
   }
   else
   {
      delete this;
   }
}

int Model::FrameAnimPoint::flush()
{
   int c = 0;
   list<FrameAnimPoint *>::iterator it = s_recycle.begin();
   while ( it != s_recycle.end() )
   {
      delete *it;
      it++;
      c++;
   }
   s_recycle.clear();
   return c;
}

void Model::FrameAnimPoint::stats()
{
   log_debug( "FrameAnimPoint: %d/%d\n", s_recycle.size(), s_allocated );
}

Model::FormatData::~FormatData()
{
   if ( data )
   {
      delete[] data;
      data = NULL;
   }
}

void Model::FormatData::serialize()
{
   // Implement this if you derive from Model::FormatData
   // The default implementation assumes that data never changes
   // after it is assigned, so it doesn't need to be re-serialized
}

#ifdef MM3D_EDIT

Model::BackgroundImage::BackgroundImage()
{
   m_filename  = "";

   m_scale     = 30.0f;

   m_center[0] =  0.0f;
   m_center[1] =  0.0f;
   m_center[2] =  0.0f;
}

Model::BackgroundImage::~BackgroundImage()
{
}

#endif // MM3D_EDIT

