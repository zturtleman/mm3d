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


#include "pluginmgr.h"
#include "pluginwin.h"

#include "helpwin.h"

#include <QtGui/QHeaderView>
#include <QtGui/QShortcut>

PluginWindow::PluginWindow()
   : QDialog( NULL )
{
   setAttribute( Qt::WA_DeleteOnClose );
   setupUi( this );
   setModal( false );

   m_pluginList->header()->setClickable( false );
   m_pluginList->header()->setMovable( false );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   refreshPluginData();
}

PluginWindow::~PluginWindow()
{
}

void PluginWindow::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_pluginwin.html", true );
   win->show();
}

void PluginWindow::refreshPluginData()
{
   PluginManager * pmgr = PluginManager::getInstance();

   list<int> plist = pmgr->getPluginIds();
   list<int>::iterator it;

   for ( it = plist.begin(); it != plist.end(); it++ )
   {
      QTreeWidgetItem * item = new QTreeWidgetItem( m_pluginList );
      item->setText( 0, QString( pmgr->getPluginName( *it ) ) + " " );
      item->setText( 1, QString( pmgr->getPluginVersion( *it ) ) + " " );
      item->setText( 2, QString( pmgr->getPluginDescription( *it ) ) + " " );

      const char * status = "Unknown";
      switch ( pmgr->getPluginStatus( *it ) )
      {
         case PluginManager::PluginActive:
            status = "Active";
            break;
         case PluginManager::PluginUserDisabled:
            status = "Disabled by user";
            break;
         case PluginManager::PluginVersionDisabled:
            status = "Disabled (incompatible version)";
            break;
         case PluginManager::PluginNotPlugin:
            status = "Not a plugin";
            break;
         case PluginManager::PluginError:
            status = "Error";
            break;
         default:
            status = "Unknown status code";
            break;
      }
      item->setText( 3, QString(status) + " " );
   }
}

