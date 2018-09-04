/*  Maverick Model 3D
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


// This file tests the CAL3D model file filter.

#include <QtTest/QtTest>
#include <unistd.h>

#include "test_common.h"
#include "testfilefactory.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "log.h"
#include "cal3dfilter.h"
#include "mm3dfilter.h"

#include "datasource.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"

#include "texmgr.h"
#include "pcxtex.h"
#include "tgatex.h"
#include "rawtex.h"
#include "qttex.h"

int init_std_texture_filters()
{
   log_debug( "initializing standard texture filters\n" );

   TextureManager * textureManager = TextureManager::getInstance();

   TextureFilter * filter;

   filter = new TgaTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new RawTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new PcxTextureFilter;
   textureManager->registerTextureFilter( filter );
   filter = new QtTextureFilter;
   textureManager->registerTextureFilter( filter );

   return 0;
}


Model * loadModelOrDie( ModelFilter & filter, const char * filename )
{
   Model * model = new Model;
   model->forceAddOrDelete( true );

   Model::ModelErrorE err = filter.readFile( model, filename );

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "fatal: read %s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      exit( -1 );
   }

   model->loadTextures();
   model->forceAddOrDelete( true );
   model->calculateNormals();
   return model;
}

Model * loadMm3dOrDie( const char * filename, FileFactory * factory = NULL )
{
   MisfitFilter f;
   if ( factory )
      f.setFactory( factory );
   return loadModelOrDie( f, filename );
}

Model * loadCal3dOrDie( const char * filename, FileFactory * factory = NULL )
{
   Cal3dFilter f;
   if ( factory )
      f.setFactory( factory );
   return loadModelOrDie( f, filename );
}

void saveCal3dOrDie( Model * model, const char * filename, FileFactory * factory = NULL )
{
   Cal3dFilter filter;
   if ( factory )
      filter.setFactory( factory );

   Cal3dFilter::Cal3dOptions * opts = new Cal3dFilter::Cal3dOptions;
   opts->setOptionsFromModel( model );

   Model::ModelErrorE err = filter.writeFile( model, filename, opts );

   opts->release();

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "fatal: write %s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      exit( -1 );
   }
}

//void model_status( Model * model, StatusTypeE type, unsigned ms, const char * fmt, ... )
//{
//   // FIXME hack
//}


static const char REFERENCE_FILE[] = "filtertest/cal3d/ref/reference.mm3d";
static const char OUT_FILE[] = "filtertest/cal3d/out/out.cal";
static const char MD_BIN_VERSION[] = "cal3d_binary_version";
static const char MD_XRF_VERSION[] = "cal3d_xrf_version";
static const char MD_SINGLE_MESH[] = "cal3d_single_mesh_file";
static const char MD_XML_MATERIAL[] = "cal3d_xml_material";


class FilterCal3dTest : public QObject
{
   Q_OBJECT
private:  // Data
   char m_pwd[1024];

private:  // Functions
   void testModelFile( const char * lhs_file, const Model * rhs, bool equivOk = false )
   {
      // The lhs pointer is from the original filter
      local_ptr<Model> lhs = loadMm3dOrDie( lhs_file );

      if ( equivOk )
      {
         QVERIFY_TRUE( lhs->equivalent( rhs, 0.001 ) );
      }
      else
      {
         QVERIFY_TRUE( lhs->propEqual( rhs, Model::PartAll, ~(Model::PropInfluences | Model::PropWeights)) );
         QVERIFY_TRUE( lhs->propEqual( rhs, Model::PartVertices, Model::PropInfluences, 1.01 ) );
      }
   }

   void testReadAndWrite( const char * infile, const char * outfile,
         const char * reffile )
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadCal3dOrDie( infile, &factory );
      testModelFile( reffile, m.get() );

      saveCal3dOrDie( m.get(), outfile, &factory );
      m = loadCal3dOrDie( outfile, &factory );
      testModelFile( reffile, m.get(), true );
   }

   std::string getMetaDataString( Model * m, const char * key )
   {
      char value[128] = "<not found>";
      m->getMetaData( key, value, sizeof(value) );
      return value;
   }

   bool factoryFileExists( TestFileFactory * factory, const char * str )
   {
      DataSource * src = factory->createSource(
            (std::string(m_pwd) + str).c_str() );
      bool rval = !src->errorOccurred();
      src->close();
      return rval;
   }

private slots:

   void initTestCase()
   {
      init_std_texture_filters();
      log_enable_debug( false );
      log_enable_warning( true );
      log_enable_error( false );

      // Less one because we need space to append a slash.
      if ( !getcwd( m_pwd, sizeof(m_pwd) - 1 ) )
      {
         fprintf( stderr, "getcwd failed.\n" );
         exit( -1 );
      }
      strcat( m_pwd, "/" );
   }

   void testBinaryVersionDefault()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      local_ptr<Model> rhs = loadCal3dOrDie( OUT_FILE, &factory );
      QVERIFY_EQ( std::string("700"), getMetaDataString( rhs.get(), MD_BIN_VERSION ) );
      QVERIFY_EQ( std::string("900"), getMetaDataString( rhs.get(), MD_XRF_VERSION ) );
      QVERIFY_TRUE( m->equivalent( rhs.get(), 0.01 ) );
   }

   void testBinaryVersion700()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_BIN_VERSION, "700" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      local_ptr<Model> rhs = loadCal3dOrDie( OUT_FILE, &factory );
      QVERIFY_EQ( std::string("700"), getMetaDataString( rhs.get(), MD_BIN_VERSION ) );
      QVERIFY_EQ( std::string("900"), getMetaDataString( rhs.get(), MD_XRF_VERSION ) );
      QVERIFY_TRUE( m->equivalent( rhs.get(), 0.01 ) );
   }

   void testBinaryVersion1000()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_BIN_VERSION, "1000" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      local_ptr<Model> rhs = loadCal3dOrDie( OUT_FILE, &factory );
      QVERIFY_EQ( std::string("1000"), getMetaDataString( rhs.get(), MD_BIN_VERSION ) );
      QVERIFY_EQ( std::string("900"), getMetaDataString( rhs.get(), MD_XRF_VERSION ) );
      QVERIFY_TRUE( m->equivalent( rhs.get(), 0.01 ) );

      DataSource * src = factory.createSource(
            (std::string(m_pwd) + "filtertest/cal3d/out/updown.caf").c_str() );
      QVERIFY_TRUE( NULL != src );

      QVERIFY_EQ( 1096, (int) src->getFileSize() );
      src->close();
   }

   void testBinaryVersion1200()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_BIN_VERSION, "1200" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      local_ptr<Model> rhs = loadCal3dOrDie( OUT_FILE, &factory );
      QVERIFY_EQ( std::string("1200"), getMetaDataString( rhs.get(), MD_BIN_VERSION ) );
      QVERIFY_EQ( std::string("900"), getMetaDataString( rhs.get(), MD_XRF_VERSION ) );
      QVERIFY_TRUE( m->equivalent( rhs.get(), 0.01 ) );

      DataSource * src = factory.createSource(
            (std::string(m_pwd) + "filtertest/cal3d/out/updown.caf").c_str() );
      QVERIFY_TRUE( NULL != src );

      QVERIFY_EQ( 1100, (int) src->getFileSize() );
      src->close();
   }

   void testXmlVersionDefault()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      local_ptr<Model> rhs = loadCal3dOrDie( OUT_FILE, &factory );
      QVERIFY_EQ( std::string("700"), getMetaDataString( rhs.get(), MD_BIN_VERSION ) );
      QVERIFY_EQ( std::string("900"), getMetaDataString( rhs.get(), MD_XRF_VERSION ) );
      QVERIFY_TRUE( m->equivalent( rhs.get(), 0.01 ) );
   }

   void testXmlVersion900()
   {
      TestFileFactory factory;
      factory.setFilePath( m_pwd );
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_XRF_VERSION, "900" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      local_ptr<Model> rhs = loadCal3dOrDie( OUT_FILE, &factory );
      QVERIFY_EQ( std::string("700"), getMetaDataString( rhs.get(), MD_BIN_VERSION ) );
      QVERIFY_EQ( std::string("900"), getMetaDataString( rhs.get(), MD_XRF_VERSION ) );
      QVERIFY_TRUE( m->equivalent( rhs.get(), 0.01 ) );

      DataSource * src = factory.createSource(
            (std::string(m_pwd) + "filtertest/cal3d/out/Red.xrf").c_str() );
      QVERIFY_TRUE( NULL != src );

      char contents[36] = "";
      QVERIFY_TRUE( src->readBytes( (uint8_t*) contents, sizeof(contents) ) );
      QVERIFY_TRUE( 0 == memcmp( contents, "<HEADER MAGIC=\"XRF\" VERSION=\"900\" />", sizeof(contents) ) );
      src->close();
   }

   void testXmlVersion1000()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_XRF_VERSION, "1000" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      local_ptr<Model> rhs = loadCal3dOrDie( OUT_FILE, &factory );
      QVERIFY_EQ( std::string("700"), getMetaDataString( rhs.get(), MD_BIN_VERSION ) );
      QVERIFY_EQ( std::string("1000"), getMetaDataString( rhs.get(), MD_XRF_VERSION ) );
      QVERIFY_TRUE( m->equivalent( rhs.get(), 0.01 ) );

      DataSource * src = factory.createSource(
            (std::string(m_pwd) + "filtertest/cal3d/out/Red.xrf").c_str() );
      QVERIFY_TRUE( NULL != src );

      char contents[37] = "";
      QVERIFY_TRUE( src->readBytes( (uint8_t*) contents, sizeof(contents) ) );
      QVERIFY_TRUE( 0 == memcmp( contents, "<MATERIAL NUMMAPS=\"0\" VERSION=\"1000\">", sizeof(contents) ) );
      src->close();
   }

   void testXmlMaterial()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_XML_MATERIAL, "1" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      QVERIFY_TRUE( factoryFileExists( &factory, "filtertest/cal3d/out/Red.xrf") );
   }

   void testBinaryMaterial()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_XML_MATERIAL, "0" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      QVERIFY_TRUE( factoryFileExists( &factory, "filtertest/cal3d/out/Red.crf") );
   }

   void testSingleMesh()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_SINGLE_MESH, "1" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      QVERIFY_TRUE( factoryFileExists( &factory, "filtertest/cal3d/out/out.cmf") );
      QVERIFY_FALSE( factoryFileExists( &factory, "filtertest/cal3d/out/red.cmf") );
      QVERIFY_FALSE( factoryFileExists( &factory, "filtertest/cal3d/out/green.cmf") );
      QVERIFY_FALSE( factoryFileExists( &factory, "filtertest/cal3d/out/blue.cmf") );
   }

   void testMultipleMesh()
   {
      TestFileFactory factory;
      local_ptr<Model> m = loadMm3dOrDie( REFERENCE_FILE, &factory );
      m->updateMetaData( MD_SINGLE_MESH, "0" );
      saveCal3dOrDie( m.get(), OUT_FILE, &factory );

      QVERIFY_FALSE( factoryFileExists( &factory, "filtertest/cal3d/out/out.cmf") );
      QVERIFY_TRUE( factoryFileExists( &factory, "filtertest/cal3d/out/group1.cmf") );
      QVERIFY_TRUE( factoryFileExists( &factory, "filtertest/cal3d/out/group2.cmf") );
      QVERIFY_TRUE( factoryFileExists( &factory, "filtertest/cal3d/out/group3.cmf") );
   }

   // XML Material, all else binary.
   void testCal3dModelA()
   {
      testReadAndWrite(
            "filtertest/cal3d/spider/spider.cal",
            "filtertest/cal3d/out/test_out.cal",
            "filtertest/cal3d/spider/spider.mm3d" );
   }

   // All binary.
   void testCal3dModelB()
   {
      testReadAndWrite(
            "filtertest/cal3d/paladin/paladin.cal",
            "filtertest/cal3d/out/test_out.cal",
            "filtertest/cal3d/paladin/paladin.mm3d" );
   }

   // FIXME add tests:
   //   Version 1200 Binary
   //     Read compressed/uncompressed
   //   error handling
};

QTEST_MAIN(FilterCal3dTest)
#include "filter_cal3d_test.moc"

