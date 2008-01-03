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
      printf( "testing normal calculation for %s\n", argv[a] );
      Model * model = loadModel( argv[a] );
      if ( model )
      {
         for ( int t = 0; t < 25; ++t )
            model->calculateNormals();
         delete model;
      }
   }

   return 0;
}
