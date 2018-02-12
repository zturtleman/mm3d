/*  Misfit Model 3D
 * 
 *  Copyright (c) 2018 Zack Middleton
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


#include "globalmenubar.h"

#include "keycfg.h"
#include "3dmprefs.h"
#include "model.h"

#include "viewwin.h"
#include "pluginwin.h"
#include "helpwin.h"
#include "licensewin.h"
#include "aboutwin.h"

// Default menu bar on macOS when no window is open or windows are minimized.
// Based on menu in viewwin.cc
GlobalMenuBar::GlobalMenuBar()
{
#ifdef HAVE_LUALIB
   QMenu *menu;
#endif
   QAction *action;

   m_menuBar = new QMenuBar( NULL );

   m_fileMenu = m_menuBar->addMenu( tr("&File", "menu bar") );
   m_fileMenu->addAction( tr("New", "File|New"), this, SLOT(newModelEvent()), g_keyConfig.getKey( "viewwin_file_new" ) );
   m_fileMenu->addAction( tr("Open...", "File|Open"), this, SLOT(openModelEvent()), g_keyConfig.getKey( "viewwin_file_open" ) );

   // Disable items that require a window
   action = m_fileMenu->addAction( tr("Save", "File|Save"), this, SLOT(dummyEvent()), g_keyConfig.getKey( "viewwin_file_save" ) );
   action->setEnabled( false );
   action = m_fileMenu->addAction( tr("Save As...", "File|Save As"), this, SLOT(dummyEvent()), g_keyConfig.getKey( "viewwin_file_save_as" ) );
   action->setEnabled( false );
   action = m_fileMenu->addAction( tr("Export...", "File|Export"), this, SLOT(dummyEvent()), g_keyConfig.getKey( "viewwin_file_export" ) );
   action->setEnabled( false );
   action = m_fileMenu->addAction( tr("Export Selected...", "File|Export Selected"), this, SLOT(dummyEvent()), g_keyConfig.getKey( "viewwin_file_export_selected" ) );
   action->setEnabled( false );
#ifdef HAVE_LUALIB
   action = m_fileMenu->addAction( tr("Run Script...", "File|Run Script"), this, SLOT(dummyEvent()), g_keyConfig.getKey( "viewwin_file_run_script" ) );
   action->setEnabled( false );
   m_scriptMruMenu = m_fileMenu->addMenu( tr("Recent Scripts", "File|Recent Script") );
   connect( m_scriptMruMenu, SIGNAL(aboutToShow()), this, SLOT(fillScriptMruMenuDisabled()) );
#endif // HAVE_LUALIB

   m_fileMenu->addSeparator();
   m_mruMenu = m_fileMenu->addMenu( tr("Recent Models", "File|Recent Models") );
   connect( m_mruMenu, SIGNAL(aboutToShow()), this, SLOT(fillMruMenu()) );
   connect( m_mruMenu, SIGNAL(triggered(QAction*)), this, SLOT(openMru(QAction*)) );
   m_fileMenu->addSeparator();
   m_fileMenu->addAction( tr("Plugins...", "File|Plugins"), this, SLOT(pluginWindowEvent()), g_keyConfig.getKey( "viewwin_file_plugins" ) );
   m_fileMenu->addSeparator();

   // Disable items that require a window
   action = m_fileMenu->addAction( tr("Close", "File|Close"), this, SLOT(dummyEvent()), g_keyConfig.getKey( "viewwin_file_close" ) );
   action->setEnabled( false );

   m_fileMenu->addAction( tr("Quit", "File|Quit"), this, SLOT(quitEvent()), g_keyConfig.getKey( "viewwin_file_quit" ) );

   m_helpMenu =  m_menuBar->addMenu( tr("&Help", "menu bar") );
   m_helpMenu->addAction( tr("Contents...", "Help|Contents"), this, SLOT(helpWindowEvent()), g_keyConfig.getKey( "viewwin_help_contents" ) );
   m_helpMenu->addAction( tr("License...", "Help|License"),  this, SLOT(licenseWindowEvent()), g_keyConfig.getKey( "viewwin_help_license" ) );
   m_helpMenu->addAction( tr("About...", "Help|About"),    this, SLOT(aboutWindowEvent()), g_keyConfig.getKey( "viewwin_help_about" ) );
}

GlobalMenuBar::~GlobalMenuBar()
{
   delete m_menuBar;
}

void GlobalMenuBar::openModelEvent()
{
   ViewWindow::openModelDialog();
}

void GlobalMenuBar::newModelEvent()
{
   ViewWindow * win = new ViewWindow( new Model, NULL );
   win->getSaved(); // Just so I don't have a warning
}

void GlobalMenuBar::quitEvent()
{
   if ( ViewWindow::closeAllWindows() )
   {
      qApp->quit();
   }
}

void GlobalMenuBar::helpWindowEvent()
{
   HelpWin * win = new HelpWin();
   win->show();
}

void GlobalMenuBar::pluginWindowEvent()
{
   // pluginWin will delete itself view WDestructiveClose
   PluginWindow * pluginWin = new PluginWindow();
   pluginWin->show();
}

void GlobalMenuBar::aboutWindowEvent()
{
   AboutWin * win = new AboutWin();
   win->show();
}

void GlobalMenuBar::licenseWindowEvent()
{
   LicenseWin * win = new LicenseWin();
   win->show();
}

void GlobalMenuBar::fillMruMenu()
{
   m_mruMenu->clear();
   for ( unsigned i = 0; i < g_prefs("mru").count(); i++ )
   {
      m_mruMenu->addAction( QString::fromUtf8( g_prefs("mru")[i].stringValue().c_str() ) );
   }
}

void GlobalMenuBar::openMru( QAction * id )
{
   ViewWindow::openModel( id->text().toUtf8() );
}

void GlobalMenuBar::fillScriptMruMenuDisabled()
{
   QAction *action;

   m_scriptMruMenu->clear();
   for ( unsigned i = 0; i < g_prefs("script_mru").count(); i++ )
   {
      action = m_scriptMruMenu->addAction( QString::fromUtf8( g_prefs("script_mru")[i].stringValue().c_str() ) );
      action->setEnabled( false );
   }
}

void GlobalMenuBar::dummyEvent()
{

}
