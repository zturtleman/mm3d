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


#include "model.h"
#include "texture.h"
#include "log.h"

#include <map>

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

extern const double EQ_TOLERANCE;

template<typename T>
bool floatCompareVector( T * lhs, T * rhs, size_t len, double tolerance = EQ_TOLERANCE )
{
   for ( size_t index = 0; index < len; ++index )
      if ( fabs( lhs[index] - rhs[index] ) > tolerance )
         return false;
   return true;
}

static bool influencesMatch( const Model::InfluenceList & lhs,
      const Model::InfluenceList & rhs, int propBits, double tolerance )
{
   typedef std::map<int, Model::InfluenceT> InfluenceMap;
   InfluenceMap lhsInfMap;

   Model::InfluenceList::const_iterator it;

   for ( it = lhs.begin(); it != lhs.end(); ++it )
   {
      lhsInfMap[ it->m_boneId ] = *it;
   }

   for ( it = rhs.begin(); it != rhs.end(); ++it )
   {
      InfluenceMap::const_iterator lhs_it = lhsInfMap.find( it->m_boneId );
      if ( lhs_it == lhsInfMap.end() )
      {
         log_warning( "match failed on missing lhs bone %d\n", it->m_boneId );
         return false;
      }

      // This doesn't matter if we only care about weights
      if ( propBits & Model::PropInfluences )
      {
         if ( lhs_it->second.m_type != it->m_type )
         {
            log_warning( "match failed on influence type (%d vs %d) for bone %d\n",
                  lhs_it->second.m_type, it->m_type, it->m_boneId );
            return false;
         }
      }

      if ( fabs( lhs_it->second.m_weight - it->m_weight ) > tolerance )
      {
         log_warning( "match failed on influence weight (%f vs %f) for bone %d\n",
               (float) lhs_it->second.m_weight, (float) it->m_weight, it->m_boneId );
         return false;
      }

      lhsInfMap.erase( it->m_boneId );
   }

   if ( !lhsInfMap.empty() )
   {
      log_warning( "match failed on missing rhs bone %d\n",
            lhsInfMap.begin()->second.m_boneId );
      return false;
   }

   return true;
}

Model::Vertex::Vertex()
   : m_selected( false ),
     m_visible( true ),
     m_free( false ),
     m_drawSource( m_coord )
{
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
   log_debug( "Vertex: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
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

bool Model::Vertex::propEqual(const Vertex & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropCoords) != 0 )
   {
      if ( !floatCompareVector( m_coord, rhs.m_coord, 3, tolerance ) )
         return false;
   }

   if ( (propBits & PropSelection) )
      if ( m_selected != rhs.m_selected )
         return false;

   if ( (propBits & PropVisibility) )
      if ( m_visible != rhs.m_visible )
         return false;

   if ( (propBits & PropFree) )
      if ( m_free != rhs.m_free )
         return false;

   // FIXME apply this to points also
   if ( (propBits & (PropInfluences | PropWeights)) != 0 )
   {
      if ( !influencesMatch( m_influences, rhs.m_influences, propBits, tolerance ) )
         return false;
   }

   return true;
}

Model::Triangle::Triangle()
   : m_selected( false ),
     m_visible( true ),
     m_marked( false ),
     m_projection( -1 )
{
   init();

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

   m_flatSource = m_flatNormals;
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
   log_debug( "Triangle: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
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

bool Model::Triangle::propEqual(const Triangle & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropVertices) != 0 )
   {
      if ( m_vertexIndices[0] != rhs.m_vertexIndices[0]
            || m_vertexIndices[1] != rhs.m_vertexIndices[1]
            || m_vertexIndices[2] != rhs.m_vertexIndices[2] )
      {
         return false;
      }
   }

   if ( (propBits & PropProjections) != 0 )
      if ( m_projection != rhs.m_projection )
         return false;

   if ( (propBits & PropTexCoords) != 0 )
   {
      for ( int i = 0; i < 3; ++i )
      {
         if ( fabs( m_s[i] - rhs.m_s[i] ) > tolerance )
            return false;
         if ( fabs( m_t[i] - rhs.m_t[i] ) > tolerance )
            return false;
      }
   }

   if ( (propBits & PropSelection) != 0 )
      if ( m_selected != rhs.m_selected )
         return false;

   if ( (propBits & PropVisibility) != 0 )
      if ( m_visible != rhs.m_visible )
         return false;

   return true;
}

Model::Group::Group()
   : m_materialIndex( -1 ),
     m_smooth( 255 ),
     m_angle( 89 ),
     m_selected( false ),
     m_visible( true )
{
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
   m_angle = 89;
   m_selected = false;
   m_visible = true;
   m_name.clear();
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
   log_debug( "Group: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
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

bool Model::Group::propEqual(const Group & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropTriangles) != 0 )
      if ( m_triangleIndices != rhs.m_triangleIndices )
         return false;

   if ( (propBits & PropNormals) != 0 )
   {
      if ( m_smooth != rhs.m_smooth )
         return false;
      if ( m_angle != rhs.m_angle )
         return false;
   }

   if ( (propBits & PropMaterials) != 0 )
      if ( m_materialIndex != rhs.m_materialIndex )
         return false;

   if ( (propBits & PropName) != 0 )
      if ( m_name != rhs.m_name )
         return false;

   if ( (propBits & PropSelection) != 0 )
      if ( m_selected != rhs.m_selected )
         return false;

   if ( (propBits & PropVisibility) != 0 )
      if ( m_visible != rhs.m_visible )
         return false;

   return true;
}

Model::Material::Material()
   : m_type( MATTYPE_TEXTURE ),
     m_sClamp( false ),
     m_tClamp( false ),
     m_texture( 0 ),
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
   m_name.clear();
   m_filename.clear();
   m_alphaFilename.clear();
   m_textureData   = NULL;
   m_texture       = 0;
   m_type          = MATTYPE_TEXTURE;
   m_sClamp        = false;
   m_tClamp        = false;
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
   log_debug( "Material: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
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

bool Model::Material::propEqual(const Material & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropType) != 0 )
      if ( m_type != rhs.m_type )
         return false;

   if ( (propBits & PropLighting) != 0 )
   {
      if ( !floatCompareVector( m_ambient, rhs.m_ambient, 4, tolerance ) )
         return false;
      if ( !floatCompareVector( m_diffuse, rhs.m_diffuse, 4, tolerance ) )
         return false;
      if ( !floatCompareVector( m_specular, rhs.m_specular, 4, tolerance ) )
         return false;
      if ( !floatCompareVector( m_emissive, rhs.m_emissive, 4, tolerance ) )
         return false;
      if ( fabs(m_shininess - rhs.m_shininess) > tolerance )
         return false;
   }

   if ( (propBits & PropClamp) != 0 )
   {
      if ( m_sClamp != rhs.m_sClamp )
         return false;
      if ( m_tClamp != rhs.m_tClamp )
         return false;
   }

   // Color is unused

   if ( (propBits & PropPixels) != 0 )
   {
      // If one is NULL, the other must be also
      if ( (m_textureData == NULL) != (rhs.m_textureData == NULL) )
         return false;

      // Compare texture itself
      if ( m_textureData )
      {
         Texture::CompareResultT res;
         if ( !m_textureData->compare( rhs.m_textureData, &res, 0 ) )
            return false;

         // Name and filename should not be an issue (should match material)
      }
      else
      {
         // If no texture data, must not be a texture-mapped material
         if ( m_type == Model::Material::MATTYPE_TEXTURE )
            return false;
      }
   }

   if ( (propBits & PropName) != 0 )
      if ( m_name != rhs.m_name )
         return false;

   if ( (propBits & PropPaths) != 0 )
      if ( m_filename != rhs.m_filename )
         return false;

   if ( (propBits & PropPaths) != 0 )
      if ( m_alphaFilename != rhs.m_alphaFilename )
         return false;

   return true;
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
   log_debug( "Keyframe: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
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

bool Model::Keyframe::propEqual(const Keyframe & rhs, int propBits, double tolerance ) const
{
   if ( m_jointIndex != rhs.m_jointIndex )
      return false;

   if ( (propBits & PropType) != 0 )
      if ( m_isRotation != rhs.m_isRotation )
         return false;

   if ( (propBits & PropTime) != 0 )
   {
      if ( m_frame != rhs.m_frame )
         return false;
      if ( fabs( m_time - rhs.m_time ) > tolerance )
         return false;
   }

   if ( (m_isRotation && (propBits & PropRotation) != 0)
         || (!m_isRotation && (propBits & PropCoords) != 0) )
      if ( !floatCompareVector( m_parameter, rhs.m_parameter, 3, tolerance ) )
         return false;

   return true;
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
   log_debug( "Joint: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
}

bool Model::Joint::propEqual(const Joint & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropRotation) != 0 )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         if ( fabs( m_localRotation[i] - rhs.m_localRotation[i]) > tolerance )
            return false;
      }
   }
   if ( (propBits & PropCoords) != 0 )
   {
      for ( unsigned i = 0; i < 3; i++ )
      {
         if ( fabs( m_localTranslation[i] - rhs.m_localTranslation[i]) > tolerance )
            return false;
      }
   }

   if ( m_parent != rhs.m_parent )
      return false;

   if ( (propBits & PropName) != 0 )
      if ( m_name != rhs.m_name )
         return false;

   if ( (propBits & PropSelection) != 0 )
      if ( m_selected != rhs.m_selected )
         return false;

   if ( (propBits & PropVisibility) != 0 )
      if ( m_visible != rhs.m_visible )
         return false;

   return true;
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
   log_debug( "Point: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
}

bool Model::Point::propEqual(const Point & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropCoords) != 0 )
      if ( !floatCompareVector( m_trans, rhs.m_trans, 3, tolerance ) )
         return false;

   if ( (propBits & PropRotation) != 0 )
      if ( !floatCompareVector( m_rot, rhs.m_rot, 3, tolerance ) )
         return false;

   if ( (propBits & PropName) != 0 )
      if ( m_name != rhs.m_name )
         return false;

   if ( (propBits & PropType) != 0 )
      if ( m_type != rhs.m_type )
         return false;

   if ( (propBits & PropSelection) != 0 )
      if ( m_selected != rhs.m_selected )
         return false;

   if ( (propBits & PropVisibility) != 0 )
      if ( m_visible != rhs.m_visible )
         return false;

   if ( (propBits & (PropInfluences | PropWeights)) != 0 )
   {
      InfluenceList::const_iterator lhs_it = m_influences.begin();
      InfluenceList::const_iterator rhs_it = rhs.m_influences.begin();

      for ( ; lhs_it != m_influences.end() && rhs_it != rhs.m_influences.end();
            ++lhs_it, ++rhs_it )
      {
         // This doesn't matter if we only care about weights
         if ( propBits & PropInfluences )
            if ( lhs_it->m_type != rhs_it->m_type )
               return false;

         // This matters if we're comparing influences or weights
         if ( lhs_it->m_boneId != rhs_it->m_boneId )
            return false;
         if ( fabs( lhs_it->m_weight - rhs_it->m_weight ) > tolerance )
            return false;
      }

      if ( lhs_it != m_influences.end() || rhs_it != rhs.m_influences.end() )
         return false;
   }

   return true;
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

bool Model::TextureProjection::propEqual(const TextureProjection & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropType) != 0 )
      if ( m_type != rhs.m_type )
         return false;

   if ( (propBits & PropCoords) != 0 )
      if ( !floatCompareVector( m_pos, rhs.m_pos, 3, tolerance ) )
         return false;

   if ( (propBits & PropRotation) != 0 )
   {
      if ( !floatCompareVector( m_upVec, rhs.m_upVec, 3, tolerance ) )
         return false;
      if ( !floatCompareVector( m_seamVec, rhs.m_seamVec, 3, tolerance ) )
         return false;
   }

   if ( (propBits & PropDimensions) != 0 )
   {
      if ( !floatCompareVector( m_range[0], rhs.m_range[0], 2, tolerance ) )
         return false;
      if ( !floatCompareVector( m_range[1], rhs.m_range[1], 2, tolerance ) )
         return false;
   }

   if ( (propBits & PropName) != 0 )
      if ( m_name != rhs.m_name )
         return false;

   if ( (propBits & PropSelection) != 0 )
      if ( m_selected != rhs.m_selected )
         return false;

   return true;
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
   releaseData();
}

void Model::SkelAnim::releaseData()
{
   for ( unsigned j = 0; j < m_jointKeyframes.size(); j++ )
   {
      for ( unsigned k = 0; k < m_jointKeyframes[j].size(); k++ )
      {
         m_jointKeyframes[j][k]->release();
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
   log_debug( "SkelAnim: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
}

bool Model::SkelAnim::propEqual(const SkelAnim & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropName) != 0 )
   {
      if ( m_name != rhs.m_name )
      {
         log_warning( "match failed at anim name, lhs = '%s', rhs = '%s'\n",
               m_name.c_str(), rhs.m_name.c_str() );
         return false;
      }
   }

   if ( (propBits & PropTime) != 0 )
   {
      if ( fabs( m_fps - rhs.m_fps ) > tolerance )
      {
         log_warning( "match failed at anim fps, lhs = %f, rhs = %f\n",
               (float) m_fps, (float) rhs.m_fps );
         return false;
      }
   }

   if ( (propBits & PropLoop) != 0 )
   {
      if ( m_loop != rhs.m_loop )
      {
         log_warning( "match failed at anim loop, lhs = %d, rhs = %d\n",
               m_loop, rhs.m_loop );
         return false;
      }
   }

   if ( (propBits & PropDimensions) != 0 )
   {
      if ( m_frameCount != rhs.m_frameCount )
      {
         log_warning( "match failed at anim frame count, lhs = %d, rhs = %d\n",
               m_frameCount, rhs.m_frameCount );
         return false;
      }
   }

   if ( (propBits & (PropCoords | PropRotation | PropType)) != 0 )
   {
      if ( m_jointKeyframes.size() != rhs.m_jointKeyframes.size() )
      {
         log_warning( "match failed at anim keyframe size, lhs = %" PORTuSIZE ", rhs = %" PORTuSIZE "\n",
               m_jointKeyframes.size(), rhs.m_jointKeyframes.size() );
         return false;
      }

      JointKeyframeList::const_iterator lhs_it = m_jointKeyframes.begin();
      JointKeyframeList::const_iterator rhs_it = rhs.m_jointKeyframes.begin();
      for ( ; lhs_it != m_jointKeyframes.end() && rhs_it != m_jointKeyframes.end();
            ++lhs_it, ++rhs_it )
      {
         if ( lhs_it->size() != rhs_it->size() )
            return false;

         KeyframeList::const_iterator l = lhs_it->begin();
         KeyframeList::const_iterator r = rhs_it->begin();
         for ( ; l != lhs_it->end() && r != rhs_it->end(); ++l, ++r )
         {
            if ( !(*l)->propEqual( **r, propBits, tolerance ) )
               return false;
         }
      }
   }

   return true;
}

void Model::FrameAnimData::releaseData()
{
   if ( m_frameVertices )
   {
      for ( unsigned v = 0; v < m_frameVertices->size(); v++ )
      {
         (*m_frameVertices)[v]->release();
      }
      m_frameVertices->clear();
   }
   if ( m_framePoints )
   {
      for ( unsigned p = 0; p < m_framePoints->size(); p++ )
      {
         (*m_framePoints)[p]->release();
      }
      m_framePoints->clear();
   }
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
   releaseData();
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

void Model::FrameAnim::releaseData()
{
   while ( ! m_frameData.empty() )
   {
      m_frameData.back()->releaseData();
      delete m_frameData.back()->m_frameVertices;
      delete m_frameData.back()->m_framePoints;
      delete m_frameData.back();
      m_frameData.pop_back();
   }
   m_frameData.clear();
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
   log_debug( "FrameAnim: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
}

bool Model::FrameAnim::propEqual(const FrameAnim & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropName) != 0 )
   {
      if ( m_name != rhs.m_name )
      {
         log_warning( "match failed at anim name lhs '%s', rhs '%s'\n",
               m_name.c_str(), rhs.m_name.c_str() );
         return false;
      }
   }

   if ( (propBits & PropTime) != 0 )
   {
      if ( fabs( m_fps - rhs.m_fps ) > tolerance )
      {
         log_warning( "match failed at anim fps lhs %f, rhs %f\n",
               (float) m_fps, (float) rhs.m_fps );
         return false;
      }
   }

   if ( (propBits & PropLoop) != 0 )
   {
      if ( m_loop != rhs.m_loop )
      {
         log_warning( "match failed at anim loop lhs = %d, rhs = %d\n",
               m_loop, rhs.m_loop );
         return false;
      }
   }

   if ( (propBits & (PropCoords | PropRotation)) != 0 )
   {
      if ( m_frameData.size() != rhs.m_frameData.size() )
      {
         log_warning( "match failed at anim frame size lhs %" PORTuSIZE ", rhs %" PORTuSIZE "\n",
               m_frameData.size(), rhs.m_frameData.size() );
         return false;
      }

      FrameAnimDataList::const_iterator lhs_it = m_frameData.begin();
      FrameAnimDataList::const_iterator rhs_it = rhs.m_frameData.begin();
      for ( ; lhs_it != m_frameData.end() && rhs_it != m_frameData.end();
            ++lhs_it, ++rhs_it )
      {
         if ( !(*lhs_it)->propEqual( **rhs_it, propBits, tolerance ) )
         {
            log_warning( "match failed at frame %d\n", lhs_it - m_frameData.begin() );
            return false;
         }
      }
   }

   return true;
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
   log_debug( "FrameAnimVertex: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
}

bool Model::FrameAnimVertex::propEqual(const FrameAnimVertex & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropCoords) != 0 )
   {
      if ( !floatCompareVector( m_coord, rhs.m_coord, 3, tolerance ) )
      {
         log_warning( "frame anim vertex coord mismatch, lhs (%f,%f,%f) rhs (%f,%f,%f)\n",
               (float) m_coord[0], (float) m_coord[1], (float) m_coord[2],
               (float) rhs.m_coord[0], (float) rhs.m_coord[1], (float) rhs.m_coord[2] );
         return false;
      }
   }

   return true;
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
   log_debug( "FrameAnimPoint: %" PORTuSIZE "/%d\n", s_recycle.size(), s_allocated );
}

bool Model::FrameAnimPoint::propEqual(const FrameAnimPoint & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropCoords) != 0 )
   {
      if ( !floatCompareVector( m_trans, rhs.m_trans, 3, tolerance ) )
      {
         log_warning( "frame anim point translation mismatch, lhs (%f,%f,%f) rhs (%f,%f,%f) tolerance (%f)\n",
               (float) m_trans[0], (float) m_trans[1], (float) m_trans[2],
               (float) rhs.m_trans[0], (float) rhs.m_trans[1], (float) rhs.m_trans[2],
               (float) tolerance );
         return false;
      }
   }

   if ( (propBits & PropRotation) != 0 )
   {
      if ( !floatCompareVector( m_rot, rhs.m_rot, 3, tolerance ) )
      {
         log_warning( "frame anim point rotation mismatch, lhs (%f,%f,%f) rhs (%f,%f,%f)\n",
               (float) m_rot[0], (float) m_rot[1], (float) m_rot[2],
               (float) rhs.m_rot[0], (float) rhs.m_rot[1], (float) rhs.m_rot[2] );
         return false;
      }
   }

   return true;
}

bool Model::FrameAnimData::propEqual(const FrameAnimData & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & (PropCoords | PropRotation)) != 0 )
   {
      if ( m_frameVertices->size() != rhs.m_frameVertices->size() )
      {
         log_warning( "anim frame vertex size mismatch lhs %" PORTuSIZE ", rhs %" PORTuSIZE "\n",
               m_frameVertices->size(), rhs.m_frameVertices->size() );
         return false;
      }

      if ( m_framePoints->size() != rhs.m_framePoints->size() )
      {
         log_warning( "anim frame point size mismatch lhs %" PORTuSIZE ", rhs %" PORTuSIZE "\n",
               m_framePoints->size(), rhs.m_framePoints->size() );
         return false;
      }

      FrameAnimVertexList::iterator lv = m_frameVertices->begin();
      FrameAnimVertexList::iterator rv = rhs.m_frameVertices->begin();

      for ( ; lv != m_frameVertices->end() && rv != rhs.m_frameVertices->end(); ++lv, ++rv )
      {
         if ( !(*lv)->propEqual( **rv, propBits, tolerance ) )
         {
            log_warning( "anim frame vertex mismatch at %d\n",
                  lv - m_frameVertices->begin() );
            return false;
         }
      }

      FrameAnimPointList::iterator lp = m_framePoints->begin();
      FrameAnimPointList::iterator rp = rhs.m_framePoints->begin();

      for ( ; lp != m_framePoints->end() && rp != rhs.m_framePoints->end(); ++lp, ++rp )
      {
         if ( !(*lp)->propEqual( **rp, propBits, tolerance ) )
         {
            log_warning( "anim frame point mismatch at %d\n",
                  lp - m_framePoints->begin() );
            return false;
         }
      }
   }

   return true;
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
   : m_scale( 30.0f )
{
   m_center[0] =  0.0f;
   m_center[1] =  0.0f;
   m_center[2] =  0.0f;
}

Model::BackgroundImage::~BackgroundImage()
{
}

bool Model::BackgroundImage::propEqual(const BackgroundImage & rhs, int propBits, double tolerance ) const
{
   if ( (propBits & PropPaths) != 0 )
      if ( m_filename != rhs.m_filename )
         return false;

   if ( (propBits & PropScale) != 0 )
      if ( fabs( m_scale - rhs.m_scale ) > tolerance )
         return false;

   if ( (propBits & PropCoords) != 0 )
      if ( !floatCompareVector( m_center, rhs.m_center, 3, tolerance ) )
         return false;

   return true;
}

#endif // MM3D_EDIT

