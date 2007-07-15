#include "model.h"

#include "modelundo.h"
#include "modelstatus.h"
#include "log.h"

#ifdef MM3D_EDIT

static void _safe_strcpy( char * dest, const char * src, size_t len )
{
   if ( len > 0 )
   {
      strncpy( dest, src, len );
      dest[len-1] = '\0';
   }
   else
   {
      dest[0] = '\0';
   }
}

void Model::addMetaData( const char * key, const char * value )
{
   MetaData md;
   md.key = key;
   md.value = value;

   MU_AddMetaData * undo = new MU_AddMetaData();
   undo->addMetaData( md.key, md.value );
   sendUndo( undo );

   m_metaData.push_back( md );
}

bool Model::getMetaData( const char * key, char * value, size_t valueLen )
{
   MetaDataList::iterator it;

   for ( it = m_metaData.begin(); it != m_metaData.end(); it++ )
   {
      if ( strcmp( key, (*it).key.c_str() ) == 0 )
      {
         _safe_strcpy( value, (*it).value.c_str(), valueLen );
         return true;
      }
   }
   return false;
}

bool Model::getMetaData( unsigned int index, char * key, size_t keyLen, char * value, size_t valueLen )
{
   if ( index < m_metaData.size() )
   {
      _safe_strcpy( key,   m_metaData[index].key.c_str(),   keyLen );
      _safe_strcpy( value, m_metaData[index].value.c_str(), valueLen );
      return true;
   }
   return false;
}

unsigned int Model::getMetaDataCount()
{
   return m_metaData.size();
}

void Model::clearMetaData()
{
   MU_ClearMetaData * undo = new MU_ClearMetaData();
   undo->clearMetaData( m_metaData );
   sendUndo( undo );
      
   m_metaData.clear();
}

void Model::removeLastMetaData()
{
   // INTERNAL USE ONLY!!!

   // This is just for undo purposes, so we don't need an undo

   if ( ! m_metaData.empty() )
   {
      m_metaData.pop_back();
   }
}

void Model::setBackgroundImage( unsigned index, const char * filename )
{
   if ( index < MAX_BACKGROUND_IMAGES )
   {
      std::string temp = ( filename != NULL ) ? filename : "";

      MU_SetBackgroundImage * undo = new MU_SetBackgroundImage();
      undo->setBackgroundImage( index, temp.c_str(),
            m_background[index]->m_filename.c_str() );
      sendUndo( undo );
      
      m_background[index]->m_filename = temp;
   }
}

void Model::setBackgroundScale( unsigned index, float scale )
{
   if ( index < MAX_BACKGROUND_IMAGES )
   {
      MU_SetBackgroundScale * undo = new MU_SetBackgroundScale();
      undo->setBackgroundScale( index, 
            scale, m_background[index]->m_scale );
      sendUndo( undo );
      
      m_background[index]->m_scale = scale;
   }
}

void Model::setBackgroundCenter( unsigned index, float x, float y, float z )
{
   if ( index < MAX_BACKGROUND_IMAGES )
   {
      MU_SetBackgroundCenter * undo = new MU_SetBackgroundCenter();
      undo->setBackgroundCenter( index, x, y, z,
            m_background[index]->m_center[0],
            m_background[index]->m_center[1],
            m_background[index]->m_center[2] );
      sendUndo( undo );
      
      m_background[index]->m_center[0] = x;
      m_background[index]->m_center[1] = y;
      m_background[index]->m_center[2] = z;
   }
}

const char * Model::getBackgroundImage( unsigned index )
{
   if ( index < MAX_BACKGROUND_IMAGES )
   {
      return m_background[index]->m_filename.c_str();
   }
   return "";
}

float Model::getBackgroundScale( unsigned index )
{
   if ( index < MAX_BACKGROUND_IMAGES )
   {
      return m_background[index]->m_scale;
   }
   return 0.0f;
}

void Model::getBackgroundCenter( unsigned index, float & x, float & y, float & z )
{
   if ( index < MAX_BACKGROUND_IMAGES )
   {
      x = m_background[index]->m_center[0];
      y = m_background[index]->m_center[1];
      z = m_background[index]->m_center[2];
      return;
   }
   x = y = z = 0.0f;
}

#endif // MM3D_EDIT
