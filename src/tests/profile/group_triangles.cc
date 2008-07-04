#include <stdio.h>
#include <stdlib.h>

#include "model.h"
#include "modelstatus.h"
#include "mm3dfilter.h"

void model_status( Model * model, StatusTypeE type, unsigned ms, const char * fmt, ... )
{
   // FIXME hack
}

Model * loadModel( const char * filename )
{
   MisfitFilter f;

   Model * model = new Model;
   Model::ModelErrorE err = f.readFile( model, filename );

   if ( err != Model::ERROR_NONE )
   {
      fprintf( stderr, "%s: %s\n", filename, Model::errorToString( err ) );
      delete model;
      return NULL;
   }

   return model;
}

int main( int argc, char * argv[] )
{
   if ( argc < 2 )
   {
      fprintf( stderr, "No model files specified\n" );
      return -1;
   }

   for ( int a = 1; a < argc; a++ )
   {
      printf( "testing group triangles for %s\n", argv[a] );
      Model * model = loadModel( argv[a] );
      if ( model )
      {
         int tcount = model->getTriangleCount();

         int grp = model->addGroup( "New group" );

         // Up, Up
         for ( int t = 0; t < tcount; ++t )
         {
            model->addTriangleToGroup( grp, t );
         }
         for ( int t = 0; t < tcount; ++t )
         {
            model->getTriangleGroup( t );
         }
         for ( int t = 0; t < tcount; ++t )
         {
            model->removeTriangleFromGroup( grp, t );
         }

         // Up, Down
         for ( int t = 0; t < tcount; ++t )
         {
            model->addTriangleToGroup( grp, t );
         }
         for ( int t = 0; t < tcount; ++t )
         {
            model->getTriangleGroup( t );
         }
         for ( int t = tcount - 1; t > 0; --t )
         {
            model->removeTriangleFromGroup( grp, t );
         }

         // Down, Down
         for ( int t = tcount - 1; t > 0; --t )
         {
            model->addTriangleToGroup( grp, t );
         }
         for ( int t = 0; t < tcount; ++t )
         {
            model->getTriangleGroup( t );
         }
         for ( int t = tcount - 1; t > 0; --t )
         {
            model->removeTriangleFromGroup( grp, t );
         }

         // Down, Up
         for ( int t = tcount - 1; t > 0; --t )
         {
            model->addTriangleToGroup( grp, t );
         }
         for ( int t = 0; t < tcount; ++t )
         {
            model->getTriangleGroup( t );
         }
         for ( int t = 0; t > tcount; ++t )
         {
            model->removeTriangleFromGroup( grp, t );
         }
      }
   }

   return 0;
}
