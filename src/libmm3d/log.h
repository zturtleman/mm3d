/*  Maverick Model 3D
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


#ifndef __LOG_H
#define __LOG_H

#ifdef __DO_PROFILE

#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>

extern FILE * _logProfileFP;

class LogProfileObject
{
   public:
      inline LogProfileObject( const char * str )  // str will *NOT* be copied!
         : m_str( str )
      {
         if ( _logProfileFP )
         {
            ftime( &m_startTb );
            fprintf( _logProfileFP, "IN:  %d.%03d %s\n", (int) m_startTb.time, m_startTb.millitm, str );
         }
      }
      inline ~LogProfileObject()
      {
         if ( _logProfileFP )
         {
            struct timeb tb;
            ftime( &tb );
            fprintf( _logProfileFP, "OUT: %d.%03d (%d) %s\n", (int) tb.time, tb.millitm, 
                  ((int)tb.time * 1000 + tb.millitm) - ((int)m_startTb.time * 1000 + m_startTb.millitm),
                  m_str );
         }
      }

   private:
      struct timeb m_startTb;
      const char * m_str;
};

extern void log_profile_init( const char * filename );
extern void log_profile_shutdown();

#define LOG_PROFILE() LogProfileObject _profileObj( __PRETTY_FUNCTION__ );
#define LOG_PROFILE_STR(x) LogProfileObject _profileObj( (x) );

#else

#define LOG_PROFILE()
#define LOG_PROFILE_STR(x)

#define log_profile_init(x)
#define log_profile_shutdown()

#endif // __DO_PROFILE

extern void log_enable_debug( bool o );
extern void log_enable_warning( bool o );
extern void log_enable_error( bool o );

extern void log_debug( const char * fmt, ... ) __attribute__ ((format (printf, 1, 2)));
extern void log_warning( const char * fmt, ... ) __attribute__ ((format (printf, 1, 2)));
extern void log_error( const char * fmt, ... ) __attribute__ ((format (printf, 1, 2)));

#endif // __LOG_H
