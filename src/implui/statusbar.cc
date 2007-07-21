/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2007 Kevin Worcester
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


#include "statusbar.h"
#include "mm3dport.h"
#include "misc.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <stdarg.h>

using std::map;

map<Model *, StatusBar *> StatusBar::s_modelMap;

StatusBar::StatusBar( Model * model, QWidget * parent, const char * name )
   : StatusBarBase( parent, name ),
     m_model( model ),
     m_queueDisplay( false )
{
   s_modelMap[ m_model ] = this;
}

StatusBar::~StatusBar()
{
   s_modelMap.erase( m_model );
}

StatusObject * StatusBar::getStatusBarFromModel( Model * model )
{
   if ( s_modelMap.find( model ) != s_modelMap.end() )
   {
      return s_modelMap[ model ];
   }
   else
   {
      return NULL;
   }
}

void StatusBar::setModel( Model * model )
{
   s_modelMap.erase( m_model );
   m_model = model;
   s_modelMap[ m_model ] = this;
}

void StatusBar::setText( const char * str )
{
   QToolTip::remove( this );
   if ( utf8len( str ) > 72 )
   {
      char * temp = strdup( str );
      QToolTip::add( this, QString::fromUtf8(temp) );
      utf8strtrunc( temp, 69 );
      strcat( temp, "..." );
      m_statusLabel->setText( QString::fromUtf8(temp) );
      free( temp );
   }
   else
   {
      m_statusLabel->setText( QString::fromUtf8(str) );
   }
}

void StatusBar::addText( StatusTypeE type, unsigned ms, const char * str )
{
   if ( m_queueDisplay )
   {
      bool removing = true;
      while ( removing && m_queue.size() > 2 )
      {
         removing = false;
         std::list< TextQueueItemT >::iterator it;

         for ( it = m_queue.begin(); it != m_queue.end(); it++ )
         {
            if ( (*it).type != StatusError )
            {
               m_queue.erase( it );
               it = m_queue.end();
               removing = true;
            }
         }
      }
      TextQueueItemT tqi;
      tqi.str  = QString::fromUtf8( str );
      tqi.ms   = ms;
      tqi.type = type;
      m_queue.push_back( tqi );
   }
   else
   {
      setText( str );
      QTimer::singleShot( ms, this, SLOT(timerExpired()));
      if ( type == StatusError ) 
      {
         m_statusLabel->setPaletteForegroundColor( QColor( 255, 255, 255 ) );
         m_statusLabel->setPaletteBackgroundColor( QColor( 255, 0, 0 ) );
      }
      m_queueDisplay = true;
   }
}

void StatusBar::timerExpired()
{
   m_statusLabel->unsetPalette();
   if ( !m_queue.empty() )
   {
      TextQueueItemT tqi = m_queue.front();
      m_queue.pop_front();

      setText( tqi.str.utf8() );

      m_queueDisplay = true;
      if ( tqi.type == StatusError ) 
      {
         m_statusLabel->setPaletteForegroundColor( QColor( 255, 255, 255 ) );
         m_statusLabel->setPaletteBackgroundColor( QColor( 255, 0, 0 ) );
      }

      if ( tqi.ms > 0 )
      {
         QTimer::singleShot( tqi.ms, this, SLOT(timerExpired()));
      }
      else
      {
         timerExpired();
      }
   }
   else
   {
      m_queueDisplay = false;
   }
}

void StatusBar::setVertices( unsigned v, unsigned sv )
{
   QString statChar = tr( "V:", "Vertices status bar label" );
   QString str;
   if ( sv )
   {
      str.sprintf( "%s%d/%d", (const char *) statChar.utf8(), sv, v );
   }
   else
   {
      str.sprintf( "%s%d", (const char *) statChar.utf8(), v );
   }

   m_vertexLabel->setText( QString(str) );
}

void StatusBar::setFaces( unsigned f, unsigned sf )
{
   QString statChar = tr( "F:", "Faces status bar label" );
   QString str;
   if ( sf )
   {
      str.sprintf( "%s%d/%d", (const char *) statChar.utf8(), sf, f );
   }
   else
   {
      str.sprintf( "%s%d", (const char *) statChar.utf8(), f );
   }

   m_faceLabel->setText( QString(str) );
}

void StatusBar::setGroups( unsigned g, unsigned sg )
{
   QString statChar = tr( "G:", "Groups status bar label" );
   QString str;
   if ( sg )
   {
      str.sprintf( "%s%d/%d", (const char *) statChar.utf8(), sg, g );
   }
   else
   {
      str.sprintf( "%s%d", (const char *) statChar.utf8(), g );
   }

   m_groupLabel->setText( QString(str) );
}

void StatusBar::setBoneJoints( unsigned b, unsigned sb )
{
   QString statChar = tr( "B:", "Bone Joints status bar label" );
   QString str;
   if ( sb )
   {
      str.sprintf( "%s%d/%d", (const char *) statChar.utf8(), sb, b );
   }
   else
   {
      str.sprintf( "%s%d", (const char *) statChar.utf8(), b );
   }

   m_boneLabel->setText( QString(str) );
}

void StatusBar::setPoints( unsigned b, unsigned sb )
{
   QString statChar = tr( "P:", "Points status bar label" );
   QString str;
   if ( sb )
   {
      str.sprintf( "%s%d/%d", (const char *) statChar.utf8(), sb, b );
   }
   else
   {
      str.sprintf( "%s%d", (const char *) statChar.utf8(), b );
   }

   m_pointLabel->setText( QString(str) );
}

void StatusBar::setTextures( unsigned t, unsigned st )
{
   QString statChar = tr( "M:", "Materials status bar label" );
   QString str;
   if ( st )
   {
      str.sprintf( "%s%d/%d", (const char *) statChar.utf8(), st, t );
   }
   else
   {
      str.sprintf( "%s%d", (const char *) statChar.utf8(), t );
   }

   m_textureLabel->setText( QString(str) );
}

extern "C" void model_status( Model * model, StatusTypeE type, unsigned ms, const char * fmt, ... )
{
   static char temp[1024];
   va_list ap;
   va_start( ap, fmt );
   PORT_vsnprintf( temp, sizeof(temp), fmt, ap );
   StatusObject * bar = StatusBar::getStatusBarFromModel( model );
   if ( bar )
   {
      bar->addText( type, ms, temp );
   }
}
