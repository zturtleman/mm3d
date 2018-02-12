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


#ifndef __GLOBALMENUBAR_H
#define __GLOBALMENUBAR_H

#include "config.h"

#include <QtCore/QObject>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>

class GlobalMenuBar : public QObject
{
   Q_OBJECT

   public:
      GlobalMenuBar();
      ~GlobalMenuBar();

   public slots:
      void openModelEvent();
      void newModelEvent();
      void quitEvent();

      void pluginWindowEvent();
      void helpWindowEvent();
      void aboutWindowEvent();
      void licenseWindowEvent();

      void fillMruMenu();
      void openMru( QAction * id );

      void fillScriptMruMenuDisabled();

      void dummyEvent();

   protected:
      QMenuBar *m_menuBar;
      QMenu *m_fileMenu;
      QMenu *m_helpMenu;

      QMenu *m_mruMenu;
      QMenu *m_scriptMruMenu;
};

#endif // __GLOBALMENUBAR_H
