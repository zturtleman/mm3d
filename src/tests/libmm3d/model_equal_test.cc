/*  Misfit Model 3D
 * 
 *  Copyright (c) 2007-2008 Kevin Worcester
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


// This file tests the equality functions of the model components (inner
// classes), and the whole model class.

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "log.h"
#include "mm3dfilter.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"


class ModelEqualTest : public QObject
{
   Q_OBJECT
private:
   void vertexIsInitialized( Model::Vertex * v )
   {
      QVERIFY_TRUE( v->m_visible );
      QVERIFY_FALSE( v->m_selected );
      QVERIFY_FALSE( v->m_free );
      QVERIFY( v->m_drawSource == v->m_coord );
   }

   void triangleIsInitialized( Model::Triangle * t )
   {
      QVERIFY_TRUE( t->m_visible );
      QVERIFY_FALSE( t->m_selected );
      QVERIFY_EQ( -1, t->m_projection );
      // FIXME need fuzzy EQ for floats
      QVERIFY( t->m_flatSource == t->m_flatNormals );
      QVERIFY( t->m_normalSource[0] == t->m_finalNormals[0] );
      QVERIFY( t->m_normalSource[1] == t->m_finalNormals[1] );
      QVERIFY( t->m_normalSource[2] == t->m_finalNormals[2] );
   }

   void groupIsInitialized( Model::Group * g )
   {
      QVERIFY_EQ( std::string(""), g->m_name );
      QVERIFY_EQ( -1, g->m_materialIndex );
      QVERIFY_EQ( (size_t) 0, g->m_triangleIndices.size() );
      QVERIFY_EQ( (uint8_t) 255, g->m_smooth );
      QVERIFY_EQ( (uint8_t) 180, g->m_angle );
      QVERIFY_TRUE( g->m_visible );
      QVERIFY_FALSE( g->m_selected );
   }

   void materialIsInitialized( Model::Material * m )
   {
      QVERIFY_EQ( std::string(""), m->m_name );
      QVERIFY_EQ( Model::Material::MATTYPE_TEXTURE, m->m_type );
      // FIXME need fuzzy EQ for floats
      QVERIFY_FALSE( m->m_sClamp );
      QVERIFY_FALSE( m->m_tClamp );
      QVERIFY_EQ( (GLuint) 0, m->m_texture );
      QVERIFY_EQ( std::string(""), m->m_filename );
      QVERIFY_EQ( std::string(""), m->m_alphaFilename );
      QVERIFY_TRUE( NULL == m->m_textureData );
   }

   void initCompareInfluence( Model::InfluenceT * inf )
   {
      inf->m_boneId = 0;
      inf->m_type = Model::IT_Custom;
      inf->m_weight = 1.0;
   }

   void initCompareVertex( Model::Vertex * v )
   {
      v->m_visible = true;
      v->m_selected = false;
      v->m_free = true;

      v->m_coord[0] = 3.0;
      v->m_coord[1] = 4.0;
      v->m_coord[2] = 5.0;

      Model::InfluenceT inf;
      initCompareInfluence( &inf );

      v->m_influences.clear();
      v->m_influences.push_back( inf );
   }

   void initCompareTriangle( Model::Triangle * tri )
   {
      tri->m_visible = true;
      tri->m_selected = false;
      tri->m_projection = -1;

      tri->m_vertexIndices[0] = 3;
      tri->m_vertexIndices[1] = 4;
      tri->m_vertexIndices[2] = 5;

      tri->m_s[0] = 0.0;
      tri->m_s[1] = 0.5;
      tri->m_s[2] = 1.0;

      tri->m_t[0] = 1.0;
      tri->m_t[1] = 0.5;
      tri->m_t[2] = 0.0;
   }

   void initCompareGroup( Model::Group * grp )
   {
      grp->m_name = "Group";

      grp->m_visible = true;
      grp->m_selected = false;
      grp->m_angle = 90;
      grp->m_smooth = 127;
      grp->m_materialIndex = 1;

      grp->m_triangleIndices.clear();
      grp->m_triangleIndices.push_back(2);
      grp->m_triangleIndices.push_back(4);
      grp->m_triangleIndices.push_back(6);
   }

   void initCompareMaterial( Model::Material * mat )
   {
      mat->m_name = "Material";
      mat->m_filename = "filename.tga";
      mat->m_alphaFilename = "alpha.tga";

      mat->m_type = Model::Material::MATTYPE_BLANK;

      mat->m_ambient[0] = 0.10;
      mat->m_ambient[1] = 0.11;
      mat->m_ambient[2] = 0.12;
      mat->m_ambient[3] = 0.13;

      mat->m_diffuse[0] = 0.20;
      mat->m_diffuse[1] = 0.21;
      mat->m_diffuse[2] = 0.22;
      mat->m_diffuse[3] = 0.23;

      mat->m_specular[0] = 0.30;
      mat->m_specular[1] = 0.31;
      mat->m_specular[2] = 0.32;
      mat->m_specular[3] = 0.33;

      mat->m_emissive[0] = 0.40;
      mat->m_emissive[1] = 0.41;
      mat->m_emissive[2] = 0.42;
      mat->m_emissive[3] = 0.43;

      mat->m_shininess = 0.5;

      // Color is unused

      mat->m_sClamp = false;
      mat->m_tClamp = false;

      mat->m_texture = 0;
   }

   void initCompareMaterialAndTexture( Model::Material * mat, Texture * tex, bool alpha )
   {
      initCompareMaterial( mat );

      static uint8_t data[] = {
         0xff, 0x00, 0x00,    0x00, 0xff, 0x00,    0x00, 0x00, 0xff,    0x00, 0x00, 0x00,
         0xff, 0xff, 0x00,    0x00, 0xff, 0x00,    0x00, 0xff, 0xff,    0x00, 0x00, 0x00,
         0xff, 0x00, 0xff,    0x00, 0xff, 0xff,    0x00, 0x00, 0xff,    0x00, 0x00, 0x00,
         0xff, 0xff, 0xff,    0xee, 0xee, 0xee,    0xdd, 0xdd, 0xdd,    0xcc, 0xcc, 0xcc,
      };

      static uint8_t alphaData[] = {
         0xff, 0x00, 0x00, 0xff,    0x00, 0xff, 0x00, 0xff,    0x00, 0x00, 0xff, 0xff,    0x00, 0x00, 0x00, 0xff,
         0xff, 0xff, 0x00, 0xff,    0x00, 0xff, 0x00, 0xff,    0x00, 0xff, 0xff, 0xff,    0x00, 0x00, 0x00, 0x88,
         0xff, 0x00, 0xff, 0xff,    0x00, 0xff, 0xff, 0xff,    0x00, 0x00, 0xff, 0xff,    0x00, 0x00, 0x00, 0x44,
         0xff, 0xff, 0xff, 0xff,    0xee, 0xee, 0xee, 0xff,    0xdd, 0xdd, 0xdd, 0xff,    0xcc, 0xcc, 0xcc, 0x00,
      };

      mat->m_type = Model::Material::MATTYPE_TEXTURE;
      mat->m_textureData = tex;

      tex->m_format = alpha ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB;
      tex->m_data = alpha ? alphaData : data;
      tex->m_width = 4;
      tex->m_height = 4;
      tex->m_isBad = false;
      
      tex->m_origWidth = 16;
      tex->m_origHeight = 16;
   }

   uint8_t * copyTextureData( Model::Material * mat )
   {
      Texture * tex = mat->m_textureData;
      size_t dataLen = ( tex->m_format == Texture::FORMAT_RGBA )
         ? 4 * 4 * 4   // With alpha channel ( h * w * depth )
         : 4 * 4 * 3;  // Without alpha channel ( h * w * depth )

      uint8_t * d = new uint8_t[ dataLen ];
      memcpy( d, tex->m_data, dataLen );
      
      tex->m_data = d;
      return d;
   }

   void initCompareJoint( Model::Joint * j )
   {
      j->m_name = "Joint";
      j->m_visible = true;
      j->m_selected = false;

      j->m_parent = 0;

      j->m_localRotation[0] = 33.3;
      j->m_localRotation[1] = 44.4;
      j->m_localRotation[2] = 55.5;

      j->m_localTranslation[0] = 3.0;
      j->m_localTranslation[1] = 4.0;
      j->m_localTranslation[2] = 5.0;
   }

   void initComparePoint( Model::Point * p )
   {
      p->m_name = "Point";
      p->m_type = 0;
      p->m_visible = true;
      p->m_selected = false;

      p->m_trans[0] = 3.0;
      p->m_trans[1] = 4.0;
      p->m_trans[2] = 5.0;

      p->m_rot[0] = 33.3;
      p->m_rot[1] = 44.4;
      p->m_rot[2] = 55.5;

      Model::InfluenceT inf;
      initCompareInfluence( &inf );

      p->m_influences.clear();
      p->m_influences.push_back( inf );
   }

   void initCompareProjection( Model::TextureProjection * p )
   {
      p->m_name = "Projection";
      p->m_type = Model::TPT_Sphere;
      p->m_selected = false;

      p->m_pos[0] = 3.0;
      p->m_pos[1] = 4.0;
      p->m_pos[2] = 5.0;

      p->m_upVec[0] = 0.0;
      p->m_upVec[1] = 1.0;
      p->m_upVec[2] = 0.0;

      p->m_seamVec[0] = 0.0;
      p->m_seamVec[1] = 0.0;
      p->m_seamVec[2] = 1.0;

      // min x/y
      p->m_range[0][0] = 0.0;
      p->m_range[0][1] = 0.25;

      // max x/y
      p->m_range[1][0] = 1.0;
      p->m_range[1][1] = 0.75;
   }

   void initCompareBackground( Model::BackgroundImage * p )
   {
      p->m_filename = "background.jpg";

      p->m_scale = 1.0;

      p->m_center[0] = 3.0;
      p->m_center[1] = 4.0;
      p->m_center[2] = 5.0;
   }

   void initCompareKeyframe( Model::Keyframe * kf )
   {
      kf->m_jointIndex = 1;

      kf->m_frame = 2;
      kf->m_time = 1.0 / 30.0;
      kf->m_isRotation = false;

      kf->m_parameter[0] = 3.0;
      kf->m_parameter[1] = 4.0;
      kf->m_parameter[2] = 5.0;
   }

   void initCompareSkelAnim( Model::SkelAnim * sa )
   {
      sa->releaseData(); // This releases any stale keyframes

      sa->m_name = "Skeletal";

      sa->m_fps = 30.0;
      sa->m_spf = 1.0 / sa->m_fps;
      sa->m_frameCount = 5;

      Model::Keyframe * kf = Model::Keyframe::get();
      initCompareKeyframe( kf );

      sa->m_jointKeyframes.resize( 3 );
      sa->m_jointKeyframes[kf->m_jointIndex].insert_sorted( kf );
   }

   void initCompareFrameVertex( Model::FrameAnimVertex * fav )
   {
      fav->m_coord[0] = 3.0;
      fav->m_coord[1] = 4.0;
      fav->m_coord[2] = 5.0;
   }

   void initCompareFramePoint( Model::FrameAnimPoint * fap )
   {
      fap->m_trans[0] = 3.0;
      fap->m_trans[1] = 4.0;
      fap->m_trans[2] = 5.0;

      fap->m_rot[0] = 30.0;
      fap->m_rot[1] = 40.0;
      fap->m_rot[2] = 50.0;
   }

   void initCompareFrameData( Model::FrameAnimData * fad )
   {
      fad->releaseData();

      if ( !fad->m_frameVertices )
         fad->m_frameVertices = new Model::FrameAnimVertexList;
      if ( !fad->m_framePoints )
         fad->m_framePoints = new Model::FrameAnimPointList;

      Model::FrameAnimVertex * fav;

      fav = Model::FrameAnimVertex::get();
      initCompareFrameVertex(fav);
      fad->m_frameVertices->push_back( fav );
      fav = Model::FrameAnimVertex::get();
      initCompareFrameVertex(fav);
      fav->m_coord[0] += 10.0;
      fav->m_coord[1] += 10.0;
      fav->m_coord[2] += 10.0;
      fad->m_frameVertices->push_back( fav );

      Model::FrameAnimPoint * fap;

      fap = Model::FrameAnimPoint::get();
      initCompareFramePoint(fap);
      fad->m_framePoints->push_back( fap );
      fap = Model::FrameAnimPoint::get();
      initCompareFramePoint(fap);
      fap->m_trans[0] += 20.0;
      fap->m_trans[1] += 20.0;
      fap->m_trans[2] += 20.0;
      fap->m_rot[0] += 90.0;
      fap->m_rot[1] += 90.0;
      fap->m_rot[2] += 90.0;
      fad->m_framePoints->push_back( fap );
   }

   void initCompareFrameAnim( Model::FrameAnim * fa )
   {
      fa->releaseData();

      fa->m_name = "Frame";
      fa->m_fps = 10.0;

      Model::FrameAnimData * fad;

      fad = new Model::FrameAnimData;
      initCompareFrameData( fad );
      fa->m_frameData.push_back( fad );

      fad = new Model::FrameAnimData;
      initCompareFrameData( fad );
      fa->m_frameData.push_back( fad );

      fad = new Model::FrameAnimData;
      initCompareFrameData( fad );
      fa->m_frameData.push_back( fad );
   }

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   // Many primitives are recycled. Test initial conditions, change conditions,
   // release, and re-get to make sure that the recyled primitives are properly
   // initialized.

   void testInfluenceCompare()
   {
      Model::InfluenceT lhs;
      Model::InfluenceT rhs;

      initCompareInfluence( &lhs );

      initCompareInfluence( &rhs );
      QVERIFY_TRUE( lhs == rhs );
      rhs.m_boneId = 2;
      QVERIFY_FALSE( lhs == rhs );

      initCompareInfluence( &rhs );
      QVERIFY_TRUE( lhs == rhs );
      rhs.m_type = Model::IT_Remainder;
      QVERIFY_FALSE( lhs == rhs );

      initCompareInfluence( &rhs );
      QVERIFY_TRUE( lhs == rhs );
      rhs.m_weight = 0.5;
      QVERIFY_FALSE( lhs == rhs );
   }

   void testVertexCompare()
   {
      Model::Vertex * lhs = Model::Vertex::get();
      Model::Vertex * rhs = Model::Vertex::get();

      initCompareVertex( lhs );

      int bits = 0;

      bits = Model::PropVisibility;

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_visible = false;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropSelection;

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_selected = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropFree;

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_free = false;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropCoords;

      for ( int i = 0; i < 3; ++i )
      {
         initCompareVertex( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_coord[i] = lhs->m_coord[i] - 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      bits = Model::PropInfluences;

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.front().m_type = Model::IT_Remainder;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.front().m_weight = 0.5;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.push_back( rhs->m_influences.back() );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.clear();
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      bits = Model::PropWeights;

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.front().m_type = Model::IT_Remainder;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.front().m_weight = 0.5;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.push_back( rhs->m_influences.back() );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initCompareVertex( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.clear();
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      lhs->release();
      rhs->release();
   }

   void testTriangleCompare()
   {
      Model::Triangle * lhs = Model::Triangle::get();
      Model::Triangle * rhs = Model::Triangle::get();

      initCompareTriangle( lhs );

      int bits = 0;

      bits = Model::PropVisibility;

      initCompareTriangle( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_visible = false;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropSelection;

      initCompareTriangle( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_selected = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropProjections;

      initCompareTriangle( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_projection = 0;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropTexCoords;

      for ( int i = 0; i < 3; ++i )
      {
         initCompareTriangle( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_s[i] = rhs->m_s[i] + 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         initCompareTriangle( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_t[i] = rhs->m_t[i] + 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      bits = Model::PropVertices;

      for ( int i = 0; i < 3; ++i )
      {
         initCompareTriangle( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_vertexIndices[i] += 1;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      lhs->release();
      rhs->release();
   }

   void testGroupCompare()
   {
      Model::Group * lhs = Model::Group::get();
      Model::Group * rhs = Model::Group::get();

      initCompareGroup( lhs );

      int bits = 0;

      bits = Model::PropName;

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_name = "group";  // different case
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropVisibility;

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_visible = false;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropSelection;

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_selected = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropMaterials;

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_materialIndex = 2;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropNormals;

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_angle = 45;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropNormals;

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_smooth = 128;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropTriangles;

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_triangleIndices.clear();
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_triangleIndices.clear();
      rhs->m_triangleIndices.push_back( 2 );
      rhs->m_triangleIndices.push_back( 4 );
      rhs->m_triangleIndices.push_back( 6 );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_triangleIndices.clear();
      rhs->m_triangleIndices.insert( 2 );
      rhs->m_triangleIndices.insert( 4 );
      rhs->m_triangleIndices.insert( 4 );
      rhs->m_triangleIndices.insert( 6 );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_triangleIndices.clear();
      rhs->m_triangleIndices.push_back( 1 );
      rhs->m_triangleIndices.push_back( 2 );
      rhs->m_triangleIndices.push_back( 4 );
      rhs->m_triangleIndices.push_back( 6 );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_triangleIndices.clear();
      rhs->m_triangleIndices.push_back( 2 );
      rhs->m_triangleIndices.push_back( 4 );
      rhs->m_triangleIndices.push_back( 6 );
      rhs->m_triangleIndices.push_back( 7 );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_triangleIndices.clear();
      rhs->m_triangleIndices.push_back( 2 );
      rhs->m_triangleIndices.push_back( 6 );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_triangleIndices.clear();
      rhs->m_triangleIndices.push_back( 2 );
      rhs->m_triangleIndices.push_back( 4 );
      rhs->m_triangleIndices.push_back( 5 );
      rhs->m_triangleIndices.push_back( 6 );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      // Same triangles in a different order, this is legal for propEqual models
      initCompareGroup( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_triangleIndices.clear();
      rhs->m_triangleIndices.push_back( 6 );
      rhs->m_triangleIndices.push_back( 4 );
      rhs->m_triangleIndices.push_back( 2 );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );

      lhs->release();
      rhs->release();
   }

   void testMaterialCompare()
   {
      Model::Material * lhs = Model::Material::get();
      Model::Material * rhs = Model::Material::get();

      initCompareMaterial( lhs );

      initCompareMaterial( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_texture = 2;  // This only matters to OpenGL
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );

      int bits = 0;

      bits = Model::PropName;

      initCompareMaterial( lhs );

      initCompareMaterial( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_name = "material";  // different case
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropPaths;

      initCompareMaterial( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_filename = "filename.jpg";
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareMaterial( lhs );
      initCompareMaterial( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_alphaFilename = "alpha.gif";
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropType;

      initCompareMaterial( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_type = Model::Material::MATTYPE_TEXTURE;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropLighting;

      for ( int i = 0; i < 4; i++ )
      {
         initCompareMaterial( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_ambient[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         initCompareMaterial( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_diffuse[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         initCompareMaterial( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_specular[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         initCompareMaterial( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_emissive[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      initCompareMaterial( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_shininess = 1.0;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropClamp;

      initCompareMaterial( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_sClamp = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareMaterial( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_tClamp = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      // FIXME these checks really belong in the texture testing code. I do want to keep
      // at least one check below to make sure that the propEquality test fails if the
      // texture compare fails.
      local_ptr<Texture> tex_lhs = new Texture();
      local_ptr<Texture> tex_rhs = new Texture();

      bits = Model::PropPixels;

      initCompareMaterialAndTexture( lhs, tex_lhs.get(), false );

      initCompareMaterialAndTexture( rhs, tex_rhs.get(), false );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_textureData->m_format = Texture::FORMAT_RGBA;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareMaterialAndTexture( lhs, tex_lhs.get(), true );

      initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_textureData->m_format = Texture::FORMAT_RGB;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_textureData->m_width = 2;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_textureData->m_height = 2;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_textureData->m_origWidth = 2;  // Compare doesn't care
      rhs->m_textureData->m_origHeight = 2;
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );

      initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_textureData->m_isBad = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      lhs->m_textureData->m_isBad = true;  // Even if both are bad, it's not a match
      rhs->m_textureData->m_isBad = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareMaterialAndTexture( lhs, tex_lhs.get(), true );

      for ( int i = 0; i < 4; ++i )
      {
         initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
         local_array<uint8_t> buf = copyTextureData( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_textureData->m_data[i] = 0x48;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
         tex_rhs->m_data = NULL;
      }

      // The last element has an alpha value of 0, so changes to the color channels
      // should not result in a texture mismatch.
      for ( int i = 0; i < 3; ++i )
      {
         initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
         local_array<uint8_t> buf = copyTextureData( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_textureData->m_data[15 * 4 + i] = 0x48;
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );  // Clear, don't care
         tex_rhs->m_data = NULL;
      }

      // Same as above, except the alpha channel is no longer clear.
      for ( int i = 0; i < 3; ++i )
      {
         initCompareMaterialAndTexture( lhs, tex_lhs.get(), true );
         initCompareMaterialAndTexture( rhs, tex_rhs.get(), true );
         local_array<uint8_t> buf = copyTextureData( rhs );
         local_array<uint8_t> buf2 = copyTextureData( lhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_textureData->m_data[15 * 4 + i] = 0x48;
         rhs->m_textureData->m_data[15 * 4 + 3] = 0x01;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
         tex_rhs->m_data = NULL;
         tex_lhs->m_data = NULL;
      }

      initCompareMaterialAndTexture( lhs, tex_lhs.get(), false );

      for ( int i = 0; i < 3; ++i )
      {
         initCompareMaterialAndTexture( rhs, tex_rhs.get(), false );
         local_array<uint8_t> buf = copyTextureData( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_textureData->m_data[i] = 0x48;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
         tex_rhs->m_data = NULL;
      }

      for ( int i = 0; i < 3; ++i )
      {
         initCompareMaterialAndTexture( lhs, tex_lhs.get(), false );
         initCompareMaterialAndTexture( rhs, tex_rhs.get(), false );
         local_array<uint8_t> buf = copyTextureData( rhs );
         local_array<uint8_t> buf2 = copyTextureData( lhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_textureData->m_data[i] = 0x48;
         rhs->m_textureData->m_data[3] = 0x00;  // Not alpha, change above matters
         lhs->m_textureData->m_data[3] = 0x00;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
         tex_rhs->m_data = NULL;
         tex_lhs->m_data = NULL;
      }

      // Test image data is statically allocated, don't free it
      tex_lhs->m_data = NULL;
      tex_rhs->m_data = NULL;

      lhs->release();
      rhs->release();
   }

   void testJointCompare()
   {
      Model::Joint * lhs = Model::Joint::get();
      Model::Joint * rhs = Model::Joint::get();

      initCompareJoint( lhs );

      int bits = 0;

      bits = Model::PropName;

      initCompareJoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_name = "joint";  // case
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropVisibility;

      initCompareJoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_visible = false;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropSelection;

      initCompareJoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_selected = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = 0;

      initCompareJoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_parent = 1;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, ~bits ) );

      for ( int i = 0; i < 3; ++i )
      {
         bits = Model::PropRotation;

         initCompareJoint( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_localRotation[i] = 11.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         bits = Model::PropCoords;

         initCompareJoint( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_localTranslation[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      lhs->release();
      rhs->release();
   }

   void testPointCompare()
   {
      Model::Point * lhs = Model::Point::get();
      Model::Point * rhs = Model::Point::get();

      initComparePoint( lhs );

      int bits = 0;

      bits = Model::PropName;

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_name = "point";  // case
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropType;

      // Point m_type doesn't actually do anything, but check it anyway.
      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_type = 2;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropVisibility;

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_visible = false;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropSelection;

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_selected = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      for ( int i = 0; i < 3; ++i )
      {
         bits = Model::PropRotation;

         initComparePoint( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_rot[i] = 11.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         bits = Model::PropCoords;

         initComparePoint( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_trans[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      bits = Model::PropInfluences;

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.front().m_type = Model::IT_Remainder;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.front().m_weight = 0.5;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.push_back( rhs->m_influences.back() );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.clear();
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      bits = Model::PropWeights;

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.front().m_type = Model::IT_Remainder;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.front().m_weight = 0.5;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.push_back( rhs->m_influences.back() );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      initComparePoint( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_influences.clear();
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~(Model::PropWeights | Model::PropInfluences) ) );

      lhs->release();
      rhs->release();
   }

   void testKeyframeCompare()
   {
      Model::Keyframe * lhs = Model::Keyframe::get();
      Model::Keyframe * rhs = Model::Keyframe::get();

      initCompareKeyframe( lhs );

      int bits = 0;

      bits = 0;

      initCompareKeyframe( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_jointIndex = 2;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropTime;

      initCompareKeyframe( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_frame = 3;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      initCompareKeyframe( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_time = 2.0 / 30.0;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      for ( int i = 0; i < 3; i++ )
      {
         bits = Model::PropRotation;

         initCompareKeyframe( rhs );
         lhs->m_isRotation = true;
         rhs->m_isRotation = true;
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_parameter[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         bits = Model::PropCoords;

         initCompareKeyframe( rhs );
         lhs->m_isRotation = false;
         rhs->m_isRotation = false;
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_parameter[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      // Restore m_isRotation to proper state
      initCompareKeyframe( lhs );

      lhs->release();
      rhs->release();
   }

   void testSkelAnimCompare()
   {
      Model::SkelAnim * lhs = Model::SkelAnim::get();
      Model::SkelAnim * rhs = Model::SkelAnim::get();

      initCompareSkelAnim( lhs );

      int bits = 0;

      bits = Model::PropName;

      initCompareSkelAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_name = "skeletal";  // case
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropTime;

      initCompareSkelAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_fps = 24.0;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropDimensions;

      initCompareSkelAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_frameCount = 4;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropType;

      // Change keyframe
      initCompareSkelAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_jointKeyframes[1][0]->m_isRotation = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      Model::Keyframe * kf;

      bits = Model::PropCoords | Model::PropRotation | Model::PropType;

      // Add same keyframe for a different joint
      initCompareSkelAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      kf = Model::Keyframe::get();
      initCompareKeyframe( kf );
      kf->m_jointIndex = 0;
      rhs->m_jointKeyframes[kf->m_jointIndex].insert_sorted( kf );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      // Add identical keyframe as rotation for the same joint
      initCompareSkelAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      kf = Model::Keyframe::get();
      initCompareKeyframe( kf );
      kf->m_isRotation = true;
      rhs->m_jointKeyframes[kf->m_jointIndex].insert_sorted( kf );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      lhs->release();
      rhs->release();
   }

   void testFrameVertexCompare()
   {
      Model::FrameAnimVertex * lhs = Model::FrameAnimVertex::get();
      Model::FrameAnimVertex * rhs = Model::FrameAnimVertex::get();

      initCompareFrameVertex( lhs );

      int bits = 0;

      bits = Model::PropCoords;

      for ( int i = 0; i < 3; i++ )
      {
         initCompareFrameVertex( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_coord[i] = 0.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      lhs->release();
      rhs->release();
   }

   void testFramePointCompare()
   {
      Model::FrameAnimPoint * lhs = Model::FrameAnimPoint::get();
      Model::FrameAnimPoint * rhs = Model::FrameAnimPoint::get();

      initCompareFramePoint( lhs );

      int bits = 0;

      for ( int i = 0; i < 3; i++ )
      {
         bits = Model::PropCoords;

         initCompareFramePoint( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_trans[i] = 0.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         bits = Model::PropRotation;

         initCompareFramePoint( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_rot[i] = 0.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      lhs->release();
      rhs->release();
   }

   void testFrameDataCompare()
   {
      Model::FrameAnimData lhs;
      Model::FrameAnimData rhs;

      initCompareFrameData( &lhs );

      int bits = 0;

      bits = Model::PropCoords;

      initCompareFrameData( &rhs );
      QVERIFY_TRUE( lhs.propEqual( rhs ) );
      (*rhs.m_frameVertices)[0]->m_coord[0] = 0.0;
      QVERIFY_FALSE( lhs.propEqual( rhs ) );
      QVERIFY_FALSE( lhs.propEqual( rhs, bits ) );
      QVERIFY_TRUE( lhs.propEqual( rhs, ~bits ) );

      initCompareFrameData( &rhs );
      QVERIFY_TRUE( lhs.propEqual( rhs ) );
      (*rhs.m_framePoints)[0]->m_trans[0] = 0.0;
      QVERIFY_FALSE( lhs.propEqual( rhs ) );
      QVERIFY_FALSE( lhs.propEqual( rhs, bits ) );
      QVERIFY_TRUE( lhs.propEqual( rhs, ~bits ) );

      bits = Model::PropRotation;

      initCompareFrameData( &rhs );
      QVERIFY_TRUE( lhs.propEqual( rhs ) );
      (*rhs.m_framePoints)[0]->m_rot[0] = 0.0;
      QVERIFY_FALSE( lhs.propEqual( rhs ) );
      QVERIFY_FALSE( lhs.propEqual( rhs, bits ) );
      QVERIFY_TRUE( lhs.propEqual( rhs, ~bits ) );

      bits = Model::PropCoords | Model::PropRotation;

      initCompareFrameData( &rhs );
      QVERIFY_TRUE( lhs.propEqual( rhs ) );
      Model::FrameAnimVertex * fav = Model::FrameAnimVertex::get();
      initCompareFrameVertex( fav );
      rhs.m_frameVertices->push_back(fav);
      QVERIFY_FALSE( lhs.propEqual( rhs ) );
      QVERIFY_FALSE( lhs.propEqual( rhs, bits ) );
      QVERIFY_TRUE( lhs.propEqual( rhs, ~bits ) );

      initCompareFrameData( &rhs );
      QVERIFY_TRUE( lhs.propEqual( rhs ) );
      Model::FrameAnimPoint * fap = Model::FrameAnimPoint::get();
      initCompareFramePoint( fap );
      rhs.m_framePoints->push_back(fap);
      QVERIFY_FALSE( lhs.propEqual( rhs ) );
      QVERIFY_FALSE( lhs.propEqual( rhs, bits ) );
      QVERIFY_TRUE( lhs.propEqual( rhs, ~bits ) );
   }

   void testFrameAnimCompare()
   {
      Model::FrameAnim * lhs = Model::FrameAnim::get();
      Model::FrameAnim * rhs = Model::FrameAnim::get();

      initCompareFrameAnim( lhs );

      int bits = 0;

      bits = Model::PropName;

      initCompareFrameAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_name = "frame";  // case
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropTime;

      initCompareFrameAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_fps = 24.0;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropCoords | Model::PropRotation;

      initCompareFrameAnim( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      Model::FrameAnimData * fad = new Model::FrameAnimData;
      initCompareFrameData( fad );
      rhs->m_frameData.push_back( fad );
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      lhs->release();
      rhs->release();
   }

   void testProjectionCompare()
   {
      Model::TextureProjection * lhs = Model::TextureProjection::get();
      Model::TextureProjection * rhs = Model::TextureProjection::get();

      initCompareProjection( lhs );

      int bits = 0;

      bits = Model::PropName;

      initCompareProjection( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_name = "projection";  // case
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropSelection;

      initCompareProjection( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_selected = true;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      bits = Model::PropType;

      initCompareProjection( rhs );
      QVERIFY_TRUE( lhs->propEqual( *rhs ) );
      rhs->m_type = Model::TPT_Cylinder;
      QVERIFY_FALSE( lhs->propEqual( *rhs ) );
      QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
      QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

      for ( int i = 0; i < 3; i++ )
      {
         bits = Model::PropCoords;

         initCompareProjection( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_pos[i] = 1.0;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         bits = Model::PropRotation;

         initCompareProjection( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_upVec[i] = 0.5;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         initCompareProjection( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_seamVec[i] = 0.5;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      for ( int i = 0; i < 2; i++ )
      {
         bits = Model::PropDimensions;

         initCompareProjection( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_range[0][i] = 0.5;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );

         initCompareProjection( rhs );
         QVERIFY_TRUE( lhs->propEqual( *rhs ) );
         rhs->m_range[1][i] = 0.5;
         QVERIFY_FALSE( lhs->propEqual( *rhs ) );
         QVERIFY_FALSE( lhs->propEqual( *rhs, bits ) );
         QVERIFY_TRUE( lhs->propEqual( *rhs, ~bits ) );
      }

      lhs->release();
      rhs->release();
   }

   void testBackgroundCompare()
   {
      Model::BackgroundImage lhs;
      Model::BackgroundImage rhs;

      initCompareBackground( &lhs );

      int bits = 0;

      bits = Model::PropPaths;

      initCompareBackground( &rhs );
      QVERIFY_TRUE( lhs.propEqual( rhs ) );
      rhs.m_filename = "background.png";
      QVERIFY_FALSE( lhs.propEqual( rhs ) );
      QVERIFY_FALSE( lhs.propEqual( rhs, bits ) );
      QVERIFY_TRUE( lhs.propEqual( rhs, ~bits ) );

      bits = Model::PropScale;

      initCompareBackground( &rhs );
      QVERIFY_TRUE( lhs.propEqual( rhs ) );
      rhs.m_scale = 0.5;
      QVERIFY_FALSE( lhs.propEqual( rhs ) );
      QVERIFY_FALSE( lhs.propEqual( rhs, bits ) );
      QVERIFY_TRUE( lhs.propEqual( rhs, ~bits ) );

      bits = Model::PropCoords;

      for ( int i = 0; i < 3; i++ )
      {
         initCompareBackground( &rhs );
         QVERIFY_TRUE( lhs.propEqual( rhs ) );
         rhs.m_center[i] = 1.0;
         QVERIFY_FALSE( lhs.propEqual( rhs ) );
         QVERIFY_FALSE( lhs.propEqual( rhs, bits ) );
         QVERIFY_TRUE( lhs.propEqual( rhs, ~bits ) );
      }
   }

   void testModelCompare()
   {
      // FIXME test for specific CompareBits matches
      const char model_file[] = "data/model_equal_test.mm3d";
      local_ptr<Model> lhs = loadModelOrDie( model_file );

      local_ptr<Model> rhs;

      int parts = Model::PartAll;
      int props = Model::PropAll;

      // Vertices
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->moveVertex( 1, 3, 4, 5 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setVertexFree( 3, true );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addVertex( 3, 4, 5 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      // Triangles
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setTriangleVertices( 0, 2, 4, 6 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->selectTriangle( 3 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addTriangle( 3, 4, 5 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      // Groups
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setGroupSmooth( 1, 160 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addGroup("new group");
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      // Materials
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setTextureShininess( 1, 55.0f );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addColorMaterial("new material");
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      // Skeleton
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setBoneJointName( 1, "renamed joint" );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addBoneJoint("new joint",
               0, 0, 0,  // position
               0, 0, 0,  // rotation
               0 );      // parent joint
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      // Meta data
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->updateMetaData( "key_1", "new value" );
         QVERIFY_EQ( lhs->getMetaDataCount(), rhs->getMetaDataCount() );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addMetaData( "key_3", "new value" );
         QVERIFY_EQ( lhs->getMetaDataCount() + 1, rhs->getMetaDataCount() );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      // Texture projections
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         double seam[3] = { 0, 1, 0 };
         rhs->setProjectionSeam( 0, seam );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addProjection( "New Projection", Model::TPT_Cylinder, 0.0, 0.0, 0.0 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      // Background images
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setBackgroundScale( 0, 0.25f );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setBackgroundImage( 1, "data/test_rgba_comp.tga" );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      // Skeletal animation
      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setSkelAnimKeyframe( 0, 0, 0, false,
               1.0, 1.0, 1.0 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->deleteSkelAnimKeyframe( 0, 0, 0, false );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addAnimation( Model::ANIMMODE_SKELETAL, "new anin" );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }

      {
         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setFrameAnimVertexCoords( 0, 0, 0, 1.0, 1.0, 1.0 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->setAnimFrameCount( Model::ANIMMODE_FRAME, 0, 25 );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );

         rhs = loadModelOrDie( model_file );
         QVERIFY_TRUE( lhs->propEqual( rhs.get(), parts, props ) );
         rhs->addAnimation( Model::ANIMMODE_FRAME, "new anin" );
         QVERIFY_FALSE( lhs->propEqual( rhs.get(), parts, props ) );
      }
   }
};

QTEST_MAIN(ModelEqualTest)
#include "model_equal_test.moc"

