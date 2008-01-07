#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

int main( int argc, char * argv[] )
{
   if ( argc != 3 )
   {
      fprintf( stderr, "Usage: ./intfile filename dword_count\n" );
      return -1;
   }

   const char * filename = argv[1];
   size_t dword_count = atoi( argv[2] );

   FILE * fp = fopen( filename, "w" );

   if ( !fp )
   {
      fprintf( stderr, "open %s: %s\n", filename, strerror(errno) );
      return -1;
   }

   for ( size_t t = 0; t < dword_count; ++t )
   {
      uint32_t val = t;
      if ( fwrite( &val, sizeof(val), 1, fp ) < 1 )
      {
         fprintf( stderr, "write %s: %s\n", filename, strerror(errno) );
         return -1;
      }
   }

   fclose( fp );

   return 0;
}
