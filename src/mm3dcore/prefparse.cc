#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <string>

#include "prefs.h"
#include "prefparse.h"

class PrefParser
{
   public:
      // Data types
      enum
      {
         BUFFER_SIZE = 512
      };

      enum _ParseState_e
      {
         PS_None,
         PS_Key,
         PS_KeyString,
         PS_KeyEnd,
         PS_Value,
         PS_ValueString,
         PS_ValueEnd,
         PS_ListItem,
         PS_ListItemEnd,
      };
      typedef enum _ParseState_e ParseStateE;

      typedef std::list< ParseStateE >  StateStack;
      typedef std::list< PrefItem * >   ItemStack;

      // Public methods
      PrefParser();
      virtual ~PrefParser();

      bool parseFile ( FILE * fp, Preferences & p );

   protected:
      // Private methods

      PrefItem * parseItem();
      PrefItem * parseHash();
      PrefItem * parseList();
      PrefItem * parseListItem();
      PrefItem * parseKeyValue( std::string & key );
      bool parseString( std::string & str );
      void replaceEscape( std::string & str );

      // indicates if we must read more data
      bool needMore();

      // reads more data
      bool readMore();

      // finds next non-whitespace character
      char * nextToken();

      // Private data
      Preferences * m_pref;
      PrefItem    * m_current;

      ItemStack     m_items;
      StateStack    m_states;

      FILE * m_fp;
      char   m_buffer[ BUFFER_SIZE ];
      char * m_pos;
      bool   m_eof;
      int    m_len;

      int    m_line;

      ParseStateE m_state;

      // string read so far
      std::string m_str;
};

PrefParser::PrefParser()
   : m_pref( NULL ),
     m_current( NULL ),
     m_pos( NULL ),
     m_eof( true ),
     m_len( 0 ),
     m_line( 1 ),
     m_str( "" )
{
}

PrefParser::~PrefParser()
{
}

bool PrefParser::needMore()
{
   if ( m_pos == NULL )
   {
      return true;
   }

   //printf( "calling need more with %d chars read\n", m_pos - m_buffer );
   if ( (m_pos - m_buffer) >= m_len )
   {
      //printf( "end of section\n" );
      return true;
   }
   else
   {
      return false;
   }
}

bool PrefParser::readMore()
{
   //printf( "reading more of the file\n" );
   if ( !m_eof )
   {
      m_len = fread( m_buffer, 1, sizeof(m_buffer), m_fp );
      //printf( "read %d bytes\n", m_len );
      if ( m_len > 0 )
      {
         m_pos = m_buffer;
         return true;
      }
      m_len = 0;
      m_eof = true;
      m_pos = m_buffer;
      memset( m_buffer, 0, sizeof(m_buffer) );
   }
   return false;
}

char * PrefParser::nextToken()
{
   //printf( "finding next token\n" );
   if ( needMore() )
   {
      readMore();
   }

   char * p = m_pos;

   bool found = false;
   while ( !m_eof && !found )
   {
      while ( !found && (p - m_buffer) < m_len )
      {
         //printf( "%d\n", p - m_buffer );
         if ( isspace( *p ) )
         {
            if ( *p == '\n' )
            {
               m_line++;
            }
            p++;
         }
         else
         {
            //printf( "token starts with char '%c'\n", *p );
            found = true;
         }
      }

      m_pos = p;
      if ( !found && needMore() )
      {
         //printf( "need more chars in nextToken()\n" );
         readMore();

         p = m_pos;
      }
   }

   if ( !m_eof )
   {
      m_pos = p;
      return p;
   }
   else
   {
      return NULL;
   }
}

PrefItem * PrefParser::parseItem()
{
   //printf( "parsing item value\n" );
   m_pos = nextToken();

   if ( !m_eof )
   {
      switch ( m_pos[0] )
      {
         case '{':
            return parseHash();

         case '(':
            return parseList();

         case '"':
            {
               std::string str = "";
               if ( parseString( str ) )
               {
                  //printf( "got string item: '%s'\n", str.c_str() );
                  PrefItem * item = new PrefItem;
                  (*item) = str;
                  return item;
               }
            }
            //printf( "failed to read string item\n" );
            return NULL;

         default:
            fprintf( stderr, "invalid character: '%c' at line %d\n",
                  m_pos[0], m_line );
            return NULL;
      }
   }
   else
   {
      return NULL;
   }
}

PrefItem * PrefParser::parseHash()
{
   //printf( "parsing hash\n" );
   if ( m_state == PS_None || m_state == PS_Value )
   {
      if ( m_pos[0] == '{' )
      {
         m_pos++;

         PrefItem * hash = new PrefItem;

         std::string key;

         while ( !m_eof )
         {
            PrefItem * item = NULL;
            m_pos = nextToken();

            switch ( m_pos[0] )
            {
               case '"':
                  m_state = PS_Key;
                  key = "";
                  item = parseKeyValue( key );
                  if ( item )
                  {
                     //printf( "got key/value (%s) for hash\n", key.c_str() );
                     (*hash)( key ) = *item;
                     delete item;
                  }
                  break;
               default:
                  fprintf( stderr, "invalid hash key character: '%c' at line %d\n",
                        m_pos[0], m_line );
                  delete hash;
                  return NULL;
            }

            m_pos = nextToken();

            if ( m_pos[0] == ',' )
            {
               m_pos++;
            }
            else if ( m_pos[0] == '}' )
            {
               return hash;
            }
            else
            {
               fprintf( stderr, "invalid hash key character: '%c' at line %d\n",
                     m_pos[0], m_line );
               return NULL;
            }
         }

         delete hash;
      }
   }
   else
   {
      fprintf( stderr, "hash is only valid at top level or as a value\n" );
   }
   return NULL;
}

PrefItem * PrefParser::parseList()
{
   //printf( "parsing list\n" );
   if ( m_state == PS_None || m_state == PS_Value )
   {
      if ( m_pos[0] == '(' )
      {
         PrefItem * item = new PrefItem;
         int count = 0;

         m_pos++;

         while ( !m_eof )
         {
            m_pos = nextToken();
            if ( m_pos[0] == ')' )
            {
               return item;
            }
            else 
            {
               m_state = PS_ListItem;
               PrefItem * child = parseItem();
               if ( child )
               {
                  item->insert( count, *child );
                  delete child;
                  count++;
               }

               m_pos = nextToken();

               if ( m_pos[0] == ',' )
               {
                  m_pos++;
               }
               else if ( m_pos[0] == ')' )
               {
                  m_pos++;
                  return item;
               }
               else
               {
                  fprintf( stderr, "Invalid list char '%c' at line %d\n",
                        m_pos[0], m_line );
                  delete item;
                  return NULL;
               }
            }
         }

         delete item;
      }
      else
      {
         fprintf( stderr, "list must begin with '(' at line %d\n", m_line );
      }
   }
   else
   {
      fprintf( stderr, "list is only valid at top level or as a value\n" );
   }
   return NULL;
}

bool PrefParser::parseString( std::string & rval )
{
   //printf( "parsing string\n" );
   if ( m_pos[0] == '"' )
   {
      m_pos++;

      char * str = m_pos;

      rval = "";

      bool escape = false;

      while( !m_eof )
      {
         if ( needMore() )
         {
            std::string add;
            add.assign( str, m_len - (str - m_buffer ) );

            //printf( "adding '%s' to string\n", add.c_str() );

            rval += add;

            readMore();
            str = m_pos;
         }

         if ( m_pos[0] == '"' && !escape)
         {
            std::string add;
            add.assign( str, m_pos - str );

            //printf( "adding '%s' to string\n", add.c_str() );

            rval += add;

            m_pos++;
            replaceEscape( rval );

            return true;
         }
         else
         {
            if ( !escape && m_pos[0] == '\\' )
            {
               escape = true;
               m_pos++;
            }
            else
            {
               escape = false;
               m_pos++;
            }
         }
      }
   }
   else
   {
      fprintf( stderr, "string is only valid as hash key or list item\n" );
   }
   return false;
}

void PrefParser::replaceEscape( std::string & str )
{
   size_t pos = 0;
   while ( (pos = str.find('\\', pos+1)) < str.size() )
   {
      if ( str[pos+1] == '"' || str[pos+1] == '\\' )
         str.erase(pos,1);
      else
         pos++;
   }
}

PrefItem * PrefParser::parseListItem()
{
   //printf( "parsing list item\n" );
   if ( m_state == PS_ListItem )
   {
      switch ( m_pos[0] )
      {
         case '"':
            {
               std::string str = "";
               if ( parseString(str) )
               {
                  PrefItem * item = new PrefItem;
                  (*item) = str;
                  return item;
               }
               return NULL;
            }

         case '(':
            return parseList();

         case '{':
            return parseHash();

         case ')':
            return NULL;

         default:
            fprintf( stderr, "invalid list item char '%c' at line %d\n",
                  m_pos[0], m_line );
            break;
      }
   }
   else
   {
      fprintf( stderr, "string is only valid as hash key or list item\n" );
   }
   return NULL;
}

PrefItem * PrefParser::parseKeyValue( std::string & key )
{
   //printf( "parsing key/value\n" );
   if ( m_state == PS_Key )
   {
      //printf( "reading key\n" );
      if ( parseString( key ) )
      {
         //printf( "key is '%s'\n", key.c_str() );
         m_pos = nextToken();

         // have to parse => chars separately (could be on BUFFER_SIZE boundary)
         if ( m_pos[0] == '=' )
         {
            m_pos++;
            m_pos = nextToken();
            if ( m_pos[0] == '>' )
            {
               //printf( "reading value\n" );
               m_pos++;

               m_state = PS_Value;
               PrefItem * item = parseItem();

               //printf( "got value\n" );

               return item;
            }
         }
      }
      else
      {
         fprintf( stderr, "failed to parse string for hash key at line %d\n", m_line );
      }
   }
   else
   {
      fprintf( stderr, "string is only valid as hash key or list item\n" );
   }
   return NULL;
}

bool PrefParser::parseFile( FILE * fp, Preferences & p )
{
   //printf( "parsing file\n" );
   bool rval = false;
   if ( fp )
   {
      m_fp = fp;
      m_eof = false;
      m_len = 0;
      m_str = "";
      m_line = 1;

      m_items.clear();
      m_states.clear();

      m_state = PS_None;

      m_current = parseItem();

      if ( m_current )
      {
         p.setRootItem( m_current );

         rval = true;
      }
   }

   m_pref    = NULL;
   m_current = NULL;

   m_fp  = NULL;
   m_pos = NULL;
   m_eof = true;
   m_len = 0;
   m_str = "";
   return rval;
}

bool prefparse_do_parse( FILE * fp, Preferences & p )
{
   PrefParser parser;

   return parser.parseFile( fp, p );
}

