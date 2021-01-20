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


// This file tests the CommandLineManager class.

#include <QtTest/QtTest>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <list>
#include <string>

#include "test_common.h"

#include "model.h"
#include "filtermgr.h"
#include "modelfilter.h"

//void model_status( Model * model, StatusTypeE type, unsigned ms, const char * fmt, ... )
//{
   // FIXME hack
//}

const int DEFAULT_OPT_VALUE = 7;
const int CHANGED_OPT_VALUE = 42;

class TestOptions : public ModelFilter::Options
{
   public:
      TestOptions()
         : value(DEFAULT_OPT_VALUE) {}
      virtual ~TestOptions() {}

      int value;
};

bool prompt_cancel( Model *, const char * const, ModelFilter::Options * )
{
   return false;
}

bool prompt_change( Model * m, const char * const, ModelFilter::Options * o )
{
   TestOptions * opts = dynamic_cast<TestOptions*>( o );
   if ( !opts )
   {
      return false;
   }
   opts->value = CHANGED_OPT_VALUE;
   return true;
}

class FilterManagerTest : public QObject
{
   Q_OBJECT

private:
   class TestFilter : public ModelFilter {
      public:
         TestFilter()
            : m_canRead( false ),
              m_canWrite( false ),
              m_canExport( false ),
              m_readError( Model::ERROR_NONE ),
              m_writeError( Model::ERROR_NONE ),
              m_expectOpts( false ),
              m_expectedOptValue( DEFAULT_OPT_VALUE ) {}
         virtual ~TestFilter();

         // Testing functions
         void addFormat( const char * fmt ) { m_formats.push_back( fmt ); }

         void setCanRead( bool o ) { m_canRead = o; }
         void setCanWrite( bool o ) { m_canWrite = o; }
         void setCanExport( bool o ) { m_canExport = o; }

         void setReadError( Model::ModelErrorE err ) { m_readError = err; }
         void setWriteError( Model::ModelErrorE err ) { m_writeError = err; }

         // ModelFilter functions
         virtual Model::ModelErrorE readFile( Model * model, const char * const filename )
         {
            if ( m_canRead )
               return m_readError;
            else
               return Model::ERROR_UNSUPPORTED_OPERATION;
         }

         virtual Model::ModelErrorE writeFile( Model * model, const char * const filename, Options * o = NULL )
         {
            if ( m_expectOpts )
            {
               TestOptions * opts = dynamic_cast<TestOptions*>( o );
               QVERIFY_EQ( m_expectedOptValue, opts->value );
            }

            if ( m_canWrite || m_canExport )
               return m_writeError;
            else
               return Model::ERROR_UNSUPPORTED_OPERATION;
         }

         bool canRead( const char * filename )
         {
            return m_canRead;
         }

         bool canWrite( const char * filename )
         {
            return m_canWrite;
         }

         bool canExport( const char * filename )
         {
            return m_canExport;
         }

         bool isSupported( const char * filename )
         {
            unsigned len = strlen( filename );

            for ( std::list<std::string>::const_iterator it = m_formats.begin();
                  it != m_formats.end(); ++it )
            {
               std::string fmtstr = std::string(".") + *it;
               unsigned fmtlen = fmtstr.size();
               if ( len >= fmtlen && strcasecmp( &filename[len-fmtlen], fmtstr.c_str() ) == 0 )
                  return true;
            }

            return false;
         }

         std::list< std::string > getReadTypes()
         {
            std::list<std::string> rval;
            for ( std::list<std::string>::const_iterator it = m_formats.begin();
                  it != m_formats.end(); ++it )
            {
               if ( m_canRead )
                  rval.push_back( "*." + *it );
            }
            return rval;
         }

         std::list<std::string > getWriteTypes()
         {
            std::list<std::string> rval;
            for ( std::list<std::string>::const_iterator it = m_formats.begin();
                  it != m_formats.end(); ++it )
            {
               if ( m_canWrite || m_canExport )
                  rval.push_back( "*." + *it );
            }
            return rval;
         }

         virtual Options * getDefaultOptions() { return new TestOptions; }

         // Data members
         std::list<std::string> m_formats;

         bool m_canRead;
         bool m_canWrite;
         bool m_canExport;

         Model::ModelErrorE m_readError;
         Model::ModelErrorE m_writeError;

         bool m_expectOpts;
         int m_expectedOptValue;
   };

private slots:
   void testRead()
   {
      TestFilter * f = new TestFilter;

      f->addFormat( "fmt" );
      f->setCanRead( true );

      FilterManager * mgr = FilterManager::getInstance();
      Model * model = new Model;

      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", true) );

      std::list<std::string> formats;

      formats = mgr->getAllReadTypes();
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( false );
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( true );
      QVERIFY_EQ( 0, (int) formats.size() );

      mgr->registerFilter( f );

      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_UNSUPPORTED_OPERATION, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNSUPPORTED_OPERATION, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", true) );

      formats = mgr->getAllReadTypes();
      QVERIFY_EQ( 2, (int) formats.size() );
      if ( 2 == (int) formats.size() )
      {
         std::list<std::string>::const_iterator it = formats.begin();
         QVERIFY_EQ( std::string("*.fmt"), *it );
         ++it;
         QVERIFY_EQ( std::string("*.FMT"), *it );
      }
      formats = mgr->getAllWriteTypes( false );
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( true );
      QVERIFY_EQ( 0, (int) formats.size() );

      mgr->release();
      delete model;
   }

   void testWrite()
   {
      TestFilter * f = new TestFilter;

      f->addFormat( "fmt" );
      f->setCanWrite( true );

      FilterManager * mgr = FilterManager::getInstance();
      Model * model = new Model;

      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", true) );

      std::list<std::string> formats;

      formats = mgr->getAllReadTypes();
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( false );
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( true );
      QVERIFY_EQ( 0, (int) formats.size() );

      mgr->registerFilter( f );

      QVERIFY_EQ( (int) Model::ERROR_UNSUPPORTED_OPERATION, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", true) );

      formats = mgr->getAllReadTypes();
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( false );
      QVERIFY_EQ( 1, (int) formats.size() );
      if ( 1 == (int) formats.size() )
      {
         std::list<std::string>::const_iterator it = formats.begin();
         QVERIFY_EQ( std::string("*.fmt"), *it );
      }
      formats = mgr->getAllWriteTypes( true );
      QVERIFY_EQ( 1, (int) formats.size() );
      if ( 1 == (int) formats.size() )
      {
         std::list<std::string>::const_iterator it = formats.begin();
         QVERIFY_EQ( std::string("*.fmt"), *it );
      }

      mgr->release();
      delete model;
   }

   void testExport()
   {
      TestFilter * f = new TestFilter;

      f->addFormat( "fmt" );
      f->setCanExport( true );

      FilterManager * mgr = FilterManager::getInstance();
      Model * model = new Model;

      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", true) );

      std::list<std::string> formats;

      formats = mgr->getAllReadTypes();
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( false );
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( true );
      QVERIFY_EQ( 0, (int) formats.size() );

      mgr->registerFilter( f );

      QVERIFY_EQ( (int) Model::ERROR_UNSUPPORTED_OPERATION, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_EXPORT_ONLY, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.ext", true) );

      formats = mgr->getAllReadTypes();
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( false );
      QVERIFY_EQ( 0, (int) formats.size() );
      formats = mgr->getAllWriteTypes( true );
      QVERIFY_EQ( 1, (int) formats.size() );
      if ( 1 == (int) formats.size() )
      {
         std::list<std::string>::const_iterator it = formats.begin();
         QVERIFY_EQ( std::string("*.fmt"), *it );
      }

      mgr->release();
      delete model;
   }

   void testMultiFormat()
   {
      TestFilter * f = new TestFilter;

      f->addFormat( "fmt" );
      f->addFormat( "ext" );
      f->addFormat( "end" );
      f->setCanRead( true );
      f->setCanWrite( true );
      f->setCanExport( true );

      FilterManager * mgr = FilterManager::getInstance();
      Model * model = new Model;

      mgr->registerFilter( f );

      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.ext", true) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.end") );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.end", false) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.end", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.non") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.non", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.non", true) );

      std::list<std::string> formats;
      formats = mgr->getAllReadTypes();
      QVERIFY_EQ( 6, (int) formats.size() );
      formats = mgr->getAllWriteTypes( false );
      QVERIFY_EQ( 3, (int) formats.size() );
      formats = mgr->getAllWriteTypes( true );
      QVERIFY_EQ( 3, (int) formats.size() );

      mgr->release();
      delete model;
   }

   void testMultiFilter()
   {
      TestFilter * f1 = new TestFilter;
      TestFilter * f2 = new TestFilter;
      TestFilter * f3 = new TestFilter;

      f1->addFormat( "fmt" );
      f2->addFormat( "ext" );
      f3->addFormat( "end" );
      f1->setCanRead( true );
      f1->setCanWrite( true );
      f1->setCanExport( true );
      f2->setCanRead( true );
      f2->setCanExport( true );
      f3->setCanRead( true );

      FilterManager * mgr = FilterManager::getInstance();
      Model * model = new Model;

      mgr->registerFilter( f1 );
      mgr->registerFilter( f2 );
      mgr->registerFilter( f3 );

      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_EXPORT_ONLY, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.ext", true) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.end") );
      QVERIFY_EQ( (int) Model::ERROR_UNSUPPORTED_OPERATION, (int) mgr->writeFile(model, "file.end", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNSUPPORTED_OPERATION, (int) mgr->writeFile(model, "file.end", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.non") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.non", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.non", true) );

      std::list<std::string> formats;
      formats = mgr->getAllReadTypes();
      QVERIFY_EQ( 6, (int) formats.size() );
      formats = mgr->getAllWriteTypes( false );
      QVERIFY_EQ( 1, (int) formats.size() );
      formats = mgr->getAllWriteTypes( true );
      QVERIFY_EQ( 2, (int) formats.size() );

      mgr->release();
      delete model;
   }

   void testErrors()
   {
      TestFilter * f1 = new TestFilter;
      TestFilter * f2 = new TestFilter;
      TestFilter * f3 = new TestFilter;
      TestFilter * f4 = new TestFilter;

      f1->addFormat( "fmt" );
      f2->addFormat( "ext" );
      f3->addFormat( "end" );
      f4->addFormat( "can" );

      f1->setCanRead( true );
      f1->setCanWrite( true );
      f1->setCanExport( true );
      f1->setWriteError( Model::ERROR_NO_ACCESS );
      f1->setReadError( Model::ERROR_BAD_MAGIC );

      f2->setCanRead( true );
      f2->setCanExport( true );

      f3->setCanRead( true );

      f4->setCanRead( true );
      f4->setCanWrite( true );
      f4->setCanExport( true );
      f4->setOptionsPrompt( prompt_cancel );

      FilterManager * mgr = FilterManager::getInstance();
      Model * model = new Model;

      mgr->registerFilter( f1 );
      mgr->registerFilter( f2 );
      mgr->registerFilter( f3 );
      mgr->registerFilter( f4 );

      QVERIFY_EQ( (int) Model::ERROR_BAD_MAGIC, (int) mgr->readFile(model, "file.fmt") );
      QVERIFY_EQ( (int) Model::ERROR_NO_ACCESS, (int) mgr->writeFile(model, "file.fmt", false) );
      QVERIFY_EQ( (int) Model::ERROR_NO_ACCESS, (int) mgr->writeFile(model, "file.fmt", true) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.ext") );
      QVERIFY_EQ( (int) Model::ERROR_EXPORT_ONLY, (int) mgr->writeFile(model, "file.ext", false) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.ext", true) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.end") );
      QVERIFY_EQ( (int) Model::ERROR_UNSUPPORTED_OPERATION, (int) mgr->writeFile(model, "file.end", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNSUPPORTED_OPERATION, (int) mgr->writeFile(model, "file.end", true) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->readFile(model, "file.can") );
      QVERIFY_EQ( (int) Model::ERROR_CANCEL, (int) mgr->writeFile(model, "file.can", false) );
      QVERIFY_EQ( (int) Model::ERROR_CANCEL, (int) mgr->writeFile(model, "file.can", true) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->readFile(model, "file.non") );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.non", false) );
      QVERIFY_EQ( (int) Model::ERROR_UNKNOWN_TYPE, (int) mgr->writeFile(model, "file.non", true) );

      mgr->release();
      delete model;
   }

   void testOptions()
   {
      TestFilter * f = new TestFilter;

      f->addFormat( "fmt" );

      f->setCanRead( true );
      f->setCanWrite( true );
      f->setCanExport( true );

      f->setOptionsPrompt( prompt_cancel );

      FilterManager * mgr = FilterManager::getInstance();
      Model * model = new Model;

      mgr->registerFilter( f );

      f->m_expectOpts = true;
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", false, FilterManager::WO_ModelNoPrompt) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", true, FilterManager::WO_ModelNoPrompt) );
      QVERIFY_EQ( (int) Model::ERROR_CANCEL, (int) mgr->writeFile(model, "file.fmt", false, FilterManager::WO_FilterDefault) );
      QVERIFY_EQ( (int) Model::ERROR_CANCEL, (int) mgr->writeFile(model, "file.fmt", true, FilterManager::WO_FilterDefault) );

      f->setOptionsPrompt( prompt_change );
      f->m_expectedOptValue = CHANGED_OPT_VALUE;
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", false, FilterManager::WO_FilterDefault) );
      QVERIFY_EQ( (int) Model::ERROR_NONE, (int) mgr->writeFile(model, "file.fmt", true, FilterManager::WO_FilterDefault) );

      mgr->release();
      delete model;
   }

};

FilterManagerTest::TestFilter::~TestFilter() {}


QTEST_MAIN(FilterManagerTest)
#include "filtermgr_test.moc"
