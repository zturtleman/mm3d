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


#include "msg.h"

#include <QtWidgets/QMessageBox>
#include <ctype.h>

extern "C" void msgqt_info( const char * str )
{
   QMessageBox::information( NULL, QString("Misfit 3D"), QString::fromUtf8(str), QMessageBox::Ok, 0 );
}

extern "C" void msgqt_warning( const char * str )
{
   QMessageBox::warning( NULL, QString("Misfit 3D"), QString::fromUtf8(str), QMessageBox::Ok, 0 );
}

extern "C" void msgqt_error( const char * str )
{
   QMessageBox::critical( NULL, QString("Misfit 3D"), QString::fromUtf8(str), QMessageBox::Ok, 0 );
}

typedef enum {
   MsgInfo,
   MsgWarning,
   MsgError
} MessageType;

static char _msgqt_info_common( MessageType type, const QString & str, const char * opts )
{
   int button[3];
   bool done = false;

   for ( int t = 0; t < 3; t++ )
   {
      if( ! done && opts[t] )
      {
         switch ( toupper( opts[t] ) )
         {
            case 'Y':
               button[t] = QMessageBox::Yes;
               break;
            case 'N':
               button[t] = QMessageBox::No;
               break;
            case 'C':
               button[t] = QMessageBox::Cancel | QMessageBox::Escape;
               break;
            case 'O':
               button[t] = QMessageBox::Ok;
               break;
            case 'A':
               button[t] = QMessageBox::Abort;
               break;
            case 'R':
               button[t] = QMessageBox::Retry;
               break;
            case 'I':
               button[t] = QMessageBox::Ignore;
               break;
            default:
               button[t] = QMessageBox::NoButton;
               break;
         }

         if ( isupper( opts[t] ) )
         {
            button[t] |= QMessageBox::Default;
         }
      }
      else
      {
         done = true;
         button[t] = 0;
      }
   }

   int result = 0;
   switch ( type )
   {
      case MsgError:
         result = QMessageBox::critical( NULL, QString("Misfit 3D"), str, button[0], button[1], button[2] );
         break;
      case MsgWarning:
         result = QMessageBox::warning( NULL, QString("Misfit 3D"), str, button[0], button[1], button[2] );
         break;
      case MsgInfo:
      default:
         result = QMessageBox::information( NULL, QString("Misfit 3D"), str, button[0], button[1], button[2] );
         break;
   }

   char rval = '!';

   switch ( result )
   {
      case QMessageBox::Ok:
         rval = 'O';
         break;
      case QMessageBox::Yes:
         rval = 'Y';
         break;
      case QMessageBox::No:
         rval = 'N';
         break;
      case QMessageBox::Abort:
         rval = 'A';
         break;
      case QMessageBox::Retry:
         rval = 'R';
         break;
      case QMessageBox::Ignore:
         rval = 'I';
         break;
      case QMessageBox::Cancel:
      default:
         rval = 'C';
         break;
   }

   return rval;
}

// Do you want to save first (yes, no, cancel) [Y/n/c]?
// Do you want to save first (abort, retry, ignore) [A/r/i]?
extern "C" char msgqt_info_prompt( const char * str, const char * opts )
{
   return _msgqt_info_common( MsgInfo, QString::fromUtf8(str), opts );
}

extern "C" char msgqt_warning_prompt( const char * str, const char * opts )
{
   return _msgqt_info_common( MsgWarning, QString::fromUtf8(str), opts );
}

extern "C" char msgqt_error_prompt( const char * str, const char * opts )
{
   return _msgqt_info_common( MsgError, QString::fromUtf8(str), opts );
}

void init_msgqt()
{
   msg_register( msgqt_info, msgqt_warning, msgqt_error );
   msg_register_prompt( msgqt_info_prompt, msgqt_warning_prompt, msgqt_error_prompt );
}

