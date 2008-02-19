/*  Misfit Model 3D
 * 
 *  Copyright (c) 2008 Kevin Worcester
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
#include "log.h"
#include "texture.h"

void Model::sprint( std::string & dest )
{
   // FIXME implement
}

void Model::Vertex::sprint( std::string & dest )
{
   char tempstr[64];

   sprintf( tempstr, "%.5f,%.5f,%.5f  ",
         (float) m_coord[0],
         (float) m_coord[1],
         (float) m_coord[2] );
   dest = tempstr;

   sprintf( tempstr, "%c%c%c",
         (m_visible ? 'V' : 'H'),
         (m_selected ? 'S' : 'U'),
         (m_free ? 'F' : 'N') );
   dest += tempstr;

   if ( !m_influences.empty() )
   {
      dest += "  I ";

      InfluenceList::const_iterator it = m_influences.begin();
      while ( it != m_influences.end() )
      {
         char typeChar = 'C';
         if ( it->m_type == IT_Auto )
            typeChar = 'A';
         if ( it->m_type == IT_Remainder )
            typeChar = 'R';

         sprintf( tempstr, "%d-%c%d", it->m_boneId, typeChar,
               (int) (it->m_weight * 100.0 + 0.5) );
         dest += tempstr;

         ++it;
         if ( it != m_influences.end() )
            dest += ",";
      }
   }
}

void Model::Triangle::sprint( std::string & dest )
{
   char tempstr[64];

   sprintf( tempstr, "%d,%d,%d  ",
         m_vertexIndices[0], m_vertexIndices[1], m_vertexIndices[2] );
   dest = tempstr;

   sprintf( tempstr, "%c%c  ",
         (m_visible ? 'V' : 'H'),
         (m_selected ? 'S' : 'U') );
   dest += tempstr;

   sprintf( tempstr, "%.2f,%.2f,%.2f  ",
         (float) m_flatNormals[0],
         (float) m_flatNormals[1],
         (float) m_flatNormals[2] );
   dest += tempstr;

   sprintf( tempstr, "%.2f,%.2f %.2f,%.2f %.2f,%.2f",
         m_s[0], m_t[0],  m_s[1], m_t[1],  m_s[2], m_t[2] );
   dest += tempstr;

   if ( m_projection >= 0 )
   {
      sprintf( tempstr, "  P%d", m_projection );
      dest += tempstr;
   }
}

void Model::Group::sprint( std::string & dest )
{
   char tempstr[64];

   dest = m_name;

   if ( m_materialIndex >= 0 )
   {
      sprintf( tempstr, "  M%d", m_materialIndex );
      dest += tempstr;
   }

   sprintf( tempstr, "  A%d", m_angle );
   dest += tempstr;

   sprintf( tempstr, "  S%d", m_smooth );
   dest += tempstr;

   sprintf( tempstr, "%c%c  ",
         (m_visible ? 'V' : 'H'),
         (m_selected ? 'S' : 'U') );
   dest += tempstr;

   sprintf( tempstr, "Faces:%d", m_triangleIndices.size() );
   dest += tempstr;
}

void Model::Material::sprint( std::string & dest )
{
   char tempstr[64];

   dest = m_name;

   if ( m_type == MATTYPE_TEXTURE )
   {
      sprintf( tempstr, "  %s %d,%d,%d  ",
            m_filename.c_str(),
            m_textureData->m_origWidth, m_textureData->m_origHeight,
            (m_textureData->m_format == Texture::FORMAT_RGB) ? 24 : 32);
      dest += tempstr;
   }
   else
   {
      dest += "  [blank]  ";
   }

   sprintf( tempstr, "%c%c  ", m_sClamp ? 'C' : 'W', m_tClamp ? 'C' : 'W' );
   dest += tempstr;

   sprintf( tempstr, "  A %.1f %.1f %.1f %.1f",
         m_ambient[0], m_ambient[1], m_ambient[2], m_ambient[3] );
   dest += tempstr;

   sprintf( tempstr, "  D %.1f %.1f %.1f %.1f",
         m_diffuse[0], m_diffuse[1], m_diffuse[2], m_diffuse[3] );
   dest += tempstr;

   sprintf( tempstr, "  A %.1f %.1f %.1f %.1f",
         m_specular[0], m_specular[1], m_specular[2], m_specular[3] );
   dest += tempstr;

   sprintf( tempstr, "  A %.1f %.1f %.1f %.1f",
         m_emissive[0], m_emissive[1], m_emissive[2], m_emissive[3] );
   dest += tempstr;

   sprintf( tempstr, "  N %.1f", m_shininess );
   dest += tempstr;
}

void Model::Point::sprint( std::string & dest )
{
   char tempstr[64];

   dest = m_name;

   sprintf( tempstr, "  %c%c  ",
         (m_visible ? 'V' : 'H'),
         (m_selected ? 'S' : 'U') );
   dest += tempstr;

   sprintf( tempstr, "T %.5f,%.5f,%.5f  ",
         (float) m_trans[0],
         (float) m_trans[1],
         (float) m_trans[2] );
   dest += tempstr;

   sprintf( tempstr, "R %.5f,%.5f,%.5f  ",
         (float) m_rot[0],
         (float) m_rot[1],
         (float) m_rot[2] );
   dest += tempstr;

   if ( !m_influences.empty() )
   {
      dest += "  I ";

      InfluenceList::const_iterator it = m_influences.begin();
      while ( it != m_influences.end() )
      {
         char typeChar = 'C';
         if ( it->m_type == IT_Auto )
            typeChar = 'A';
         if ( it->m_type == IT_Remainder )
            typeChar = 'R';

         sprintf( tempstr, "%d-%c%d", it->m_boneId, typeChar,
               (int) (it->m_weight * 100.0 + 0.5) );
         dest += tempstr;

         ++it;
         if ( it != m_influences.end() )
            dest += ",";
      }
   }
}

void Model::Joint::sprint( std::string & dest )
{
   char tempstr[64];

   dest = m_name;

   sprintf( tempstr, "  P%d  ", m_parent );
   dest += tempstr;

   sprintf( tempstr, "%c%c  ",
         (m_visible ? 'V' : 'H'),
         (m_selected ? 'S' : 'U') );
   dest += tempstr;

   sprintf( tempstr, "T %.5f,%.5f,%.5f  ",
         (float) m_localTranslation[0],
         (float) m_localTranslation[1],
         (float) m_localTranslation[2] );
   dest += tempstr;

   sprintf( tempstr, "R %.5f,%.5f,%.5f  ",
         (float) m_localRotation[0],
         (float) m_localRotation[1],
         (float) m_localRotation[2] );
   dest += tempstr;
}

