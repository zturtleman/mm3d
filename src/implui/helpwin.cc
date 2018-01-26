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


#include "helpwin.h"

#include "sysconf.h"

#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QPushButton>

HelpWin::HelpWin( const char * document, bool modal, QWidget * parent )
   : QDialog( parent )
{
   setAttribute( Qt::WA_DeleteOnClose );
   setModal( modal );
   setupUi( this );
#ifdef WIN32
   QString source = 
        QString( getDocDirectory().c_str() )
      + QString( "\\olh_index.html" );
#else
   QString source =
        QString( getDocDirectory().c_str() )
      + QString( "/olh_index.html" );
#endif
   m_text->setSource( QUrl::fromLocalFile( source ) );

   if ( document )
   {
#ifdef WIN32
      QString source = 
           QString( getDocDirectory().c_str() )
         + QString( "\\" )
         + QString( document );
#else
      QString source =
           QString( getDocDirectory().c_str() )
         + QString( "/" )
         + QString( document );
#endif
      m_text->setSource( QUrl::fromLocalFile( source ) );
   }
   //m_text->home();
   m_forwardButton->setEnabled( false );
   m_backButton->setEnabled( false );
}

HelpWin::~HelpWin()
{
}

