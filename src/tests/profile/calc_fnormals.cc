#include <stdio.h>
#include <stdlib.h>

#include "model.h"
#include "modelstatus.h"
#include "mm3dfilter.h"
#include "md3filter.h"
#include "msg.h"

char prompt_func( const char * str, const char * opts )
{
   return 'Y';
}

void model_status( Model * model, StatusTypeE type, unsigned ms, const char * fmt, ... )
{
   // FIXME hack
}

Model * loadModel( const char * filename )
{
   Md3Filter f;

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

   msg_register_prompt( prompt_func, prompt_func, prompt_func );

   for ( int a = 1; a < argc; a++ )
   {
      printf( "testing normal calculation for %s\n", argv[a] );
      Model * model = loadModel( argv[a] );
      if ( model )
      {
         unsigned int acount = model->getAnimCount( Model::ANIMMODE_FRAME );

         for ( unsigned int anim = 0; anim < acount; anim++ )
            model->calculateFrameNormals( anim );

         delete model;
      }
   }

   return 0;
}
